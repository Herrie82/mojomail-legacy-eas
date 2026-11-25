# @@@LICENSE
#
#      Copyright (c) 2007-2013 LG Electronics, Inc.
#      Copyright (c) 2024 Herman van Hazendonk <github.com@herrie.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

cmake_minimum_required(VERSION 2.8.7)

project(mojomail-eas CXX)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC2)
webos_component(1 0 0)

webos_add_compiler_flags(ALL "-DMOJ_LINUX")
webos_add_compiler_flags(ALL "-O -ggdb -Wuninitialized -Winit-self")

webos_add_linker_options(ALL "--unresolved-symbols=ignore-in-shared-libs -rpath-link ${WEBOS_INSTALL_ROOT}/lib")

webos_machine_dep()

include_directories(inc)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)
include_directories(Boost_INCLUDE_DIRS)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS})

pkg_check_modules(LUNASERVICE REQUIRED luna-service2)
include_directories(${LUNASERVICE_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE_CFLAGS})

pkg_check_modules(CJSON cjson)
include_directories(${CJSON_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${CJSON_CFLAGS})

pkg_check_modules(EMAILCOMMON email-common)
include_directories(${EMAILCOMMON_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${EMAILCOMMON_CFLAGS})

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS})

pkg_check_modules(DB8 REQUIRED db8)
include_directories(${DB8_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${DB8_CFLAGS_OTHER})

pkg_check_modules(CURL REQUIRED libcurl)
include_directories(${CURL_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${CURL_CFLAGS})

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
include_directories(${LIBXML2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LIBXML2_CFLAGS})

find_library(ICU NAMES icuuc)
if(ICU STREQUAL "ICU-NOTFOUND")
   message(FATAL_ERROR "Failed to find ICU4C libraries. Please install.")
endif()

# Source files
aux_source_directory(src/activity activity_files)
aux_source_directory(src/client client_files)
aux_source_directory(src/commands commands_files)
aux_source_directory(src/data data_files)
aux_source_directory(src/exceptions exceptions_files)
aux_source_directory(src/protocol protocol_files)
aux_source_directory(src/sync sync_files)
aux_source_directory(src/wbxml wbxml_files)

add_executable(mojomail-eas
    src/EasBusDispatcher.cpp
    src/EasConfig.cpp
    src/EasClient.cpp
    src/EasServiceApp.cpp
    ${activity_files}
    ${client_files}
    ${commands_files}
    ${data_files}
    ${exceptions_files}
    ${protocol_files}
    ${sync_files}
    ${wbxml_files}
)

target_link_libraries(mojomail-eas
    ${GLIB2_LDFLAGS}
    ${DB8_LDFLAGS}
    ${ICU}
    ${EMAILCOMMON_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${CURL_LDFLAGS}
    ${LIBXML2_LDFLAGS}
    ${Boost_LIBRARIES}
)

webos_build_program(NAME mojomail-eas)
webos_build_system_bus_files()
webos_build_db8_files()
