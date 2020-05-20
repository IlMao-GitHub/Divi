// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validationinterface.h"

MainNotificationSignals ValidationInterfaceRegistry::g_signals;

void ValidationInterfaceRegistry::RegisterValidationInterface(CValidationInterface* pwalletIn)
{
    registeredInterfaces.insert(pwalletIn);
    pwalletIn->RegisterWith(g_signals);
}
void ValidationInterfaceRegistry::UnregisterValidationInterface(CValidationInterface* pwalletIn)
{
    registeredInterfaces.erase(pwalletIn);
    pwalletIn->UnregisterWith(g_signals);
}

void ValidationInterfaceRegistry::UnregisterAllValidationInterfaces() {
    for(CValidationInterface* interfaceObj: registeredInterfaces)
    {
        interfaceObj->UnregisterWith(g_signals);
    }
    registeredInterfaces.clear();
}

void ValidationInterfaceRegistry::SyncWithWallets(const CTransaction &tx, const CBlock *pblock = NULL) {
    g_signals.SyncTransaction(tx, pblock);
}

MainNotificationSignals& ValidationInterfaceRegistry::getSignals() const
{
    return g_signals;
}

void CValidationInterface::RegisterWith(MainNotificationSignals& signals){
    signals.SyncTransaction.connect(boost::bind(&CValidationInterface::SyncTransaction, this, _1, _2));
    signals.UpdatedTransaction.connect(boost::bind(&CValidationInterface::UpdatedTransaction, this, _1));
    signals.SetBestChain.connect(boost::bind(&CValidationInterface::SetBestChain, this, _1));
    signals.Inventory.connect(boost::bind(&CValidationInterface::Inventory, this, _1));
    signals.Broadcast.connect(boost::bind(&CValidationInterface::ResendWalletTransactions, this));
    signals.BlockChecked.connect(boost::bind(&CValidationInterface::BlockChecked, this, _1, _2));
}

void CValidationInterface::UnregisterWith(MainNotificationSignals& signals) {
    signals.BlockChecked.disconnect(boost::bind(&CValidationInterface::BlockChecked, this, _1, _2));
    signals.Broadcast.disconnect(boost::bind(&CValidationInterface::ResendWalletTransactions, this));
    signals.Inventory.disconnect(boost::bind(&CValidationInterface::Inventory, this, _1));
    signals.SetBestChain.disconnect(boost::bind(&CValidationInterface::SetBestChain, this, _1));
    signals.UpdatedTransaction.disconnect(boost::bind(&CValidationInterface::UpdatedTransaction, this, _1));
    signals.SyncTransaction.disconnect(boost::bind(&CValidationInterface::SyncTransaction, this, _1, _2));
}