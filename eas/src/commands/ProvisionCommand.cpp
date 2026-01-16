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

#include "commands/ProvisionCommand.h"
#include "EasClient.h"
#include "wbxml/WbxmlEncoder.h"
#include "wbxml/WbxmlParser.h"
#include "wbxml/EasTagCode.h"
#include "data/EasPolicy.h"

// Policy type string for MS-EAS-Provisioning-WBXML
static const char* POLICY_TYPE_WBXML = "MS-EAS-Provisioning-WBXML";

/**
 * Response handler for parsing Provision WBXML response
 */
class ProvisionCommand::ResponseHandler : public WbxmlContentHandler
{
public:
    ResponseHandler(ProvisionCommand& cmd)
        : m_cmd(cmd)
        , m_inPolicy(false)
        , m_inEASProvisionDoc(false)
        , m_currentTag(0)
    {
    }

    void StartElement(EasTagCode::EasTag tag)
    {
        m_currentTag = tag;

        switch (tag) {
            case EasTagCode::Provision_Policy:
                m_inPolicy = true;
                break;
            case EasTagCode::Provision_EASProvisionDoc:
                m_inEASProvisionDoc = true;
                break;
            case EasTagCode::Provision_RemoteWipe:
                m_cmd.m_remoteWipeRequired = true;
                break;
            default:
                break;
        }
    }

    void EndElement(EasTagCode::EasTag tag)
    {
        switch (tag) {
            case EasTagCode::Provision_Policy:
                m_inPolicy = false;
                break;
            case EasTagCode::Provision_EASProvisionDoc:
                m_inEASProvisionDoc = false;
                break;
            default:
                break;
        }
        m_currentTag = 0;
    }

    void Text(const std::string& text)
    {
        switch (m_currentTag) {
            case EasTagCode::Provision_Status:
                if (m_inPolicy) {
                    m_cmd.m_policyStatus = static_cast<ProvisionCommand::PolicyStatus>(atoi(text.c_str()));
                } else {
                    m_cmd.m_status = static_cast<ProvisionCommand::ProvisionStatus>(atoi(text.c_str()));
                }
                break;

            case EasTagCode::Provision_PolicyKey:
                if (m_cmd.m_phase == ProvisionCommand::Phase_Initial) {
                    m_cmd.m_tempPolicyKey = text;
                } else {
                    m_cmd.m_policyKey = text;
                }
                break;

            // Policy settings within EASProvisionDoc
            case EasTagCode::Provision_DevicePasswordEnabled:
                m_cmd.m_policy.SetDevicePasswordEnabled(text == "1");
                break;
            case EasTagCode::Provision_AlphanumericDevicePasswordRequired:
                m_cmd.m_policy.SetAlphanumericPasswordRequired(text == "1");
                break;
            case EasTagCode::Provision_MinDevicePasswordLength:
                m_cmd.m_policy.SetMinPasswordLength(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_MaxInactivityTimeDeviceLock:
                m_cmd.m_policy.SetMaxInactivityTimeout(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_MaxDevicePasswordFailedAttempts:
                m_cmd.m_policy.SetMaxPasswordFailedAttempts(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_AllowSimpleDevicePassword:
                m_cmd.m_policy.SetAllowSimplePassword(text == "1");
                break;
            case EasTagCode::Provision_DevicePasswordExpiration:
                m_cmd.m_policy.SetPasswordExpiration(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_DevicePasswordHistory:
                m_cmd.m_policy.SetPasswordHistory(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_AttachmentsEnabled:
                m_cmd.m_policy.SetAttachmentsEnabled(text == "1");
                break;
            case EasTagCode::Provision_MaxAttachmentSize:
                m_cmd.m_policy.SetMaxAttachmentSize(atoi(text.c_str()));
                break;
            case EasTagCode::Provision_RequireDeviceEncryption:
                m_cmd.m_policy.SetDeviceEncryptionEnabled(text == "1");
                break;
            case EasTagCode::Provision_AllowStorageCard:
                // Not directly mapped in our policy
                break;
            case EasTagCode::Provision_AllowCamera:
                m_cmd.m_policy.SetAllowCamera(text == "1");
                break;
            case EasTagCode::Provision_AllowBrowser:
                m_cmd.m_policy.SetAllowBrowser(text == "1");
                break;
            case EasTagCode::Provision_AllowWiFi:
                m_cmd.m_policy.SetAllowWiFi(text == "1");
                break;
            case EasTagCode::Provision_AllowBluetooth:
                m_cmd.m_policy.SetAllowBluetooth(text == "1");
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
    ProvisionCommand&       m_cmd;
    bool                    m_inPolicy;
    bool                    m_inEASProvisionDoc;
    EasTagCode::EasTag      m_currentTag;
};

ProvisionCommand::ProvisionCommand(EasClient& client)
    : EasNetworkCommand(client, EasCommandType::Provision)
    , m_phase(Phase_Initial)
    , m_status(Status_Success)
    , m_policyStatus(PolicyStatus_Success)
    , m_remoteWipeRequired(false)
{
}

ProvisionCommand::~ProvisionCommand()
{
}

void ProvisionCommand::GenerateRequest(WbxmlEncoder& encoder)
{
    // Not used - ProvisionCommand uses Run() with phase-specific methods
    (void)encoder;
}

void ProvisionCommand::ProcessResponse(WbxmlParser& parser)
{
    // Not used - ProvisionCommand uses HandleHttpResponse
    (void)parser;
}

void ProvisionCommand::Run()
{
    MojLogInfo(s_log, "Running Provision command, phase: %d", static_cast<int>(m_phase));
    m_running = true;

    // Generate appropriate request based on phase
    WbxmlEncoder encoder;

    switch (m_phase) {
        case Phase_Initial:
            GenerateInitialRequest(encoder);
            break;
        case Phase_Acknowledge:
            GenerateAcknowledgeRequest(encoder);
            break;
        case Phase_Complete:
            Complete();
            return;
    }

    // Send request
    SendRequest(encoder.GetWbxml());
}

void ProvisionCommand::GenerateInitialRequest(WbxmlEncoder& encoder)
{
    MojLogInfo(s_log, "Generating initial provision request");

    // Phase 1: Request policy from server
    // <Provision>
    //   <Policies>
    //     <Policy>
    //       <PolicyType>MS-EAS-Provisioning-WBXML</PolicyType>
    //     </Policy>
    //   </Policies>
    // </Provision>

    encoder.StartTag(EasTagCode::Provision_Provision);
    encoder.StartTag(EasTagCode::Provision_Policies);
    encoder.StartTag(EasTagCode::Provision_Policy);
    encoder.Text(EasTagCode::Provision_PolicyType, POLICY_TYPE_WBXML);
    encoder.EndTag(EasTagCode::Provision_Policy);
    encoder.EndTag(EasTagCode::Provision_Policies);
    encoder.EndTag(EasTagCode::Provision_Provision);
}

void ProvisionCommand::GenerateAcknowledgeRequest(WbxmlEncoder& encoder)
{
    MojLogInfo(s_log, "Generating acknowledge provision request with tempKey: %s",
               m_tempPolicyKey.c_str());

    // Phase 2: Acknowledge policy
    // <Provision>
    //   <Policies>
    //     <Policy>
    //       <PolicyType>MS-EAS-Provisioning-WBXML</PolicyType>
    //       <PolicyKey>temp_key_from_phase1</PolicyKey>
    //       <Status>1</Status>
    //     </Policy>
    //   </Policies>
    // </Provision>

    encoder.StartTag(EasTagCode::Provision_Provision);
    encoder.StartTag(EasTagCode::Provision_Policies);
    encoder.StartTag(EasTagCode::Provision_Policy);
    encoder.Text(EasTagCode::Provision_PolicyType, POLICY_TYPE_WBXML);
    encoder.Text(EasTagCode::Provision_PolicyKey, m_tempPolicyKey);
    encoder.TextInt(EasTagCode::Provision_Status, 1);  // 1 = Success/Compliant
    encoder.EndTag(EasTagCode::Provision_Policy);
    encoder.EndTag(EasTagCode::Provision_Policies);
    encoder.EndTag(EasTagCode::Provision_Provision);
}

void ProvisionCommand::GenerateRemoteWipeAck(WbxmlEncoder& encoder)
{
    MojLogInfo(s_log, "Generating remote wipe acknowledgement");

    // <Provision>
    //   <RemoteWipe>
    //     <Status>1</Status>
    //   </RemoteWipe>
    // </Provision>

    encoder.StartTag(EasTagCode::Provision_Provision);
    encoder.StartTag(EasTagCode::Provision_RemoteWipe);
    encoder.TextInt(EasTagCode::Provision_Status, 1);
    encoder.EndTag(EasTagCode::Provision_RemoteWipe);
    encoder.EndTag(EasTagCode::Provision_Provision);
}

void ProvisionCommand::HandleHttpResponse(int statusCode,
                                          const std::vector<unsigned char>& body)
{
    MojLogInfo(s_log, "Provision HTTP response: %d, body size: %zu",
               statusCode, body.size());

    if (statusCode != 200) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Provision failed with HTTP %d", statusCode);
        Failure(MailError::INTERNAL_ERROR, msg);
        return;
    }

    if (body.empty()) {
        Failure(MailError::INTERNAL_ERROR, "Empty provision response");
        return;
    }

    // Parse WBXML response
    ResponseHandler handler(*this);
    WbxmlParser parser(handler);

    if (!parser.HandleInput(reinterpret_cast<const char*>(body.data()),
                            static_cast<unsigned int>(body.size()))) {
        Failure(MailError::INTERNAL_ERROR, "Failed to parse provision response");
        return;
    }

    // Process based on phase
    ProcessProvisionResponse();
}

void ProvisionCommand::ProcessProvisionResponse()
{
    MojLogInfo(s_log, "Processing provision response: status=%d, policyStatus=%d, phase=%d",
               static_cast<int>(m_status), static_cast<int>(m_policyStatus),
               static_cast<int>(m_phase));

    // Check for remote wipe request
    if (m_remoteWipeRequired) {
        MojLogWarning(s_log, "Server requested remote wipe!");
        // In a real implementation, we would wipe the device here
        // For now, just fail
        Failure(MailError::INTERNAL_ERROR, "Remote wipe requested");
        return;
    }

    // Check provision status
    if (m_status != Status_Success) {
        MojLogError(s_log, "Provision failed with status %d", static_cast<int>(m_status));
        Failure(MailError::INTERNAL_ERROR, "Provisioning failed");
        return;
    }

    switch (m_phase) {
        case Phase_Initial:
            // Got policy from server, now need to acknowledge
            if (m_tempPolicyKey.empty()) {
                MojLogError(s_log, "No policy key received in initial response");
                Failure(MailError::INTERNAL_ERROR, "No policy key received");
                return;
            }

            MojLogInfo(s_log, "Received temp policy key: %s", m_tempPolicyKey.c_str());

            // Check policy compliance
            CheckPolicyCompliance();

            // Move to acknowledge phase
            m_phase = Phase_Acknowledge;
            Run();
            break;

        case Phase_Acknowledge:
            // Got final policy key
            if (m_policyKey.empty()) {
                MojLogError(s_log, "No final policy key received");
                Failure(MailError::INTERNAL_ERROR, "No final policy key received");
                return;
            }

            MojLogInfo(s_log, "Provision successful, policy key: %s", m_policyKey.c_str());

            // Save policy to database
            SavePolicyToDatabase();

            // Notify client
            m_client.ProvisionSuccess(m_policyKey);

            m_phase = Phase_Complete;
            Complete();
            break;

        case Phase_Complete:
            Complete();
            break;
    }
}

void ProvisionCommand::CheckPolicyCompliance()
{
    MojLogInfo(s_log, "Checking policy compliance");

    // Log policy requirements
    if (m_policy.IsDevicePasswordEnabled()) {
        MojLogInfo(s_log, "Policy requires device password");
        if (m_policy.GetMinPasswordLength() > 0) {
            MojLogInfo(s_log, "  Min length: %d", m_policy.GetMinPasswordLength());
        }
        if (m_policy.IsAlphanumericPasswordRequired()) {
            MojLogInfo(s_log, "  Alphanumeric required");
        }
    }

    if (m_policy.IsDeviceEncryptionEnabled()) {
        MojLogInfo(s_log, "Policy requires device encryption");
    }

    // In a full implementation, we would check if the device meets
    // the policy requirements and potentially prompt the user
    // For now, we just acknowledge compliance
}

void ProvisionCommand::SavePolicyToDatabase()
{
    MojLogInfo(s_log, "Saving policy to database");

    // Convert policy to MojObject
    MojObject policyObj;
    m_policy.ToDatabase(policyObj);

    // Add account ID
    policyObj.put("accountId", m_client.GetAccountId());

    // TODO: Actually save to db8
    // m_dbInterface->Put(policyObj, ...);
}
