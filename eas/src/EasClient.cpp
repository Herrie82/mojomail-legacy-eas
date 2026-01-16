// @@@LICENSE
//
//      Copyright (c) 2024 WebOS Ports Project
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

#include "EasClient.h"
#include "EasBusDispatcher.h"
#include "data/EasAccount.h"
#include "data/EasFolder.h"
#include "commands/EasCommand.h"

using namespace boost;
using namespace std;

MojLogger EasClient::s_log("com.palm.eas.client");

// State names for logging (matches binary's stateNames array)
static const char* s_stateNames[] = {
    "None",
    "NeedsAccountInfo",
    "GettingAccount",
    "NeedsOptions",
    "GettingOptions",
    "NeedsProvision",
    "Provisioning",
    "NeedsFolderSync",
    "FolderSyncing",
    "Idle",
    "Syncing",
    "Pinging",
    "Error",
    "Disabled",
    "Shutdown"
};

EasClient::EasClient(MojLunaService* service,
                     EasBusDispatcher* dispatcher,
                     const MojObject& accountId,
                     const boost::shared_ptr<DatabaseInterface>& dbInterface)
    : BusClient(service)
    , m_busDispatcher(dispatcher)
    , m_dbInterface(dbInterface)
    , m_commandManager(new CommandManager(10, true))  // Max 10 concurrent commands
    , m_accountId(accountId)
    , m_log(s_log)
    , m_state(State_NeedsAccountInfo)
    , m_pingActive(false)
    , m_pingHeartbeat(540)  // Default 9 minutes
{
    // Convert account ID to string for logging
    MojString idStr;
    m_accountId.stringValue(idStr);
    m_accountIdString = idStr.data();

    MojLogInfo(m_log, "EasClient created for account %s", m_accountIdString.c_str());
}

EasClient::~EasClient()
{
    MojLogInfo(m_log, "EasClient destroyed for account %s", m_accountIdString.c_str());
}

const char* EasClient::GetStateName(State state)
{
    if (state >= 0 && state < sizeof(s_stateNames) / sizeof(s_stateNames[0])) {
        return s_stateNames[state];
    }
    return "Unknown";
}

void EasClient::SetState(State state)
{
    if (m_state != state) {
        MojLogInfo(m_log, "EasClient state change: %s -> %s (account %s)",
                   GetStateName(m_state), GetStateName(state), m_accountIdString.c_str());
        m_state = state;
    }
}

void EasClient::SetAccount(const boost::shared_ptr<EasAccount>& account)
{
    MojLogTrace(m_log);

    if (account.get()) {
        m_account = account;

        // Get protocol settings from account
        m_protocolVersion = account->GetProtocolVersion();
        m_policyKey = account->GetPolicyKey();
        m_folderSyncKey = account->GetFolderSyncKey();

        if (m_state == State_GettingAccount) {
            // Determine next state based on what we have
            if (m_protocolVersion.empty()) {
                SetState(State_NeedsOptions);
            } else if (m_policyKey.empty()) {
                SetState(State_NeedsProvision);
            } else if (m_folderSyncKey.empty() || m_folderSyncKey == "0") {
                SetState(State_NeedsFolderSync);
            } else {
                SetState(State_Idle);
            }
            CheckQueue();
        }
    } else {
        MojLogError(m_log, "Failed to get account %s from database",
                    m_accountIdString.c_str());
        SetState(State_Error);
    }
}

bool EasClient::HasAccount() const
{
    return m_account.get() != NULL;
}

void EasClient::QueueCommand(MojRefCountedPtr<EasCommand> command)
{
    MojLogTrace(m_log);

    if (m_state == State_Shutdown || m_state == State_Disabled) {
        MojLogWarning(m_log, "Cannot queue command - client is %s",
                      GetStateName(m_state));
        return;
    }

    m_commandManager->QueueCommand(command, false);
    CheckQueue();
}

void EasClient::CheckQueue()
{
    MojLogTrace(m_log);

    if (m_state == State_Shutdown || m_state == State_Disabled) {
        return;
    }

    // State machine - handle protocol requirements before running commands
    switch (m_state) {
        case State_NeedsAccountInfo:
            // Need to fetch account from database
            SetState(State_GettingAccount);
            // Queue account finder command
            // GetAccountFromDatabase();
            return;

        case State_GettingAccount:
            // Waiting for account info
            return;

        case State_NeedsOptions:
            // Need to run OPTIONS command
            SetState(State_GettingOptions);
            // RunOptionsCommand();
            return;

        case State_GettingOptions:
            // Waiting for OPTIONS response
            return;

        case State_NeedsProvision:
            // Need to run Provision command
            SetState(State_Provisioning);
            // RunProvisionCommand();
            return;

        case State_Provisioning:
            // Waiting for provision response
            return;

        case State_NeedsFolderSync:
            // Need to run FolderSync command
            SetState(State_FolderSyncing);
            // RunFolderSyncCommand();
            return;

        case State_FolderSyncing:
            // Waiting for folder sync response
            return;

        case State_Idle:
        case State_Syncing:
        case State_Pinging:
            // Can run queued commands
            m_commandManager->RunCommands();
            break;

        case State_Error:
        case State_Disabled:
        case State_Shutdown:
            // Don't run commands
            break;

        default:
            break;
    }
}

void EasClient::CommandComplete(Command* command)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "Command complete: %s", command->Describe().c_str());

    // Remove from command manager
    m_commandManager->CommandComplete(command);

    // Check for more commands
    CheckQueue();
}

void EasClient::SyncAccount()
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "SyncAccount requested for %s", m_accountIdString.c_str());

    if (m_state == State_Idle) {
        SetState(State_Syncing);
        // Queue sync commands for all enabled folders
        // QueueSyncCommands();
    }
}

void EasClient::SyncFolder(const MojObject& folderId)
{
    MojLogTrace(m_log);

    MojString folderIdStr;
    folderId.stringValue(folderIdStr);
    MojLogInfo(m_log, "SyncFolder requested: %s", folderIdStr.data());

    // Queue sync command for specific folder
    // QueueFolderSyncCommand(folderId);
}

void EasClient::StartPing()
{
    MojLogTrace(m_log);

    if (m_pingActive) {
        MojLogDebug(m_log, "Ping already active");
        return;
    }

    if (m_state != State_Idle) {
        MojLogWarning(m_log, "Cannot start ping in state %s", GetStateName(m_state));
        return;
    }

    MojLogInfo(m_log, "Starting ping for account %s", m_accountIdString.c_str());
    m_pingActive = true;
    SetState(State_Pinging);
    // QueuePingCommand();
}

void EasClient::StopPing()
{
    MojLogTrace(m_log);

    if (!m_pingActive) {
        return;
    }

    MojLogInfo(m_log, "Stopping ping for account %s", m_accountIdString.c_str());
    m_pingActive = false;

    // Cancel any active ping command
    // CancelPingCommand();

    if (m_state == State_Pinging) {
        SetState(State_Idle);
    }
}

void EasClient::OptionsSuccess(float version, const std::string& serverType, bool searchSupported)
{
    MojLogInfo(m_log, "Options success: version %.1f, serverType %s, search %s",
               version, serverType.c_str(), searchSupported ? "yes" : "no");

    // Convert version to string (e.g., 12.1)
    char versionStr[16];
    snprintf(versionStr, sizeof(versionStr), "%.1f", version);
    m_protocolVersion = versionStr;

    // Save to account
    if (m_account) {
        m_account->SetProtocolVersion(m_protocolVersion);
        // SaveAccountToDatabase();
    }

    // Move to next state
    if (m_policyKey.empty()) {
        SetState(State_NeedsProvision);
    } else {
        SetState(State_NeedsFolderSync);
    }
    CheckQueue();
}

void EasClient::ProvisionSuccess(const std::string& policyKey)
{
    MojLogInfo(m_log, "Provision success: policyKey %s", policyKey.c_str());

    m_policyKey = policyKey;

    // Save to account
    if (m_account) {
        m_account->SetPolicyKey(m_policyKey);
        // SaveAccountToDatabase();
    }

    // Move to folder sync
    SetState(State_NeedsFolderSync);
    CheckQueue();
}

void EasClient::FolderSyncSuccess(const std::string& syncKey,
                                    const std::vector<EasFolder>& folders)
{
    MojLogInfo(m_log, "FolderSync success: syncKey %s, %zu folders",
               syncKey.c_str(), folders.size());

    m_folderSyncKey = syncKey;

    // Save to account
    if (m_account) {
        m_account->SetFolderSyncKey(m_folderSyncKey);
        // SaveAccountToDatabase();
    }

    // Save folders to database
    for (size_t i = 0; i < folders.size(); ++i) {
        MojLogInfo(m_log, "  Folder: %s (%s)", folders[i].GetDisplayName().c_str(),
                   folders[i].GetServerId().c_str());
        // SaveFolderToDatabase(folders[i]);
    }

    // Move to idle
    SetState(State_Idle);
    CheckQueue();
}

void EasClient::FolderSyncRequired()
{
    MojLogInfo(m_log, "FolderSync required - folder hierarchy changed");

    // Reset folder sync key to force full re-sync
    m_folderSyncKey = "0";
    if (m_account) {
        m_account->SetFolderSyncKey(m_folderSyncKey);
    }

    SetState(State_NeedsFolderSync);
    CheckQueue();
}

void EasClient::SyncComplete(const std::string& folderId,
                              const std::vector<MojObject>& adds,
                              const std::vector<MojObject>& changes,
                              const std::vector<std::string>& deletes,
                              bool moreAvailable)
{
    MojLogInfo(m_log, "Sync complete for folder %s: %zu adds, %zu changes, %zu deletes",
               folderId.c_str(), adds.size(), changes.size(), deletes.size());

    // Process added items
    for (size_t i = 0; i < adds.size(); ++i) {
        // SaveItemToDatabase(folderId, adds[i]);
    }

    // Process changed items
    for (size_t i = 0; i < changes.size(); ++i) {
        // UpdateItemInDatabase(folderId, changes[i]);
    }

    // Process deleted items
    for (size_t i = 0; i < deletes.size(); ++i) {
        MojLogInfo(m_log, "  Delete: %s", deletes[i].c_str());
        // DeleteItemFromDatabase(folderId, deletes[i]);
    }

    // If more available, queue another sync for this folder
    if (moreAvailable) {
        MojLogInfo(m_log, "More items available, queueing another sync");
        // QueueSyncCommand(folderId);
    }

    // Check if any more syncs pending
    if (m_state == State_Syncing) {
        // If no more syncs, start ping
        // SetState(State_Idle);
        // StartPing();
    }
}

void EasClient::SyncKeyInvalid(const std::string& folderId)
{
    MojLogWarning(m_log, "Sync key invalid for folder %s, need to re-sync",
                  folderId.c_str());

    // Reset sync key for this folder and re-sync
    // ResetFolderSyncKey(folderId);
    // QueueSyncCommand(folderId);
}

void EasClient::PingNoChanges()
{
    MojLogInfo(m_log, "Ping: no changes detected");

    // Restart ping if still active
    if (m_pingActive && m_state == State_Pinging) {
        // QueuePingCommand();
    }
}

void EasClient::PingChangesDetected(const std::vector<std::string>& folderIds)
{
    MojLogInfo(m_log, "Ping: changes detected in %zu folders", folderIds.size());

    for (size_t i = 0; i < folderIds.size(); ++i) {
        MojLogInfo(m_log, "  Changed folder: %s", folderIds[i].c_str());
    }

    // Stop pinging and start syncing changed folders
    m_pingActive = false;
    SetState(State_Syncing);

    for (size_t i = 0; i < folderIds.size(); ++i) {
        // QueueSyncCommand(folderIds[i]);
    }

    CheckQueue();
}

void EasClient::SetAccountError(MailError::ErrorCode errorCode, const std::string& errorText)
{
    MojLogError(m_log, "Account error: %d - %s", errorCode, errorText.c_str());

    if (m_account) {
        m_account->SetHasError(true);
        m_account->SetErrorText(errorText);
        // SaveAccountErrorToDatabase();
    }

    SetState(State_Error);
}

void EasClient::ClearAccountError()
{
    MojLogInfo(m_log, "Clearing account error");

    if (m_account) {
        m_account->SetHasError(false);
        m_account->SetErrorText("");
        // SaveAccountToDatabase();
    }

    if (m_state == State_Error) {
        SetState(State_Idle);
        CheckQueue();
    }
}
