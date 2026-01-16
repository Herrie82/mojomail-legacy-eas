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

#ifndef EASCONTACTADAPTER_H_
#define EASCONTACTADAPTER_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasContactAdapter - Convert EAS contact data to/from database format
 *
 * Handles conversion between:
 * - EAS Contacts namespace (code page 0x01)
 * - webOS contacts database (com.palm.contact:1)
 */
class EasContactAdapter
{
public:
    EasContactAdapter();
    virtual ~EasContactAdapter();

    // Database kind
    static const char* const EAS_CONTACT_KIND;
    static const char* const SERVER_ID;

    /**
     * Parse EAS ApplicationData into contact database object
     */
    static void ParseFromEas(const MojObject& easData, MojObject& dbObj);

    /**
     * Serialize contact for EAS upload
     */
    static void SerializeToEas(const MojObject& dbObj, MojObject& easData);

private:
    /**
     * Parse name fields (FirstName, LastName, etc.)
     */
    static void ParseName(const MojObject& easData, MojObject& dbObj);

    /**
     * Parse phone numbers
     */
    static void ParsePhoneNumbers(const MojObject& easData, MojObject& phones);

    /**
     * Parse email addresses
     */
    static void ParseEmails(const MojObject& easData, MojObject& emails);

    /**
     * Parse addresses (home, work, other)
     */
    static void ParseAddresses(const MojObject& easData, MojObject& addresses);

    /**
     * Parse organization info
     */
    static void ParseOrganization(const MojObject& easData, MojObject& org);

    /**
     * Serialize name to EAS
     */
    static void SerializeName(const MojObject& dbObj, MojObject& easData);

    /**
     * Serialize phone numbers to EAS
     */
    static void SerializePhoneNumbers(const MojObject& phones, MojObject& easData);

    /**
     * Serialize emails to EAS
     */
    static void SerializeEmails(const MojObject& emails, MojObject& easData);

    /**
     * Serialize addresses to EAS
     */
    static void SerializeAddresses(const MojObject& addresses, MojObject& easData);
};

#endif /* EASCONTACTADAPTER_H_ */
