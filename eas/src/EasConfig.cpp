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

#include "EasConfig.h"

EasConfig EasConfig::s_instance;

EasConfig::EasConfig()
    : m_defaultHeartbeat(480)       // 8 minutes
    , m_maxHeartbeat(3540)          // 59 minutes (max allowed by EAS)
    , m_minHeartbeat(60)            // 1 minute minimum
    , m_syncWindowDays(3)
    , m_maxSyncItems(50)
    , m_sslVerifyPeer(true)
    , m_sslVerifyHost(true)
    , m_userAgent("webOS/TouchPad")
    , m_deviceType("TouchPad")
{
}

EasConfig::~EasConfig()
{
}

EasConfig& EasConfig::GetConfig()
{
    return s_instance;
}

MojErr EasConfig::ParseConfig(const MojObject& config)
{
    bool found;

    // Heartbeat settings - MojInt64 returns bool, no found parameter
    MojInt64 intVal;
    if (config.get("defaultHeartbeat", intVal)) {
        m_defaultHeartbeat = static_cast<int>(intVal);
    }
    if (config.get("maxHeartbeat", intVal)) {
        m_maxHeartbeat = static_cast<int>(intVal);
    }
    if (config.get("minHeartbeat", intVal)) {
        m_minHeartbeat = static_cast<int>(intVal);
    }

    // Sync settings
    if (config.get("syncWindowDays", intVal)) {
        m_syncWindowDays = static_cast<int>(intVal);
    }
    if (config.get("maxSyncItems", intVal)) {
        m_maxSyncItems = static_cast<int>(intVal);
    }

    // SSL settings - bool returns bool, no found parameter
    bool boolVal;
    if (config.get("sslVerifyPeer", boolVal)) {
        m_sslVerifyPeer = boolVal;
    }
    if (config.get("sslVerifyHost", boolVal)) {
        m_sslVerifyHost = boolVal;
    }

    // Device info - MojString uses found parameter
    MojString strVal;
    if (config.get("userAgent", strVal, found) == MojErrNone && found) {
        m_userAgent = strVal.data();
    }
    if (config.get("deviceType", strVal, found) == MojErrNone && found) {
        m_deviceType = strVal.data();
    }

    return MojErrNone;
}
