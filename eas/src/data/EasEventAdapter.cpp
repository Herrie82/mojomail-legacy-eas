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

#include "data/EasEventAdapter.h"
#include <ctime>
#include <cstring>

const char* const EasEventAdapter::EAS_EVENT_KIND = "com.palm.calendarevent:1";
const char* const EasEventAdapter::SERVER_ID = "easServerId";

EasEventAdapter::EasEventAdapter()
{
}

EasEventAdapter::~EasEventAdapter()
{
}

void EasEventAdapter::ParseFromEas(const MojObject& easData, MojObject& dbObj)
{
    bool found;

    // Set kind
    dbObj.putString("_kind", EAS_EVENT_KIND);

    // Server ID
    MojString serverId;
    if (easData.get("serverId", serverId, found) == MojErrNone && found) {
        dbObj.putString(SERVER_ID, serverId.data());
    }

    // Subject/summary
    MojString subject;
    if (easData.get("subject", subject, found) == MojErrNone && found) {
        dbObj.putString("subject", subject.data());
    }

    // Location
    MojString location;
    if (easData.get("location", location, found) == MojErrNone && found) {
        dbObj.putString("location", location.data());
    }

    // Start time
    MojString startTime;
    if (easData.get("startTime", startTime, found) == MojErrNone && found) {
        int64_t ts = ParseEasDate(startTime.data());
        dbObj.putInt("dtstart", ts);
    }

    // End time
    MojString endTime;
    if (easData.get("endTime", endTime, found) == MojErrNone && found) {
        int64_t ts = ParseEasDate(endTime.data());
        dbObj.putInt("dtend", ts);
    }

    // All day event
    bool allDay = false;
    if (easData.get("allDayEvent", allDay)) {
        dbObj.putBool("allDay", allDay);
    }

    // Busy status (0=free, 1=tentative, 2=busy, 3=outOfOffice)
    MojInt64 busyStatus = 2;
    if (easData.get("busyStatus", busyStatus)) {
        dbObj.putString("showTime", ConvertBusyStatus(busyStatus).c_str());
    }

    // Sensitivity (0=normal, 1=personal, 2=private, 3=confidential)
    MojInt64 sensitivity = 0;
    if (easData.get("sensitivity", sensitivity)) {
        dbObj.putString("privacy", ConvertSensitivity(sensitivity).c_str());
    }

    // Description/notes
    MojString body;
    if (easData.get("body", body, found) == MojErrNone && found) {
        dbObj.putString("note", body.data());
    }

    // Organizer
    MojString organizerEmail, organizerName;
    if (easData.get("organizerEmail", organizerEmail, found) == MojErrNone && found) {
        MojObject organizer;
        organizer.putString("email", organizerEmail.data());
        if (easData.get("organizerName", organizerName, found) == MojErrNone && found) {
            organizer.putString("commonName", organizerName.data());
        }
        dbObj.put("organizer", organizer);
    }

    // UID (for iCal compatibility)
    MojString uid;
    if (easData.get("uid", uid, found) == MojErrNone && found) {
        dbObj.putString("icalUid", uid.data());
    }

    // Reminder (in minutes before event)
    MojInt64 reminder = 0;
    if (easData.get("reminder", reminder) && reminder > 0) {
        MojObject alarm;
        alarm.putString("action", "display");
        alarm.putInt("alarmTrigger", -reminder * 60);  // Convert to seconds
        MojObject alarms;
        alarms.push(alarm);
        dbObj.put("alarm", alarms);
    }

    // Recurrence
    MojObject recurrence;
    if (easData.get("recurrence", recurrence)) {
        MojObject rrule;
        ParseRecurrence(recurrence, rrule);
        if (!rrule.empty()) {
            dbObj.put("rrule", rrule);
        }
    }

    // Attendees
    MojObject attendees;
    if (easData.get("attendees", attendees)) {
        MojObject parsedAttendees;
        ParseAttendees(attendees, parsedAttendees);
        if (parsedAttendees.size() > 0) {
            dbObj.put("attendees", parsedAttendees);
        }
    }

    // Categories
    MojObject categories;
    if (easData.get("categories", categories)) {
        dbObj.put("categories", categories);
    }
}

void EasEventAdapter::SerializeToEas(const MojObject& dbObj, MojObject& easData)
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

    // Location
    MojString location;
    if (dbObj.get("location", location, found) == MojErrNone && found) {
        easData.putString("location", location.data());
    }

    // Start time
    MojInt64 dtstart = 0;
    if (dbObj.get("dtstart", dtstart)) {
        easData.putString("startTime", FormatEasDate(dtstart).c_str());
    }

    // End time
    MojInt64 dtend = 0;
    if (dbObj.get("dtend", dtend)) {
        easData.putString("endTime", FormatEasDate(dtend).c_str());
    }

    // All day
    bool allDay = false;
    if (dbObj.get("allDay", allDay)) {
        easData.putBool("allDayEvent", allDay);
    }

    // Description
    MojString note;
    if (dbObj.get("note", note, found) == MojErrNone && found) {
        easData.putString("body", note.data());
    }
}

void EasEventAdapter::ParseRecurrence(const MojObject& easData, MojObject& rrule)
{
    bool found;

    // Recurrence type (0=daily, 1=weekly, 2=monthly, 3=monthlyNth, 5=yearly, 6=yearlyNth)
    MojInt64 type = 0;
    if (!easData.get("type", type)) {
        return;
    }

    switch (type) {
        case 0:
            rrule.putString("freq", "DAILY");
            break;
        case 1:
            rrule.putString("freq", "WEEKLY");
            break;
        case 2:
        case 3:
            rrule.putString("freq", "MONTHLY");
            break;
        case 5:
        case 6:
            rrule.putString("freq", "YEARLY");
            break;
        default:
            return;
    }

    // Interval
    MojInt64 interval = 1;
    if (easData.get("interval", interval) && interval > 1) {
        rrule.putInt("interval", interval);
    }

    // Occurrences
    MojInt64 occurrences = 0;
    if (easData.get("occurrences", occurrences) && occurrences > 0) {
        rrule.putInt("count", occurrences);
    }

    // Until date - MojString uses found parameter
    MojString until;
    bool foundUntil = false;
    if (easData.get("until", until, foundUntil) == MojErrNone && foundUntil) {
        rrule.putString("until", until.data());
    }

    // Day of week (bitmask: 1=Su, 2=Mo, 4=Tu, 8=We, 16=Th, 32=Fr, 64=Sa)
    MojInt64 dayOfWeek = 0;
    if (easData.get("dayOfWeek", dayOfWeek) && dayOfWeek > 0) {
        MojObject days;
        MojString dayStr;
        if (dayOfWeek & 1) { dayStr.assign("SU"); days.push(dayStr); }
        if (dayOfWeek & 2) { dayStr.assign("MO"); days.push(dayStr); }
        if (dayOfWeek & 4) { dayStr.assign("TU"); days.push(dayStr); }
        if (dayOfWeek & 8) { dayStr.assign("WE"); days.push(dayStr); }
        if (dayOfWeek & 16) { dayStr.assign("TH"); days.push(dayStr); }
        if (dayOfWeek & 32) { dayStr.assign("FR"); days.push(dayStr); }
        if (dayOfWeek & 64) { dayStr.assign("SA"); days.push(dayStr); }
        rrule.put("byday", days);
    }

    // Day of month
    MojInt64 dayOfMonth = 0;
    if (easData.get("dayOfMonth", dayOfMonth) && dayOfMonth > 0) {
        MojObject days;
        days.push(dayOfMonth);
        rrule.put("bymonthday", days);
    }

    // Month of year
    MojInt64 monthOfYear = 0;
    if (easData.get("monthOfYear", monthOfYear) && monthOfYear > 0) {
        MojObject months;
        months.push(monthOfYear);
        rrule.put("bymonth", months);
    }
}

void EasEventAdapter::ParseAttendees(const MojObject& easData, MojObject& attendees)
{
    MojObject::ConstArrayIterator it = easData.arrayBegin();

    for (; it != easData.arrayEnd(); ++it) {
        bool found;
        MojObject attendee;

        MojString email;
        if (it->get("email", email, found) == MojErrNone && found) {
            attendee.putString("email", email.data());
        }

        MojString name;
        if (it->get("name", name, found) == MojErrNone && found) {
            attendee.putString("commonName", name.data());
        }

        // Attendee type (1=required, 2=optional, 3=resource) - MojInt64 returns bool directly
        MojInt64 type = 1;
        if (it->get("attendeeType", type)) {
            switch (type) {
                case 1: attendee.putString("role", "REQ-PARTICIPANT"); break;
                case 2: attendee.putString("role", "OPT-PARTICIPANT"); break;
                case 3: attendee.putString("cutype", "RESOURCE"); break;
            }
        }

        // Attendee status (0=response unknown, 2=tentative, 3=accept, 4=decline, 5=not responded)
        MojInt64 status = 0;
        if (it->get("attendeeStatus", status)) {
            switch (status) {
                case 2: attendee.putString("participationStatus", "TENTATIVE"); break;
                case 3: attendee.putString("participationStatus", "ACCEPTED"); break;
                case 4: attendee.putString("participationStatus", "DECLINED"); break;
                default: attendee.putString("participationStatus", "NEEDS-ACTION"); break;
            }
        }

        attendees.push(attendee);
    }
}

int64_t EasEventAdapter::ParseEasDate(const std::string& dateStr)
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

    if (parsed >= 6) {
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        time_t timestamp = timegm(&tm);
        return static_cast<int64_t>(timestamp) * 1000 + msec;
    }

    return 0;
}

std::string EasEventAdapter::FormatEasDate(int64_t timestamp)
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

std::string EasEventAdapter::ConvertBusyStatus(int easStatus)
{
    switch (easStatus) {
        case 0: return "free";
        case 1: return "tentative";
        case 2: return "busy";
        case 3: return "outOfOffice";
        default: return "busy";
    }
}

std::string EasEventAdapter::ConvertSensitivity(int easSensitivity)
{
    switch (easSensitivity) {
        case 0: return "public";
        case 1: return "personal";
        case 2: return "private";
        case 3: return "confidential";
        default: return "public";
    }
}
