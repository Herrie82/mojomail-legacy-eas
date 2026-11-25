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

#include "data/EasFolder.h"

EasFolder::EasFolder()
    : m_type(EasFolderType::Unknown)
{
}

EasFolder::~EasFolder()
{
}

void EasFolder::FromWbxml(const std::string& serverId,
                          const std::string& parentId,
                          const std::string& displayName,
                          EasFolderType::Type type)
{
    m_serverId = serverId;
    m_parentId = parentId;
    m_displayName = displayName;
    m_type = type;
}

MojErr EasFolder::FromDatabase(const MojObject& obj)
{
    bool found;

    // Get _id - returns bool, no found parameter
    obj.get("_id", m_id);

    MojString strVal;
    if (obj.get("serverId", strVal, found) == MojErrNone && found) {
        m_serverId = strVal.data();
    }
    if (obj.get("parentId", strVal, found) == MojErrNone && found) {
        m_parentId = strVal.data();
    }
    if (obj.get("displayName", strVal, found) == MojErrNone && found) {
        m_displayName = strVal.data();
    }
    if (obj.get("syncKey", strVal, found) == MojErrNone && found) {
        m_syncKey = strVal.data();
    }

    // Get type - MojInt64 returns bool directly
    MojInt64 intVal;
    if (obj.get("type", intVal)) {
        m_type = static_cast<EasFolderType::Type>(intVal);
    }

    return MojErrNone;
}

MojErr EasFolder::ToDatabase(MojObject& obj) const
{
    MojErr err;

    err = obj.putString("_kind", "com.palm.eas.folder:1");
    MojErrCheck(err);

    if (!m_id.undefined()) {
        err = obj.put("_id", m_id);
        MojErrCheck(err);
    }

    err = obj.putString("serverId", m_serverId.c_str());
    MojErrCheck(err);
    err = obj.putString("parentId", m_parentId.c_str());
    MojErrCheck(err);
    err = obj.putString("displayName", m_displayName.c_str());
    MojErrCheck(err);
    err = obj.putInt("type", static_cast<int>(m_type));
    MojErrCheck(err);
    err = obj.putString("syncKey", m_syncKey.c_str());
    MojErrCheck(err);

    return MojErrNone;
}

std::string EasFolder::GetCollectionClass() const
{
    switch (m_type) {
        case EasFolderType::DefaultInbox:
        case EasFolderType::DefaultDrafts:
        case EasFolderType::DefaultDeleted:
        case EasFolderType::DefaultSent:
        case EasFolderType::DefaultOutbox:
        case EasFolderType::UserMail:
            return "Email";

        case EasFolderType::DefaultCalendar:
        case EasFolderType::UserCalendar:
            return "Calendar";

        case EasFolderType::DefaultContacts:
        case EasFolderType::UserContacts:
        case EasFolderType::RecipientCache:
            return "Contacts";

        case EasFolderType::DefaultTasks:
        case EasFolderType::UserTasks:
            return "Tasks";

        case EasFolderType::DefaultNotes:
        case EasFolderType::UserNotes:
            return "Notes";

        default:
            return "";
    }
}

bool EasFolder::IsEmailFolder() const
{
    switch (m_type) {
        case EasFolderType::DefaultInbox:
        case EasFolderType::DefaultDrafts:
        case EasFolderType::DefaultDeleted:
        case EasFolderType::DefaultSent:
        case EasFolderType::DefaultOutbox:
        case EasFolderType::UserMail:
            return true;
        default:
            return false;
    }
}

bool EasFolder::IsCalendarFolder() const
{
    return m_type == EasFolderType::DefaultCalendar ||
           m_type == EasFolderType::UserCalendar;
}

bool EasFolder::IsContactsFolder() const
{
    return m_type == EasFolderType::DefaultContacts ||
           m_type == EasFolderType::UserContacts ||
           m_type == EasFolderType::RecipientCache;
}

bool EasFolder::IsTasksFolder() const
{
    return m_type == EasFolderType::DefaultTasks ||
           m_type == EasFolderType::UserTasks;
}
