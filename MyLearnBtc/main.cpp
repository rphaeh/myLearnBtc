//
//  main.cpp
//  MyLearnBtc
//
//  Created by rphaeh on 2018/7/19.
//  Copyright © 2018年 rphaeh. All rights reserved.
//
#include "headers.h"
#include "sha.h"

CCriticalSection cs_main;

map<uin256, CTransaction> mapTransactions;
CCriticalSection cs_mapTransactions;
unsigned int nTransactionsUpdated = 0;
map<COutPoint,CInPoint> mapNextTx;

map<uin256,CBlockIndex*> mapBlockIndex;
const uin256 hashGenesisBlock("0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
CBlockIndex* pindexGenesisBlock = NULL;
int nBestHeight =-1;
uin256 hashBestChain = 0;
CBlockIndex *pindexBest = NULL;

map<uin256,CBlock*> mapOrphanBlocks;
multimap<uin256,CBlock*> mapOrphanBlocksByPrev;

map<uin256,CDataStream*> mapOrphanTransactions;
multimap<uin256,CDataStream*> mapOrphanTransactionsByPrev;

map<uin256,CWalletTx> mapWallet;
vector<pair<uin256,bool> > vWalletUpdated;
CCriticalSection cs_mapWallet;

map<vector<unsigned char>, CPrivKey> mapKeys;
map<uint160, vector<unsigned char> > mapPubKeys;
CCriticalSection cs_mapKeys;
CKey keyUser;

string strSetDataDir;
int nDropMessagesTest = 0;

int fGenerateBitcoins;
int64 nTransactionFee = 0;
CAddress addrIncoming;

bool AddKey(const CKey& key)
{
    CRITICAL_BLOCK(cs_mapKeys)
    {
        mapKeys[key.GetPubKey()]=key.GerPrivKey();
        mapPubKeys[Hash160(key.GetPubKey())]=key.GetPubKey();

    }
    return CWalletDB().WriteKey(key.GetPubKey(),key.GerPrivKey());
}

vector<unsigned char> GenerateNewKey()
{
    CKey key;
    key.MakeNewKey();
    if(!AddKey(key))
        throw runtime_error("GenerateNewKey():AddKey failed\n");
    return key.GetPubKey();
}


bool AddToWallet(const CWalletTx& wtxIn)
{
    uin256 hash = wtxIn.GetHash();
    CRITICAL_BLOCK(cs_mapWallet)
    {
        pair<map<uin256,CWalletTx>::iterator,bool> ret=mapWallet.insert(make_pair(hash,wtxIn));
        CWalletTx& wtx=(*ret.first).second;
        bool fInsertedNew=ret.second;
        if(fInsertedNew)
            wtx.nTimeReceived=GetAdjustedTime();
        printf("AddToWallet %s %s\n",wtxIn.GetHash(),fInsertedNew?"new":"update" );
        if(!fInsertedNew)
        {
            bool fUpdted=false;
            if(wtxIn.hashBlock!=0&&wtxIn.hashBlock!=wtx.hashBlock)
            {
                wtx.hashBlock=wtxIn.hashBlock;
                fUpdted=true;
            }
            if(wtxIn.nIndex!=-1&&(wtxIn.vMerkleBranch!=wtx.vMerkleBranch||wtxIn.nIndex!=wtx.nIndex))
            {
                wtx.vMerkleBranch=wtxIn.vMerkleBranch;
                wtx.nIndex=wtxIn.nIndex;
                fUpdted=true;
            }
            if(wtxIn.fFromMe&&wtxIn.fFromMe!=wtx.fFromMe)
            {
                wtx.fFromMe=wtxIn.fFromMe;
                fUpdted=true;
            }
            if(wtxIn.fSpent&&wtxIn.fSpent!=wtx.fSpent)
            {
                wtx.fSpent=wtxIn.fSpent;
                fUpdted=true;
            }
            if(!fUpdted)
                return true;

        }
        if(!wtx.WriteToDisk())
            return false;
        vWalletUpdated.push_back(make_pair(hash,fInsertedNew));

    }

    MainFrameRepaint();
    return true;
}

bool AddToWalletIfMine(const CTransaction& tx,const CBlock* pblock)
{
    if(tx.IsMine()||mapWallet.count(tx.GetHash()))
    {
        CWalletTx wtx(tx);
        if(pblock)
            wtx.SetMerkleBranch(pblock);
        return AddToWallet(wtx);
    }
    return true;
}

bool EraseFromWallet(uin256 hash)
{
    CRITICAL_BLOCK(cs_mapWallet)
    {
        if(mapWallet.erase(hash))
            CWalletDB().EraseTx(hash);
    }
    return true;
}


void AddOrphanTx(const CDataStream& vMsg)
{
    CTransaction tx;
    CDataStream(vMsg)>>tx;
    uin256 hash=tx.GetHash();
    if(mapOrphanTransactions.count(hash))
        return;
    CDataStream* pvMsg=mapOrphanTransactions[hash]=new CDataStream(vMsg);
    foreach(const CTxIn& txin,tx.vin)
        mapOrphanTransactionsByPrev.insert(make_pair(txin.prevout.hash,pvMsg));
}

void EraseOrphanTx(uin256 hash)
{
    if(!mapOrphanTransactions.count(hash))
        return;
    const CDataStream* pvMsg=mapOrphanTransactions[hash];
    CTransaction tx;
    CDataStream(*pvMsg)>>tx;
    foreach(const CTxIn& txin,tx.vin)
    {
        for(multimap<uin256,CDataStream*>::iterator mi=mapOrphanTransactionsByPrev.lower_bound(txin.prevout.hash);
            mi!=mapOrphanTransactionsByPrev.upper_bound(txin.prevout.hash);)
        {
            if((*mi).second==pvMsg)
                mapOrphanTransactionsByPrev.erase(mi++);
            else
                mi++;
        }
    }
    delete pvMsg;
    mapOrphanTransactions.erase(hash);
}


bool CTxIn::IsMine() const
{
    CRITICAL_BLOCK(cs_mapWallet)
    {
        map<uin256,CWalletTx>::iterator mi=mapWallet.find(prevout.hash);
        if(mi!=mapWallet.end())
        {
            const CWalletTx& prev=(*mi).second;
            if(prevout.n<prev.vout.size())
                if(prev.vout[prevout.n].IsMine())
                    return true;
        }
    }
    return false;
}

int64 CTxIn::GetDebit() const
{
    CRITICAL_BLOCK(cs_mapWallet)
    {
        map<uin256,CWalletTx>::iterator mi=mapWallet.find(prevout.hash);
        if(mi!=mapWallet.end())
        {
            const CWalletTx& prev=(*mi).second;
            if(prevout.n<prev.vout.size())
                if(prev.vout[prevout.n].IsMine())
                    return prev.vout[prevout.n].nValue;
        }
    }
    return 0;
}


int64 CWalletTx::GetTxtTime() const
{
    if(!fTimeReceivedIsTxTime&& hashBlock!=0)
    {
        map<uin256,CBlockIndex*>::iterator mi=mapBlockIndex.find(hashBlock);
        if(mi!=mapBlockIndex.end())
        {
            CBlockIndex* pindex=(*mi).second;
            if(pindex)
                return pindex->GetMedianTime();
        }
    }
    return nTimeReceived;
}

int CMerkleTx::SetMerkleBranch(const CBlock* pblock)
{
    if(fClient)
    {
        if(hashBlock==0)
            return 0;
    }
    else
    {
        CBlock blockTmp;
        if(pblock==NULL)
        {
            CTxIndex txindex;
            if(!CTxDB("r").ReadTxIndex(GetHash(),txindex))
                return 0;
            if(!blockTmp.ReadFromDisk(txindex.pos.nFile,txindex.pos.nBlockPos,true))
                return 0;
            pblock=&blockTmp;
        }
        hashBlock=pblock->GetHash();
        for(nIndex=0;nIndex<pblock->vtx.size();nIndex++)
            if(pblock->vtx[nIndex]==*(CTransaction*)this)
                break;
        if(nIndex==pblock->vtx.size())
        {
            vMerkleBranch.clear();
            nIndex=-1;
            printf("ERROR:SetMerkleBranch():couldn't find tx in block\n");
            return 0;
        }
        vMerkleBranch=pblock->GetMerkleBranch(nIndex);
    }
    map<uin256,CBlockIndex*>::iterator mi=mapBlockIndex.find(hashBlock);
    if(mi==mapBlockIndex.end())
        return 0;
    return pindexBest->nHeight-pindex->nHeight+1;
}


void CWalletTx::AddSupportingTransactions(CTxDB& txdb)
{
    vtxPrev.clear();
    const int COPY_DEPTH = 3;
    if(SetMerkleBranch()<COPY_DEPTH)
    {
        vector<uin256> vWorkQueue;
        foreach(const CTxIn& txin,vin)
            vWorkQueue.push_back(txin.prevout.hash);
        CRITICAL_BLOCK(cs_mapWallet)
        {
            map<uin256,const CMerkleTx*>
                mapWalletPrev;
            set<uin256>setAlreadyDone;
            for(int i=0;i<vWorkQueue.size();i++)
            {
                uin256 hash = vWorkQueue[i];
                if(setAlreadyDone.count(hash))
                    continue;
                setAlreadyDone.insert(hash);
                CMerkleTx tx;
                if(mapWallet.count(hash))
                {
                    tx=mapWallet[hash];
                    foreach(const CMerkleTx& txWalletPrev,mapWallet[hash].vtxPrev)
                        mapWalletPrev[txWalletPrev.GetHash()]=&txWalletPrev;
                }
                else if(mapWalletPrev.count(hash))
                {
                    tx=*mapWalletPrev[hash];
                }
                else if(!fClient&&txdb.ReadDiskTx(hash,tx))
                {
                    ;
                }
                else
                {
                    printf("ERROR:AddSupportingTransactions():unsupported transaction\n");
                    continue;
                }
                int nDepth=tx.SetMerkleBranch();
                vtxPrev.push_back(tx);
                if(nDepth<COPY_DEPTH)
                    foreach(const CTxIn& txin,tx.vin)
                        vWorkQueue.push_back(txin.prevout.hash);
            }
        }
    }
    reverse(vtxPrev.begin().vtxPrev.end());
}


bool CTransaction::AcceptTransaction(CTxDB& txdb,bool fCheckInputs,bool* pfMissingInputs)
{
    if(pfMissingInputs)
        *pfMissingInputs=false;
    if(IsCoinBase())
        return error("AcceptTransaction():coinbase as individual tx");
    if(!CheckTransaction())
        return error("AcceptTransaction():CheckTransaction failed");
    uin256 hash=GetHash();
    CRITICAL_BLOCK(cs_mapTransactions)
        if(mapTransactions.count(hash))
            return false;
    if(fCheckInputs)
        if(txdb.ContainsTx(hash))
            return false;

    CTransaction* ptxOld=NULL;
    for(int i=0;i<vin.size();i++)
    {
        COutPoint outpoint=vin[i].prevout;
        if(mapNextTx.count(outpoint))
        {
            if(i!=0)
                return false;
            ptxOld=mapNextTx[outpoint].ptx;
            if(!IsNewerThan(*ptxOld))
                return false;
            for(int i=0;i<vin.size();i++)
            {
                COutPoint outpoint=vin[i].prevout;
                if(!mapNextTx.count(outpoint)||mapNextTx[outpoint].ptx!=ptxOld)
                    return false;
            }
            break;
        }
    }

    map<uint256,CTxIndex>mapUnused;
    int64 nFees=0;
    if(fCheckInputs&&!ConnectInputs(txdb,mapUnused,CDiskTxPos(1,1,1),0,nFees,false,false))
    {
        if(pfMissingInputs)
            *pfMissingInputs=true;
        return error("AcceptTransaction():ConnectInputs failed %s",hash.ToString().substr(0,6).c_str());
    }

    CRITICAL_BLOCK(cs_mapTransactions)
    {
        if(ptxOld)
        {
            printf("mapTransaction.erase(%s) replacing with new version\n",ptxOld->GetHash().ToString().c_str());
            mapTransactions.erase(ptxOld->GetHash());
        }
        AddToMemoryPool();
    }
    if(ptxOld)
        EraseFromWallet(ptxOld->GetHash());
    printf("AcceptTransaction():accept %s\n",hash.ToString().substr(0,6).c_str());
    return true;
}

bool CTransaction::AddToMemoryPool()
{
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        uin256 hash=GetHash();
        mapTransactions[hash]=*this;
        for(int i=0;i<vin.size();i++)
            mapNextTx[vin[i].prevout]=CInPoint(&mapTransactions[hash],i);
        nTransactionsUpdated++;
    }
    return true;
}

bool CTransaction::RemoveFromMemoryPool()
{
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        foreach(const CTxIn& txin,vin)
           mapNextTx.erase(txin.prevout);
        mapTransactions.erase(GetHash());
        nTransactionsUpdated++;
    }
    return true;
}

int CMerkleTx::GetDepthInMainChain()
{
    if(hashBlock==0||nIndex==-1)
        return 0;
    map<uin256,CBlockIndex*>::iterator mi=mapBlockIndex.find(hashBlock);
    if(mi==mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex=(*mi).second;
    if(!pindex||!pindex->IsInMainChain())
        return 0;
    if(!fMerkleVerified)
    {
        if(CBlock::CheckMerkleBranch(GetHash(),vMerkleBranch,nIndex)!=pindex->hashMerkleRoot)
            return 0;
        fMerkleVerified=true;
    }
    return pindexBest->nHeight-pindex->nHeight+1;
}

int CMerkleTx::GetBlocksToMaturity() const
{
    if(!IsCoinBase())
        return 0;
    return max(0,(COINBASE_MATURITY+20)-GetDepthInMainChain());
}











#include <iostream>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
