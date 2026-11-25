// @@@LICENSE
//
//      Copyright (c) 2024 Herman van Hazendonk <github.com@herrie.org>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#ifndef EASCLIENT_H_
#define EASCLIENT_H_

#include "core/MojObject.h"
#include "core/MojServiceMessage.h"
#include "luna/MojLunaService.h"
#include "client/BusClient.h"
#include "client/CommandManager.h"
#include "EasCoreDefs.h"
#include "data/EasAccount.h"
#include "CommonErrors.h"
#include <boost/shared_ptr.hpp>
#include <string>

class EasBusDispatcher;
class EasConfig;
class DatabaseInterface;
class PowerManager;

/**
 * EasClient - Main client class for Exchange ActiveSync
 *
 * Manages connection to Exchange server and coordinates sync operations.
 * Based on ImapClient architecture from mojomail-imap.
 */
class EasClient : public BusClient, public Command::Listener
{
public:
    // Client states (matching binary symbols)
    enum State {
        State_None,
        State_NeedsAccountInfo,
        State_GettingAccount,
        State_NeedsOptions,
        State_GettingOptions,
        State_NeedsProvision,
        State_Provisioning,
        State_NeedsFolderSync,
        State_FolderSyncing,
        State_Idle,
        State_Syncing,
        State_Pinging,
        State_Error,
        State_Disabled,
        State_Shutdown
    };

    EasClient(MojLunaService* service,
              EasBusDispatcher* dispatcher,
              const MojObject& accountId,
              const boost::shared_ptr<DatabaseInterface>& dbInterface);
    virtual ~EasClient();

    // Account management
    void SetAccount(const boost::shared_ptr<EasAccount>& account);
    const boost::shared_ptr<EasAccount>& GetAccount() const { return m_account; }
    const MojObject& GetAccountId() const { return m_accountId; }
    std::string GetAccountIdString() const { return m_accountIdString; }
    bool HasAccount() const;

    // State management
    State GetState() const { return m_state; }
    void SetState(State state);
    bool IsShutdown() const { return m_state == State_Shutdown; }
    const char* GetStateName(State state);

    // Command management
    void QueueCommand(MojRefCountedPtr<class EasCommand> command);
    void CheckQueue();

    // Command::Listener interface
    virtual void CommandComplete(Command* command);

    // Sync operations
    void SyncAccount();
    void SyncFolder(const MojObject& folderId);
    void StartPing();
    void StopPing();

    // Protocol info
    std::string GetProtocolVersion() const { return m_protocolVersion; }
    void SetProtocolVersion(const std::string& version) { m_protocolVersion = version; }
    std::string GetPolicyKey() const { return m_policyKey; }
    void SetPolicyKey(const std::string& key) { m_policyKey = key; }

    // Service access
    EasBusDispatcher* GetBusDispatcher() const { return m_busDispatcher; }
    const boost::shared_ptr<DatabaseInterface>& GetDatabaseInterface() const { return m_dbInterface; }
    MojLogger& GetLogger() { return m_log; }

    // Error handling
    void SetAccountError(MailError::ErrorCode errorCode, const std::string& errorText);
    void ClearAccountError();

    // Command callbacks
    void OptionsSuccess(float version, const std::string& serverType, bool searchSupported);
    void ProvisionSuccess(const std::string& policyKey);
    void FolderSyncSuccess(const std::string& syncKey,
                           const std::vector<class EasFolder>& folders);
    void FolderSyncRequired();
    void SyncComplete(const std::string& folderId,
                      const std::vector<MojObject>& adds,
                      const std::vector<MojObject>& changes,
                      const std::vector<std::string>& deletes,
                      bool moreAvailable);
    void SyncKeyInvalid(const std::string& folderId);
    void PingNoChanges();
    void PingChangesDetected(const std::vector<std::string>& folderIds);

protected:
    static MojLogger s_log;

    EasBusDispatcher*                       m_busDispatcher;
    boost::shared_ptr<DatabaseInterface>    m_dbInterface;
    boost::shared_ptr<EasAccount>           m_account;
    boost::shared_ptr<CommandManager>       m_commandManager;
    boost::shared_ptr<PowerManager>         m_powerManager;

    MojObject       m_accountId;
    std::string     m_accountIdString;
    MojLogger&      m_log;
    State           m_state;

    // Protocol state
    std::string     m_protocolVersion;
    std::string     m_policyKey;
    std::string     m_folderSyncKey;

    // Ping state
    bool            m_pingActive;
    int             m_pingHeartbeat;
};

#endif /* EASCLIENT_H_ */
