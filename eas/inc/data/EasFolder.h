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

#ifndef EASFOLDER_H_
#define EASFOLDER_H_

#include "core/MojObject.h"
#include "EasCoreDefs.h"
#include <string>

/**
 * EasFolder - Exchange folder data
 */
class EasFolder
{
public:
    EasFolder();
    virtual ~EasFolder();

    // Serialization
    MojErr FromDatabase(const MojObject& obj);
    MojErr ToDatabase(MojObject& obj) const;

    // From WBXML
    void FromWbxml(const std::string& serverId,
                   const std::string& parentId,
                   const std::string& displayName,
                   EasFolderType::Type type);

    // Properties
    const MojObject& GetId() const { return m_id; }
    void SetId(const MojObject& id) { m_id = id; }

    const std::string& GetServerId() const { return m_serverId; }
    void SetServerId(const std::string& id) { m_serverId = id; }

    const std::string& GetParentId() const { return m_parentId; }
    void SetParentId(const std::string& id) { m_parentId = id; }

    const std::string& GetDisplayName() const { return m_displayName; }
    void SetDisplayName(const std::string& name) { m_displayName = name; }

    EasFolderType::Type GetType() const { return m_type; }
    void SetType(EasFolderType::Type type) { m_type = type; }

    const std::string& GetSyncKey() const { return m_syncKey; }
    void SetSyncKey(const std::string& key) { m_syncKey = key; }

    // Helpers
    std::string GetCollectionClass() const;
    bool IsEmailFolder() const;
    bool IsCalendarFolder() const;
    bool IsContactsFolder() const;
    bool IsTasksFolder() const;

private:
    MojObject       m_id;
    std::string     m_serverId;
    std::string     m_parentId;
    std::string     m_displayName;
    EasFolderType::Type m_type;
    std::string     m_syncKey;
};

#endif /* EASFOLDER_H_ */
