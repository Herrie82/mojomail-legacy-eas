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

#ifndef SYNCCOMMAND_H_
#define SYNCCOMMAND_H_

#include "commands/EasCommand.h"
#include "core/MojObject.h"
#include <vector>
#include <string>

/**
 * SyncCommand - Synchronize collection items (emails, contacts, calendar, tasks)
 *
 * The main workhorse command for EAS. Handles bidirectional sync of
 * items in a collection (folder).
 *
 * Protocol: [MS-ASCMD] Section 2.2.1.21
 */
class SyncCommand : public EasNetworkCommand
{
public:
    // Sync filter types (how much history to sync)
    enum FilterType {
        Filter_All          = 0,
        Filter_1Day         = 1,
        Filter_3Days        = 2,
        Filter_1Week        = 3,
        Filter_2Weeks       = 4,
        Filter_1Month       = 5,
        Filter_3Months      = 6,
        Filter_6Months      = 7
    };

    // Body preference types
    enum BodyType {
        Body_PlainText      = 1,
        Body_HTML           = 2,
        Body_RTF            = 3,
        Body_MIME           = 4
    };

    SyncCommand(EasClient& client, const MojObject& folderId);
    virtual ~SyncCommand();

    // Configuration
    void SetFilterType(FilterType filter) { m_filterType = filter; }
    void SetBodyType(BodyType type) { m_bodyType = type; }
    void SetTruncationSize(int size) { m_truncationSize = size; }
    void SetWindowSize(int size) { m_windowSize = size; }
    void SetGetChanges(bool get) { m_getChanges = get; }

    // Results
    bool HasMoreAvailable() const { return m_moreAvailable; }
    const std::string& GetNewSyncKey() const { return m_newSyncKey; }

    // Items received from server
    const std::vector<MojObject>& GetAddedItems() const { return m_serverAdds; }
    const std::vector<MojObject>& GetChangedItems() const { return m_serverChanges; }
    const std::vector<std::string>& GetDeletedItems() const { return m_serverDeletes; }

    virtual void Run();

protected:
    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);

    static MojLogger s_log;

private:
    class ResponseHandler;

    void GenerateSyncRequest(class WbxmlEncoder& encoder);
    void GenerateCollectionRequest(class WbxmlEncoder& encoder);
    void GenerateOptions(class WbxmlEncoder& encoder);
    void GenerateLocalChanges(class WbxmlEncoder& encoder);

    void ProcessSyncResponse();

    // Collection info
    MojObject           m_folderId;
    std::string         m_folderServerId;
    std::string         m_collectionClass;
    std::string         m_syncKey;
    std::string         m_newSyncKey;

    // Sync options
    FilterType          m_filterType;
    BodyType            m_bodyType;
    int                 m_truncationSize;
    int                 m_windowSize;
    bool                m_getChanges;

    // Response state
    int                 m_status;
    bool                m_moreAvailable;

    // Items received from server
    std::vector<MojObject>      m_serverAdds;
    std::vector<MojObject>      m_serverChanges;
    std::vector<std::string>    m_serverDeletes;

    // Pending local changes to upload
    std::vector<MojObject>  m_localAdds;
    std::vector<MojObject>  m_localChanges;
    std::vector<MojObject>  m_localDeletes;
};

#endif /* SYNCCOMMAND_H_ */
