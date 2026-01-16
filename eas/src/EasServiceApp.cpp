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

#include "EasServiceApp.h"
#include "EasBusDispatcher.h"
#include "EasConfig.h"
#include "data/DatabaseInterface.h"

int main(int argc, char** argv)
{
    EasServiceApp app;
    return app.main(argc, argv);
}

const char* const EasServiceApp::ServiceName = "com.palm.eas";

EasServiceApp::EasServiceApp()
{
}

EasServiceApp::~EasServiceApp()
{
}

MojErr EasServiceApp::open()
{
    MojErr err;

    // Call superclass open
    err = Base::open();
    MojErrCheck(err);

    // Initialize Luna service
    err = m_service.open(ServiceName);
    MojErrCheck(err);

    err = m_service.attach(m_reactor.impl());
    MojErrCheck(err);

    // Create database interface (stub for now)
    m_dbInterface.reset(new DatabaseInterface());

    // Create bus dispatcher
    m_handler.reset(new EasBusDispatcher(m_service, m_dbInterface));

    err = m_handler->open();
    MojErrCheck(err);

    err = m_service.addCategory(MojLunaService::DefaultCategory, m_handler.get());
    MojErrCheck(err);

    fprintf(stderr, "mojomail-eas running!\n");

    return MojErrNone;
}

MojErr EasServiceApp::close()
{
    MojErr err;

    if (m_handler) {
        err = m_handler->close();
        MojErrCheck(err);
        m_handler.reset();
    }

    err = m_service.close();
    MojErrCheck(err);

    err = Base::close();
    MojErrCheck(err);

    return MojErrNone;
}

MojErr EasServiceApp::configure(const MojObject& conf)
{
    MojErr err;

    err = MojServiceApp::configure(conf);
    MojErrCheck(err);

    // Parse EAS-specific configuration
    err = EasConfig::GetConfig().ParseConfig(conf);
    MojErrCheck(err);

    return MojErrNone;
}
