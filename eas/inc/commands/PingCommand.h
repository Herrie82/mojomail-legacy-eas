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

#ifndef PINGCOMMAND_H_
#define PINGCOMMAND_H_

#include "commands/EasCommand.h"
#include <vector>
#include <string>

/**
 * PingCommand - Push notification for folder changes
 *
 * Maintains a long-running connection to the server that returns
 * when any monitored folder has changes. Used for "push email".
 *
 * Protocol: [MS-ASCMD] Section 2.2.1.13
 */
class PingCommand : public EasNetworkCommand
{
public:
    // Ping status codes
    enum PingStatus {
        Status_NoChanges        = 1,  // Heartbeat expired, no changes
        Status_ChangesExist     = 2,  // Changes detected in folder(s)
        Status_MissingParams    = 3,
        Status_SyntaxError      = 4,
        Status_InvalidInterval  = 5,
        Status_TooManyFolders   = 6,
        Status_FolderSyncRequired = 7,
        Status_ServerError      = 8
    };

    // Folder info for ping
    struct PingFolder {
        std::string serverId;
        std::string collectionClass;
    };

    PingCommand(EasClient& client);
    virtual ~PingCommand();

    // Configuration
    void SetHeartbeatInterval(int seconds) { m_heartbeatInterval = seconds; }
    void AddFolder(const std::string& serverId, const std::string& collectionClass);
    void ClearFolders() { m_folders.clear(); }

    // Status
    PingStatus GetStatus() const { return m_status; }
    const std::vector<std::string>& GetChangedFolders() const { return m_changedFolders; }
    int GetServerHeartbeat() const { return m_serverHeartbeat; }

    virtual void Run();
    void Cancel();

protected:
    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);

    static MojLogger s_log;

private:
    class ResponseHandler;

    void GeneratePingRequest(class WbxmlEncoder& encoder);
    void ProcessPingResponse();
    int                         m_heartbeatInterval;
    std::vector<PingFolder>     m_folders;

    // Response
    PingStatus                  m_status;
    std::vector<std::string>    m_changedFolders;
    int                         m_serverHeartbeat;
    int                         m_maxFolders;
};

#endif /* PINGCOMMAND_H_ */
