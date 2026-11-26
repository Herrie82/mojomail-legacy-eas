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

#ifndef OPTIONSCOMMAND_H_
#define OPTIONSCOMMAND_H_

#include "commands/EasCommand.h"
#include <vector>
#include <string>

/**
 * OptionsCommand - Query server capabilities
 *
 * HTTP OPTIONS request to the Exchange server to determine
 * supported protocol versions and commands. This is the first
 * command run when connecting to a new server.
 *
 * Protocol: [MS-ASHTTP] Section 2.2.3
 *
 * Response headers:
 *   MS-ASProtocolVersions: 2.0,2.1,2.5,12.0,12.1,14.0,14.1
 *   MS-ASProtocolCommands: Sync,SendMail,SmartForward,...
 *   MS-Server-ActiveSync: 14.1
 */
class OptionsCommand : public EasNetworkCommand
{
public:
    OptionsCommand(EasClient& client);
    virtual ~OptionsCommand();

    // Results
    const std::vector<std::string>& GetSupportedVersions() const { return m_supportedVersions; }
    const std::vector<std::string>& GetSupportedCommands() const { return m_supportedCommands; }
    const std::string& GetServerType() const { return m_serverType; }
    bool IsSearchSupported() const { return m_searchSupported; }

    /**
     * Get the preferred protocol version to use
     * Selects highest mutually supported version
     */
    std::string GetPreferredVersion() const;

    virtual void Run();

protected:
    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);

    void ParseResponseHeaders();

private:
    void ParseOptionsHeaders(const std::vector<std::string>& headers);
    void ParseCommaSeparatedList(const std::string& value, std::vector<std::string>& list);

    // CURL header callback
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userp);

    // Header constants
    static const char* PROTOCOL_VERSIONS_HEADER;
    static const char* PROTOCOL_COMMANDS_HEADER;
    static const char* SERVER_TYPE_HEADER;

    // Results
    std::vector<std::string>    m_supportedVersions;
    std::vector<std::string>    m_supportedCommands;
    std::string                 m_serverType;
    bool                        m_searchSupported;
};

#endif /* OPTIONSCOMMAND_H_ */
