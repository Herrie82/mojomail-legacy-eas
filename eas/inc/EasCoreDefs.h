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

#ifndef EASCOREDEFS_H_
#define EASCOREDEFS_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

// EAS Protocol versions
// Reference: [MS-ASHTTP] Section 2.2.1.1.2.6
namespace EasProtocol {
    const std::string VERSION_2_5  = "2.5";   // Exchange 2003
    const std::string VERSION_12_0 = "12.0";  // Exchange 2007
    const std::string VERSION_12_1 = "12.1";  // Exchange 2007 SP1
    const std::string VERSION_14_0 = "14.0";  // Exchange 2010
    const std::string VERSION_14_1 = "14.1";  // Exchange 2010 SP1
    const std::string VERSION_16_0 = "16.0";  // Exchange 2016
    const std::string VERSION_16_1 = "16.1";  // Exchange 2016 CU3+
}

// EAS Command types (matching binary symbols)
// Using struct+enum pattern for C++03 compatibility (instead of enum class)
struct EasCommandType {
    enum Type {
        Unknown,
        Sync,
        SendMail,
        SmartForward,
        SmartReply,
        GetAttachment,
        FolderSync,
        FolderCreate,
        FolderDelete,
        FolderUpdate,
        MoveItems,
        GetItemEstimate,
        MeetingResponse,
        Search,
        Settings,
        Ping,
        ItemOperations,
        Provision,
        ResolveRecipients,
        ValidateCert,
        Options
    };
};

// EAS Folder types
struct EasFolderType {
    enum Type {
        UserCreated = 1,
        DefaultInbox = 2,
        DefaultDrafts = 3,
        DefaultDeleted = 4,
        DefaultSent = 5,
        DefaultOutbox = 6,
        DefaultTasks = 7,
        DefaultCalendar = 8,
        DefaultContacts = 9,
        DefaultNotes = 10,
        DefaultJournal = 11,
        UserMail = 12,
        UserCalendar = 13,
        UserContacts = 14,
        UserTasks = 15,
        UserJournal = 16,
        UserNotes = 17,
        Unknown = 18,
        RecipientCache = 19
    };
};

// EAS Sync status codes
struct EasSyncStatus {
    enum Type {
        Success = 1,
        InvalidSyncKey = 3,
        ProtocolError = 4,
        ServerError = 5,
        ClientError = 6,
        ConflictMatching = 7,
        ObjectNotFound = 8,
        OutOfDisk = 9,
        NotificationGUID = 10,
        NotYetProvisionable = 11,
        HierarchyChanged = 12,
        PartialFailure = 13,
        WaitInterval = 14,
        TooManyFolders = 15,
        SyncKeyInvalid = 16
    };
};

// EAS Collection class names
namespace EasCollectionClass {
    const std::string Email    = "Email";
    const std::string Calendar = "Calendar";
    const std::string Contacts = "Contacts";
    const std::string Tasks    = "Tasks";
    const std::string Notes    = "Notes";
}

#endif /* EASCOREDEFS_H_ */
