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

#ifndef EASEVENTADAPTER_H_
#define EASEVENTADAPTER_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasEventAdapter - Convert EAS calendar data to/from database format
 *
 * Handles conversion between:
 * - EAS Calendar namespace (code page 0x04)
 * - webOS calendar database (com.palm.calendarevent:1)
 */
class EasEventAdapter
{
public:
    EasEventAdapter();
    virtual ~EasEventAdapter();

    // Database kind
    static const char* const EAS_EVENT_KIND;
    static const char* const SERVER_ID;

    /**
     * Parse EAS ApplicationData into calendar event database object
     */
    static void ParseFromEas(const MojObject& easData, MojObject& dbObj);

    /**
     * Serialize calendar event for EAS upload
     */
    static void SerializeToEas(const MojObject& dbObj, MojObject& easData);

private:
    /**
     * Parse recurrence rule
     */
    static void ParseRecurrence(const MojObject& easData, MojObject& rrule);

    /**
     * Parse attendees
     */
    static void ParseAttendees(const MojObject& easData, MojObject& attendees);

    /**
     * Parse reminder/alarm
     */
    static void ParseReminder(const MojObject& easData, MojObject& alarm);

    /**
     * Convert EAS date to timestamp
     */
    static int64_t ParseEasDate(const std::string& dateStr);

    /**
     * Convert timestamp to EAS date
     */
    static std::string FormatEasDate(int64_t timestamp);

    /**
     * Convert EAS busy status to webOS
     */
    static std::string ConvertBusyStatus(int easStatus);

    /**
     * Convert EAS sensitivity to webOS
     */
    static std::string ConvertSensitivity(int easSensitivity);
};

#endif /* EASEVENTADAPTER_H_ */
