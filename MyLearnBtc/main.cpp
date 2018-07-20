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

int 

















#include <iostream>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
