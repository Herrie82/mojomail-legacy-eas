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

#include "data/EasPolicy.h"

EasPolicy::EasPolicy()
    : m_devicePasswordEnabled(false)
    , m_alphanumericPasswordRequired(false)
    , m_minPasswordLength(0)
    , m_minPasswordComplexChars(0)
    , m_maxPasswordFailedAttempts(0)
    , m_maxInactivityTimeout(0)
    , m_passwordExpiration(0)
    , m_passwordHistory(0)
    , m_allowSimplePassword(true)
    , m_deviceEncryptionEnabled(false)
    , m_storageCardEncryptionRequired(false)
    , m_attachmentsEnabled(true)
    , m_maxAttachmentSize(0)
    , m_allowCamera(true)
    , m_allowBluetooth(true)
    , m_allowWiFi(true)
    , m_allowBrowser(true)
{
}

EasPolicy::~EasPolicy()
{
}

MojErr EasPolicy::FromDatabase(const MojObject& obj)
{
    bool found;

    // Password settings
    if (obj.get("devicePasswordEnabled", m_devicePasswordEnabled)) {}
    if (obj.get("alphanumericPasswordRequired", m_alphanumericPasswordRequired)) {}

    MojInt64 tempInt;
    if (obj.get("minPasswordLength", tempInt)) {
        m_minPasswordLength = static_cast<int>(tempInt);
    }
    if (obj.get("minPasswordComplexChars", tempInt)) {
        m_minPasswordComplexChars = static_cast<int>(tempInt);
    }
    if (obj.get("maxPasswordFailedAttempts", tempInt)) {
        m_maxPasswordFailedAttempts = static_cast<int>(tempInt);
    }
    if (obj.get("maxInactivityTimeout", tempInt)) {
        m_maxInactivityTimeout = static_cast<int>(tempInt);
    }
    if (obj.get("passwordExpiration", tempInt)) {
        m_passwordExpiration = static_cast<int>(tempInt);
    }
    if (obj.get("passwordHistory", tempInt)) {
        m_passwordHistory = static_cast<int>(tempInt);
    }
    if (obj.get("allowSimplePassword", m_allowSimplePassword)) {}

    // Encryption settings
    if (obj.get("deviceEncryptionEnabled", m_deviceEncryptionEnabled)) {}
    if (obj.get("storageCardEncryptionRequired", m_storageCardEncryptionRequired)) {}

    // Feature settings
    if (obj.get("attachmentsEnabled", m_attachmentsEnabled)) {}
    if (obj.get("maxAttachmentSize", tempInt)) {
        m_maxAttachmentSize = static_cast<int>(tempInt);
    }
    if (obj.get("allowCamera", m_allowCamera)) {}
    if (obj.get("allowBluetooth", m_allowBluetooth)) {}
    if (obj.get("allowWiFi", m_allowWiFi)) {}
    if (obj.get("allowBrowser", m_allowBrowser)) {}

    return MojErrNone;
}

MojErr EasPolicy::ToDatabase(MojObject& obj) const
{
    MojErr err;

    // Password settings
    err = obj.put("devicePasswordEnabled", m_devicePasswordEnabled);
    if (err != MojErrNone) return err;

    err = obj.put("alphanumericPasswordRequired", m_alphanumericPasswordRequired);
    if (err != MojErrNone) return err;

    err = obj.put("minPasswordLength", static_cast<MojInt64>(m_minPasswordLength));
    if (err != MojErrNone) return err;

    err = obj.put("minPasswordComplexChars", static_cast<MojInt64>(m_minPasswordComplexChars));
    if (err != MojErrNone) return err;

    err = obj.put("maxPasswordFailedAttempts", static_cast<MojInt64>(m_maxPasswordFailedAttempts));
    if (err != MojErrNone) return err;

    err = obj.put("maxInactivityTimeout", static_cast<MojInt64>(m_maxInactivityTimeout));
    if (err != MojErrNone) return err;

    err = obj.put("passwordExpiration", static_cast<MojInt64>(m_passwordExpiration));
    if (err != MojErrNone) return err;

    err = obj.put("passwordHistory", static_cast<MojInt64>(m_passwordHistory));
    if (err != MojErrNone) return err;

    err = obj.put("allowSimplePassword", m_allowSimplePassword);
    if (err != MojErrNone) return err;

    // Encryption settings
    err = obj.put("deviceEncryptionEnabled", m_deviceEncryptionEnabled);
    if (err != MojErrNone) return err;

    err = obj.put("storageCardEncryptionRequired", m_storageCardEncryptionRequired);
    if (err != MojErrNone) return err;

    // Feature settings
    err = obj.put("attachmentsEnabled", m_attachmentsEnabled);
    if (err != MojErrNone) return err;

    err = obj.put("maxAttachmentSize", static_cast<MojInt64>(m_maxAttachmentSize));
    if (err != MojErrNone) return err;

    err = obj.put("allowCamera", m_allowCamera);
    if (err != MojErrNone) return err;

    err = obj.put("allowBluetooth", m_allowBluetooth);
    if (err != MojErrNone) return err;

    err = obj.put("allowWiFi", m_allowWiFi);
    if (err != MojErrNone) return err;

    err = obj.put("allowBrowser", m_allowBrowser);
    if (err != MojErrNone) return err;

    return MojErrNone;
}
