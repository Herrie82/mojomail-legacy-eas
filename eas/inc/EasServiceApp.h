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

#ifndef EASSERVICEAPP_H_
#define EASSERVICEAPP_H_

// Use real db8 headers with compatibility shims
#include "core/MojCoreDefs.h"
#include "core/MojReactorApp.h"
#include "core/MojGmainReactor.h"  // compat version
#include "luna/MojLunaService.h"
#include <boost/shared_ptr.hpp>

class EasBusDispatcher;
class DatabaseInterface;

/**
 * EasServiceApp - Main application class for mojomail-eas
 *
 * Creates the Luna service, initializes database connection,
 * and runs the main event loop.
 */
class EasServiceApp : public MojReactorApp<MojGmainReactor>
{
public:
    static const char* const ServiceName;

    EasServiceApp();
    virtual ~EasServiceApp();

    virtual MojErr open();
    virtual MojErr close();
    virtual MojErr configure(const MojObject& conf);

    MojLunaService& GetService() { return m_service; }

protected:
    typedef MojReactorApp<MojGmainReactor> Base;

private:
    MojLunaService                          m_service;
    boost::shared_ptr<DatabaseInterface>    m_dbInterface;
    boost::shared_ptr<EasBusDispatcher>     m_handler;
};

#endif /* EASSERVICEAPP_H_ */
