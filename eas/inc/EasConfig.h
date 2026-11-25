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

#ifndef EASCONFIG_H_
#define EASCONFIG_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasConfig - Configuration settings for EAS service
 *
 * Singleton class that holds configuration from /etc/palm/eas.conf
 */
class EasConfig
{
public:
    static EasConfig& GetConfig();

    MojErr ParseConfig(const MojObject& config);

    // Configuration values
    int GetDefaultHeartbeat() const { return m_defaultHeartbeat; }
    int GetMaxHeartbeat() const { return m_maxHeartbeat; }
    int GetMinHeartbeat() const { return m_minHeartbeat; }
    int GetSyncWindowDays() const { return m_syncWindowDays; }
    int GetMaxSyncItems() const { return m_maxSyncItems; }
    bool GetSslVerifyPeer() const { return m_sslVerifyPeer; }
    bool GetSslVerifyHost() const { return m_sslVerifyHost; }
    const std::string& GetUserAgent() const { return m_userAgent; }
    const std::string& GetDeviceType() const { return m_deviceType; }

private:
    EasConfig();
    ~EasConfig();

    // Ping/Push settings
    int         m_defaultHeartbeat;     // Default heartbeat interval (seconds)
    int         m_maxHeartbeat;         // Maximum heartbeat interval
    int         m_minHeartbeat;         // Minimum heartbeat interval

    // Sync settings
    int         m_syncWindowDays;       // Default sync window (days)
    int         m_maxSyncItems;         // Max items per sync response

    // SSL settings
    bool        m_sslVerifyPeer;
    bool        m_sslVerifyHost;

    // Device info
    std::string m_userAgent;
    std::string m_deviceType;

    static EasConfig s_instance;
};

#endif /* EASCONFIG_H_ */
