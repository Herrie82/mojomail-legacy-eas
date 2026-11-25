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

#include "commands/PingCommand.h"
#include "EasClient.h"
#include "wbxml/WbxmlEncoder.h"
#include "wbxml/WbxmlParser.h"
#include "wbxml/EasTagCode.h"

MojLogger PingCommand::s_log("com.palm.eas.command.ping");

/**
 * Response handler for parsing Ping WBXML response
 */
class PingCommand::ResponseHandler : public WbxmlContentHandler
{
public:
    ResponseHandler(PingCommand& cmd)
        : m_cmd(cmd)
        , m_inFolders(false)
        , m_inFolder(false)
        , m_currentTag(0)
    {
    }

    void StartElement(EasTagCode::EasTag tag)
    {
        m_currentTag = tag;

        switch (tag) {
            case EasTagCode::Ping_Folders:
                m_inFolders = true;
                break;
            case EasTagCode::Ping_Folder:
                m_inFolder = true;
                break;
            default:
                break;
        }
    }

    void EndElement(EasTagCode::EasTag tag)
    {
        switch (tag) {
            case EasTagCode::Ping_Folders:
                m_inFolders = false;
                break;
            case EasTagCode::Ping_Folder:
                m_inFolder = false;
                break;
            default:
                break;
        }
        m_currentTag = 0;
    }

    void Text(const std::string& text)
    {
        switch (m_currentTag) {
            case EasTagCode::Ping_Status:
                m_cmd.m_status = static_cast<PingCommand::PingStatus>(atoi(text.c_str()));
                break;

            case EasTagCode::Ping_HeartbeatInterval:
                m_cmd.m_serverHeartbeat = atoi(text.c_str());
                break;

            case EasTagCode::Ping_MaxFolders:
                m_cmd.m_maxFolders = atoi(text.c_str());
                break;

            case EasTagCode::Ping_Id:
                // Folder ID in the changed folders list
                if (m_inFolders && m_inFolder) {
                    m_cmd.m_changedFolders.push_back(text);
                }
                break;

            default:
                break;
        }
    }

    void OpaqueData(const std::vector<unsigned char>& data)
    {
        (void)data;
    }

private:
    PingCommand&            m_cmd;
    bool                    m_inFolders;
    bool                    m_inFolder;
    EasTagCode::EasTag      m_currentTag;
};

PingCommand::PingCommand(EasClient& client)
    : EasNetworkCommand(client, EasCommandType::Ping)
    , m_heartbeatInterval(470)  // ~8 minutes default
    , m_status(Status_NoChanges)
    , m_serverHeartbeat(0)
    , m_maxFolders(0)
{
}

PingCommand::~PingCommand()
{
}

void PingCommand::AddFolder(const std::string& serverId, const std::string& collectionClass)
{
    PingFolder folder;
    folder.serverId = serverId;
    folder.collectionClass = collectionClass;
    m_folders.push_back(folder);
}

void PingCommand::Run()
{
    MojLogInfo(s_log, "Running Ping command with heartbeat=%d, folders=%zu",
               m_heartbeatInterval, m_folders.size());
    m_running = true;
    m_cancelled = false;

    if (m_folders.empty()) {
        MojLogWarning(s_log, "No folders configured for Ping");
        Failure(MailError::INTERNAL_ERROR, "No folders to monitor");
        return;
    }

    // Generate WBXML request
    WbxmlEncoder encoder;
    GeneratePingRequest(encoder);

    // Send request (this is a long-poll, may take up to heartbeatInterval seconds)
    SendRequest(encoder.GetWbxml());
}

void PingCommand::Cancel()
{
    MojLogInfo(s_log, "Cancelling Ping command");
    m_cancelled = true;
    EasCommand::Cancel();
}

void PingCommand::GeneratePingRequest(WbxmlEncoder& encoder)
{
    MojLogInfo(s_log, "Generating Ping request");

    // <Ping>
    //   <HeartbeatInterval>470</HeartbeatInterval>
    //   <Folders>
    //     <Folder>
    //       <Id>5</Id>
    //       <Class>Email</Class>
    //     </Folder>
    //     ...
    //   </Folders>
    // </Ping>

    encoder.StartTag(EasTagCode::Ping_Ping);

    // Heartbeat interval
    encoder.TextInt(EasTagCode::Ping_HeartbeatInterval, m_heartbeatInterval);

    // Folders to monitor
    encoder.StartTag(EasTagCode::Ping_Folders);

    for (size_t i = 0; i < m_folders.size(); ++i) {
        encoder.StartTag(EasTagCode::Ping_Folder);
        encoder.Text(EasTagCode::Ping_Id, m_folders[i].serverId);
        encoder.Text(EasTagCode::Ping_Class, m_folders[i].collectionClass);
        encoder.EndTag(EasTagCode::Ping_Folder);
    }

    encoder.EndTag(EasTagCode::Ping_Folders);
    encoder.EndTag(EasTagCode::Ping_Ping);
}

void PingCommand::HandleHttpResponse(int statusCode,
                                     const std::vector<unsigned char>& body)
{
    MojLogInfo(s_log, "Ping HTTP response: %d, body size: %zu",
               statusCode, body.size());

    if (m_cancelled) {
        MojLogInfo(s_log, "Ping was cancelled, ignoring response");
        return;
    }

    if (statusCode != 200) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Ping failed with HTTP %d", statusCode);
        Failure(MailError::INTERNAL_ERROR, msg);
        return;
    }

    if (body.empty()) {
        // Empty response can happen on timeout
        m_status = Status_NoChanges;
        ProcessPingResponse();
        return;
    }

    // Parse WBXML response
    m_changedFolders.clear();
    ResponseHandler handler(*this);
    WbxmlParser parser(handler);

    if (!parser.HandleInput(reinterpret_cast<const char*>(body.data()),
                            static_cast<unsigned int>(body.size()))) {
        Failure(MailError::INTERNAL_ERROR, "Failed to parse Ping response");
        return;
    }

    ProcessPingResponse();
}

void PingCommand::ProcessPingResponse()
{
    MojLogInfo(s_log, "Processing Ping response: status=%d, changedFolders=%zu",
               static_cast<int>(m_status), m_changedFolders.size());

    switch (m_status) {
        case Status_NoChanges:
            // Heartbeat expired, no changes detected
            MojLogInfo(s_log, "Ping: No changes detected");
            m_client.PingNoChanges();
            Complete();
            break;

        case Status_ChangesExist:
            // Changes detected in one or more folders
            MojLogInfo(s_log, "Ping: Changes detected in %zu folders",
                       m_changedFolders.size());
            for (size_t i = 0; i < m_changedFolders.size(); ++i) {
                MojLogInfo(s_log, "  Changed folder: %s", m_changedFolders[i].c_str());
            }
            m_client.PingChangesDetected(m_changedFolders);
            Complete();
            break;

        case Status_MissingParams:
        case Status_SyntaxError:
            MojLogError(s_log, "Ping syntax/param error: %d", static_cast<int>(m_status));
            Failure(MailError::INTERNAL_ERROR, "Ping request error");
            break;

        case Status_InvalidInterval:
            // Server wants us to use a different heartbeat interval
            MojLogInfo(s_log, "Server requested heartbeat change to %d seconds",
                       m_serverHeartbeat);
            if (m_serverHeartbeat > 0) {
                m_heartbeatInterval = m_serverHeartbeat;
            }
            // Retry with new heartbeat
            Run();
            break;

        case Status_TooManyFolders:
            MojLogWarning(s_log, "Too many folders for Ping (max=%d)", m_maxFolders);
            // Reduce folder count and retry
            if (m_maxFolders > 0 && m_folders.size() > static_cast<size_t>(m_maxFolders)) {
                m_folders.resize(m_maxFolders);
                Run();
            } else {
                Failure(MailError::INTERNAL_ERROR, "Too many folders");
            }
            break;

        case Status_FolderSyncRequired:
            // Folder hierarchy has changed, need to re-sync
            MojLogInfo(s_log, "Folder sync required");
            m_client.FolderSyncRequired();
            Complete();
            break;

        case Status_ServerError:
            MojLogError(s_log, "Server error during Ping");
            Failure(MailError::SERVER_ERROR, "Server error");
            break;

        default:
            MojLogWarning(s_log, "Unknown Ping status: %d", static_cast<int>(m_status));
            Failure(MailError::INTERNAL_ERROR, "Unknown Ping status");
            break;
    }
}
