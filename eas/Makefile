# Makefile for mojomail-eas
# Cross-compiling for ARM (HP TouchPad)

# ARM Toolchain (CodeSourcery 2009q1)
TOOLCHAIN = /home/herrie/Documents/GitHub/build/toolchain/cs09q1armel/build/arm-2009q1
CROSS_COMPILE = $(TOOLCHAIN)/bin/arm-none-linux-gnueabi-

# Build dependencies
DEPS_DIR = /home/herrie/webos/touchpad-kernel/doctor305/build-deps
ISIS_STAGING = /home/herrie/webos/touchpad-kernel/doctor305/isis-project/staging/armv7
ROOTFS_DIR = /home/herrie/webos/touchpad-kernel/doctor305/untouched-rootfs
ROOTFS_LIB = $(ROOTFS_DIR)/usr/lib

# OpenSSL (for CURL)
OPENSSL_DIR = /home/herrie/webos/touchpad-kernel/doctor305/OpenSSL-11-Update/openssl-1.1.1w
CURL_LIB = /home/herrie/webos/touchpad-kernel/doctor305/OpenSSL-11-Update/package/usr/lib
CURL_DIR = $(DEPS_DIR)/woce-build-support/staging/arm-none-linux-gnueabi

CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# Toolchain sysroot
SYSROOT = $(TOOLCHAIN)/arm-none-linux-gnueabi/libc

# Mojo/DB8 headers (from github.com/openwebos/db8)
MOJO_DIR = $(DEPS_DIR)/db8/inc

# email-common headers (from mojomail common)
COMMON_DIR = ../common/inc

# Compatibility shims directory
COMPAT_DIR = ./compat

# Include directories
INCLUDES = -I./inc \
           -I$(COMMON_DIR) \
           -I$(COMPAT_DIR) \
           -I$(MOJO_DIR) \
           -I$(MOJO_DIR)/core \
           -I$(MOJO_DIR)/db \
           -I$(MOJO_DIR)/luna \
           -I$(ISIS_STAGING)/usr/include \
           -I$(ISIS_STAGING)/usr/include/glib-2.0 \
           -I$(DEPS_DIR)/woce-build-support/staging/arm-none-linux-gnueabi/lib/glib-2.0/include \
           -I$(CURL_DIR)/include

# Compiler flags
# -std=c++0x enables C++11 features (partial support in GCC 4.3/4.4)
CXXFLAGS = -Wall -Wextra -g -O2 \
           -std=c++0x \
           -DMOJ_LINUX \
           -march=armv7-a \
           $(INCLUDES)

# Linker flags
LDFLAGS = --sysroot=$(SYSROOT) \
          -L$(SYSROOT)/lib -L$(SYSROOT)/usr/lib \
          -L$(ISIS_STAGING)/usr/lib -lglib-2.0 -lgthread-2.0 \
          -L$(ROOTFS_LIB) -lemail-common -lmojocore -lmojoluna -lmojodb -llunaservice -lcjson \
          -L$(CURL_LIB) -lcurl \
          -L$(OPENSSL_DIR) -lssl -lcrypto \
          -Wl,-rpath-link,$(ROOTFS_DIR)/lib \
          -Wl,-rpath-link,$(ROOTFS_LIB) \
          -Wl,-rpath,/usr/lib

# Compatibility shims for symbols missing from old libmojocore
COMPAT_SRCS = src/compat/MojObjectCompat.cpp

# Source files
WBXML_SRCS = src/wbxml/WbxmlEncoder.cpp \
             src/wbxml/WbxmlParser.cpp \
             src/wbxml/EasTagCode.cpp

COMMAND_SRCS = src/commands/EasCommand.cpp \
               src/commands/FolderSyncCommand.cpp \
               src/commands/OptionsCommand.cpp \
               src/commands/ProvisionCommand.cpp \
               src/commands/PingCommand.cpp \
               src/commands/SyncCommand.cpp

DATA_SRCS = src/data/EasAccount.cpp \
            src/data/EasFolder.cpp \
            src/data/EasPolicy.cpp \
            src/data/EasEmailAdapter.cpp \
            src/data/EasContactAdapter.cpp \
            src/data/EasEventAdapter.cpp \
            src/data/EasTaskAdapter.cpp

CORE_SRCS = src/EasClient.cpp \
            src/EasBusDispatcher.cpp \
            src/EasConfig.cpp \
            src/EasServiceApp.cpp

ALL_SRCS = $(WBXML_SRCS) $(COMMAND_SRCS) $(DATA_SRCS) $(CORE_SRCS) $(COMPAT_SRCS)
ALL_OBJS = $(ALL_SRCS:.cpp=.o)

# Main target
all: mojomail-eas

mojomail-eas: $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(ALL_OBJS)
	rm -f mojomail-eas

# Check syntax only (no linking)
check-syntax: $(ALL_OBJS)
	@echo "Syntax check passed!"

.PHONY: all clean check-syntax
