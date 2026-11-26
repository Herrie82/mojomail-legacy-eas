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

#include "data/EasContactAdapter.h"

const char* const EasContactAdapter::EAS_CONTACT_KIND = "com.palm.contact:1";
const char* const EasContactAdapter::SERVER_ID = "easServerId";

EasContactAdapter::EasContactAdapter()
{
}

EasContactAdapter::~EasContactAdapter()
{
}

void EasContactAdapter::ParseFromEas(const MojObject& easData, MojObject& dbObj)
{
    bool found;

    // Set kind
    dbObj.putString("_kind", EAS_CONTACT_KIND);

    // Server ID
    MojString serverId;
    if (easData.get("serverId", serverId, found) == MojErrNone && found) {
        dbObj.putString(SERVER_ID, serverId.data());
    }

    // Parse name
    ParseName(easData, dbObj);

    // Parse phone numbers
    MojObject phones;
    ParsePhoneNumbers(easData, phones);
    if (phones.size() > 0) {
        dbObj.put("phoneNumbers", phones);
    }

    // Parse emails
    MojObject emails;
    ParseEmails(easData, emails);
    if (emails.size() > 0) {
        dbObj.put("emails", emails);
    }

    // Parse addresses
    MojObject addresses;
    ParseAddresses(easData, addresses);
    if (addresses.size() > 0) {
        dbObj.put("addresses", addresses);
    }

    // Parse organization
    MojObject org;
    ParseOrganization(easData, org);
    if (!org.empty()) {
        MojObject orgs;
        orgs.push(org);
        dbObj.put("organizations", orgs);
    }

    // Notes/comments
    MojString body;
    if (easData.get("body", body, found) == MojErrNone && found) {
        MojObject note;
        note.putString("value", body.data());
        MojObject notes;
        notes.push(note);
        dbObj.put("notes", notes);
    }

    // Birthday
    MojString birthday;
    if (easData.get("birthday", birthday, found) == MojErrNone && found) {
        dbObj.putString("birthday", birthday.data());
    }

    // Photo (base64 encoded)
    MojString photo;
    if (easData.get("picture", photo, found) == MojErrNone && found) {
        MojObject photoObj;
        photoObj.putString("value", photo.data());
        photoObj.putString("type", "base64");
        MojObject photos;
        photos.push(photoObj);
        dbObj.put("photos", photos);
    }
}

void EasContactAdapter::SerializeToEas(const MojObject& dbObj, MojObject& easData)
{
    bool found;

    // Server ID
    MojString serverId;
    if (dbObj.get(SERVER_ID, serverId, found) == MojErrNone && found) {
        easData.putString("serverId", serverId.data());
    }

    // Serialize name
    SerializeName(dbObj, easData);

    // Serialize phone numbers - MojObject get returns bool directly
    MojObject phones;
    if (dbObj.get("phoneNumbers", phones)) {
        SerializePhoneNumbers(phones, easData);
    }

    // Serialize emails - MojObject get returns bool directly
    MojObject emails;
    if (dbObj.get("emails", emails)) {
        SerializeEmails(emails, easData);
    }

    // Serialize addresses - MojObject get returns bool directly
    MojObject addresses;
    if (dbObj.get("addresses", addresses)) {
        SerializeAddresses(addresses, easData);
    }

    // Notes - MojObject get returns bool directly
    MojObject notes;
    if (dbObj.get("notes", notes) && notes.size() > 0) {
        MojObject::ConstArrayIterator it = notes.arrayBegin();
        if (it != notes.arrayEnd()) {
            MojString noteVal;
            if (it->get("value", noteVal, found) == MojErrNone && found) {
                easData.putString("body", noteVal.data());
            }
        }
    }
}

void EasContactAdapter::ParseName(const MojObject& easData, MojObject& dbObj)
{
    bool found;
    MojObject name;

    MojString firstName;
    if (easData.get("firstName", firstName, found) == MojErrNone && found) {
        name.putString("givenName", firstName.data());
    }

    MojString lastName;
    if (easData.get("lastName", lastName, found) == MojErrNone && found) {
        name.putString("familyName", lastName.data());
    }

    MojString middleName;
    if (easData.get("middleName", middleName, found) == MojErrNone && found) {
        name.putString("middleName", middleName.data());
    }

    MojString title;
    if (easData.get("title", title, found) == MojErrNone && found) {
        name.putString("honorificPrefix", title.data());
    }

    MojString suffix;
    if (easData.get("suffix", suffix, found) == MojErrNone && found) {
        name.putString("honorificSuffix", suffix.data());
    }

    MojString nickname;
    if (easData.get("nickName", nickname, found) == MojErrNone && found) {
        dbObj.putString("nickname", nickname.data());
    }

    if (!name.empty()) {
        dbObj.put("name", name);
    }
}

void EasContactAdapter::ParsePhoneNumbers(const MojObject& easData, MojObject& phones)
{
    bool found;
    
    // EAS phone field mappings
    struct PhoneMapping {
        const char* easField;
        const char* type;
    };
    
    static const PhoneMapping mappings[] = {
        {"homePhoneNumber", "home"},
        {"home2PhoneNumber", "home"},
        {"businessPhoneNumber", "work"},
        {"business2PhoneNumber", "work"},
        {"mobilePhoneNumber", "mobile"},
        {"carPhoneNumber", "car"},
        {"homeFaxNumber", "homeFax"},
        {"businessFaxNumber", "workFax"},
        {"pagerNumber", "pager"},
        {"assistantPhoneNumber", "assistant"},
        {"companyMainPhone", "company"},
        {"radioPhoneNumber", "radio"}
    };

    for (size_t i = 0; i < sizeof(mappings)/sizeof(mappings[0]); ++i) {
        MojString value;
        if (easData.get(mappings[i].easField, value, found) == MojErrNone && found && value.length() > 0) {
            MojObject phone;
            phone.putString("value", value.data());
            phone.putString("type", mappings[i].type);
            phones.push(phone);
        }
    }
}

void EasContactAdapter::ParseEmails(const MojObject& easData, MojObject& emails)
{
    bool found;

    // EAS supports up to 3 email addresses
    MojString email1, email2, email3;

    if (easData.get("email1Address", email1, found) == MojErrNone && found && email1.length() > 0) {
        MojObject email;
        email.putString("value", email1.data());
        email.putString("type", "work");
        email.putBool("primary", true);
        emails.push(email);
    }

    if (easData.get("email2Address", email2, found) == MojErrNone && found && email2.length() > 0) {
        MojObject email;
        email.putString("value", email2.data());
        email.putString("type", "home");
        emails.push(email);
    }

    if (easData.get("email3Address", email3, found) == MojErrNone && found && email3.length() > 0) {
        MojObject email;
        email.putString("value", email3.data());
        email.putString("type", "other");
        emails.push(email);
    }
}

void EasContactAdapter::ParseAddresses(const MojObject& easData, MojObject& addresses)
{
    bool found;

    // Parse home address
    MojString homeStreet, homeCity, homeState, homePostal, homeCountry;
    bool hasHome = false;
    MojObject homeAddr;

    if (easData.get("homeAddressStreet", homeStreet, found) == MojErrNone && found && homeStreet.length() > 0) {
        homeAddr.putString("streetAddress", homeStreet.data());
        hasHome = true;
    }
    if (easData.get("homeAddressCity", homeCity, found) == MojErrNone && found && homeCity.length() > 0) {
        homeAddr.putString("locality", homeCity.data());
        hasHome = true;
    }
    if (easData.get("homeAddressState", homeState, found) == MojErrNone && found && homeState.length() > 0) {
        homeAddr.putString("region", homeState.data());
        hasHome = true;
    }
    if (easData.get("homeAddressPostalCode", homePostal, found) == MojErrNone && found && homePostal.length() > 0) {
        homeAddr.putString("postalCode", homePostal.data());
        hasHome = true;
    }
    if (easData.get("homeAddressCountry", homeCountry, found) == MojErrNone && found && homeCountry.length() > 0) {
        homeAddr.putString("country", homeCountry.data());
        hasHome = true;
    }

    if (hasHome) {
        homeAddr.putString("type", "home");
        addresses.push(homeAddr);
    }

    // Parse business address
    MojString bizStreet, bizCity, bizState, bizPostal, bizCountry;
    bool hasBiz = false;
    MojObject bizAddr;

    if (easData.get("businessAddressStreet", bizStreet, found) == MojErrNone && found && bizStreet.length() > 0) {
        bizAddr.putString("streetAddress", bizStreet.data());
        hasBiz = true;
    }
    if (easData.get("businessAddressCity", bizCity, found) == MojErrNone && found && bizCity.length() > 0) {
        bizAddr.putString("locality", bizCity.data());
        hasBiz = true;
    }
    if (easData.get("businessAddressState", bizState, found) == MojErrNone && found && bizState.length() > 0) {
        bizAddr.putString("region", bizState.data());
        hasBiz = true;
    }
    if (easData.get("businessAddressPostalCode", bizPostal, found) == MojErrNone && found && bizPostal.length() > 0) {
        bizAddr.putString("postalCode", bizPostal.data());
        hasBiz = true;
    }
    if (easData.get("businessAddressCountry", bizCountry, found) == MojErrNone && found && bizCountry.length() > 0) {
        bizAddr.putString("country", bizCountry.data());
        hasBiz = true;
    }

    if (hasBiz) {
        bizAddr.putString("type", "work");
        addresses.push(bizAddr);
    }
}

void EasContactAdapter::ParseOrganization(const MojObject& easData, MojObject& org)
{
    bool found;

    MojString company;
    if (easData.get("companyName", company, found) == MojErrNone && found && company.length() > 0) {
        org.putString("name", company.data());
    }

    MojString department;
    if (easData.get("department", department, found) == MojErrNone && found && department.length() > 0) {
        org.putString("department", department.data());
    }

    MojString jobTitle;
    if (easData.get("jobTitle", jobTitle, found) == MojErrNone && found && jobTitle.length() > 0) {
        org.putString("title", jobTitle.data());
    }

    MojString officeLocation;
    if (easData.get("officeLocation", officeLocation, found) == MojErrNone && found && officeLocation.length() > 0) {
        org.putString("location", officeLocation.data());
    }
}

void EasContactAdapter::SerializeName(const MojObject& dbObj, MojObject& easData)
{
    bool found;
    MojObject name;

    // MojObject get returns bool directly
    if (dbObj.get("name", name)) {
        MojString givenName, familyName, middleName, prefix, suffix;

        if (name.get("givenName", givenName, found) == MojErrNone && found) {
            easData.putString("firstName", givenName.data());
        }
        if (name.get("familyName", familyName, found) == MojErrNone && found) {
            easData.putString("lastName", familyName.data());
        }
        if (name.get("middleName", middleName, found) == MojErrNone && found) {
            easData.putString("middleName", middleName.data());
        }
        if (name.get("honorificPrefix", prefix, found) == MojErrNone && found) {
            easData.putString("title", prefix.data());
        }
        if (name.get("honorificSuffix", suffix, found) == MojErrNone && found) {
            easData.putString("suffix", suffix.data());
        }
    }

    MojString nickname;
    if (dbObj.get("nickname", nickname, found) == MojErrNone && found) {
        easData.putString("nickName", nickname.data());
    }
}

void EasContactAdapter::SerializePhoneNumbers(const MojObject& phones, MojObject& easData)
{
    // Map back to EAS fields based on type
    MojObject::ConstArrayIterator it = phones.arrayBegin();
    int homeCount = 0, workCount = 0;

    for (; it != phones.arrayEnd(); ++it) {
        bool found;
        MojString value, type;

        if (it->get("value", value, found) != MojErrNone || !found) continue;
        it->get("type", type, found);

        const char* typeStr = type.data();

        if (strcmp(typeStr, "mobile") == 0) {
            easData.putString("mobilePhoneNumber", value.data());
        } else if (strcmp(typeStr, "home") == 0) {
            if (homeCount == 0) {
                easData.putString("homePhoneNumber", value.data());
            } else {
                easData.putString("home2PhoneNumber", value.data());
            }
            homeCount++;
        } else if (strcmp(typeStr, "work") == 0) {
            if (workCount == 0) {
                easData.putString("businessPhoneNumber", value.data());
            } else {
                easData.putString("business2PhoneNumber", value.data());
            }
            workCount++;
        } else if (strcmp(typeStr, "homeFax") == 0) {
            easData.putString("homeFaxNumber", value.data());
        } else if (strcmp(typeStr, "workFax") == 0) {
            easData.putString("businessFaxNumber", value.data());
        } else if (strcmp(typeStr, "pager") == 0) {
            easData.putString("pagerNumber", value.data());
        }
    }
}

void EasContactAdapter::SerializeEmails(const MojObject& emails, MojObject& easData)
{
    MojObject::ConstArrayIterator it = emails.arrayBegin();
    int count = 0;

    for (; it != emails.arrayEnd() && count < 3; ++it, ++count) {
        bool found;
        MojString value;

        if (it->get("value", value, found) != MojErrNone || !found) continue;

        switch (count) {
            case 0: easData.putString("email1Address", value.data()); break;
            case 1: easData.putString("email2Address", value.data()); break;
            case 2: easData.putString("email3Address", value.data()); break;
        }
    }
}

void EasContactAdapter::SerializeAddresses(const MojObject& addresses, MojObject& easData)
{
    MojObject::ConstArrayIterator it = addresses.arrayBegin();

    for (; it != addresses.arrayEnd(); ++it) {
        bool found;
        MojString type;
        it->get("type", type, found);

        const char* prefix = "";
        if (strcmp(type.data(), "home") == 0) {
            prefix = "homeAddress";
        } else if (strcmp(type.data(), "work") == 0) {
            prefix = "businessAddress";
        } else {
            continue;  // Skip other types
        }

        MojString street, city, state, postal, country;
        char fieldName[64];

        if (it->get("streetAddress", street, found) == MojErrNone && found) {
            snprintf(fieldName, sizeof(fieldName), "%sStreet", prefix);
            easData.putString(fieldName, street.data());
        }
        if (it->get("locality", city, found) == MojErrNone && found) {
            snprintf(fieldName, sizeof(fieldName), "%sCity", prefix);
            easData.putString(fieldName, city.data());
        }
        if (it->get("region", state, found) == MojErrNone && found) {
            snprintf(fieldName, sizeof(fieldName), "%sState", prefix);
            easData.putString(fieldName, state.data());
        }
        if (it->get("postalCode", postal, found) == MojErrNone && found) {
            snprintf(fieldName, sizeof(fieldName), "%sPostalCode", prefix);
            easData.putString(fieldName, postal.data());
        }
        if (it->get("country", country, found) == MojErrNone && found) {
            snprintf(fieldName, sizeof(fieldName), "%sCountry", prefix);
            easData.putString(fieldName, country.data());
        }
    }
}
