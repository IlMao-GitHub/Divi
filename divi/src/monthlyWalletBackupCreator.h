#ifndef MONTHLYWALLETBACKUPCREATOR_H
#define MONTHLYWALLETBACKUPCREATOR_H

#define NUMBER_OF_SECONDS_IN_A_MONTH 3600 * 24 * 30.

#include <ctime>
#include <i_walletBackupCreator.h>
#include <i_filesystem.h>



class MonthlyWalletBackupCreator : public I_WalletBackupCreator
{
private:
    I_WalletBackupCreator& backupWalletCreator_;
    I_FileSystem& fileSystem_;
    std::time_t getTimeOfLastBackupDate() const;
    std::string backupDirectory_;
public:
    MonthlyWalletBackupCreator(
        I_WalletBackupCreator& backupWalletCreator,
        I_FileSystem& fileSystem,
        std::string backupDirectory
        ): backupWalletCreator_(backupWalletCreator)
        , fileSystem_(fileSystem)
        , backupDirectory_(backupDirectory) {

        }
    virtual ~MonthlyWalletBackupCreator(){}
    virtual bool BackupWallet();
    virtual bool CheckWalletIntegrity(bool resync);
    virtual std::string GetBackupSubfolderDirectory() const;
};

#endif //MONTHLYWALLETBACKUPCREATOR_H