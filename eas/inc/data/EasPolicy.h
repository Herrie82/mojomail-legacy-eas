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

#ifndef EASPOLICY_H_
#define EASPOLICY_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasPolicy - Exchange security policy settings
 *
 * Contains security requirements from the server including
 * password policies, encryption requirements, etc.
 *
 * Reference: [MS-ASPROV] Section 2.2.2.41 (EASProvisionDoc)
 */
class EasPolicy
{
public:
    EasPolicy();
    virtual ~EasPolicy();

    // Serialization
    MojErr FromDatabase(const MojObject& obj);
    MojErr ToDatabase(MojObject& obj) const;

    // Password policies
    bool IsDevicePasswordEnabled() const { return m_devicePasswordEnabled; }
    void SetDevicePasswordEnabled(bool enabled) { m_devicePasswordEnabled = enabled; }

    bool IsAlphanumericPasswordRequired() const { return m_alphanumericPasswordRequired; }
    void SetAlphanumericPasswordRequired(bool required) { m_alphanumericPasswordRequired = required; }

    int GetMinPasswordLength() const { return m_minPasswordLength; }
    void SetMinPasswordLength(int length) { m_minPasswordLength = length; }

    int GetMinPasswordComplexChars() const { return m_minPasswordComplexChars; }
    void SetMinPasswordComplexChars(int count) { m_minPasswordComplexChars = count; }

    int GetMaxPasswordFailedAttempts() const { return m_maxPasswordFailedAttempts; }
    void SetMaxPasswordFailedAttempts(int attempts) { m_maxPasswordFailedAttempts = attempts; }

    int GetMaxInactivityTimeout() const { return m_maxInactivityTimeout; }
    void SetMaxInactivityTimeout(int seconds) { m_maxInactivityTimeout = seconds; }

    int GetPasswordExpiration() const { return m_passwordExpiration; }
    void SetPasswordExpiration(int days) { m_passwordExpiration = days; }

    int GetPasswordHistory() const { return m_passwordHistory; }
    void SetPasswordHistory(int count) { m_passwordHistory = count; }

    // Encryption policies
    bool IsDeviceEncryptionEnabled() const { return m_deviceEncryptionEnabled; }
    void SetDeviceEncryptionEnabled(bool enabled) { m_deviceEncryptionEnabled = enabled; }

    bool IsStorageCardEncryptionRequired() const { return m_storageCardEncryptionRequired; }
    void SetStorageCardEncryptionRequired(bool required) { m_storageCardEncryptionRequired = required; }

    // Feature restrictions
    bool IsAttachmentsEnabled() const { return m_attachmentsEnabled; }
    void SetAttachmentsEnabled(bool enabled) { m_attachmentsEnabled = enabled; }

    int GetMaxAttachmentSize() const { return m_maxAttachmentSize; }
    void SetMaxAttachmentSize(int size) { m_maxAttachmentSize = size; }

    bool AllowSimplePassword() const { return m_allowSimplePassword; }
    void SetAllowSimplePassword(bool allow) { m_allowSimplePassword = allow; }

    bool AllowCamera() const { return m_allowCamera; }
    void SetAllowCamera(bool allow) { m_allowCamera = allow; }

    bool AllowBluetooth() const { return m_allowBluetooth; }
    void SetAllowBluetooth(bool allow) { m_allowBluetooth = allow; }

    bool AllowWiFi() const { return m_allowWiFi; }
    void SetAllowWiFi(bool allow) { m_allowWiFi = allow; }

    bool AllowBrowser() const { return m_allowBrowser; }
    void SetAllowBrowser(bool allow) { m_allowBrowser = allow; }

private:
    // Password
    bool    m_devicePasswordEnabled;
    bool    m_alphanumericPasswordRequired;
    int     m_minPasswordLength;
    int     m_minPasswordComplexChars;
    int     m_maxPasswordFailedAttempts;
    int     m_maxInactivityTimeout;
    int     m_passwordExpiration;
    int     m_passwordHistory;
    bool    m_allowSimplePassword;

    // Encryption
    bool    m_deviceEncryptionEnabled;
    bool    m_storageCardEncryptionRequired;

    // Features
    bool    m_attachmentsEnabled;
    int     m_maxAttachmentSize;
    bool    m_allowCamera;
    bool    m_allowBluetooth;
    bool    m_allowWiFi;
    bool    m_allowBrowser;
};

#endif /* EASPOLICY_H_ */
