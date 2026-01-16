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

#include "commands/FolderSyncCommand.h"
#include "EasClient.h"
#include "wbxml/WbxmlEncoder.h"
#include "wbxml/WbxmlParser.h"
#include "wbxml/EasTagCode.h"
#include "data/EasFolder.h"

/**
 * Response handler for parsing FolderSync WBXML response
 */
class FolderSyncCommand::ResponseHandler : public WbxmlContentHandler
{
public:
    ResponseHandler(FolderSyncCommand& cmd)
        : m_cmd(cmd)
        , m_inChanges(false)
        , m_inAdd(false)
        , m_inDelete(false)
        , m_inUpdate(false)
    {
    }

    void StartElement(EasTagCode::EasTag tag)
    {
        m_currentTag = tag;

        switch (tag) {
            case EasTagCode::FolderHierarchy_Changes:
                m_inChanges = true;
                break;
            case EasTagCode::FolderHierarchy_Add:
                m_inAdd = true;
                m_currentFolder = EasFolder();
                break;
            case EasTagCode::FolderHierarchy_Delete:
                m_inDelete = true;
                m_deleteServerId.clear();
                break;
            case EasTagCode::FolderHierarchy_Update:
                m_inUpdate = true;
                m_currentFolder = EasFolder();
                break;
            default:
                break;
        }
    }

    void EndElement(EasTagCode::EasTag tag)
    {
        switch (tag) {
            case EasTagCode::FolderHierarchy_Changes:
                m_inChanges = false;
                break;
            case EasTagCode::FolderHierarchy_Add:
                if (m_inAdd) {
                    m_cmd.m_addedFolders.push_back(m_currentFolder);
                }
                m_inAdd = false;
                break;
            case EasTagCode::FolderHierarchy_Delete:
                if (m_inDelete && !m_deleteServerId.empty()) {
                    m_cmd.m_deletedFolders.push_back(m_deleteServerId);
                }
                m_inDelete = false;
                break;
            case EasTagCode::FolderHierarchy_Update:
                if (m_inUpdate) {
                    m_cmd.m_updatedFolders.push_back(m_currentFolder);
                }
                m_inUpdate = false;
                break;
            default:
                break;
        }
        m_currentTag = 0;
    }

    void Text(const std::string& text)
    {
        switch (m_currentTag) {
            case EasTagCode::FolderHierarchy_Status:
                m_cmd.m_status = atoi(text.c_str());
                break;
            case EasTagCode::FolderHierarchy_SyncKey:
                m_cmd.m_syncKey = text;
                break;
            case EasTagCode::FolderHierarchy_ServerId:
                if (m_inAdd || m_inUpdate) {
                    m_currentFolder.SetServerId(text);
                } else if (m_inDelete) {
                    m_deleteServerId = text;
                }
                break;
            case EasTagCode::FolderHierarchy_ParentId:
                m_currentFolder.SetParentId(text);
                break;
            case EasTagCode::FolderHierarchy_DisplayName:
                m_currentFolder.SetDisplayName(text);
                break;
            case EasTagCode::FolderHierarchy_Type:
                m_currentFolder.SetType(static_cast<EasFolderType::Type>(atoi(text.c_str())));
                break;
            default:
                break;
        }
    }

    void OpaqueData(const std::vector<unsigned char>& data)
    {
        // FolderSync doesn't use opaque data
        (void)data;
    }

private:
    FolderSyncCommand&      m_cmd;
    EasTagCode::EasTag      m_currentTag;
    bool                    m_inChanges;
    bool                    m_inAdd;
    bool                    m_inDelete;
    bool                    m_inUpdate;
    EasFolder               m_currentFolder;
    std::string             m_deleteServerId;
};

FolderSyncCommand::FolderSyncCommand(EasClient& client)
    : EasNetworkCommand(client, EasCommandType::FolderSync)
    , m_status(0)
{
    // Get current sync key from client's account
    if (client.GetAccount()) {
        m_syncKey = client.GetAccount()->GetFolderSyncKey();
    }
    if (m_syncKey.empty()) {
        m_syncKey = "0";  // Initial sync
    }
}

FolderSyncCommand::~FolderSyncCommand()
{
}

void FolderSyncCommand::Run()
{
    MojLogInfo(s_log, "Running FolderSync with syncKey: %s", m_syncKey.c_str());

    // Generate request WBXML
    WbxmlEncoder encoder;
    GenerateRequest(encoder);

    // Send the request
    std::vector<unsigned char> wbxml = encoder.GetWbxml();
    SendRequest(wbxml);
}

void FolderSyncCommand::GenerateRequest(WbxmlEncoder& encoder)
{
    // FolderSync request:
    // <FolderSync>
    //   <SyncKey>0</SyncKey>
    // </FolderSync>

    encoder.StartTag(EasTagCode::FolderHierarchy_FolderSync);
    encoder.Text(EasTagCode::FolderHierarchy_SyncKey, m_syncKey);
    encoder.EndTag(EasTagCode::FolderHierarchy_FolderSync);
}

void FolderSyncCommand::ProcessResponse(WbxmlParser& parser)
{
    // Response is already parsed via ResponseHandler
    // Now process the results

    MojLogInfo(s_log, "FolderSync response: status=%d, syncKey=%s, added=%zu, deleted=%zu, updated=%zu",
               m_status, m_syncKey.c_str(),
               m_addedFolders.size(), m_deletedFolders.size(), m_updatedFolders.size());

    if (m_status == 1) {
        // Success
        SaveFoldersToDatabase();
    } else if (m_status == 9) {
        // Invalid sync key - reset and try again
        MojLogWarning(s_log, "FolderSync invalid sync key, resetting to 0");
        m_syncKey = "0";
        // ResetSyncKey();
    } else {
        // Error
        MojLogError(s_log, "FolderSync failed with status %d", m_status);
        Failure(MailError::INTERNAL_ERROR, "FolderSync failed");
    }
}

void FolderSyncCommand::SaveFoldersToDatabase()
{
    MojLogInfo(s_log, "Saving %zu added folders to database", m_addedFolders.size());

    // Process added folders
    for (size_t i = 0; i < m_addedFolders.size(); ++i) {
        ProcessAddedFolder(m_addedFolders[i]);
    }

    // Process deleted folders
    for (size_t i = 0; i < m_deletedFolders.size(); ++i) {
        ProcessDeletedFolder(m_deletedFolders[i]);
    }

    // Process updated folders
    for (size_t i = 0; i < m_updatedFolders.size(); ++i) {
        ProcessUpdatedFolder(m_updatedFolders[i]);
    }

    // Save sync key
    // SaveSyncKey();

    // Notify client
    GetClient().FolderSyncSuccess(m_syncKey, m_addedFolders);

    // Complete the command
    Complete();
}

void FolderSyncCommand::ProcessAddedFolder(const EasFolder& folder)
{
    MojLogInfo(s_log, "Adding folder: %s (%s) type=%d",
               folder.GetDisplayName().c_str(),
               folder.GetServerId().c_str(),
               static_cast<int>(folder.GetType()));

    // Create MojObject and save to database
    MojObject dbObj;
    folder.ToDatabase(dbObj);

    // Add account ID
    dbObj.put("accountId", GetClient().GetAccountId());

    // TODO: Save to db8
    // m_dbInterface->Put(dbObj, ...);
}

void FolderSyncCommand::ProcessDeletedFolder(const std::string& serverId)
{
    MojLogInfo(s_log, "Deleting folder: %s", serverId.c_str());

    // TODO: Delete from db8
    // m_dbInterface->Delete(...);
}

void FolderSyncCommand::ProcessUpdatedFolder(const EasFolder& folder)
{
    MojLogInfo(s_log, "Updating folder: %s (%s)",
               folder.GetDisplayName().c_str(),
               folder.GetServerId().c_str());

    // TODO: Update in db8
    // m_dbInterface->Merge(...);
}
