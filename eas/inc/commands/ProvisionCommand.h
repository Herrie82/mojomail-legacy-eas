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

#ifndef PROVISIONCOMMAND_H_
#define PROVISIONCOMMAND_H_

#include "commands/EasCommand.h"
#include "data/EasPolicy.h"

/**
 * ProvisionCommand - Device security policy provisioning
 *
 * Exchange servers can require devices to acknowledge security
 * policies before allowing sync. This command retrieves and
 * acknowledges those policies.
 *
 * Protocol: [MS-ASCMD] Section 2.2.1.14
 *           [MS-ASPROV] Provisioning Protocol
 */
class ProvisionCommand : public EasNetworkCommand
{
public:
    // Provision phases
    enum Phase {
        Phase_Initial,          // Get policy from server
        Phase_Acknowledge,      // Acknowledge policy
        Phase_Complete
    };

    // Provision status codes
    enum ProvisionStatus {
        Status_Success          = 1,
        Status_SyntaxError      = 2,
        Status_ServerError      = 3,
        Status_DeviceExternal   = 4,  // Device managed externally
        Status_PolicyCorrupt    = 5
    };

    // Policy status codes (for acknowledgment)
    enum PolicyStatus {
        PolicyStatus_Success    = 1,
        PolicyStatus_NoPolicyDefined = 2,
        PolicyStatus_PolicyKeyInvalid = 3,
        PolicyStatus_PartialSuccess = 4
    };

    ProvisionCommand(EasClient& client);
    virtual ~ProvisionCommand();

    // Get results
    const std::string& GetPolicyKey() const { return m_policyKey; }
    const EasPolicy& GetPolicy() const { return m_policy; }
    bool IsRemoteWipeRequired() const { return m_remoteWipeRequired; }

    virtual void Run();

protected:
    virtual void GenerateRequest(class WbxmlEncoder& encoder);
    virtual void ProcessResponse(class WbxmlParser& parser);
    virtual void HandleHttpResponse(int statusCode,
                                    const std::vector<unsigned char>& body);

    // Response parsing state
    class ResponseHandler;

private:
    void GenerateInitialRequest(class WbxmlEncoder& encoder);
    void GenerateAcknowledgeRequest(class WbxmlEncoder& encoder);
    void GenerateRemoteWipeAck(class WbxmlEncoder& encoder);

    void ProcessProvisionResponse();
    void ProcessPolicyResponse();

    void SavePolicyToDatabase();
    void CheckPolicyCompliance();

    Phase               m_phase;
    ProvisionStatus     m_status;
    PolicyStatus        m_policyStatus;
    std::string         m_policyKey;
    std::string         m_tempPolicyKey;
    EasPolicy           m_policy;
    bool                m_remoteWipeRequired;
};

#endif /* PROVISIONCOMMAND_H_ */
