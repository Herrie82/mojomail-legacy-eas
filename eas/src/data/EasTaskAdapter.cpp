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

#include "data/EasTaskAdapter.h"
#include <ctime>
#include <cstring>

const char* const EasTaskAdapter::EAS_TASK_KIND = "com.palm.task:1";
const char* const EasTaskAdapter::SERVER_ID = "easServerId";

EasTaskAdapter::EasTaskAdapter()
{
}

EasTaskAdapter::~EasTaskAdapter()
{
}

void EasTaskAdapter::ParseFromEas(const MojObject& easData, MojObject& dbObj)
{
    bool found;

    // Set kind
    dbObj.putString("_kind", EAS_TASK_KIND);

    // Server ID
    MojString serverId;
    if (easData.get("serverId", serverId, found) == MojErrNone && found) {
        dbObj.putString(SERVER_ID, serverId.data());
    }

    // Subject/title
    MojString subject;
    if (easData.get("subject", subject, found) == MojErrNone && found) {
        dbObj.putString("subject", subject.data());
    }

    // Body/notes
    MojString body;
    if (easData.get("body", body, found) == MojErrNone && found) {
        dbObj.putString("note", body.data());
    }

    // Due date
    MojString dueDate;
    if (easData.get("dueDate", dueDate, found) == MojErrNone && found) {
        MojInt64 ts = ParseEasDate(dueDate.data());
        dbObj.putInt("dueDate", ts);
    }

    // Start date
    MojString startDate;
    if (easData.get("startDate", startDate, found) == MojErrNone && found) {
        MojInt64 ts = ParseEasDate(startDate.data());
        dbObj.putInt("startDate", ts);
    }

    // Date completed
    MojString completedDate;
    if (easData.get("dateCompleted", completedDate, found) == MojErrNone && found) {
        MojInt64 ts = ParseEasDate(completedDate.data());
        dbObj.putInt("completedDate", ts);
    }

    // Importance (0=low, 1=normal, 2=high)
    MojInt64 importance = 1;
    if (easData.get("importance", importance)) {
        dbObj.putString("priority", ConvertImportance(importance).c_str());
    }

    // Complete status
    bool complete = false;
    if (easData.get("complete", complete)) {
        dbObj.putBool("completed", complete);
    }

    // Sensitivity (0=normal, 1=personal, 2=private, 3=confidential)
    MojInt64 sensitivity = 0;
    if (easData.get("sensitivity", sensitivity)) {
        const char* privacy = "public";
        switch (sensitivity) {
            case 1: privacy = "personal"; break;
            case 2: privacy = "private"; break;
            case 3: privacy = "confidential"; break;
        }
        dbObj.putString("privacy", privacy);
    }

    // Categories
    MojObject categories;
    if (easData.get("categories", categories)) {
        dbObj.put("categories", categories);
    }

    // Recurrence
    MojObject recurrence;
    if (easData.get("recurrence", recurrence)) {
        // Tasks can have recurrence too - similar to events
        // For now, just store the raw data
        dbObj.put("recurrence", recurrence);
    }

    // Reminder
    MojString reminderTime;
    if (easData.get("reminderTime", reminderTime, found) == MojErrNone && found) {
        MojInt64 ts = ParseEasDate(reminderTime.data());
        dbObj.putInt("reminderTime", ts);

        bool reminderSet = false;
        if (easData.get("reminderSet", reminderSet)) {
            dbObj.putBool("reminderSet", reminderSet);
        }
    }
}

void EasTaskAdapter::SerializeToEas(const MojObject& dbObj, MojObject& easData)
{
    bool found;

    // Server ID
    MojString serverId;
    if (dbObj.get(SERVER_ID, serverId, found) == MojErrNone && found) {
        easData.putString("serverId", serverId.data());
    }

    // Subject
    MojString subject;
    if (dbObj.get("subject", subject, found) == MojErrNone && found) {
        easData.putString("subject", subject.data());
    }

    // Notes
    MojString note;
    if (dbObj.get("note", note, found) == MojErrNone && found) {
        easData.putString("body", note.data());
    }

    // Due date
    MojInt64 dueDate = 0;
    if (dbObj.get("dueDate", dueDate) && dueDate > 0) {
        easData.putString("dueDate", FormatEasDate(dueDate).c_str());
    }

    // Start date
    MojInt64 startDate = 0;
    if (dbObj.get("startDate", startDate) && startDate > 0) {
        easData.putString("startDate", FormatEasDate(startDate).c_str());
    }

    // Completed date
    MojInt64 completedDate = 0;
    if (dbObj.get("completedDate", completedDate) && completedDate > 0) {
        easData.putString("dateCompleted", FormatEasDate(completedDate).c_str());
    }

    // Priority
    MojString priority;
    if (dbObj.get("priority", priority, found) == MojErrNone && found) {
        easData.putInt("importance", ConvertPriority(priority.data()));
    }

    // Completed
    bool completed = false;
    if (dbObj.get("completed", completed)) {
        easData.putBool("complete", completed);
    }

    // Categories
    MojObject categories;
    if (dbObj.get("categories", categories)) {
        easData.put("categories", categories);
    }
}

MojInt64 EasTaskAdapter::ParseEasDate(const std::string& dateStr)
{
    struct tm tm = {};
    int msec = 0;

    int parsed = sscanf(dateStr.c_str(), "%d-%d-%dT%d:%d:%d.%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &msec);

    if (parsed < 6) {
        parsed = sscanf(dateStr.c_str(), "%d-%d-%dT%d:%d:%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    }

    // Also try date-only format
    if (parsed < 3) {
        parsed = sscanf(dateStr.c_str(), "%d-%d-%d",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
        if (parsed == 3) {
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
        }
    }

    if (parsed >= 3) {
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        time_t timestamp = timegm(&tm);
        return static_cast<MojInt64>(timestamp) * 1000 + msec;
    }

    return 0;
}

std::string EasTaskAdapter::FormatEasDate(MojInt64 timestamp)
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

std::string EasTaskAdapter::ConvertImportance(int importance)
{
    switch (importance) {
        case 0: return "low";
        case 2: return "high";
        default: return "normal";
    }
}

int EasTaskAdapter::ConvertPriority(const std::string& priority)
{
    if (priority == "low") return 0;
    if (priority == "high") return 2;
    return 1;  // normal
}
