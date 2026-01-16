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

#ifndef EASEMAILADAPTER_H_
#define EASEMAILADAPTER_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasEmailAdapter - Convert EAS email data to/from database format
 *
 * Handles conversion between:
 * - EAS ApplicationData (from Sync command)
 * - mojomail database objects (com.palm.email:1)
 */
class EasEmailAdapter
{
public:
    EasEmailAdapter();
    virtual ~EasEmailAdapter();

    // Database kind
    static const char* const EAS_EMAIL_KIND;

    // EAS-specific fields
    static const char* const SERVER_ID;
    static const char* const SYNC_KEY;
    static const char* const CONVERSATION_ID;

    /**
     * Parse EAS ApplicationData into email database object
     * @param easData - MojObject containing parsed EAS email fields
     * @param dbObj - Output database object
     */
    static void ParseFromEas(const MojObject& easData, MojObject& dbObj);

    /**
     * Serialize email changes for EAS upload (Change command)
     * @param dbObj - Database email object
     * @param easData - Output for EAS fields that changed
     */
    static void SerializeToEas(const MojObject& dbObj, MojObject& easData);

    /**
     * Parse email flags from EAS data
     * @param easData - EAS data
     * @param flags - Output flags object
     */
    static void ParseFlags(const MojObject& easData, MojObject& flags);

    /**
     * Convert flag changes for EAS upload
     * @param flags - Flags to sync
     * @param easData - Output EAS data
     */
    static void SerializeFlags(const MojObject& flags, MojObject& easData);

private:
    /**
     * Parse EAS address format (RFC 2822) into address object
     */
    static void ParseAddress(const std::string& address, MojObject& addrObj);

    /**
     * Parse multiple addresses from comma-separated string
     */
    static void ParseAddressList(const std::string& addresses, MojObject& addrArray);

    /**
     * Convert EAS date string to Unix timestamp
     * EAS dates are in format: 2024-01-15T10:30:00.000Z
     */
    static int64_t ParseEasDate(const std::string& dateStr);

    /**
     * Convert Unix timestamp to EAS date string
     */
    static std::string FormatEasDate(int64_t timestamp);
};

#endif /* EASEMAILADAPTER_H_ */
