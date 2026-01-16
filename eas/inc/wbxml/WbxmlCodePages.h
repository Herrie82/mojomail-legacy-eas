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

#ifndef WBXMLCODEPAGES_H_
#define WBXMLCODEPAGES_H_

/**
 * EAS WBXML Code Pages
 *
 * Reference: [MS-ASWBXML] Section 2.2.2
 * Each code page contains tags specific to a functional area of EAS.
 */
enum class EasCodePage : unsigned char
{
    AirSync         = 0x00,  // Sync command
    Contacts        = 0x01,  // Contact data
    Email           = 0x02,  // Email data
    AirNotify       = 0x03,  // Notifications (deprecated)
    Calendar        = 0x04,  // Calendar data
    Move            = 0x05,  // MoveItems command
    ItemEstimate    = 0x06,  // GetItemEstimate command
    FolderHierarchy = 0x07,  // FolderSync, FolderCreate, etc.
    MeetingResponse = 0x08,  // MeetingResponse command
    Tasks           = 0x09,  // Task data
    ResolveRecipients = 0x0A,
    ValidateCert    = 0x0B,
    Contacts2       = 0x0C,  // Extended contact fields
    Ping            = 0x0D,  // Ping command
    Provision       = 0x0E,  // Provision command
    Search          = 0x0F,  // Search command (GAL, etc.)
    GAL             = 0x10,  // Global Address List
    AirSyncBase     = 0x11,  // Base types
    Settings        = 0x12,  // Settings command
    DocumentLibrary = 0x13,
    ItemOperations  = 0x14,
    ComposeMail     = 0x15,  // SmartForward, SmartReply, SendMail
    Email2          = 0x16,  // Extended email fields
    Notes           = 0x17,
    RightsManagement = 0x18
};

// WBXML Global tokens
namespace WbxmlToken {
    const unsigned char SWITCH_PAGE = 0x00;
    const unsigned char END         = 0x01;
    const unsigned char ENTITY      = 0x02;
    const unsigned char STR_I       = 0x03;  // Inline string
    const unsigned char LITERAL     = 0x04;
    const unsigned char EXT_I_0     = 0x40;
    const unsigned char EXT_I_1     = 0x41;
    const unsigned char EXT_I_2     = 0x42;
    const unsigned char PI          = 0x43;
    const unsigned char LITERAL_C   = 0x44;
    const unsigned char EXT_T_0     = 0x80;
    const unsigned char EXT_T_1     = 0x81;
    const unsigned char EXT_T_2     = 0x82;
    const unsigned char STR_T       = 0x83;  // String table reference
    const unsigned char LITERAL_A   = 0x84;
    const unsigned char EXT_0       = 0xC0;
    const unsigned char EXT_1       = 0xC1;
    const unsigned char EXT_2       = 0xC2;
    const unsigned char OPAQUE      = 0xC3;  // Opaque data follows
    const unsigned char LITERAL_AC  = 0xC4;
}

// Tag bit flags
namespace WbxmlTagFlag {
    const unsigned char HAS_CONTENT    = 0x40;
    const unsigned char HAS_ATTRIBUTES = 0x80;
    const unsigned char TAG_MASK       = 0x3F;
}

// ============================================================================
// AirSync (Code Page 0x00) - Sync command tags
// ============================================================================
namespace AirSyncTag {
    const unsigned char Sync            = 0x05;
    const unsigned char Collections     = 0x06;
    const unsigned char Collection      = 0x07;
    const unsigned char Class           = 0x08;
    const unsigned char SyncKey         = 0x09;
    const unsigned char CollectionId    = 0x0A;
    const unsigned char Status          = 0x0B;
    const unsigned char Add             = 0x0C;
    const unsigned char Change          = 0x0D;
    const unsigned char Delete          = 0x0E;
    const unsigned char Fetch           = 0x0F;
    const unsigned char Options         = 0x10;
    const unsigned char FilterType      = 0x11;
    const unsigned char Truncation      = 0x12;  // Deprecated
    const unsigned char RTFTruncation   = 0x13;  // Deprecated
    const unsigned char Conflict        = 0x14;
    const unsigned char Responses       = 0x15;
    const unsigned char ApplicationData = 0x16;
    const unsigned char GetChanges      = 0x17;
    const unsigned char MoreAvailable   = 0x18;
    const unsigned char WindowSize      = 0x19;
    const unsigned char Commands        = 0x1A;
    const unsigned char ServerId        = 0x1B;
    const unsigned char ClientId        = 0x1C;
    const unsigned char Supported       = 0x1D;
    const unsigned char SoftDelete      = 0x1E;
    const unsigned char MIMESupport     = 0x1F;
    const unsigned char MIMETruncation  = 0x20;
    const unsigned char Wait            = 0x21;
    const unsigned char Limit           = 0x22;
    const unsigned char Partial         = 0x23;
    const unsigned char ConversationMode = 0x24;
    const unsigned char MaxItems        = 0x25;
    const unsigned char HeartbeatInterval = 0x26;
}

// ============================================================================
// FolderHierarchy (Code Page 0x07) - Folder sync tags
// ============================================================================
namespace FolderHierarchyTag {
    const unsigned char Folders         = 0x05;  // Deprecated
    const unsigned char Folder          = 0x06;  // Deprecated
    const unsigned char DisplayName     = 0x07;
    const unsigned char ServerId        = 0x08;
    const unsigned char ParentId        = 0x09;
    const unsigned char Type            = 0x0A;
    const unsigned char Response        = 0x0B;  // Deprecated
    const unsigned char Status          = 0x0C;
    const unsigned char ContentClass    = 0x0D;  // Deprecated
    const unsigned char Changes         = 0x0E;
    const unsigned char Add             = 0x0F;
    const unsigned char Delete          = 0x10;
    const unsigned char Update          = 0x11;
    const unsigned char SyncKey         = 0x12;
    const unsigned char FolderCreate    = 0x13;
    const unsigned char FolderDelete    = 0x14;
    const unsigned char FolderUpdate    = 0x15;
    const unsigned char FolderSync      = 0x16;
    const unsigned char Count           = 0x17;
}

// ============================================================================
// Ping (Code Page 0x0D) - Ping command tags
// ============================================================================
namespace PingTag {
    const unsigned char Ping            = 0x05;
    const unsigned char AutdState       = 0x06;  // Deprecated
    const unsigned char Status          = 0x07;
    const unsigned char HeartbeatInterval = 0x08;
    const unsigned char Folders         = 0x09;
    const unsigned char Folder          = 0x0A;
    const unsigned char Id              = 0x0B;
    const unsigned char Class           = 0x0C;
    const unsigned char MaxFolders      = 0x0D;
}

// ============================================================================
// Provision (Code Page 0x0E) - Provision command tags
// ============================================================================
namespace ProvisionTag {
    const unsigned char Provision       = 0x05;
    const unsigned char Policies        = 0x06;
    const unsigned char Policy          = 0x07;
    const unsigned char PolicyType      = 0x08;
    const unsigned char PolicyKey       = 0x09;
    const unsigned char Data            = 0x0A;
    const unsigned char Status          = 0x0B;
    const unsigned char RemoteWipe      = 0x0C;
    const unsigned char EASProvisionDoc = 0x0D;
    // ... many policy settings follow
}

// ============================================================================
// Email (Code Page 0x02) - Email data tags
// ============================================================================
namespace EmailTag {
    const unsigned char Attachment      = 0x05;  // Deprecated
    const unsigned char Attachments     = 0x06;  // Deprecated
    const unsigned char AttName         = 0x07;  // Deprecated
    const unsigned char AttSize         = 0x08;  // Deprecated
    const unsigned char AttOid          = 0x09;  // Deprecated
    const unsigned char AttMethod       = 0x0A;  // Deprecated
    const unsigned char AttRemoved      = 0x0B;  // Deprecated
    const unsigned char Body            = 0x0C;  // Deprecated
    const unsigned char BodySize        = 0x0D;  // Deprecated
    const unsigned char BodyTruncated   = 0x0E;  // Deprecated
    const unsigned char DateReceived    = 0x0F;
    const unsigned char DisplayName     = 0x10;  // Deprecated
    const unsigned char DisplayTo       = 0x11;
    const unsigned char Importance      = 0x12;
    const unsigned char MessageClass    = 0x13;
    const unsigned char Subject         = 0x14;
    const unsigned char Read            = 0x15;
    const unsigned char To              = 0x16;
    const unsigned char Cc              = 0x17;
    const unsigned char From            = 0x18;
    const unsigned char ReplyTo         = 0x19;
    const unsigned char AllDayEvent     = 0x1A;
    const unsigned char Categories      = 0x1B;  // Deprecated
    const unsigned char Category        = 0x1C;  // Deprecated
    const unsigned char DtStamp         = 0x1D;  // Deprecated
    const unsigned char EndTime         = 0x1E;  // Deprecated
    const unsigned char InstanceType    = 0x1F;  // Deprecated
    const unsigned char BusyStatus      = 0x20;  // Deprecated
    const unsigned char Location        = 0x21;  // Deprecated
    const unsigned char MeetingRequest  = 0x22;
    const unsigned char Organizer       = 0x23;  // Deprecated
    const unsigned char RecurrenceId    = 0x24;  // Deprecated
    const unsigned char Reminder        = 0x25;  // Deprecated
    const unsigned char ResponseRequested = 0x26;  // Deprecated
    const unsigned char Recurrences     = 0x27;  // Deprecated
    const unsigned char Recurrence      = 0x28;  // Deprecated
    const unsigned char Type            = 0x29;  // Deprecated
    const unsigned char Until           = 0x2A;  // Deprecated
    const unsigned char Occurrences     = 0x2B;  // Deprecated
    const unsigned char Interval        = 0x2C;  // Deprecated
    const unsigned char DayOfWeek       = 0x2D;  // Deprecated
    const unsigned char DayOfMonth      = 0x2E;  // Deprecated
    const unsigned char WeekOfMonth     = 0x2F;  // Deprecated
    const unsigned char MonthOfYear     = 0x30;  // Deprecated
    const unsigned char StartTime       = 0x31;  // Deprecated
    const unsigned char Sensitivity     = 0x32;  // Deprecated
    const unsigned char TimeZone        = 0x33;  // Deprecated
    const unsigned char GlobalObjId     = 0x34;
    const unsigned char ThreadTopic     = 0x35;
    const unsigned char MIMEData        = 0x36;  // Deprecated
    const unsigned char MIMETruncated   = 0x37;  // Deprecated
    const unsigned char MIMESize        = 0x38;  // Deprecated
    const unsigned char InternetCPID    = 0x39;
    const unsigned char Flag            = 0x3A;
    const unsigned char FlagStatus      = 0x3B;  // Deprecated
    const unsigned char ContentClass    = 0x3C;
    const unsigned char FlagType        = 0x3D;  // Deprecated
    const unsigned char CompleteTime    = 0x3E;
    const unsigned char DisallowNewTimeProposal = 0x3F;  // Deprecated
}

// ============================================================================
// Calendar (Code Page 0x04) - Calendar data tags
// ============================================================================
namespace CalendarTag {
    const unsigned char Timezone        = 0x05;
    const unsigned char AllDayEvent     = 0x06;
    const unsigned char Attendees       = 0x07;
    const unsigned char Attendee        = 0x08;
    const unsigned char Email           = 0x09;
    const unsigned char Name            = 0x0A;
    const unsigned char Body            = 0x0B;  // Deprecated
    const unsigned char BodyTruncated   = 0x0C;  // Deprecated
    const unsigned char BusyStatus      = 0x0D;
    const unsigned char Categories      = 0x0E;
    const unsigned char Category        = 0x0F;
    const unsigned char Compressed_RTF  = 0x10;  // Deprecated
    const unsigned char DtStamp         = 0x11;
    const unsigned char EndTime         = 0x12;
    const unsigned char Exception       = 0x13;
    const unsigned char Exceptions      = 0x14;
    const unsigned char Deleted         = 0x15;
    const unsigned char ExceptionStartTime = 0x16;
    const unsigned char Location        = 0x17;
    const unsigned char MeetingStatus   = 0x18;
    const unsigned char OrganizerEmail  = 0x19;
    const unsigned char OrganizerName   = 0x1A;
    const unsigned char Recurrence      = 0x1B;
    const unsigned char Type            = 0x1C;
    const unsigned char Until           = 0x1D;
    const unsigned char Occurrences     = 0x1E;
    const unsigned char Interval        = 0x1F;
    const unsigned char DayOfWeek       = 0x20;
    const unsigned char DayOfMonth      = 0x21;
    const unsigned char WeekOfMonth     = 0x22;
    const unsigned char MonthOfYear     = 0x23;
    const unsigned char Reminder        = 0x24;
    const unsigned char Sensitivity     = 0x25;
    const unsigned char Subject         = 0x26;
    const unsigned char StartTime       = 0x27;
    const unsigned char UID             = 0x28;
    const unsigned char AttendeeStatus  = 0x29;
    const unsigned char AttendeeType    = 0x2A;
}

// ============================================================================
// Contacts (Code Page 0x01) - Contact data tags
// ============================================================================
namespace ContactsTag {
    const unsigned char Anniversary     = 0x05;
    const unsigned char AssistantName   = 0x06;
    const unsigned char AssistantTelephone = 0x07;
    const unsigned char Birthday        = 0x08;
    const unsigned char Body            = 0x09;  // Deprecated
    const unsigned char BodySize        = 0x0A;  // Deprecated
    const unsigned char BodyTruncated   = 0x0B;  // Deprecated
    const unsigned char Business2Phone  = 0x0C;
    const unsigned char BusinessCity    = 0x0D;
    const unsigned char BusinessCountry = 0x0E;
    const unsigned char BusinessPostalCode = 0x0F;
    const unsigned char BusinessState   = 0x10;
    const unsigned char BusinessStreet  = 0x11;
    const unsigned char BusinessFax     = 0x12;
    const unsigned char BusinessPhone   = 0x13;
    const unsigned char CarPhone        = 0x14;
    const unsigned char Categories      = 0x15;
    const unsigned char Category        = 0x16;
    const unsigned char Children        = 0x17;
    const unsigned char Child           = 0x18;
    const unsigned char CompanyName     = 0x19;
    const unsigned char Department      = 0x1A;
    const unsigned char Email1          = 0x1B;
    const unsigned char Email2          = 0x1C;
    const unsigned char Email3          = 0x1D;
    const unsigned char FileAs          = 0x1E;
    const unsigned char FirstName       = 0x1F;
    const unsigned char Home2Phone      = 0x20;
    const unsigned char HomeCity        = 0x21;
    const unsigned char HomeCountry     = 0x22;
    const unsigned char HomePostalCode  = 0x23;
    const unsigned char HomeState       = 0x24;
    const unsigned char HomeStreet      = 0x25;
    const unsigned char HomeFax         = 0x26;
    const unsigned char HomePhone       = 0x27;
    const unsigned char JobTitle        = 0x28;
    const unsigned char LastName        = 0x29;
    const unsigned char MiddleName      = 0x2A;
    const unsigned char MobilePhone     = 0x2B;
    const unsigned char OfficeLocation  = 0x2C;
    const unsigned char OtherCity       = 0x2D;
    const unsigned char OtherCountry    = 0x2E;
    const unsigned char OtherPostalCode = 0x2F;
    const unsigned char OtherState      = 0x30;
    const unsigned char OtherStreet     = 0x31;
    const unsigned char PagerNumber     = 0x32;
    const unsigned char RadioPhone      = 0x33;
    const unsigned char Spouse          = 0x34;
    const unsigned char Suffix          = 0x35;
    const unsigned char Title           = 0x36;
    const unsigned char Webpage         = 0x37;
    const unsigned char YomiCompany     = 0x38;
    const unsigned char YomiFirstName   = 0x39;
    const unsigned char YomiLastName    = 0x3A;
    const unsigned char CompressedRTF   = 0x3B;  // Deprecated
    const unsigned char Picture         = 0x3C;
}

// ============================================================================
// Tasks (Code Page 0x09) - Task data tags
// ============================================================================
namespace TasksTag {
    const unsigned char Body            = 0x05;  // Deprecated
    const unsigned char BodySize        = 0x06;  // Deprecated
    const unsigned char BodyTruncated   = 0x07;  // Deprecated
    const unsigned char Categories      = 0x08;
    const unsigned char Category        = 0x09;
    const unsigned char Complete        = 0x0A;
    const unsigned char DateCompleted   = 0x0B;
    const unsigned char DueDate         = 0x0C;
    const unsigned char Importance      = 0x0D;
    const unsigned char Recurrence      = 0x0E;
    const unsigned char RecurrenceType  = 0x0F;
    const unsigned char RecurrenceStart = 0x10;
    const unsigned char RecurrenceRegenerate = 0x11;
    const unsigned char RecurrenceDeadOccur = 0x12;
    const unsigned char ReminderSet     = 0x13;
    const unsigned char ReminderTime    = 0x14;
    const unsigned char Sensitivity     = 0x15;
    const unsigned char StartDate       = 0x16;
    const unsigned char Subject         = 0x17;
    const unsigned char UTCStartDate    = 0x18;
    const unsigned char UTCDueDate      = 0x19;
    const unsigned char OrdinalDate     = 0x1A;
    const unsigned char SubOrdinalDate  = 0x1B;
    const unsigned char CalendarType    = 0x1C;
    const unsigned char IsLeapMonth     = 0x1D;
    const unsigned char FirstDayOfWeek  = 0x1E;
}

// ============================================================================
// AirSyncBase (Code Page 0x11) - Base types
// ============================================================================
namespace AirSyncBaseTag {
    const unsigned char BodyPreference  = 0x05;
    const unsigned char Type            = 0x06;
    const unsigned char TruncationSize  = 0x07;
    const unsigned char AllOrNone       = 0x08;
    const unsigned char Reserved        = 0x09;
    const unsigned char Body            = 0x0A;
    const unsigned char Data            = 0x0B;
    const unsigned char EstimatedDataSize = 0x0C;
    const unsigned char Truncated       = 0x0D;
    const unsigned char Attachments     = 0x0E;
    const unsigned char Attachment      = 0x0F;
    const unsigned char DisplayName     = 0x10;
    const unsigned char FileReference   = 0x11;
    const unsigned char Method          = 0x12;
    const unsigned char ContentId       = 0x13;
    const unsigned char ContentLocation = 0x14;  // Deprecated
    const unsigned char IsInline        = 0x15;
    const unsigned char NativeBodyType  = 0x16;
    const unsigned char ContentType     = 0x17;
    const unsigned char Preview         = 0x18;
    const unsigned char BodyPartPreference = 0x19;
    const unsigned char BodyPart        = 0x1A;
    const unsigned char Status          = 0x1B;
}

#endif /* WBXMLCODEPAGES_H_ */
