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

#ifndef EASCOMMAND_H_
#define EASCOMMAND_H_

#include "core/MojObject.h"
#include "core/MojLog.h"
#include "core/MojSignal.h"
#include "client/Command.h"
#include "CommonErrors.h"
#include "EasCoreDefs.h"
#include <string>
#include <vector>

class EasClient;

/**
 * EasCommand - Base class for all EAS commands
 *
 * Provides common functionality for sending HTTP requests to
 * Exchange server and processing WBXML responses.
 * Inherits from Command for compatibility with CommandManager.
 */
class EasCommand : public Command
{
public:
    static const std::string UNKNOWN_COMMAND_TYPE;

    EasCommand(EasClient& client, EasCommandType::Type type, Priority priority = NormalPriority);
    virtual ~EasCommand();

    // Command interface (inherited from Command)
    virtual void Run() = 0;
    virtual void Cancel();
    virtual void Complete();
    virtual void Failure(MailError::ErrorCode error, const std::string& message);

    // Override Describe from Command
    virtual std::string Describe() const;

    // Command info
    EasCommandType::Type GetCommandType() const { return m_commandType; }
    std::string GetCommandName() const;
    bool IsRunning() const { return m_running; }

    // Client access
    EasClient& GetClient() { return m_client; }

protected:
    // HTTP request building
    std::string BuildRequestUrl(const std::string& command);
    void SetRequestHeaders(std::vector<std::string>& headers);

    // Response handling
    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);

    // WBXML helpers
    std::vector<unsigned char> GenerateWbxml();
    bool ParseWbxmlResponse(const std::vector<unsigned char>& data);

    // Subclass callbacks
    virtual void GenerateRequest(class WbxmlEncoder& encoder) {}
    virtual void ProcessResponse(class WbxmlParser& parser) {}

    static MojLogger s_log;

    EasClient&      m_client;
    EasCommandType::Type  m_commandType;
    MojLogger&      m_log;
    bool            m_running;
    bool            m_cancelled;
};

/**
 * EasNetworkCommand - Base class for commands that make network requests
 */
class EasNetworkCommand : public EasCommand
{
public:
    EasNetworkCommand(EasClient& client, EasCommandType::Type type, Priority priority = NormalPriority);
    virtual ~EasNetworkCommand();

    virtual void Run();

protected:
    void SendRequest();
    void SendRequest(const std::vector<unsigned char>& wbxml);
    void SendOptionsRequest();

    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);
    void HandleHttpError(int errorCode, const std::string& message);

    // Response storage (accessible by subclasses)
    std::vector<unsigned char>  m_responseBody;
    std::vector<std::string>    m_responseHeaders;

private:
    // CURL callback
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userp);
};

#endif /* EASCOMMAND_H_ */
