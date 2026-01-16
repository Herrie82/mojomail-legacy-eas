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

#include "commands/EasCommand.h"
#include "EasClient.h"
#include "wbxml/WbxmlEncoder.h"
#include "wbxml/WbxmlParser.h"
#include <curl/curl.h>

MojLogger EasCommand::s_log("com.palm.eas.command");

const std::string EasCommand::UNKNOWN_COMMAND_TYPE = "UnknownCommand";

// Command type names (matches binary's commandTypeNames array)
static const char* s_commandTypeNames[] = {
    "Unknown",
    "Sync",
    "SendMail",
    "SmartForward",
    "SmartReply",
    "GetAttachment",
    "FolderSync",
    "FolderCreate",
    "FolderDelete",
    "FolderUpdate",
    "MoveItems",
    "GetItemEstimate",
    "MeetingResponse",
    "Search",
    "Settings",
    "Ping",
    "ItemOperations",
    "Provision",
    "ResolveRecipients",
    "ValidateCert",
    "Options"
};

EasCommand::EasCommand(EasClient& client, EasCommandType::Type type, Priority priority)
    : Command(client, priority)
    , m_client(client)
    , m_commandType(type)
    , m_log(s_log)
    , m_running(false)
    , m_cancelled(false)
{
}

std::string EasCommand::Describe() const
{
    return GetCommandName();
}

EasCommand::~EasCommand()
{
}

std::string EasCommand::GetCommandName() const
{
    int typeIndex = static_cast<int>(m_commandType);
    if (typeIndex >= 0 && typeIndex < sizeof(s_commandTypeNames) / sizeof(s_commandTypeNames[0])) {
        return s_commandTypeNames[typeIndex];
    }
    return UNKNOWN_COMMAND_TYPE;
}

void EasCommand::Cancel()
{
    MojLogInfo(m_log, "Cancelling command: %s", GetCommandName().c_str());
    m_cancelled = true;
}

void EasCommand::Complete()
{
    MojLogInfo(m_log, "Command complete: %s", GetCommandName().c_str());
    m_running = false;
    Command::Complete();  // Calls listener's CommandComplete
}

void EasCommand::Failure(MailError::ErrorCode error, const std::string& message)
{
    MojLogError(m_log, "Command failed: %s - %s (error %d)",
                GetCommandName().c_str(), message.c_str(), error);
    m_running = false;
    // Notify client of failure
    // m_client.CommandFailure(this);
}

void EasCommand::HandleHttpResponse(int statusCode, const std::vector<unsigned char>& body)
{
    // Default implementation - subclasses should override
    (void)statusCode;
    (void)body;
}

std::string EasCommand::BuildRequestUrl(const std::string& command)
{
    // Build EAS command URL:
    // https://server/Microsoft-Server-ActiveSync?Cmd=<command>&User=<user>&DeviceId=<id>&DeviceType=<type>

    std::string url;

    if (m_client.GetAccount()) {
        boost::shared_ptr<EasAccount> account = m_client.GetAccount();
        url = account->GetServerUrl();

        // Ensure URL ends with /Microsoft-Server-ActiveSync
        if (url.find("/Microsoft-Server-ActiveSync") == std::string::npos) {
            if (url[url.length() - 1] != '/') {
                url += "/";
            }
            url += "Microsoft-Server-ActiveSync";
        }

        // Add query parameters
        url += "?Cmd=" + command;
        url += "&User=" + account->GetUsername();
        url += "&DeviceId=" + account->GetDeviceId();
        url += "&DeviceType=" + account->GetDeviceType();
    }

    return url;
}

void EasCommand::SetRequestHeaders(std::vector<std::string>& headers)
{
    // Set common EAS headers
    headers.push_back("Content-Type: application/vnd.ms-sync.wbxml");

    // Protocol version
    if (!m_client.GetProtocolVersion().empty()) {
        headers.push_back("MS-ASProtocolVersion: " + m_client.GetProtocolVersion());
    }

    // Policy key (if we have one)
    if (!m_client.GetPolicyKey().empty()) {
        headers.push_back("X-MS-PolicyKey: " + m_client.GetPolicyKey());
    }

    // User agent
    headers.push_back("User-Agent: TouchPad/1.0");
}

// ============================================================================
// EasNetworkCommand implementation
// ============================================================================

EasNetworkCommand::EasNetworkCommand(EasClient& client, EasCommandType::Type type, Priority priority)
    : EasCommand(client, type, priority)
{
}

EasNetworkCommand::~EasNetworkCommand()
{
}

void EasNetworkCommand::Run()
{
    MojLogInfo(m_log, "Running network command: %s", GetCommandName().c_str());
    m_running = true;

    // Generate WBXML request
    WbxmlEncoder encoder;
    GenerateRequest(encoder);

    // Send request
    SendRequest(encoder.GetWbxml());
}

void EasNetworkCommand::SendRequest(const std::vector<unsigned char>& wbxml)
{
    MojLogInfo(m_log, "Sending request: %zu bytes", wbxml.size());

    // Build URL
    std::string url = BuildRequestUrl(GetCommandName());
    MojLogDebug(m_log, "Request URL: %s", url.c_str());

    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        Failure(MailError::INTERNAL_ERROR, "Failed to initialize CURL");
        return;
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set POST data
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wbxml.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, wbxml.size());

    // Set headers
    struct curl_slist* headers = NULL;
    std::vector<std::string> headerList;
    SetRequestHeaders(headerList);
    for (size_t i = 0; i < headerList.size(); ++i) {
        headers = curl_slist_append(headers, headerList[i].c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set callbacks
    m_responseBody.clear();
    m_responseHeaders.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    // Set authentication
    if (m_client.GetAccount()) {
        // Basic auth - domain\username
        std::string credentials;
        boost::shared_ptr<EasAccount> account = m_client.GetAccount();
        if (!account->GetDomain().empty()) {
            credentials = account->GetDomain() + "\\";
        }
        credentials += account->GetUsername();
        curl_easy_setopt(curl, CURLOPT_USERNAME, credentials.c_str());
        // Password retrieved separately via credential manager
        // curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_NTLM);
    }

    // SSL options
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        MojLogError(m_log, "CURL error: %s", curl_easy_strerror(res));
        HandleHttpError(res, curl_easy_strerror(res));
        return;
    }

    // Process response
    HandleHttpResponse(httpCode, m_responseBody);
}

void EasNetworkCommand::SendOptionsRequest()
{
    MojLogInfo(m_log, "Sending OPTIONS request");

    // OPTIONS doesn't send body, just checks server capabilities
    std::string url;
    if (m_client.GetAccount()) {
        url = m_client.GetAccount()->GetServerUrl();
        if (url.find("/Microsoft-Server-ActiveSync") == std::string::npos) {
            if (url[url.length() - 1] != '/') {
                url += "/";
            }
            url += "Microsoft-Server-ActiveSync";
        }
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        Failure(MailError::INTERNAL_ERROR, "Failed to initialize CURL");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    // Set callbacks for headers
    m_responseHeaders.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        HandleHttpError(res, curl_easy_strerror(res));
        return;
    }

    // Process headers
    HandleHttpResponse(httpCode, m_responseBody);
}

void EasNetworkCommand::HandleHttpResponse(int statusCode,
                                           const std::vector<unsigned char>& body)
{
    MojLogInfo(m_log, "HTTP response: %d, body size: %zu", statusCode, body.size());

    if (statusCode == 200) {
        if (!body.empty()) {
            // Parse WBXML response
            // Create response handler and parse
            // ProcessResponse() will be called by subclass
        }
    } else if (statusCode == 401) {
        Failure(MailError::BAD_USERNAME_OR_PASSWORD, "Authentication failed");
    } else if (statusCode == 449) {
        // Need to provision
        MojLogInfo(m_log, "Server requires provisioning");
        // m_client.SetState(EasClient::State_NeedsProvision);
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "HTTP error %d", statusCode);
        Failure(MailError::INTERNAL_ERROR, msg);
    }
}

void EasNetworkCommand::HandleHttpError(int errorCode, const std::string& message)
{
    MojLogError(m_log, "HTTP error: %d - %s", errorCode, message.c_str());
    Failure(MailError::CONNECTION_FAILED, message);
}

size_t EasNetworkCommand::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    EasNetworkCommand* cmd = static_cast<EasNetworkCommand*>(userp);

    const unsigned char* data = static_cast<const unsigned char*>(contents);
    cmd->m_responseBody.insert(cmd->m_responseBody.end(), data, data + totalSize);

    return totalSize;
}

size_t EasNetworkCommand::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userp)
{
    size_t totalSize = size * nitems;
    EasNetworkCommand* cmd = static_cast<EasNetworkCommand*>(userp);

    std::string header(buffer, totalSize);
    // Remove trailing newlines
    while (!header.empty() && (header[header.length()-1] == '\r' || header[header.length()-1] == '\n')) {
        header.erase(header.length()-1);
    }

    if (!header.empty()) {
        cmd->m_responseHeaders.push_back(header);
    }

    return totalSize;
}
