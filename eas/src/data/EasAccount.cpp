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

#include "data/EasAccount.h"

EasAccount::EasAccount()
    : m_syncFrequency(0)
    , m_syncWindow(3)  // 3 days default
    , m_calendarEnabled(true)
    , m_contactsEnabled(true)
    , m_tasksEnabled(true)
    , m_initialSync(true)
    , m_hasError(false)
{
}

EasAccount::~EasAccount()
{
}

MojErr EasAccount::FromDatabase(const MojObject& obj)
{
    bool found;

    // Get _id - returns bool, not MojErr
    obj.get("_id", m_id);

    // Get accountId - returns bool, not MojErr
    obj.get("accountId", m_accountId);

    // Server config
    MojString strVal;

    if (obj.get("serverUrl", strVal, found) == MojErrNone && found) {
        m_serverUrl = strVal.data();
    }
    if (obj.get("domain", strVal, found) == MojErrNone && found) {
        m_domain = strVal.data();
    }
    if (obj.get("username", strVal, found) == MojErrNone && found) {
        m_username = strVal.data();
    }

    // Protocol state
    if (obj.get("protocolVersion", strVal, found) == MojErrNone && found) {
        m_protocolVersion = strVal.data();
    }
    if (obj.get("policyKey", strVal, found) == MojErrNone && found) {
        m_policyKey = strVal.data();
    }
    if (obj.get("deviceId", strVal, found) == MojErrNone && found) {
        m_deviceId = strVal.data();
    }
    if (obj.get("deviceType", strVal, found) == MojErrNone && found) {
        m_deviceType = strVal.data();
    }
    if (obj.get("folderSyncKey", strVal, found) == MojErrNone && found) {
        m_folderSyncKey = strVal.data();
    }

    // Sync settings - get for MojInt64 returns bool directly
    MojInt64 intVal;
    if (obj.get("syncFrequencyMins", intVal)) {
        m_syncFrequency = static_cast<int>(intVal);
    }
    if (obj.get("syncWindowDays", intVal)) {
        m_syncWindow = static_cast<int>(intVal);
    }

    // Feature flags - get for bool returns bool directly, no found param
    bool boolVal;
    if (obj.get("calendarEnabled", boolVal)) {
        m_calendarEnabled = boolVal;
    }
    if (obj.get("contactsEnabled", boolVal)) {
        m_contactsEnabled = boolVal;
    }
    if (obj.get("tasksEnabled", boolVal)) {
        m_tasksEnabled = boolVal;
    }

    // State
    if (obj.get("initialSync", boolVal)) {
        m_initialSync = boolVal;
    }
    if (obj.get("hasError", boolVal)) {
        m_hasError = boolVal;
    }
    if (obj.get("errorText", strVal, found) == MojErrNone && found) {
        m_errorText = strVal.data();
    }

    return MojErrNone;
}

MojErr EasAccount::ToDatabase(MojObject& obj) const
{
    MojErr err;

    // _kind for db8
    err = obj.putString("_kind", "com.palm.eas.account:1");
    MojErrCheck(err);

    // IDs
    if (!m_id.undefined()) {
        err = obj.put("_id", m_id);
        MojErrCheck(err);
    }
    if (!m_accountId.undefined()) {
        err = obj.put("accountId", m_accountId);
        MojErrCheck(err);
    }

    // Server config
    err = obj.putString("serverUrl", m_serverUrl.c_str());
    MojErrCheck(err);
    err = obj.putString("domain", m_domain.c_str());
    MojErrCheck(err);
    err = obj.putString("username", m_username.c_str());
    MojErrCheck(err);

    // Protocol state
    err = obj.putString("protocolVersion", m_protocolVersion.c_str());
    MojErrCheck(err);
    err = obj.putString("policyKey", m_policyKey.c_str());
    MojErrCheck(err);
    err = obj.putString("deviceId", m_deviceId.c_str());
    MojErrCheck(err);
    err = obj.putString("deviceType", m_deviceType.c_str());
    MojErrCheck(err);
    err = obj.putString("folderSyncKey", m_folderSyncKey.c_str());
    MojErrCheck(err);

    // Sync settings
    err = obj.putInt("syncFrequencyMins", m_syncFrequency);
    MojErrCheck(err);
    err = obj.putInt("syncWindowDays", m_syncWindow);
    MojErrCheck(err);

    // Feature flags
    err = obj.putBool("calendarEnabled", m_calendarEnabled);
    MojErrCheck(err);
    err = obj.putBool("contactsEnabled", m_contactsEnabled);
    MojErrCheck(err);
    err = obj.putBool("tasksEnabled", m_tasksEnabled);
    MojErrCheck(err);

    // State
    err = obj.putBool("initialSync", m_initialSync);
    MojErrCheck(err);
    err = obj.putBool("hasError", m_hasError);
    MojErrCheck(err);
    err = obj.putString("errorText", m_errorText.c_str());
    MojErrCheck(err);

    return MojErrNone;
}
