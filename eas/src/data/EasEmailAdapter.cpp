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

#include "data/EasEmailAdapter.h"
#include <ctime>
#include <cstring>
#include <sstream>

const char* const EasEmailAdapter::EAS_EMAIL_KIND = "com.palm.email:1";
const char* const EasEmailAdapter::SERVER_ID = "easServerId";
const char* const EasEmailAdapter::SYNC_KEY = "easSyncKey";
const char* const EasEmailAdapter::CONVERSATION_ID = "easConversationId";

EasEmailAdapter::EasEmailAdapter()
{
}

EasEmailAdapter::~EasEmailAdapter()
{
}

void EasEmailAdapter::ParseFromEas(const MojObject& easData, MojObject& dbObj)
{
    MojErr err;
    bool found;

    // Set kind
    err = dbObj.putString("_kind", EAS_EMAIL_KIND);

    // Server ID (required for tracking)
    MojString serverId;
    if (easData.get("serverId", serverId, found) == MojErrNone && found) {
        dbObj.putString(SERVER_ID, serverId.data());
    }

    // Subject
    MojString subject;
    if (easData.get("subject", subject, found) == MojErrNone && found) {
        dbObj.putString("subject", subject.data());
    }

    // Date received
    MojString dateStr;
    if (easData.get("dateReceived", dateStr, found) == MojErrNone && found) {
        int64_t timestamp = ParseEasDate(dateStr.data());
        dbObj.putInt("timestamp", timestamp);
    }

    // From address
    MojString from;
    if (easData.get("from", from, found) == MojErrNone && found) {
        MojObject fromObj;
        ParseAddress(from.data(), fromObj);
        dbObj.put("from", fromObj);
    }

    // To addresses
    MojString to;
    if (easData.get("to", to, found) == MojErrNone && found) {
        MojObject toArray;
        ParseAddressList(to.data(), toArray);
        dbObj.put("to", toArray);
    }

    // CC addresses
    MojString cc;
    if (easData.get("cc", cc, found) == MojErrNone && found) {
        MojObject ccArray;
        ParseAddressList(cc.data(), ccArray);
        dbObj.put("cc", ccArray);
    }

    // Read flag - bool get returns bool directly
    bool read = false;
    if (easData.get("read", read)) {
        MojObject flags;
        flags.putBool("read", read);
        dbObj.put("flags", flags);
    }

    // Importance (0=low, 1=normal, 2=high) - int64 get returns bool directly
    MojInt64 importance = 1;
    if (easData.get("importance", importance)) {
        MojString priority;
        switch (importance) {
            case 0: priority.assign("low"); break;
            case 2: priority.assign("high"); break;
            default: priority.assign("normal"); break;
        }
        dbObj.putString("priority", priority.data());
    }

    // Body content
    MojString body;
    if (easData.get("body", body, found) == MojErrNone && found) {
        MojObject parts;
        MojObject part;

        MojInt64 bodyType = 1;
        easData.get("bodyType", bodyType);
        
        switch (bodyType) {
            case 2:  // HTML
                part.putString("type", "text/html");
                break;
            case 4:  // MIME
                part.putString("type", "message/rfc822");
                break;
            default: // Plain text
                part.putString("type", "text/plain");
                break;
        }
        part.putString("content", body.data());
        
        parts.push(part);
        dbObj.put("parts", parts);
    }

    // Truncated flag - bool get returns bool directly
    bool truncated = false;
    if (easData.get("truncated", truncated)) {
        dbObj.putBool("bodyTruncated", truncated);
    }

    // Message class (for meeting requests, etc.)
    MojString msgClass;
    if (easData.get("messageClass", msgClass, found) == MojErrNone && found) {
        dbObj.putString("messageClass", msgClass.data());
    }
}

void EasEmailAdapter::SerializeToEas(const MojObject& dbObj, MojObject& easData)
{
    MojErr err;
    bool found;

    // Server ID
    MojString serverId;
    if (dbObj.get(SERVER_ID, serverId, found) == MojErrNone && found) {
        easData.putString("serverId", serverId.data());
    }

    // Flags (read status) - MojObject get returns bool directly
    MojObject flags;
    if (dbObj.get("flags", flags)) {
        SerializeFlags(flags, easData);
    }
}

void EasEmailAdapter::ParseFlags(const MojObject& easData, MojObject& flags)
{
    // Read flag - bool get returns bool directly
    bool read = false;
    if (easData.get("read", read)) {
        flags.putBool("read", read);
    }

    // Flagged (starred) - EAS uses Flag element
    // For simplicity, check for flagStatus - int64 get returns bool directly
    MojInt64 flagStatus = 0;
    if (easData.get("flagStatus", flagStatus)) {
        flags.putBool("flagged", flagStatus == 2);  // 2 = flagged
    }
}

void EasEmailAdapter::SerializeFlags(const MojObject& flags, MojObject& easData)
{
    // Read flag - bool get returns bool directly
    bool read = false;
    if (flags.get("read", read)) {
        easData.putBool("read", read);
    }

    // Flagged status - bool get returns bool directly
    bool flagged = false;
    if (flags.get("flagged", flagged)) {
        easData.putInt("flagStatus", flagged ? 2 : 0);
    }
}

void EasEmailAdapter::ParseAddress(const std::string& address, MojObject& addrObj)
{
    // Parse RFC 2822 address format: "Display Name" <email@domain.com>
    // or just: email@domain.com

    std::string displayName;
    std::string email;

    size_t ltPos = address.find('<');
    size_t gtPos = address.find('>');

    if (ltPos != std::string::npos && gtPos != std::string::npos && gtPos > ltPos) {
        // Has display name
        email = address.substr(ltPos + 1, gtPos - ltPos - 1);
        displayName = address.substr(0, ltPos);
        
        // Trim whitespace and quotes from display name
        size_t start = displayName.find_first_not_of(" \t\"");
        size_t end = displayName.find_last_not_of(" \t\"");
        if (start != std::string::npos && end != std::string::npos) {
            displayName = displayName.substr(start, end - start + 1);
        }
    } else {
        // Just email
        email = address;
        // Trim whitespace
        size_t start = email.find_first_not_of(" \t");
        size_t end = email.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            email = email.substr(start, end - start + 1);
        }
    }

    addrObj.putString("addr", email.c_str());
    if (!displayName.empty()) {
        addrObj.putString("name", displayName.c_str());
    }
}

void EasEmailAdapter::ParseAddressList(const std::string& addresses, MojObject& addrArray)
{
    // Split by comma, handling quoted strings
    std::string current;
    bool inQuotes = false;
    bool inAngleBrackets = false;

    for (size_t i = 0; i < addresses.length(); i++) {
        char c = addresses[i];

        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (c == '<') {
            inAngleBrackets = true;
            current += c;
        } else if (c == '>') {
            inAngleBrackets = false;
            current += c;
        } else if (c == ',' && !inQuotes && !inAngleBrackets) {
            // End of address
            if (!current.empty()) {
                MojObject addrObj;
                ParseAddress(current, addrObj);
                addrArray.push(addrObj);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    // Add last address
    if (!current.empty()) {
        MojObject addrObj;
        ParseAddress(current, addrObj);
        addrArray.push(addrObj);
    }
}

int64_t EasEmailAdapter::ParseEasDate(const std::string& dateStr)
{
    // Parse EAS date format: 2024-01-15T10:30:00.000Z
    struct tm tm = {};
    int msec = 0;

    // Try full format with milliseconds
    int parsed = sscanf(dateStr.c_str(), "%d-%d-%dT%d:%d:%d.%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &msec);

    if (parsed < 6) {
        // Try without milliseconds
        parsed = sscanf(dateStr.c_str(), "%d-%d-%dT%d:%d:%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    }

    if (parsed >= 6) {
        tm.tm_year -= 1900;  // years since 1900
        tm.tm_mon -= 1;      // 0-based month

        // Convert to timestamp (UTC)
        time_t timestamp = timegm(&tm);
        return static_cast<int64_t>(timestamp) * 1000 + msec;
    }

    return 0;
}

std::string EasEmailAdapter::FormatEasDate(int64_t timestamp)
{
    time_t secs = timestamp / 1000;
    int msec = timestamp % 1000;

    struct tm tm;
    gmtime_r(&secs, &tm);

    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, msec);

    return buf;
}
