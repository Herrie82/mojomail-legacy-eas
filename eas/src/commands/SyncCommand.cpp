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

#include "commands/SyncCommand.h"
#include "EasClient.h"
#include "wbxml/WbxmlEncoder.h"
#include "wbxml/WbxmlParser.h"
#include "wbxml/EasTagCode.h"

MojLogger SyncCommand::s_log("com.palm.eas.command.sync");

/**
 * Response handler for parsing Sync WBXML response
 *
 * Sync responses have this structure:
 * <Sync>
 *   <Collections>
 *     <Collection>
 *       <Class>Email</Class>
 *       <SyncKey>1234567890</SyncKey>
 *       <CollectionId>5</CollectionId>
 *       <Status>1</Status>
 *       <MoreAvailable/>
 *       <Commands>
 *         <Add>
 *           <ServerId>5:123</ServerId>
 *           <ApplicationData>...</ApplicationData>
 *         </Add>
 *         <Change>...</Change>
 *         <Delete><ServerId>5:456</ServerId></Delete>
 *       </Commands>
 *     </Collection>
 *   </Collections>
 * </Sync>
 */
class SyncCommand::ResponseHandler : public WbxmlContentHandler
{
public:
    ResponseHandler(SyncCommand& cmd)
        : m_cmd(cmd)
        , m_inCollection(false)
        , m_inCommands(false)
        , m_inAdd(false)
        , m_inChange(false)
        , m_inDelete(false)
        , m_inApplicationData(false)
        , m_currentTag(0)
    {
    }

    void StartElement(EasTagCode::EasTag tag)
    {
        m_currentTag = tag;

        switch (tag) {
            case EasTagCode::AirSync_Collection:
                m_inCollection = true;
                break;
            case EasTagCode::AirSync_Commands:
                m_inCommands = true;
                break;
            case EasTagCode::AirSync_Add:
                m_inAdd = true;
                m_currentItem = MojObject();
                m_currentServerId.clear();
                break;
            case EasTagCode::AirSync_Change:
                m_inChange = true;
                m_currentItem = MojObject();
                m_currentServerId.clear();
                break;
            case EasTagCode::AirSync_Delete:
                m_inDelete = true;
                m_currentServerId.clear();
                break;
            case EasTagCode::AirSync_ApplicationData:
                m_inApplicationData = true;
                break;
            case EasTagCode::AirSync_MoreAvailable:
                m_cmd.m_moreAvailable = true;
                break;
            default:
                break;
        }
    }

    void EndElement(EasTagCode::EasTag tag)
    {
        switch (tag) {
            case EasTagCode::AirSync_Collection:
                m_inCollection = false;
                break;
            case EasTagCode::AirSync_Commands:
                m_inCommands = false;
                break;
            case EasTagCode::AirSync_Add:
                if (!m_currentServerId.empty()) {
                    m_currentItem.putString("serverId", m_currentServerId.c_str());
                    m_cmd.m_serverAdds.push_back(m_currentItem);
                }
                m_inAdd = false;
                break;
            case EasTagCode::AirSync_Change:
                if (!m_currentServerId.empty()) {
                    m_currentItem.putString("serverId", m_currentServerId.c_str());
                    m_cmd.m_serverChanges.push_back(m_currentItem);
                }
                m_inChange = false;
                break;
            case EasTagCode::AirSync_Delete:
                if (!m_currentServerId.empty()) {
                    m_cmd.m_serverDeletes.push_back(m_currentServerId);
                }
                m_inDelete = false;
                break;
            case EasTagCode::AirSync_ApplicationData:
                m_inApplicationData = false;
                break;
            default:
                break;
        }
        m_currentTag = 0;
    }

    void Text(const std::string& text)
    {
        // Collection-level properties
        if (m_inCollection && !m_inCommands) {
            switch (m_currentTag) {
                case EasTagCode::AirSync_Status:
                    m_cmd.m_status = atoi(text.c_str());
                    break;
                case EasTagCode::AirSync_SyncKey:
                    m_cmd.m_newSyncKey = text;
                    break;
                case EasTagCode::AirSync_CollectionId:
                    // Verify it matches our request
                    break;
                default:
                    break;
            }
        }

        // Item-level properties
        if (m_inCommands) {
            switch (m_currentTag) {
                case EasTagCode::AirSync_ServerId:
                    m_currentServerId = text;
                    break;
                default:
                    break;
            }
        }

        // ApplicationData properties (email fields, etc.)
        if (m_inApplicationData && (m_inAdd || m_inChange)) {
            ParseApplicationDataField(text);
        }
    }

    void OpaqueData(const std::vector<unsigned char>& data)
    {
        (void)data;
    }

private:
    void ParseApplicationDataField(const std::string& text)
    {
        // Parse email/calendar/contact/task fields
        switch (m_currentTag) {
            // Email fields
            case EasTagCode::Email_Subject:
                m_currentItem.putString("subject", text.c_str());
                break;
            case EasTagCode::Email_From:
                m_currentItem.putString("from", text.c_str());
                break;
            case EasTagCode::Email_To:
                m_currentItem.putString("to", text.c_str());
                break;
            case EasTagCode::Email_CC:
                m_currentItem.putString("cc", text.c_str());
                break;
            case EasTagCode::Email_DateReceived:
                m_currentItem.putString("dateReceived", text.c_str());
                break;
            case EasTagCode::Email_Read:
                m_currentItem.putBool("read", text == "1");
                break;
            case EasTagCode::Email_Importance:
                m_currentItem.putInt("importance", atoi(text.c_str()));
                break;
            case EasTagCode::Email_MessageClass:
                m_currentItem.putString("messageClass", text.c_str());
                break;

            // Body content (AirSyncBase for protocol 12.0+)
            case EasTagCode::AirSyncBase_Data:
                m_currentItem.putString("body", text.c_str());
                break;
            case EasTagCode::AirSyncBase_Type:
                m_currentItem.putInt("bodyType", atoi(text.c_str()));
                break;
            case EasTagCode::AirSyncBase_EstimatedDataSize:
                m_currentItem.putInt("estimatedSize", atoi(text.c_str()));
                break;
            case EasTagCode::AirSyncBase_Truncated:
                m_currentItem.putBool("truncated", text == "1");
                break;

            // Legacy body (protocol < 12.0)
            case EasTagCode::Email_Body:
                m_currentItem.putString("body", text.c_str());
                break;

            default:
                break;
        }
    }

    SyncCommand&            m_cmd;
    bool                    m_inCollection;
    bool                    m_inCommands;
    bool                    m_inAdd;
    bool                    m_inChange;
    bool                    m_inDelete;
    bool                    m_inApplicationData;
    EasTagCode::EasTag      m_currentTag;

    // Current item being parsed
    MojObject               m_currentItem;
    std::string             m_currentServerId;
};

SyncCommand::SyncCommand(EasClient& client, const MojObject& folderId)
    : EasNetworkCommand(client, EasCommandType::Sync)
    , m_folderId(folderId)
    , m_filterType(Filter_3Days)
    , m_bodyType(Body_PlainText)
    , m_truncationSize(32768)
    , m_windowSize(25)
    , m_getChanges(true)
    , m_status(0)
    , m_moreAvailable(false)
{
}

SyncCommand::~SyncCommand()
{
}

void SyncCommand::Run()
{
    MojLogInfo(s_log, "Running Sync command for folder: %s", m_folderServerId.c_str());
    m_running = true;

    // Clear previous results
    m_serverAdds.clear();
    m_serverChanges.clear();
    m_serverDeletes.clear();
    m_moreAvailable = false;
    m_newSyncKey.clear();

    // Generate WBXML request
    WbxmlEncoder encoder;
    GenerateSyncRequest(encoder);

    // Send request
    SendRequest(encoder.GetWbxml());
}

void SyncCommand::GenerateSyncRequest(WbxmlEncoder& encoder)
{
    MojLogInfo(s_log, "Generating Sync request, syncKey=%s", m_syncKey.c_str());

    // <Sync>
    //   <Collections>
    //     <Collection>
    //       <Class>Email</Class>
    //       <SyncKey>0</SyncKey>
    //       <CollectionId>5</CollectionId>
    //       <DeletesAsMoves>1</DeletesAsMoves>
    //       <GetChanges>1</GetChanges>
    //       <WindowSize>25</WindowSize>
    //       <Options>
    //         <FilterType>2</FilterType>
    //         <BodyPreference>
    //           <Type>1</Type>
    //           <TruncationSize>32768</TruncationSize>
    //         </BodyPreference>
    //       </Options>
    //     </Collection>
    //   </Collections>
    // </Sync>

    encoder.StartTag(EasTagCode::AirSync_Sync);
    encoder.StartTag(EasTagCode::AirSync_Collections);

    GenerateCollectionRequest(encoder);

    encoder.EndTag(EasTagCode::AirSync_Collections);
    encoder.EndTag(EasTagCode::AirSync_Sync);
}

void SyncCommand::GenerateCollectionRequest(WbxmlEncoder& encoder)
{
    encoder.StartTag(EasTagCode::AirSync_Collection);

    // Collection class (Email, Calendar, Contacts, Tasks)
    if (!m_collectionClass.empty()) {
        encoder.Text(EasTagCode::AirSync_Class, m_collectionClass);
    }

    // Sync key (use "0" for initial sync)
    std::string syncKey = m_syncKey.empty() ? "0" : m_syncKey;
    encoder.Text(EasTagCode::AirSync_SyncKey, syncKey);

    // Collection ID (folder server ID)
    encoder.Text(EasTagCode::AirSync_CollectionId, m_folderServerId);

    // Only include these options for non-initial sync
    if (m_syncKey != "0" && !m_syncKey.empty()) {
        // DeletesAsMoves - move deleted items to trash
        // encoder.TextInt(EasTagCode::AirSync_DeletesAsMoves, 1);

        // GetChanges
        if (m_getChanges) {
            encoder.TextInt(EasTagCode::AirSync_GetChanges, 1);
        }

        // WindowSize - max items per response
        encoder.TextInt(EasTagCode::AirSync_WindowSize, m_windowSize);

        // Generate options
        GenerateOptions(encoder);

        // Generate local changes to upload
        GenerateLocalChanges(encoder);
    }

    encoder.EndTag(EasTagCode::AirSync_Collection);
}

void SyncCommand::GenerateOptions(WbxmlEncoder& encoder)
{
    encoder.StartTag(EasTagCode::AirSync_Options);

    // Filter type (how much history to sync)
    if (m_filterType != Filter_All) {
        encoder.TextInt(EasTagCode::AirSync_FilterType, static_cast<int>(m_filterType));
    }

    // Body preference (protocol 12.0+)
    encoder.StartTag(EasTagCode::AirSyncBase_BodyPreference);
    encoder.TextInt(EasTagCode::AirSyncBase_Type, static_cast<int>(m_bodyType));
    encoder.TextInt(EasTagCode::AirSyncBase_TruncationSize, m_truncationSize);
    encoder.EndTag(EasTagCode::AirSyncBase_BodyPreference);

    encoder.EndTag(EasTagCode::AirSync_Options);
}

void SyncCommand::GenerateLocalChanges(WbxmlEncoder& encoder)
{
    // Skip if no local changes
    if (m_localAdds.empty() && m_localChanges.empty() && m_localDeletes.empty()) {
        return;
    }

    encoder.StartTag(EasTagCode::AirSync_Commands);

    // Add new items
    for (size_t i = 0; i < m_localAdds.size(); ++i) {
        encoder.StartTag(EasTagCode::AirSync_Add);

        // ClientId - unique ID for tracking
        MojString clientId;
        bool found = false;
        if (m_localAdds[i].get("clientId", clientId, found) == MojErrNone && found) {
            encoder.Text(EasTagCode::AirSync_ClientId, clientId.data());
        }

        // ApplicationData - item content
        encoder.StartTag(EasTagCode::AirSync_ApplicationData);
        // TODO: Serialize item properties based on collection class
        encoder.EndTag(EasTagCode::AirSync_ApplicationData);

        encoder.EndTag(EasTagCode::AirSync_Add);
    }

    // Change existing items
    for (size_t i = 0; i < m_localChanges.size(); ++i) {
        encoder.StartTag(EasTagCode::AirSync_Change);

        // ServerId
        MojString serverId;
        bool found = false;
        if (m_localChanges[i].get("serverId", serverId, found) == MojErrNone && found) {
            encoder.Text(EasTagCode::AirSync_ServerId, serverId.data());
        }

        // ApplicationData
        encoder.StartTag(EasTagCode::AirSync_ApplicationData);
        // TODO: Serialize changed properties
        encoder.EndTag(EasTagCode::AirSync_ApplicationData);

        encoder.EndTag(EasTagCode::AirSync_Change);
    }

    // Delete items
    for (size_t i = 0; i < m_localDeletes.size(); ++i) {
        encoder.StartTag(EasTagCode::AirSync_Delete);

        MojString serverId;
        bool found = false;
        if (m_localDeletes[i].get("serverId", serverId, found) == MojErrNone && found) {
            encoder.Text(EasTagCode::AirSync_ServerId, serverId.data());
        }

        encoder.EndTag(EasTagCode::AirSync_Delete);
    }

    encoder.EndTag(EasTagCode::AirSync_Commands);
}

void SyncCommand::HandleHttpResponse(int statusCode,
                                     const std::vector<unsigned char>& body)
{
    MojLogInfo(s_log, "Sync HTTP response: %d, body size: %zu",
               statusCode, body.size());

    if (statusCode != 200) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Sync failed with HTTP %d", statusCode);
        Failure(MailError::INTERNAL_ERROR, msg);
        return;
    }

    if (body.empty()) {
        // Empty response is valid for initial sync (syncKey=0)
        if (m_syncKey == "0" || m_syncKey.empty()) {
            MojLogInfo(s_log, "Initial sync complete, need to sync again with new key");
        }
        ProcessSyncResponse();
        return;
    }

    // Parse WBXML response
    ResponseHandler handler(*this);
    WbxmlParser parser(handler);

    if (!parser.HandleInput(reinterpret_cast<const char*>(body.data()),
                            static_cast<unsigned int>(body.size()))) {
        Failure(MailError::INTERNAL_ERROR, "Failed to parse Sync response");
        return;
    }

    ProcessSyncResponse();
}

void SyncCommand::ProcessSyncResponse()
{
    MojLogInfo(s_log, "Processing Sync response: status=%d, newSyncKey=%s",
               m_status, m_newSyncKey.c_str());
    MojLogInfo(s_log, "  Adds: %zu, Changes: %zu, Deletes: %zu, MoreAvailable: %s",
               m_serverAdds.size(), m_serverChanges.size(), m_serverDeletes.size(),
               m_moreAvailable ? "true" : "false");

    // Check sync status
    switch (m_status) {
        case 1:  // Success
            break;

        case 3:  // Invalid sync key
            MojLogWarning(s_log, "Invalid sync key, need to re-sync");
            m_client.SyncKeyInvalid(m_folderServerId);
            Failure(MailError::INTERNAL_ERROR, "Invalid sync key");
            return;

        case 4:  // Protocol error
            MojLogError(s_log, "Protocol error in Sync");
            Failure(MailError::INTERNAL_ERROR, "Sync protocol error");
            return;

        case 5:  // Server error
            MojLogError(s_log, "Server error in Sync");
            Failure(MailError::INTERNAL_ERROR, "Server error");
            return;

        case 6:  // Error in client request
            MojLogError(s_log, "Client request error in Sync");
            Failure(MailError::INTERNAL_ERROR, "Invalid sync request");
            return;

        case 7:  // Conflict
            MojLogWarning(s_log, "Sync conflict detected");
            break;

        case 8:  // Object not found
            MojLogWarning(s_log, "Object not found in Sync");
            break;

        case 12: // Folder hierarchy changed
            MojLogInfo(s_log, "Folder hierarchy changed");
            m_client.FolderSyncRequired();
            Failure(MailError::INTERNAL_ERROR, "Folder hierarchy changed");
            return;

        default:
            if (m_status != 0) {
                MojLogWarning(s_log, "Unknown sync status: %d", m_status);
            }
            break;
    }

    // Update sync key for next sync
    if (!m_newSyncKey.empty()) {
        m_syncKey = m_newSyncKey;
    }

    // Notify client of sync results
    m_client.SyncComplete(m_folderServerId, m_serverAdds, m_serverChanges,
                          m_serverDeletes, m_moreAvailable);

    Complete();
}
