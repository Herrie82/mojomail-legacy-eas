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

#ifndef FOLDERSYNCCOMMAND_H_
#define FOLDERSYNCCOMMAND_H_

#include "commands/EasCommand.h"
#include "data/EasFolder.h"
#include <vector>

/**
 * FolderSyncCommand - Synchronize folder hierarchy
 *
 * Retrieves the list of folders from the Exchange server and
 * updates the local database. This must be run before any
 * collection sync operations.
 *
 * Protocol: [MS-ASCMD] Section 2.2.1.5
 */
class FolderSyncCommand : public EasNetworkCommand
{
public:
    FolderSyncCommand(EasClient& client);
    virtual ~FolderSyncCommand();

    virtual void Run();

protected:
    virtual void GenerateRequest(class WbxmlEncoder& encoder);
    virtual void ProcessResponse(class WbxmlParser& parser);

private:
    // Response parsing state
    class ResponseHandler;

    void SaveFoldersToDatabase();
    void ProcessAddedFolder(const EasFolder& folder);
    void ProcessDeletedFolder(const std::string& serverId);
    void ProcessUpdatedFolder(const EasFolder& folder);

    std::string                 m_syncKey;
    std::vector<EasFolder>      m_addedFolders;
    std::vector<std::string>    m_deletedFolders;
    std::vector<EasFolder>      m_updatedFolders;
    int                         m_status;
};

#endif /* FOLDERSYNCCOMMAND_H_ */
