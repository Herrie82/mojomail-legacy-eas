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

#include "wbxml/EasTagCode.h"
#include <cstdio>

namespace EasTagCode {

// Tag name lookup table
// Format matches binary: "CodePage_TagName"
const char* GetTagName(EasTag id)
{
    unsigned char codePage = GetCodePage(id);
    unsigned char tag = GetTagNumber(id);

    // Static buffer for unknown tags
    static char unknownBuf[32];

    switch (codePage) {
        case CodePage_AirSync:
            switch (tag) {
                case 0x05: return "AirSync_Sync";
                case 0x06: return "AirSync_Collections";
                case 0x07: return "AirSync_Collection";
                case 0x08: return "AirSync_Class";
                case 0x09: return "AirSync_SyncKey";
                case 0x0A: return "AirSync_CollectionId";
                case 0x0B: return "AirSync_Status";
                case 0x0C: return "AirSync_Add";
                case 0x0D: return "AirSync_Change";
                case 0x0E: return "AirSync_Delete";
                case 0x0F: return "AirSync_Fetch";
                case 0x10: return "AirSync_Options";
                case 0x11: return "AirSync_FilterType";
                case 0x12: return "AirSync_Truncation";
                case 0x13: return "AirSync_RTFTruncation";
                case 0x14: return "AirSync_Conflict";
                case 0x15: return "AirSync_Responses";
                case 0x16: return "AirSync_ApplicationData";
                case 0x17: return "AirSync_GetChanges";
                case 0x18: return "AirSync_MoreAvailable";
                case 0x19: return "AirSync_WindowSize";
                case 0x1A: return "AirSync_Commands";
                case 0x1B: return "AirSync_ServerId";
                case 0x1C: return "AirSync_ClientId";
                case 0x1D: return "AirSync_Supported";
                case 0x1E: return "AirSync_SoftDelete";
                case 0x1F: return "AirSync_MIMESupport";
                case 0x20: return "AirSync_MIMETruncation";
                case 0x21: return "AirSync_Wait";
                case 0x22: return "AirSync_Limit";
                case 0x23: return "AirSync_Partial";
            }
            break;

        case CodePage_FolderHierarchy:
            switch (tag) {
                case 0x05: return "FolderHierarchy_Folders";
                case 0x06: return "FolderHierarchy_Folder";
                case 0x07: return "FolderHierarchy_DisplayName";
                case 0x08: return "FolderHierarchy_ServerId";
                case 0x09: return "FolderHierarchy_ParentId";
                case 0x0A: return "FolderHierarchy_Type";
                case 0x0B: return "FolderHierarchy_Response";
                case 0x0C: return "FolderHierarchy_Status";
                case 0x0D: return "FolderHierarchy_ContentClass";
                case 0x0E: return "FolderHierarchy_Changes";
                case 0x0F: return "FolderHierarchy_Add";
                case 0x10: return "FolderHierarchy_Delete";
                case 0x11: return "FolderHierarchy_Update";
                case 0x12: return "FolderHierarchy_SyncKey";
                case 0x13: return "FolderHierarchy_FolderCreate";
                case 0x14: return "FolderHierarchy_FolderDelete";
                case 0x15: return "FolderHierarchy_FolderUpdate";
                case 0x16: return "FolderHierarchy_FolderSync";
                case 0x17: return "FolderHierarchy_Count";
            }
            break;

        case CodePage_Ping:
            switch (tag) {
                case 0x05: return "Ping_Ping";
                case 0x06: return "Ping_AutdState";
                case 0x07: return "Ping_Status";
                case 0x08: return "Ping_HeartbeatInterval";
                case 0x09: return "Ping_Folders";
                case 0x0A: return "Ping_Folder";
                case 0x0B: return "Ping_Id";
                case 0x0C: return "Ping_Class";
                case 0x0D: return "Ping_MaxFolders";
            }
            break;

        case CodePage_Provision:
            switch (tag) {
                case 0x05: return "Provision_Provision";
                case 0x06: return "Provision_Policies";
                case 0x07: return "Provision_Policy";
                case 0x08: return "Provision_PolicyType";
                case 0x09: return "Provision_PolicyKey";
                case 0x0A: return "Provision_Data";
                case 0x0B: return "Provision_Status";
                case 0x0C: return "Provision_RemoteWipe";
                case 0x0D: return "Provision_EASProvisionDoc";
                case 0x0E: return "Provision_DevicePasswordEnabled";
                case 0x0F: return "Provision_AlphanumericDevicePasswordRequired";
                case 0x13: return "Provision_MinDevicePasswordLength";
                case 0x14: return "Provision_MaxInactivityTimeDeviceLock";
                case 0x15: return "Provision_MaxDevicePasswordFailedAttempts";
                case 0x18: return "Provision_AttachmentsEnabled";
                case 0x1D: return "Provision_MaxAttachmentSize";
                case 0x1F: return "Provision_AllowSimpleDevicePassword";
                case 0x20: return "Provision_DevicePasswordExpiration";
                case 0x21: return "Provision_DevicePasswordHistory";
                case 0x22: return "Provision_AllowStorageCard";
                case 0x23: return "Provision_AllowCamera";
                case 0x24: return "Provision_RequireDeviceEncryption";
                case 0x28: return "Provision_AllowBrowser";
                case 0x2D: return "Provision_AllowWiFi";
                case 0x30: return "Provision_AllowBluetooth";
            }
            break;

        case CodePage_Email:
            switch (tag) {
                case 0x05: return "Email_Attachment";
                case 0x06: return "Email_Attachments";
                case 0x0F: return "Email_DateReceived";
                case 0x11: return "Email_DisplayTo";
                case 0x12: return "Email_Importance";
                case 0x13: return "Email_MessageClass";
                case 0x14: return "Email_Subject";
                case 0x15: return "Email_Read";
                case 0x16: return "Email_To";
                case 0x17: return "Email_CC";
                case 0x18: return "Email_From";
                case 0x19: return "Email_ReplyTo";
                case 0x22: return "Email_MeetingRequest";
                case 0x34: return "Email_GlobalObjId";
                case 0x35: return "Email_ThreadTopic";
                case 0x39: return "Email_InternetCPID";
                case 0x3A: return "Email_Flag";
                case 0x3C: return "Email_ContentClass";
                case 0x3E: return "Email_CompleteTime";
            }
            break;

        case CodePage_AirSyncBase:
            switch (tag) {
                case 0x05: return "AirSyncBase_BodyPreference";
                case 0x06: return "AirSyncBase_Type";
                case 0x07: return "AirSyncBase_TruncationSize";
                case 0x08: return "AirSyncBase_AllOrNone";
                case 0x0A: return "AirSyncBase_Body";
                case 0x0B: return "AirSyncBase_Data";
                case 0x0C: return "AirSyncBase_EstimatedDataSize";
                case 0x0D: return "AirSyncBase_Truncated";
                case 0x0E: return "AirSyncBase_Attachments";
                case 0x0F: return "AirSyncBase_Attachment";
                case 0x10: return "AirSyncBase_DisplayName";
                case 0x11: return "AirSyncBase_FileReference";
                case 0x12: return "AirSyncBase_Method";
                case 0x13: return "AirSyncBase_ContentId";
                case 0x15: return "AirSyncBase_IsInline";
                case 0x16: return "AirSyncBase_NativeBodyType";
                case 0x17: return "AirSyncBase_ContentType";
            }
            break;

        case CodePage_Move:
            switch (tag) {
                case 0x05: return "Move_MoveItems";
                case 0x06: return "Move_Move";
                case 0x07: return "Move_SrcMsgId";
                case 0x08: return "Move_SrcFldId";
                case 0x09: return "Move_DstFldId";
                case 0x0A: return "Move_Response";
                case 0x0B: return "Move_Status";
                case 0x0C: return "Move_DstMsgId";
            }
            break;
    }

    // Unknown tag
    snprintf(unknownBuf, sizeof(unknownBuf), "Unknown_%02X_%02X", codePage, tag);
    return unknownBuf;
}

} // namespace EasTagCode
