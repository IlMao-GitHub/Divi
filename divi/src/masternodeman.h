// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODEMAN_H
#define MASTERNODEMAN_H

#include "key.h"
#include "masternode.h"
#include "net.h"
#include "sync.h"

#include <memory>

#define MASTERNODES_DUMP_SECONDS (15 * 60)
#define MASTERNODES_DSEG_SECONDS (3 * 60 * 60)

class CMasternodeMan;

extern CMasternodeMan mnodeman;

class CMasternodeMan
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    // critical section to protect the inner data structures specifically on messaging
    mutable CCriticalSection cs_process_message;

    // map to hold all MNs
    std::vector<CMasternode> vMasternodes;
    // who's asked for the Masternode list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForMasternodeList;
    // who we asked for the Masternode list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForMasternodeList;
    // which Masternodes we've asked for
    std::map<COutPoint, int64_t> mWeAskedForMasternodeListEntry;

    // Cache of the most recent masternode ranks, so we can efficiently check
    // if some masternode is in the top-20 for a recent block height.
    class RankingCache;
    std::unique_ptr<RankingCache> rankingCache;

public:
    // Keep track of all broadcasts I've seen
    std::map<uint256, CMasternodeBroadcast> mapSeenMasternodeBroadcast;
    // Keep track of all pings I've seen
    std::map<uint256, CMasternodePing> mapSeenMasternodePing;

    // keep track of dsq count to prevent masternodes from gaming obfuscation queue
    int64_t nDsqCount;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        LOCK(cs);
        READWRITE(vMasternodes);
        READWRITE(mAskedUsForMasternodeList);
        READWRITE(mWeAskedForMasternodeList);
        READWRITE(mWeAskedForMasternodeListEntry);
        READWRITE(nDsqCount);

        READWRITE(mapSeenMasternodeBroadcast);
        READWRITE(mapSeenMasternodePing);
    }

    CMasternodeMan();
    CMasternodeMan(const CMasternodeMan& other) = delete;
    ~CMasternodeMan();

    /// Add an entry
    bool Add(const CMasternode& mn);

    /// Ask (source) node for mnb
    void AskForMN(CNode* pnode, const CTxIn& vin);

    /// Check all Masternodes
    void Check();

    /// Check all Masternodes and remove inactive
    void CheckAndRemoveInnactive(bool forceExpiredRemoval = false);
    void CheckAndRemove() {} // dummy overload for loading/storing from db cache

    /// Clear Masternode vector
    void Clear();

    int CountEnabled(int protocolVersion = -1) const;

    void CountNetworks(int protocolVersion, int& ipv4, int& ipv6, int& onion);

    void DsegUpdate(CNode* pnode);

    /// Find an entry
    CMasternode* Find(const CScript& payee) = delete;
    CMasternode* Find(const CTxIn& vin);
    CMasternode* Find(const CPubKey& pubKeyMasternode);

    /// Find an entry in the masternode list that is next to be paid
    std::vector<CMasternode*> GetMasternodePaymentQueue(const uint256& seedHash, int nBlockHeight, bool fFilterSigTime);
    std::vector<CMasternode*> GetMasternodePaymentQueue(const CBlockIndex* pindex, int offset, bool fFilterSigTime);
    CMasternode* GetNextMasternodeInQueueForPayment(const CBlockIndex* pindex, int offset, bool fFilterSigTime);

    /// Find a random entry
    CMasternode* FindRandomNotInVec(std::vector<CTxIn>& vecToExclude, int protocolVersion = -1);

    std::vector<CMasternode> GetFullMasternodeVector() const
    {
        LOCK(cs);
        return vMasternodes;
    }

    /** Returns the given masternode's rank among all active and with the
     *  given minimum protocol version.  Returns (unsigned)-1 if the node is not
     *  found or not active itself.
     *
     *  If the given node is not in the top-"nCheckNum" masternodes by rank, then
     *  nCheckNum + 1 is returned (instead of the exact rank).  */
    unsigned GetMasternodeRank(const CTxIn& vin, const uint256& seedHash,
                               int minProtocol, unsigned nCheckNum);

    void ProcessMasternodeConnections();

    /** Records a ping in the list of our seen ping messages, and also updates the
     *  list of known broadcasts if the ping corresponds to one we know (i.e. updates
     *  the ping contained in the seen broadcast).
     *
     *  This method assumes that the ping has already been checked and is valid.
     */
    void RecordSeenPing(const CMasternodePing& mnp);

    /** Processes a masternode broadcast.  It is verified first, and then
     *  the masternode updated or added accordingly.
     *
     *  If pfrom is null, we assume this is a startmasternode or broadcaststartmasternode
     *  command.  Otherwise, we apply any potential DoS banscore.
     *
     *  Returns true if all was valid, and false if not.  */
    bool ProcessBroadcast(CNode* pfrom, CMasternodeBroadcast& mnb);

    /** Processes a masternode ping.  It is verified first, and if valid,
     *  used to update our state and inserted into mapSeenMasternodePing.
     *
     *  If pfrom is null, we assume this is from a local RPC command.  Otherwise
     *  we apply potential DoS banscores.
     *
     *  Returns true if the ping message was valid.  */
    bool ProcessPing(CNode* pfrom, CMasternodePing& mnp);

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    /// Return the number of (unique) Masternodes
    int size() const { return vMasternodes.size(); }

    /// Return the number of Masternodes older than (default) 8000 seconds
    int stable_size ();

    std::string ToString() const;

    void Remove(const CTxIn& vin);

    void ResetRankingCache();
};

#endif
