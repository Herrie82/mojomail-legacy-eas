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

#include "EasBusDispatcher.h"
#include "EasClient.h"

MojLogger EasBusDispatcher::s_log("com.palm.eas.dispatcher");

const MojChar* const EasBusDispatcher::ServiceName = "com.palm.eas";

// Luna service method table
const MojService::CategoryHandler::Method EasBusDispatcher::s_methods[] = {
    { "accountCreated", (Callback)&EasBusDispatcher::AccountCreated },
    { "accountDeleted", (Callback)&EasBusDispatcher::AccountDeleted },
    { "accountUpdated", (Callback)&EasBusDispatcher::AccountUpdated },
    { "accountEnabled", (Callback)&EasBusDispatcher::AccountEnabled },
    { "syncAccount", (Callback)&EasBusDispatcher::SyncAccount },
    { "syncFolder", (Callback)&EasBusDispatcher::SyncFolder },
    { "validateAccount", (Callback)&EasBusDispatcher::ValidateAccount },
    { "autoDiscover", (Callback)&EasBusDispatcher::AutoDiscover },
    { "downloadAttachment", (Callback)&EasBusDispatcher::DownloadAttachment },
    { "moveEmail", (Callback)&EasBusDispatcher::MoveEmail },
    { "deleteEmail", (Callback)&EasBusDispatcher::DeleteEmail },
    { "sendMail", (Callback)&EasBusDispatcher::SendMail },
    { "meetingResponse", (Callback)&EasBusDispatcher::MeetingResponse },
    { "searchGal", (Callback)&EasBusDispatcher::SearchGal },
    { "ping", (Callback)&EasBusDispatcher::Ping },
    { "status", (Callback)&EasBusDispatcher::Status },
    { NULL, NULL }
};

EasBusDispatcher::EasBusDispatcher(MojLunaService& service,
                                   boost::shared_ptr<DatabaseInterface> dbInterface)
    : m_service(service)
    , m_dbInterface(dbInterface)
    , m_log(s_log)
{
    MojLogTrace(m_log);
}

EasBusDispatcher::~EasBusDispatcher()
{
    MojLogTrace(m_log);
}

MojErr EasBusDispatcher::open()
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "Opening EAS bus dispatcher");

    // Add methods to the service
    MojErr err = addMethods(s_methods);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr EasBusDispatcher::close()
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "Closing EAS bus dispatcher");

    // Clean up all clients
    m_clients.clear();

    return MojErrNone;
}

boost::shared_ptr<EasClient> EasBusDispatcher::GetOrCreateClient(const MojObject& accountId)
{
    MojLogTrace(m_log);

    std::map<MojObject, boost::shared_ptr<EasClient> >::iterator it = m_clients.find(accountId);
    if (it != m_clients.end()) {
        return it->second;
    }

    // Create new client for this account
    boost::shared_ptr<EasClient> client(
        new EasClient(&m_service, this, accountId, m_dbInterface));

    m_clients[accountId] = client;
    return client;
}

void EasBusDispatcher::RemoveClient(const MojObject& accountId)
{
    MojLogTrace(m_log);
    m_clients.erase(accountId);
}

MojErr EasBusDispatcher::AccountCreated(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "AccountCreated called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        MojLogError(m_log, "AccountCreated: no accountId in payload");
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    // Get or create client for this account
    boost::shared_ptr<EasClient> client = GetOrCreateClient(accountId);

    // Reply success
    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::AccountDeleted(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "AccountDeleted called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    // Remove client for this account
    RemoveClient(accountId);

    // TODO: Clean up database entries for this account

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::AccountUpdated(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "AccountUpdated called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    // Get client and update its settings
    boost::shared_ptr<EasClient> client = GetOrCreateClient(accountId);
    // TODO: Update client settings

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::AccountEnabled(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "AccountEnabled called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    bool enabled = true;
    payload.get("enabled", enabled);

    boost::shared_ptr<EasClient> client = GetOrCreateClient(accountId);
    // TODO: Enable/disable client

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::SyncAccount(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "SyncAccount called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    boost::shared_ptr<EasClient> client = GetOrCreateClient(accountId);
    client->SyncAccount();

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::SyncFolder(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "SyncFolder called");

    MojObject accountId, folderId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    if (!payload.get("folderId", folderId)) {
        msg->replyError(MojErrInvalidArg, "Missing folderId");
        return MojErrNone;
    }

    boost::shared_ptr<EasClient> client = GetOrCreateClient(accountId);
    client->SyncFolder(folderId);

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::ValidateAccount(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "ValidateAccount called");

    // TODO: Implement account validation
    // This tests connection to the server with provided credentials

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::AutoDiscover(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "AutoDiscover called");

    // TODO: Implement EAS autodiscover
    // Uses email address to find Exchange server URL

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::DownloadAttachment(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "DownloadAttachment called");

    MojObject accountId;
    if (!payload.get("accountId", accountId)) {
        msg->replyError(MojErrInvalidArg, "Missing accountId");
        return MojErrNone;
    }

    // TODO: Queue download attachment command

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::MoveEmail(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "MoveEmail called");

    // TODO: Implement MoveItems command

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::DeleteEmail(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "DeleteEmail called");

    // TODO: Implement delete via Sync command

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::SendMail(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "SendMail called");

    // TODO: Implement SendMail command

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::MeetingResponse(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "MeetingResponse called");

    // TODO: Implement MeetingResponse command

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::SearchGal(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "SearchGal called");

    // TODO: Implement Search command for GAL

    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::Ping(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "Ping called");

    // Simple ping response for health check
    MojObject reply;
    reply.putBool("returnValue", true);
    msg->reply(reply);

    return MojErrNone;
}

MojErr EasBusDispatcher::Status(MojServiceMessage* msg, MojObject& payload)
{
    MojLogTrace(m_log);
    MojLogInfo(m_log, "Status called");

    MojObject reply;
    reply.putBool("returnValue", true);
    reply.putInt("clientCount", m_clients.size());

    // Add status for each client
    MojObject clients;
    typedef std::map<MojObject, boost::shared_ptr<EasClient> >::const_iterator ClientIter;
    for (ClientIter it = m_clients.begin(); it != m_clients.end(); ++it) {
        MojObject clientStatus;
        MojString idStr;
        it->first.stringValue(idStr);
        clientStatus.putString("accountId", idStr.data());
        clientStatus.putInt("state", static_cast<int>(it->second->GetState()));
        clients.push(clientStatus);
    }
    reply.put("clients", clients);

    msg->reply(reply);

    return MojErrNone;
}
