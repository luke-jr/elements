// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodeltransaction.h"

#include "policy/policy.h"
#include "wallet/wallet.h"

WalletModelTransaction::WalletModelTransaction(const QList<SendAssetsRecipient> &_recipients) :
    recipients(_recipients),
    walletTransaction(0),
    fee(0)
{
    walletTransaction = new CWalletTx();
}

WalletModelTransaction::~WalletModelTransaction()
{
    delete walletTransaction;
}

QList<SendAssetsRecipient> WalletModelTransaction::getRecipients()
{
    return recipients;
}

CWalletTx *WalletModelTransaction::getTransaction()
{
    return walletTransaction;
}

unsigned int WalletModelTransaction::getTransactionSize()
{
    return (!walletTransaction ? 0 : ::GetVirtualTransactionSize(*walletTransaction));
}

CAmount WalletModelTransaction::getTransactionFee()
{
    return fee;
}

void WalletModelTransaction::setTransactionFee(const CAmount& newFee)
{
    fee = newFee;
}

void WalletModelTransaction::reassignAmounts(const std::vector<CAmount>& outAmounts, int nChangePosRet)
{
    int i = 0;
    for (auto it = recipients.begin(); it != recipients.end(); ++it)
    {
        auto& rcp = (*it);

        if (rcp.paymentRequest.IsInitialized())
        {
            CAmount subtotal = 0;
            const payments::PaymentDetails& details = rcp.paymentRequest.getDetails();
            for (int j = 0; j < details.outputs_size(); j++)
            {
                const payments::Output& out = details.outputs(j);
                if (out.amount() <= 0) continue;
                if (i == nChangePosRet)
                    i++;
                subtotal += outAmounts[i];
                i++;
            }
            rcp.amount = subtotal;
        }
        else // normal recipient (no payment request)
        {
            if (i == nChangePosRet)
                i++;
            rcp.amount = outAmounts[i];
            i++;
        }
    }
}

CAmount WalletModelTransaction::getTotalTransactionAmount()
{
    CAmount totalTransactionAmount = 0;
    Q_FOREACH(const SendAssetsRecipient &rcp, recipients)
    {
        totalTransactionAmount += rcp.amount;
    }
    return totalTransactionAmount;
}

void WalletModelTransaction::newPossibleKeyChange(CWallet *wallet)
{
    keyChange.emplace_back(wallet);
}

std::vector<CReserveKey> *WalletModelTransaction::getPossibleKeyChange()
{
    return &keyChange;
}
