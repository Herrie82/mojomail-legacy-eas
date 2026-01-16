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

#include "commands/OptionsCommand.h"
#include "EasClient.h"
#include <curl/curl.h>
#include <algorithm>
#include <sstream>

// Header names for EAS OPTIONS response
const char* OptionsCommand::PROTOCOL_VERSIONS_HEADER = "MS-ASProtocolVersions";
const char* OptionsCommand::PROTOCOL_COMMANDS_HEADER = "MS-ASProtocolCommands";
const char* OptionsCommand::SERVER_TYPE_HEADER = "MS-Server-ActiveSync";

OptionsCommand::OptionsCommand(EasClient& client)
    : EasNetworkCommand(client, EasCommandType::Options)
    , m_searchSupported(false)
{
}

OptionsCommand::~OptionsCommand()
{
}

void OptionsCommand::Run()
{
    MojLogInfo(s_log, "Running OPTIONS command");
    m_running = true;

    // Build URL for OPTIONS request
    std::string url;
    if (m_client.GetAccount()) {
        url = m_client.GetAccount()->GetServerUrl();
        // Ensure URL ends with /Microsoft-Server-ActiveSync
        if (url.find("/Microsoft-Server-ActiveSync") == std::string::npos) {
            if (!url.empty() && url[url.length() - 1] != '/') {
                url += "/";
            }
            url += "Microsoft-Server-ActiveSync";
        }
    }

    if (url.empty()) {
        Failure(MailError::INTERNAL_ERROR, "No server URL configured");
        return;
    }

    MojLogInfo(s_log, "OPTIONS URL: %s", url.c_str());

    // Initialize CURL for OPTIONS request
    CURL* curl = curl_easy_init();
    if (!curl) {
        Failure(MailError::INTERNAL_ERROR, "Failed to initialize CURL");
        return;
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set OPTIONS method
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    // We only care about headers, not body
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);

    // Set header callback
    m_responseHeaders.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    // Set authentication if we have credentials
    if (m_client.GetAccount()) {
        boost::shared_ptr<EasAccount> account = m_client.GetAccount();
        std::string credentials;
        if (!account->GetDomain().empty()) {
            credentials = account->GetDomain() + "\\";
        }
        credentials += account->GetUsername();
        curl_easy_setopt(curl, CURLOPT_USERNAME, credentials.c_str());
        // Password would come from credential manager
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_NTLM);
    }

    // SSL options
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        MojLogError(s_log, "CURL error: %s", curl_easy_strerror(res));
        Failure(MailError::CONNECTION_FAILED, curl_easy_strerror(res));
        return;
    }

    MojLogInfo(s_log, "OPTIONS response: HTTP %ld", httpCode);

    // Process response
    HandleHttpResponse(httpCode, m_responseBody);
}

void OptionsCommand::HandleHttpResponse(int statusCode,
                                        const std::vector<unsigned char>& body)
{
    (void)body;  // OPTIONS response has no body

    if (statusCode == 200) {
        // Parse the headers we received
        ParseOptionsHeaders(m_responseHeaders);

        if (m_supportedVersions.empty()) {
            MojLogError(s_log, "No protocol versions in OPTIONS response");
            Failure(MailError::INTERNAL_ERROR, "Server did not return protocol versions");
            return;
        }

        // Get the best version to use
        std::string version = GetPreferredVersion();
        MojLogInfo(s_log, "Selected protocol version: %s", version.c_str());

        // Check if search is supported
        for (size_t i = 0; i < m_supportedCommands.size(); ++i) {
            if (m_supportedCommands[i] == "Search") {
                m_searchSupported = true;
                break;
            }
        }

        // Notify client of success
        float versionFloat = static_cast<float>(atof(version.c_str()));
        m_client.OptionsSuccess(versionFloat, m_serverType, m_searchSupported);

        Complete();
    } else if (statusCode == 401) {
        Failure(MailError::BAD_USERNAME_OR_PASSWORD, "Authentication failed");
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "OPTIONS failed with HTTP %d", statusCode);
        Failure(MailError::INTERNAL_ERROR, msg);
    }
}

void OptionsCommand::ParseOptionsHeaders(const std::vector<std::string>& headers)
{
    for (size_t i = 0; i < headers.size(); ++i) {
        const std::string& header = headers[i];
        // Find colon separator
        size_t colonPos = header.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string name = header.substr(0, colonPos);
        std::string value = header.substr(colonPos + 1);

        // Trim whitespace from value
        size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos) {
            value = value.substr(start);
        }

        // Check which header this is
        if (name == PROTOCOL_VERSIONS_HEADER) {
            // Parse comma-separated list of versions
            // e.g., "2.0,2.1,2.5,12.0,12.1,14.0,14.1"
            ParseCommaSeparatedList(value, m_supportedVersions);
            MojLogInfo(s_log, "Server supports %zu protocol versions", m_supportedVersions.size());
        } else if (name == PROTOCOL_COMMANDS_HEADER) {
            // Parse comma-separated list of commands
            ParseCommaSeparatedList(value, m_supportedCommands);
            MojLogInfo(s_log, "Server supports %zu commands", m_supportedCommands.size());
        } else if (name == SERVER_TYPE_HEADER) {
            m_serverType = value;
            MojLogInfo(s_log, "Server type: %s", m_serverType.c_str());
        }
    }
}

void OptionsCommand::ParseCommaSeparatedList(const std::string& value,
                                             std::vector<std::string>& list)
{
    std::stringstream ss(value);
    std::string item;

    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            list.push_back(item.substr(start, end - start + 1));
        }
    }
}

std::string OptionsCommand::GetPreferredVersion() const
{
    // Preferred versions in order (highest to lowest we support)
    // 16.1 = Exchange 2016 CU3+, 16.0 = Exchange 2016
    // 14.1 = Exchange 2010 SP1, 14.0 = Exchange 2010
    // 12.1 = Exchange 2007 SP1, 12.0 = Exchange 2007, 2.5 = Exchange 2003
    static const char* preferredVersions[] = {
        "16.1", "16.0", "14.1", "14.0", "12.1", "12.0", "2.5"
    };

    for (size_t p = 0; p < sizeof(preferredVersions)/sizeof(preferredVersions[0]); ++p) {
        for (size_t s = 0; s < m_supportedVersions.size(); ++s) {
            if (m_supportedVersions[s] == preferredVersions[p]) {
                return m_supportedVersions[s];
            }
        }
    }

    // Fall back to highest version server supports
    if (!m_supportedVersions.empty()) {
        return m_supportedVersions[m_supportedVersions.size() - 1];
    }

    return "12.1";  // Default fallback
}

size_t OptionsCommand::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userp)
{
    size_t totalSize = size * nitems;
    OptionsCommand* cmd = static_cast<OptionsCommand*>(userp);

    std::string header(buffer, totalSize);

    // Remove trailing newlines
    while (!header.empty() && (header[header.length()-1] == '\r' || header[header.length()-1] == '\n')) {
        header.erase(header.length()-1);
    }

    if (!header.empty()) {
        cmd->m_responseHeaders.push_back(header);
        MojLogDebug(s_log, "Header: %s", header.c_str());
    }

    return totalSize;
}
