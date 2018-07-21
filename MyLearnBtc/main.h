//
//  main.h
//  MyLearnBtc
//
//  Created by rphaeh on 2018/7/19.
//  Copyright © 2018年 rphaeh. All rights reserved.
//

#ifndef main_h
#define main_h
class COutPoint;
class CInPoint;
class CDiskTxPos;
class CCoinBase;
class CTxIn;
class CTxOut;
class CTransaction;
class CBlock;
class CBlockIndex;
class CWalletTx;
class CKeyItem;

static const unsigned int MAX_SIZE = 0x2000000;
static const int64 COIN = 100000000;
static const int64 CENT = 1000000;
static const int COINBASE_MATURITY = 100;
static const CBigNum bnProofOfWorkLimit(~uint256(0)>>32);

extern CCriticalSection cs_main;
extern map<uint256, CBlockIndex*>mapBlockIndex;
extern const uint256 hashGenesisBlock;
extern CBlockIndex* pindexGenesisBlock;
extern int nBestHeight;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionUpdated;
extern string strSetDataDir;
extern int nDropMessagesTest;

//setting
extern int fGenerateBitcoins;
extern int64 nTransactionFee;
extern CAddress addrIncoming;

//文件操作
string GetAppDir();
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int &nFileRet);

//公钥
bool AddKey(const Ckey& key);
vector<unsigned char> GenerateNewKey();

//钱包
bool AddToWallet(const CWalletTx& wtxIn);

//交易
void ReacceptWalletTransactions();
void RelayWalletTransactions();
bool CreateTransaction(CSScript scriptPubKey, int64 nValue, CWallet& txNew, int64& neFeeRequiredRet);
bool CommitTransactionSpent(cont CWalletTx& wtxNew);
int64 GetBalance();
bool SendMoney(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew);

//区块
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();

//挖矿
bool BitCoinMiner();

//信息处理
bool ProcessMessage(CNode* pfrom);
bool ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv);
bool SendMessages(CNode* pto);

//交易保存到磁盘
class CDiskTxPos
{
public:
    unsigned int nFile;
    unsigned int nBlockPos;
    unsigned int nTxPos;

    CDiskTxPos()
    {
        SetNull();
    }

    CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
    {
        nFile = nFileIn;
        nBlockPos = nBlockPosIn;
        nTxPos = nTxPosIn;
    }

    IMPLEMENT_SERIALIZE(READWRITE(FLATDATA(*this));)
    void SetNull() {nFile = -1; nBlockPos = 0; nTxPos = 0;}
    bool IsNull() const {return(nFile==-1);}

    friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return(a.nFile == b.nFile && a.nBlockPos == b.nBlockPos && a.nTxPos == b.nTxPos);
    }

    friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return!(a==b);
    }

    string ToString() const
    {
        if(IsNull())
        {
            return strprintf("null");
        }
        else
            return strprintf("(nFile=%d, nBlockPos=%d, nTxPos=%d)",nFile,nBlockPos,nTxPos);
    }

    void print() const
    {
        printf("%s",ToString().c_str());
    }
};

class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() {SetNull();}
    CInPoint(CTransaction* ptxIn, unsigned int nIn) {ptx = ptxIn; n = nIn;}
    void SetNull() {ptx = NULL; n=-1;}
    bool IsNull() const {return (ptx==NULL && n==-1);}
};

class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() {SetNull();}
    COutPoint(uint256 hashIn, unsigned int nIn) {hash=hashIn; n=nIn;}
    IMPLEMENT_SERIALIZE(READWRITE(FLATDATA(*this));)
    void SetNull() {hash=0;n=-1;}
    bool IsNull() const {return (hash ==0 && n==-1);}

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return(a.hash < b.hash || (a.hash == b.hash&&a.n < b.n));
    }
    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return(a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return!(a==b);
    }
    string ToString() const
    {
        return strprintf("COutPoint(%s,%d)",hash.ToString().substr(0,6).c_str(),n);
    }

    void print() const
    {
        printf("%s",ToString().c_str());
    }
};

class CTxIn
{

public:
    COutPoint prevout;
    CScript scriptSig;
    unsigned int nSequence;

    CTxIn()
    {
        nSequence = UINT_MAX:
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSignIn=CScript(),unsigned int nSequence=UINT_MAX)
    {
        prevout = prevoutIn;
        scriptSig = scriptSignIn;
        nSequence = nSequenceIn;
    }

    CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSignIn=CScript(),unsigned int nSequence=UINT_MAX)
    {
        prevout = COutPoint(hashPrevTx,nOut);
        scriptSig = scriptSignIn;
        nSequence = nSequenceIn;
    }
    IMPLEMENT_SERIALIZE
    (
     READWRITE(prevout);
     READWRITE(scriptSig);
     READWRITE(nSequence);
     )

    bool isFinal() const
    {
        return (nSequence == UINT_MAX);
    }
    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return(a.prevout == b.prevout && a.scriptSig == b.scriptSig && a.nSequence == b.nSequence);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return!(a==b);
    }
    string ToString() const
    {
        string str;
        str += strprintf("CTxIn(");
        str += prevout.ToString();
        if (prevout.IsNull()) {
            str += strprintf(", coinbase %s",HexStr(scriptSig.begin(),scriptSig.end(),false).c_str());
        } else {
            str += strprintf(", scriptSig=%s",scriptSig.ToString().substr(0,24).c_str());
        }
        if (nSequence != UINT_MAX) {
            str+=strprintf(",nSequence=%u",nSequence);
        }
        str+=")";
        return str;
    }
    void pirnt() const
    {
        printf("%s\n",ToString().c_str());
    }
    bool IsMine() const;
    int64 GetDebit() const;
};

class CTxOut {
public:
    int64 nValue;
    CScript scriptPubKey;
public:
    CTxOut()
    {
        SetNull();
    }
    CTxOut(int64 nValueIn, CScript scriptPubKeyIn)
    {
        nValue = nValueIn;
        scriptPubKey = scriptPubKeyIn;
    }
    IMPLEMENT_SERIALIZE
    (
     READWRITE(nValue);
     READWRITE(scriptPubKey);
     )
    void SetNull() {nValue=-1;scriptPubKey.clear();}
    bool IsNull() {return (nValue==-1);}
    bool IsMine() const {return ::IsMine(scriptPubKey);}
    int64 GetCredit() const
    {
        if (IsMine())
            return nValue;
        return 0;
    }
    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return(a.nValue == b.nValue && a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return!(a==b);
    }
    string ToString() const
    {
        if (scriptPubKey.size()<6)
            return "CTxOut(error)";
        return strprintf("CTxOut(nValue=%I64d.%8I64d,scriptPubKey=%s",nValue/COIN,nValue%COIN,scriptPubKey.ToString().substr(0,24).c_str());
    }
    void print() const {printf("%s\n",ToString().c_str());}

};

class CTransaction
{
public:
    int nVersion;
    vector<CTxIn> vin;
    vector<CTxOut> vout;
    int nLockTime;

    CTransaction() {SetNull();}
    IMPLEMENT_SERIALIZE
    (
     READWRITE(this->nVersion);
     nVersion = this->nVersion;
     READWRITE(vin);
     READWRITE(vout);
     READWRITE(nLockTime);
     )

    void SetNull()
    {
        nVersion=0;
        vin.clear();
        vout.clear();
        nLockTime=0;
    }
    bool IsNull() const {return (vin.empty() && vout.empty());}
    //SerializeHash?
    uint256 GetHash() const {return SerializeHash(*this);}
    bool IsFinal() const
    {
        //nBestHeight?
        if (nLockTime==0||nLockTime<nBestHeight)
            return true;
        foreach(const CTxIn& txin, vin)
            if (!txin.IsFinal())
                return false;
        return true;
    }
    //包含nSequence的最小值是最新的
    bool IsNewerThan(const CTransaction& old) const
    {
        if (vin.size()!=old.vin.size())
            return false;
        for (int i=0; i<vin.size(); i++)
        {
            if (vin[i].prevout!=old.vin[i].prevout)
            {
                return false;
            }
        }
        bool fNewer = false;
        unsigned int nLowest = UINT_MAX;
        for (int i=0; i<vin.size(); i++)
        {
            if (vin[i].nSequence <= nLowest)
            {
                fNewer = false;
                nLowest = vin[i].nSequence;
            }
            if (old.vin[i].nSequence < nLowest)
            {
                fNewer = true;
                nLowest = old.vin[i].nSequence;
            }
        }
        return fNewer;
    }
    bool IsCoinBase() const {return (vin.size() ==1 && vin[0].prevout.IsNull());}

    bool CheckTransaction() const
    {
        if (vin.empty()||vout.empty())
            return error("CTransaction::CheckTransaction():vin or vout empty");
        foreach(const CTxOut& txout, vout)
            if (txout.nValue<0)
                return error("CTransaction::CheckTransaction() : txout.nValue negative");
        if (IsCoinBase())
        {
            if(vin[0].scriptSig.size()<2||vin[0].scriptSig.size()>100)
                return error("CTransaction::CheckTransaction() : coinbase script size");
        }

        else
        {
            foreach(const CTxIn& txin, vin)
            if (txin.prevout.IsNull())
                return error("CTransaction::CheckTransaction() : prevout is null");
        }

        return true;
    }
    bool IsMine() const
    {
        foreach(const CTxOut& txout, vout)
            if (txout.IsMine())
                return true;
        return false;
    }
    int64 GetDebit() const
    {
        int64 nDebit = 0;
        foreach(const CTxIn& txin, vin)
            nDebit += txin.GetDebit();
        return nDebit;
    }
    int64 GetCredit() const
    {
        int64 nCredit =0;
        foreach(const CTxOut& txout, vout)
            nCredit += txout.GetCredit();
        return nCredit;
    }
    int64 GetValueOut() const
    {
        int64 nValueOut = 0;
        foreach(const CTxOut& txout, vout)
        {
            if (txout.nValue<0)
                throw runtime_error("CTransaction::GetValueOut() : negative value");
            nValue += txout.nValue;
        }
        return nValueOut;
    }
    //
    int64 GetMinFee(bool fDiscout=false) const
    {
        unsigned int nBytes=::GetSerializeSize(*this,SER_NETWORK);
        if (fDiscount && nBytes<10000)
            return 0 ;
        return (1+(int64)nBytes/1000)*CENT;
    }

    bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
    {
        CAutoFile filein=OpenBlockFile(pos.nFile,0,pfileRet?"rb+":"rb");
        if (!filein)
        {
            return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");
        }
        if (fseek(filein,pos.nTxPos,SEEK_SET)!=0)
            return error("CTransaction::ReadFromDisk() : fseek failed");
        filein >> *this;

        if (pfileRet)
        {
            if (fseek(filein,pos.nTxPos,SEEK_SET)!=0)
                return error("CTransaction::ReadFromDisk() : second fseek failed");
            *pfileRet=filein.release();
        }
        return true;
    }
    friend bool operator==(const CTransaction&a,const CTransaction& b)
    {
        return(a.nVersion == b.nVersion&& a.vin==b.vin&& a.vout==b.vout&&a.nLockTime==b.nLockTime);
    }
    friend bool operator!=(const CTransaction&a, const CTransaction& b)
    {
        return !(a==b);
    }
    string ToString()const
    {
        string str;
        str += strprintf("CTransaction(hash=%s,ver=%d,vin=%d,vin.size=%d,vout.size=%d,nLockTime=%d)n",GetHash(),ToString(),substr(0,6).c_str(),nVersion,vin.size(),vout.size(),nLockTime);
        for (int i = 0; i < vin.size();i++)
        {
            str+="   "+vin[i].ToString()+"\n";
        }
        for (int i = 0; i <vout.size();i++)
        {
             str+="   "+vout[i].ToString()+"\n";
        }
        return str;

    }
    void print() const
    {
        printf("(%s)\n",ToString().c_str());
    }

    bool DisConnectInputs(CTxDB& txdb);
    bool ConnectInputs(CTxDB& txdb,std::map<uint256, CTxIndex> &mapTestPool,CDiskTxPos posThisTx, int nHeight, int64& nFees, bool fBlock, bool fMiner, int64 nMinFee=0);
    bool ClinetConnectInputs();
    bool AcceptTransaction(CTxDB &txdb,bool fCheckInputs=true, bool* pfMissingInputs=NULL);
    bool AcceptTransaction(bool fCheckInputs=true,bool *pfMissingInputs=NULL)
    {
        CTxDB txdb("r");
        return AcceptTransaction(txdb,fCheckInputs,pfMissingInputs);
    }

protected:
    bool AddToMemoryPool();
public:
    bool RemoveFromMemoryPool();
};


class CMerkleTx:public CTransaction
{
public:
    uint256 hashBlock;
    vector<uint256> vMerkleBranch;
    int nIndex;

    mutable bool fMerkleVerified;

    CMerkleTx() {Init();}
    CMerkleTx(const CTransaction& txIn):CTransaction(txIn)
    {
        Init();
    }

    void Init()
    {
        hashBlock=0;
        nIndex=-1;
        fMerkleVerified=false;
    }

    int64 GetCredit() const
    {
        if (IsCoinBase()&&GetBlocksToMaturity()>0)
            return 0;
       return CTransaction::GetCredit();
    }

    IMPLEMENT_SERIALIZE
    {
        nSerSize+=SerReadWrite(s,*(CTransaction*)this,nType,nVersion,ser_action);
        nVersion=this->nVersion;
        READWRITE(hashBlock) ;
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    }
    int SetMerkleBranch(const CBlock* pblock=NULL);
    int GetDepthInMainChain() const;
    bool IsInMainChain() const{return GetDepthInMainChain()>0;}
    int GetBlocksToMaturity() const;
    bool AcceptTransaction(CTxDB& txdb,bool fCheckInputs=true);
    bool AcceptTransaction() {CTxDB txdb("r"); return AcceptTransaction(txdb);}

};

class CWalletTx:public CMerkleTx
{
public:
    vector<CMerkleTx> vtxPrev;
    map<string, string> mapValue;
    vector<pair<string,string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived;

    char fFromMe;
    char fSpent;

    mutable unsigned int nTimeDisplayed;

    CWalletTx(){Init();}
    CWalletTX(const CMerkleTx& txIn):CMerkleTx(txIn)
    {
        Init();

    }
    CWalletTX(const CTransaction& txIn):CMerkleTx(txIn)
    {
        Init();

    }
    void Init()
    {
        fTimeReceivedIsTxTime = false;
        nTimeReceived = 0 ;
        fFromMe = false;
        fSpent = false;
        nTimeDisplayed = 0;
    }

    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CMerkleTx*)this,nType,nVersion,ser_action);
        nVersion = this->nVersion;
        READWRITE(vtxPrev)；
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);
    )

    bool WriteToDisk()
    {
        return CWalletDB().WriteTx(GetHash(),*this);
    }

    int64 GetTxtTime() const;
    void AddSupportingTransactions(CTxDB& txdb);
    bool AcceptWalletTransaction(CTxDB& txdb,bool fCheckInputs=true);
    bool AcceptWalletTransaction(){CTxDB txdb("r");return AcceptWalletTransaction(txdb);}
    void RelayWalletTransaction(CTxDB& txdb);
    void RelayWalletTransaction(){CTxDB txdb("r");RelayWalletTransaction(txdb);


};


class CTxIndex
{
public:
    CDiskTxPos pos;
    vector<CDiskTxPos> vSpent;
    CTxIndex() {SetNull();}
    CTxIndex(const CDiskTxPos& posIn,unsigned int nOutputs)
    {
        pos=posIn;
        vSpent.resize(nOutputs);
    }

    IMPLEMENT_SERIALIZE
    (
        if(!(type&SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(pos);
        READWRITE(vSpent);
    )
    void SetNull()
    {
        pos.SetNull();
        vSpent.clear();
    }
    bool IsNull() {return pos.IsNull();}
    friend bool operator==(const CTxIndex& a, const CTxIndex& b)
    {
        if（a.pos != b.pos || a.vSpent.size()!=b.vSpent.size())
            return false;
        for (int i = 0; i < a.vSpent.size();i++)
        {
            if (a.vSpent[i] != b.vSpent[i])
                return false;
        }
        return true;
    }
    friend bool operator!=(const CTxIndex& a,const CTxIndex& b) {return !(a==b);}

};


class CBlock
{
public:
    int nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;
    vector<CTransaction> vtx;
    mutable vector<uint256> vMerkleTree;
    CBlock() {SetNull()；}
    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion=this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        if (!(nType&(SER_GETHASH|SER_BLOCKHEADERONLY)))
            READWRITE(vtx);
        else if (fRead)
            const_cast<CBlock*>(this)->vtx.clear();

    )
   void SetNull()
   {
    nVersion=1;
    hashPrevBlock=0;
    hashMerkleRoot=0;
    nTime=0;
    nBits=0;
    nNonce=0;
    vtx.clear();
    vMerkleTree.clear();
   }

    bool IsNull() const {return (nBits == 0);}
    uint256 GetHash() const {return Hash(BEGIN(nVersion),END(nNonce));}
    uint256 BuildMerkleTree() const
    {
        vMerkleTree.clear();
        foreach(const CTransaction& tx,vtx)
            vMerkleTree.push_back(tx.GetHash());
        int j=0;
        for(int nSize=vtx.size();nSize>1;nSize=(nSize+1)/2)
        {
            for(int i=0;i<nSize;i+=2)
            {
                int i2=min(i+1,nSize-1);
                vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),END(vMerkleTree[j+i]),BEGIN(vMerkleTree[j+i2]),END(vMerkleTree[j+i2])));
            }
            j+=nSize;
        }
        return (vMerkleTree.empty()?0:vMerkleTree.back());
    }

    vector<uint256> GetMerkleBranch(int nIndex) const
    {
        if(vMerkleTree.empty())
            BuildMerkleTree();
        vector<uint256> vMerkleBranch;
        int j=0;
        for(int nSize=vtx.size();nSize>1;nSize=(nSize+1)/2)
        {
            int i=min(nIndex^1,nSize-1);
            vMerkleBranch.push_back(vMerkleTree[j+i]);
            nIndex>>=1;
            j+=nSize;
        }
        return vMerkleBranch;
    }
    static uint256 CheckMerkleBranch(uint256 hash,const vector<uint256>& vMerkleBranch, int nIndex)
    {
        if (nIndex==-1)
        {
            return 0;
        }
        foreach(const uint256& otherside, vMerkleBranch)
        {
            if(nIndex&1)
                hash=Hash(BEGIN(otherside),END(otherside),BEGIN(hash),END(hash));
            else
                hash=Hash(BEGIN(hash),END(hash),BEGIN(otherside),END(otherside));
            nIndex>>=1;
        }
        return hash;
    }

    bool WriteToDisk(bool fWriteTransactions,unsigned int& nFileRet,unsigned int& nBlockPosRet)
    {
        CAutoFile fileout=AppendBlockFile(nFileRet);
        if(!fileout)
            return error("CBlock::WriteToDisk():AppendBlockFile failed");
        if(!fWriteTransactions)
            fileout.nType |=SER_BLOCKHEADERONLY;

        unsigned int nSize=fileout.GetSerializeSize(*this);
        fileout<<FLATDATA(pchMessageStart)<<nSize;
        nBlockPosRet=ftell(fileout);
        if(nBlockPosRet==-1)
            return error("CBlock::WriteToDisk():ftell failed");
        fileout<<*this;
        return true;
    }
    bool ReadFromDisk(unsigned int nFile,unsigned int nBlockPos,bool fReadTransactions)
    {
        SetNull();
        CAutoFile filein=OpenBlockFile(nFile,nBlockPos,"rb");
        if(!filein)
            return error("CBlock::ReadFromDisk():OpenBlockFile failed");
        if(!fReadTransactions)
            filein.nType!=SER_BLOCKHEADERONLY;
        filein>>*this;
        if(CBigNum().SetCompact(nBits)>bnProofOfWorkLimit)
            return error("CBlock::ReadFromDisk():nBits errors in block header");
        if(GetHash()>CBigNum().SetCompact(nBits).getuint256())
            return error("CBlock::ReadFromDisk():GetHash() errors in block header");
        return true;
    }

    void print() const
    {
        printf("CBlock(hash=%s,ver=%d,hashPrevBlock=%s,hashMerkleRoot=%s,nTime=%u,nBits=%08x,nNonce=%u,vtx=%d)\n",GetHash().ToString().substr(0,14).c_str(),nVersion,hashPrevBlock.ToString().substr(0,14).c_str(),hashMerkleRoot.ToString().substr(0,6).c_str(),nTime,nBits,nNonce,vtx.size());
        for (int i=0;i<vtx.size();i++）
        {
            printf("  ");
            vtx[i].pirnt();
        }
        pirntf("  vMerkleTree:  ");
        for（int i=0;i<vMerkleTree.size();i++)
            printf("%s",vMerkleTree[i].ToString().substr(0,6).c_str());
        printf("\n ");
    }
    int64 GetBlockValue(int64 nFees) const;
    bool DisConnectBlock(CTxDB& txdb,CBlockIndex* pindex);
    bool ConnectBlock(CTxDB& txdb,CBlockIndex* pindex);
    bool ReadFromDisk(const CBlockIndex* blockindex,bool fReadTransactions);
    bool AddToBlockIndex(unsigned int nFile,unsigned int nBlockPos);
    bool CheckBlock() const;
    bool AcceptBlock();

};


class CBlockIndex
{
public:
    const uint256* phashBlock;
    CBlockIndex* pprev;
    CBlockIndex* pnext;
    unsigned int nFile；
    unsigned int nBlockPos;
    int nHeight;
    int nVersion;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    CBlockIndex()
    {
        phashBlock=NULL;
        pprev=NULL;
        pnext=NULL;
        nFile=0;
        nBlockPos=0;
        nHeight=0;
        nVersion=0;
        hashMerkleRoot=0;
        nTime=0;
        nBits=0;
        nNonce=0;
    }
    CBlockIndex(unsigned int nFileIn,unsigned int nBlockPosIn,CBlock& block)
    {
        phashBlock=NULL;
        pprev=NULL;
        pnext=NULL;
        nFile=nFileIn;
        nBlockPos=nBlockPosIn;
        nHeight=0;
        nVersion=block.nVersion;
        hashMerkleRoot=block.hashMerkleRoot;
        nTime=block.nTime;
        nBits=block.nBits;
        nNonce=block.nNonce;
    }
    uint256 GetBlockHash() const {return *phashBlock；}
    bool IsInMainChain() const { return (pnext||this==pindexBest);}
    bool EraseBlockFromDisk()
    {
        CAutoFile fileout=OpenBlockFile(nFile,nBlockPos,"rb+");
        if(!fileout)
            return false;
        CBlock block;
        block.SetNull();
        fileout<<block;
        return true;
    }

    enum {nMedianTimeSpan=11};
    int64 GetMedianTimePast() const
    {
        unsigned int pmedian[nMedianTimeSpan];
        unsigned int* pbegin=&pmedian[nMedianTimeSpan];
        unsigned int* pend=&pmedian[nMedianTimeSpan];
        const CBlockIndex* pindex=this;
        for (int i=0;i<nMedianTimeSpan&&pindex;i++,pindex=pindex->pprev)*(--pbegin)=pindex->nTime;
        sort(pbegin,pend);
        return pbegin[(pend-pbegin)/2];
    }
    int64 GetMedianTime() const
    {
        const CBlockIndex* pindex=this;
        for(int i=0;i<nMedianTimeSpan/2;i++)
        {
            if(!pindex->pnext)
                return nTime;
            pindex=pindex->pnext;
        }
        return pindex->GetMedianTimePast();
    }

    string ToString() const
    {
        return strprintf("CBlockIndex(nprev=%8x,pnext=%08x,nFile=%d,nBlockPos=%-6d,nHeight=%d,merkle=%s,hashBlock=%s)",pprev,pnext,nFile,nBlockPos,nHeight,hashMerkleRoot.ToString().substr(0,6).c_str,GetBlockHash().ToString().substr(0,14).c_str());
    }

    void print() const {printf("%s\n",ToString().c_str());
};


class CDiskBlockIndex:public CBlockIndex
{
public:
    uint256 hashPrev;
    uint256 hashNext;
    CDiskBlockIndex() {hashPrev=0;hashNext=0;}
    explicit CDiskBlockIndex(CBlockIndex* pindex):CBlockIndex(*pindex)
    {
        hashPrev=(pprev?pprev->GetBlockHash():0);
        hashNext=(pnext?pnext->GetBlockHash():0);
    }

    IMPLEMENT_SERIALIZE
    (
        if(!(nType&SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(hashNext);
        READWRITE(nFile);
        READWRITE(nBlockPos);
        READWRITE(nHeight);
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    )
    uint256 GetBlockHash() const
    {
        CBlock block;
        block.nVersion=nVersion;
        block.hashPrevBlock=hashPrev;
        block.hashMerkleRoot=hashMerkleRoot;
        block.nTime=nTime;
        block.nBits=nBits;
        block.nNonce=nNonce;
        return block.GetHash();

    }
    string ToString() const
    {
        string str= "CDiskBlockIndex(";
        str+=CBlockIndex::ToString();
        str+=strprintf("\n hashBlock=%s,hashPrev=%s,hashNext=%s)",GetBlockHash().ToString().c_str(),hashPrev.ToString().substr(0,14).c_str(),hashNext.ToString().substr(0,14).c_str());
        return str;
    }
    void print() const {printf("%s\n",ToString().c_str());}



};

class  CBlockLocator
{
protected:
    vector<uint256> vHave;
public:
     CBlockLocator();
    explicit CBlockLocator(const CBlockIndex*pindex) {Set(pindex)；}
    explicit CBlockLocator(uint256 hashBlock)
    {
        map<uint256,CBlockIndex*>::iterator mi=mapBlockIndex.find(hashBlock);
        if(mi!=mapBlockIndex.end())
            Set((*mi).second);
    }
    IMPLEMENT_SERIALIZE
    (
        if(!(nType&SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    )
    void Set(const CBlockIndex* pindex)
    {
        vHave.clear();
        int nStep=1；
        while(pindex)
        {
            vHave.push_back(pindex->GetBlockHash());
            for (int i=0;pindex&&i<nStep；i++)
                pindex=pindex->pprev;
            if(vHave.size()>10)
                nStep*=2;
        }
        vHave.push_back(hashGenesisBlock);
    }
    CBlockIndex *GetBlockIndex()
    {
        foreach(const uint256&hash,vHave)
        {
            map<uint256,CBlockIndex*>::iterator mi=mapBlockIndex,find(hash);
            if(mi!=mapBlockIndex.end())
            {
                CBlockIndex* pindex=(*mi).second;
                if(pindex->IsInMainChain())
                    return pindex;
            }
        }
        return pindexGenesisBlock;
    }

    uint256 GetBlockHash()
    {
        foreach(const uint256& hash,vHave)
        {
            map<uint256,CBlockIndex*>::iterator mi=mapBlockIndex.find(hash);
            if(mi!=mapBlockIndex.end())
            {
                CBlockIndex* pindex=(*mi).second;
                if(pindex->IsInMainChain())
                    return hash;
            }
        }
        return hashGenesisBlock;
    }
    int GetHeight()
    {
        CBlockIndex* pindex=GetBlockIndex();
        if(!pindex)
            return 0;
        return pindex->nHeight;
    }
};

extern map<uint256,CTransaction> mapTransactions;
extern map<uint256,CWalletTx> mapWallet;
extern vector<pair<uint256,bool> >vWalletUpdated;
extern CCriticalSection cs_mapWallet;
extern map<vector<unsigned char>,CPrivKey> mapKeys;
extern map<uint160,vector<unsigned char> >mapPubKeys;
extern CCriticalSection cs_mapKeys;
extern CKey keyUser;

#endif /* main_h */
