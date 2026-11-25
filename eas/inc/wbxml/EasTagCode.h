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

#ifndef EASTAGCODE_H_
#define EASTAGCODE_H_

#include <string>

/**
 * EasTagCode - WBXML tag codes for EAS protocol
 *
 * Tags are represented as 16-bit values combining code page and tag number.
 * Format: (codePage << 8) | tagNumber
 *
 * This matches the original binary's implementation at EasTagCode::BuildId()
 */
namespace EasTagCode {

    // Tag type (16-bit: high byte = code page, low byte = tag)
    typedef unsigned short EasTag;

    // Build a tag ID from code page and tag number
    inline EasTag BuildId(unsigned char codePage, unsigned char tag) {
        return (static_cast<EasTag>(codePage) << 8) | tag;
    }

    // Parse a tag ID into code page and tag number
    inline void ParseId(EasTag id, unsigned char& codePage, unsigned char& tag) {
        codePage = static_cast<unsigned char>(id >> 8);
        tag = static_cast<unsigned char>(id & 0xFF);
    }

    // Get code page from tag ID
    inline unsigned char GetCodePage(EasTag id) {
        return static_cast<unsigned char>(id >> 8);
    }

    // Get tag number from tag ID
    inline unsigned char GetTagNumber(EasTag id) {
        return static_cast<unsigned char>(id & 0xFF);
    }

    // Get tag name for debugging
    const char* GetTagName(EasTag id);

    // ========================================================================
    // Code Pages
    // ========================================================================

    const unsigned char CodePage_AirSync         = 0x00;
    const unsigned char CodePage_Contacts        = 0x01;
    const unsigned char CodePage_Email           = 0x02;
    const unsigned char CodePage_Calendar        = 0x04;
    const unsigned char CodePage_Move            = 0x05;
    const unsigned char CodePage_ItemEstimate    = 0x06;
    const unsigned char CodePage_FolderHierarchy = 0x07;
    const unsigned char CodePage_MeetingResponse = 0x08;
    const unsigned char CodePage_Tasks           = 0x09;
    const unsigned char CodePage_ResolveRecipients = 0x0A;
    const unsigned char CodePage_ValidateCert    = 0x0B;
    const unsigned char CodePage_Contacts2       = 0x0C;
    const unsigned char CodePage_Ping            = 0x0D;
    const unsigned char CodePage_Provision       = 0x0E;
    const unsigned char CodePage_Search          = 0x0F;
    const unsigned char CodePage_GAL             = 0x10;
    const unsigned char CodePage_AirSyncBase     = 0x11;
    const unsigned char CodePage_Settings        = 0x12;
    const unsigned char CodePage_DocumentLibrary = 0x13;
    const unsigned char CodePage_ItemOperations  = 0x14;
    const unsigned char CodePage_ComposeMail     = 0x15;
    const unsigned char CodePage_Email2          = 0x16;
    const unsigned char CodePage_Notes           = 0x17;
    const unsigned char CodePage_RightsManagement = 0x18;  // EAS 14.1+
    const unsigned char CodePage_Find            = 0x19;   // EAS 16.0+

    // ========================================================================
    // AirSync Tags (Code Page 0x00)
    // Using enum for compile-time constants usable in switch statements
    // ========================================================================

    enum {
        AirSync_Sync           = 0x0005,
        AirSync_Collections    = 0x0006,
        AirSync_Collection     = 0x0007,
        AirSync_Class          = 0x0008,
        AirSync_SyncKey        = 0x0009,
        AirSync_CollectionId   = 0x000A,
        AirSync_Status         = 0x000B,
        AirSync_Add            = 0x000C,
        AirSync_Change         = 0x000D,
        AirSync_Delete         = 0x000E,
        AirSync_Fetch          = 0x000F,
        AirSync_Options        = 0x0010,
        AirSync_FilterType     = 0x0011,
        AirSync_Truncation     = 0x0012,
        AirSync_RTFTruncation  = 0x0013,
        AirSync_Conflict       = 0x0014,
        AirSync_Responses      = 0x0015,
        AirSync_ApplicationData = 0x0016,
        AirSync_GetChanges     = 0x0017,
        AirSync_MoreAvailable  = 0x0018,
        AirSync_WindowSize     = 0x0019,
        AirSync_Commands       = 0x001A,
        AirSync_ServerId       = 0x001B,
        AirSync_ClientId       = 0x001C,
        AirSync_Supported      = 0x001D,
        AirSync_SoftDelete     = 0x001E,
        AirSync_MIMESupport    = 0x001F,
        AirSync_MIMETruncation = 0x0020,
        AirSync_Wait           = 0x0021,
        AirSync_Limit          = 0x0022,
        AirSync_Partial        = 0x0023
    };

    // ========================================================================
    // FolderHierarchy Tags (Code Page 0x07)
    // ========================================================================

    enum {
        FolderHierarchy_Folders      = 0x0705,
        FolderHierarchy_Folder       = 0x0706,
        FolderHierarchy_DisplayName  = 0x0707,
        FolderHierarchy_ServerId     = 0x0708,
        FolderHierarchy_ParentId     = 0x0709,
        FolderHierarchy_Type         = 0x070A,
        FolderHierarchy_Response     = 0x070B,
        FolderHierarchy_Status       = 0x070C,
        FolderHierarchy_ContentClass = 0x070D,
        FolderHierarchy_Changes      = 0x070E,
        FolderHierarchy_Add          = 0x070F,
        FolderHierarchy_Delete       = 0x0710,
        FolderHierarchy_Update       = 0x0711,
        FolderHierarchy_SyncKey      = 0x0712,
        FolderHierarchy_FolderCreate = 0x0713,
        FolderHierarchy_FolderDelete = 0x0714,
        FolderHierarchy_FolderUpdate = 0x0715,
        FolderHierarchy_FolderSync   = 0x0716,
        FolderHierarchy_Count        = 0x0717
    };

    // ========================================================================
    // Ping Tags (Code Page 0x0D)
    // ========================================================================

    enum {
        Ping_Ping              = 0x0D05,
        Ping_AutdState         = 0x0D06,
        Ping_Status            = 0x0D07,
        Ping_HeartbeatInterval = 0x0D08,
        Ping_Folders           = 0x0D09,
        Ping_Folder            = 0x0D0A,
        Ping_Id                = 0x0D0B,
        Ping_Class             = 0x0D0C,
        Ping_MaxFolders        = 0x0D0D
    };

    // ========================================================================
    // Provision Tags (Code Page 0x0E)
    // ========================================================================

    enum {
        Provision_Provision    = 0x0E05,
        Provision_Policies     = 0x0E06,
        Provision_Policy       = 0x0E07,
        Provision_PolicyType   = 0x0E08,
        Provision_PolicyKey    = 0x0E09,
        Provision_Data         = 0x0E0A,
        Provision_Status       = 0x0E0B,
        Provision_RemoteWipe   = 0x0E0C,
        Provision_EASProvisionDoc = 0x0E0D,
        Provision_DevicePasswordEnabled = 0x0E0E,
        Provision_AlphanumericDevicePasswordRequired = 0x0E0F,
        Provision_MinDevicePasswordLength = 0x0E13,
        Provision_MaxInactivityTimeDeviceLock = 0x0E14,
        Provision_MaxDevicePasswordFailedAttempts = 0x0E15,
        Provision_AttachmentsEnabled = 0x0E18,
        Provision_MaxAttachmentSize = 0x0E1D,
        Provision_AllowSimpleDevicePassword = 0x0E1F,
        Provision_DevicePasswordExpiration = 0x0E20,
        Provision_DevicePasswordHistory = 0x0E21,
        Provision_AllowStorageCard = 0x0E22,
        Provision_AllowCamera  = 0x0E23,
        Provision_RequireDeviceEncryption = 0x0E24,
        Provision_AllowBrowser = 0x0E28,
        Provision_AllowWiFi    = 0x0E2D,
        Provision_AllowBluetooth = 0x0E30
    };

    // ========================================================================
    // AirSyncBase Tags (Code Page 0x11)
    // ========================================================================

    enum {
        AirSyncBase_BodyPreference = 0x1105,
        AirSyncBase_Type       = 0x1106,
        AirSyncBase_TruncationSize = 0x1107,
        AirSyncBase_AllOrNone  = 0x1108,
        AirSyncBase_Body       = 0x110A,
        AirSyncBase_Data       = 0x110B,
        AirSyncBase_EstimatedDataSize = 0x110C,
        AirSyncBase_Truncated  = 0x110D,
        AirSyncBase_Attachments = 0x110E,
        AirSyncBase_Attachment = 0x110F,
        AirSyncBase_DisplayName = 0x1110,
        AirSyncBase_FileReference = 0x1111,
        AirSyncBase_Method     = 0x1112,
        AirSyncBase_ContentId  = 0x1113,
        AirSyncBase_ContentLocation = 0x1114,
        AirSyncBase_IsInline   = 0x1115,
        AirSyncBase_NativeBodyType = 0x1116,
        AirSyncBase_ContentType = 0x1117
    };

    // ========================================================================
    // Email Tags (Code Page 0x02)
    // ========================================================================

    enum {
        Email_Attachment       = 0x0205,
        Email_Attachments      = 0x0206,
        Email_AttName          = 0x0207,
        Email_AttSize          = 0x0208,
        Email_Att0Id           = 0x0209,
        Email_AttMethod        = 0x020A,
        Email_AttRemoved       = 0x020B,
        Email_Body             = 0x020C,
        Email_BodySize         = 0x020D,
        Email_BodyTruncated    = 0x020E,
        Email_DateReceived     = 0x020F,
        Email_DisplayName      = 0x0210,
        Email_DisplayTo        = 0x0211,
        Email_Importance       = 0x0212,
        Email_MessageClass     = 0x0213,
        Email_Subject          = 0x0214,
        Email_Read             = 0x0215,
        Email_To               = 0x0216,
        Email_CC               = 0x0217,
        Email_From             = 0x0218,
        Email_ReplyTo          = 0x0219,
        Email_AllDayEvent      = 0x021A,
        Email_Categories       = 0x021B,
        Email_Category         = 0x021C,
        Email_DTStamp          = 0x021D,
        Email_EndTime          = 0x021E,
        Email_InstanceType     = 0x021F,
        Email_IntDBusyStatus   = 0x0220,
        Email_Location         = 0x0221,
        Email_MeetingRequest   = 0x0222,
        Email_Organizer        = 0x0223,
        Email_RecurrenceId     = 0x0224,
        Email_Reminder         = 0x0225,
        Email_ResponseRequested = 0x0226,
        Email_Recurrences      = 0x0227,
        Email_Recurrence       = 0x0228,
        Email_Sensitivity      = 0x0232,
        Email_TimeZone         = 0x0233,
        Email_GlobalObjId      = 0x0234,
        Email_ThreadTopic      = 0x0235,
        Email_MIMEData         = 0x0236,
        Email_MIMETruncated    = 0x0237,
        Email_MIMESize         = 0x0238,
        Email_InternetCPID     = 0x0239,
        Email_Flag             = 0x023A,
        Email_FlagStatus       = 0x023B,
        Email_ContentClass     = 0x023C,
        Email_FlagType         = 0x023D,
        Email_CompleteTime     = 0x023E
    };

    // ========================================================================
    // Move Tags (Code Page 0x05)
    // ========================================================================

    enum {
        Move_MoveItems         = 0x0505,
        Move_Move              = 0x0506,
        Move_SrcMsgId          = 0x0507,
        Move_SrcFldId          = 0x0508,
        Move_DstFldId          = 0x0509,
        Move_Response          = 0x050A,
        Move_Status            = 0x050B,
        Move_DstMsgId          = 0x050C
    };

    // ========================================================================
    // Find Tags (Code Page 0x19) - EAS 16.0+
    // ========================================================================

    enum {
        Find_Find              = 0x1905,
        Find_SearchId          = 0x1906,
        Find_ExecuteSearch     = 0x1907,
        Find_MailBoxSearchCriterion = 0x1908,
        Find_Query             = 0x1909,
        Find_Status            = 0x190A,
        Find_FreeText          = 0x190B,
        Find_Options           = 0x190C,
        Find_Range             = 0x190D,
        Find_DeepTraversal     = 0x190E,
        Find_Response          = 0x190F,
        Find_Result            = 0x1910,
        Find_Properties        = 0x1911,
        Find_Preview           = 0x1912,
        Find_HasAttachments    = 0x1913,
        Find_Total             = 0x1914,
        Find_DisplayCc         = 0x1915,
        Find_DisplayBcc        = 0x1916,
        Find_GalSearchCriterion = 0x1917
    };

} // namespace EasTagCode

#endif /* EASTAGCODE_H_ */
