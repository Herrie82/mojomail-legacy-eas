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

#ifndef EASACCOUNT_H_
#define EASACCOUNT_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasAccount - Exchange ActiveSync account data
 *
 * Stores account configuration including server URL, credentials,
 * sync settings, and protocol state.
 */
class EasAccount
{
public:
    EasAccount();
    virtual ~EasAccount();

    // Serialization
    MojErr FromDatabase(const MojObject& obj);
    MojErr ToDatabase(MojObject& obj) const;

    // Account identifiers
    const MojObject& GetId() const { return m_id; }
    void SetId(const MojObject& id) { m_id = id; }

    const MojObject& GetAccountId() const { return m_accountId; }
    void SetAccountId(const MojObject& id) { m_accountId = id; }

    // Server configuration
    const std::string& GetServerUrl() const { return m_serverUrl; }
    void SetServerUrl(const std::string& url) { m_serverUrl = url; }

    const std::string& GetDomain() const { return m_domain; }
    void SetDomain(const std::string& domain) { m_domain = domain; }

    const std::string& GetUsername() const { return m_username; }
    void SetUsername(const std::string& username) { m_username = username; }

    // Credentials are handled separately via credential manager

    // Protocol settings
    const std::string& GetProtocolVersion() const { return m_protocolVersion; }
    void SetProtocolVersion(const std::string& version) { m_protocolVersion = version; }

    const std::string& GetPolicyKey() const { return m_policyKey; }
    void SetPolicyKey(const std::string& key) { m_policyKey = key; }

    const std::string& GetDeviceId() const { return m_deviceId; }
    void SetDeviceId(const std::string& id) { m_deviceId = id; }

    const std::string& GetDeviceType() const { return m_deviceType; }
    void SetDeviceType(const std::string& type) { m_deviceType = type; }

    // Sync state
    const std::string& GetFolderSyncKey() const { return m_folderSyncKey; }
    void SetFolderSyncKey(const std::string& key) { m_folderSyncKey = key; }

    // Sync settings
    int GetSyncFrequency() const { return m_syncFrequency; }  // Minutes, 0 = push
    void SetSyncFrequency(int freq) { m_syncFrequency = freq; }

    int GetSyncWindow() const { return m_syncWindow; }  // Days of email to sync
    void SetSyncWindow(int window) { m_syncWindow = window; }

    // Feature flags
    bool IsCalendarEnabled() const { return m_calendarEnabled; }
    void SetCalendarEnabled(bool enabled) { m_calendarEnabled = enabled; }

    bool IsContactsEnabled() const { return m_contactsEnabled; }
    void SetContactsEnabled(bool enabled) { m_contactsEnabled = enabled; }

    bool IsTasksEnabled() const { return m_tasksEnabled; }
    void SetTasksEnabled(bool enabled) { m_tasksEnabled = enabled; }

    // State flags
    bool IsInitialSync() const { return m_initialSync; }
    void SetInitialSync(bool initial) { m_initialSync = initial; }

    bool HasError() const { return m_hasError; }
    void SetHasError(bool error) { m_hasError = error; }

    const std::string& GetErrorText() const { return m_errorText; }
    void SetErrorText(const std::string& text) { m_errorText = text; }

private:
    // Identifiers
    MojObject       m_id;
    MojObject       m_accountId;

    // Server config
    std::string     m_serverUrl;
    std::string     m_domain;
    std::string     m_username;

    // Protocol state
    std::string     m_protocolVersion;
    std::string     m_policyKey;
    std::string     m_deviceId;
    std::string     m_deviceType;
    std::string     m_folderSyncKey;

    // Sync settings
    int             m_syncFrequency;
    int             m_syncWindow;

    // Feature flags
    bool            m_calendarEnabled;
    bool            m_contactsEnabled;
    bool            m_tasksEnabled;

    // State
    bool            m_initialSync;
    bool            m_hasError;
    std::string     m_errorText;
};

#endif /* EASACCOUNT_H_ */
