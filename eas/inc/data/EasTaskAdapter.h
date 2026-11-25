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

#ifndef EASTASKADAPTER_H_
#define EASTASKADAPTER_H_

#include "core/MojObject.h"
#include <string>

/**
 * EasTaskAdapter - Convert EAS task data to/from database format
 *
 * Handles conversion between:
 * - EAS Tasks namespace (code page 0x09)
 * - webOS tasks database (com.palm.task:1)
 */
class EasTaskAdapter
{
public:
    EasTaskAdapter();
    virtual ~EasTaskAdapter();

    // Database kind
    static const char* const EAS_TASK_KIND;
    static const char* const SERVER_ID;

    /**
     * Parse EAS ApplicationData into task database object
     */
    static void ParseFromEas(const MojObject& easData, MojObject& dbObj);

    /**
     * Serialize task for EAS upload
     */
    static void SerializeToEas(const MojObject& dbObj, MojObject& easData);

private:
    /**
     * Convert EAS date to timestamp
     */
    static int64_t ParseEasDate(const std::string& dateStr);

    /**
     * Convert timestamp to EAS date
     */
    static std::string FormatEasDate(int64_t timestamp);

    /**
     * Convert EAS importance to priority
     */
    static std::string ConvertImportance(int importance);

    /**
     * Convert priority to EAS importance
     */
    static int ConvertPriority(const std::string& priority);
};

#endif /* EASTASKADAPTER_H_ */
