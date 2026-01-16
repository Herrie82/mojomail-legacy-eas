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

#ifndef EASBUSDISPATCHER_H_
#define EASBUSDISPATCHER_H_

#include "core/MojCoreDefs.h"
#include "core/MojService.h"
#include "core/MojServiceMessage.h"
#include "core/MojLog.h"
#include "luna/MojLunaService.h"
#include "client/BusClient.h"
#include "CommonErrors.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

class EasClient;
class EasConfig;
class DatabaseInterface;

/**
 * EasBusDispatcher - Luna service bus handler for EAS
 *
 * Handles Luna bus messages and dispatches to appropriate EasClient instances.
 * Each account gets its own EasClient.
 */
class EasBusDispatcher : public MojService::CategoryHandler
{
public:
    // Listener interfaces (matching binary symbols)
    class AutoDiscoverListener {
    public:
        virtual ~AutoDiscoverListener() {}
        virtual void AutoDiscoverSuccess(const std::string& serverUrl) = 0;
        virtual void AutoDiscoverFailure(MailError::ErrorCode error) = 0;
    };

    class ValidationListener {
    public:
        virtual ~ValidationListener() {}
        virtual void ValidationSuccess() = 0;
        virtual void ValidationFailure(MailError::ErrorCode error) = 0;
    };

    EasBusDispatcher(MojLunaService& service,
                     boost::shared_ptr<DatabaseInterface> dbInterface);
    virtual ~EasBusDispatcher();

    // MojService::CategoryHandler methods
    virtual MojErr open();
    virtual MojErr close();

    // Client management
    boost::shared_ptr<EasClient> GetOrCreateClient(const MojObject& accountId);
    void RemoveClient(const MojObject& accountId);

    // Service access
    MojLunaService& GetService() { return m_service; }
    const boost::shared_ptr<DatabaseInterface>& GetDatabaseInterface() const { return m_dbInterface; }

protected:
    static MojLogger s_log;

    // Luna bus method handlers
    MojErr AccountCreated(MojServiceMessage* msg, MojObject& payload);
    MojErr AccountDeleted(MojServiceMessage* msg, MojObject& payload);
    MojErr AccountUpdated(MojServiceMessage* msg, MojObject& payload);
    MojErr AccountEnabled(MojServiceMessage* msg, MojObject& payload);
    MojErr SyncAccount(MojServiceMessage* msg, MojObject& payload);
    MojErr SyncFolder(MojServiceMessage* msg, MojObject& payload);
    MojErr ValidateAccount(MojServiceMessage* msg, MojObject& payload);
    MojErr AutoDiscover(MojServiceMessage* msg, MojObject& payload);
    MojErr DownloadAttachment(MojServiceMessage* msg, MojObject& payload);
    MojErr MoveEmail(MojServiceMessage* msg, MojObject& payload);
    MojErr DeleteEmail(MojServiceMessage* msg, MojObject& payload);
    MojErr SendMail(MojServiceMessage* msg, MojObject& payload);
    MojErr MeetingResponse(MojServiceMessage* msg, MojObject& payload);
    MojErr SearchGal(MojServiceMessage* msg, MojObject& payload);
    MojErr Ping(MojServiceMessage* msg, MojObject& payload);
    MojErr Status(MojServiceMessage* msg, MojObject& payload);

    // Declare Luna method table
    static const MojChar* const ServiceName;
    static const Method s_methods[];

    MojLunaService&                             m_service;
    boost::shared_ptr<DatabaseInterface>        m_dbInterface;
    std::map<MojObject, boost::shared_ptr<EasClient>> m_clients;
    MojLogger&                                  m_log;
};

#endif /* EASBUSDISPATCHER_H_ */
