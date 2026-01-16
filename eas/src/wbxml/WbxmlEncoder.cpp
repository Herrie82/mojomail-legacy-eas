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

#include "wbxml/WbxmlEncoder.h"
#include "wbxml/EasTagCode.h"
#include <cstring>

// WBXML version and encoding constants
namespace {
    const unsigned char WBXML_VERSION = 0x03;      // WBXML 1.3
    const unsigned char WBXML_PUBLIC_ID = 0x01;    // Unknown/None
    const unsigned char WBXML_CHARSET_UTF8 = 0x6A; // UTF-8 (106)
    const unsigned char WBXML_STRING_TABLE_LENGTH = 0x00;

    // Token constants
    const unsigned char TOKEN_SWITCH_PAGE = 0x00;
    const unsigned char TOKEN_END = 0x01;
    const unsigned char TOKEN_STR_I = 0x03;        // Inline string
    const unsigned char TOKEN_OPAQUE = 0xC3;       // Opaque data

    // Tag flags
    const unsigned char TAG_HAS_CONTENT = 0x40;
    const unsigned char TAG_HAS_ATTRS = 0x80;
}

WbxmlEncoder::WbxmlEncoder()
    : m_currentCodePage(0)
{
    Init();
}

WbxmlEncoder::~WbxmlEncoder()
{
}

void WbxmlEncoder::Init()
{
    m_buffer.clear();
    m_currentCodePage = 0;

    // Write WBXML header
    // Version
    m_buffer.push_back(WBXML_VERSION);
    // Public ID (unknown)
    m_buffer.push_back(WBXML_PUBLIC_ID);
    // Charset (UTF-8 as multi-byte int)
    m_buffer.push_back(0x01);  // First byte of 106
    m_buffer.push_back(0x6A);  // Second byte of 106
    // String table length
    m_buffer.push_back(WBXML_STRING_TABLE_LENGTH);
}

void WbxmlEncoder::Reset()
{
    m_buffer.clear();
    m_currentCodePage = 0;
    while (!m_tagStack.empty()) {
        m_tagStack.pop();
    }
    Init();
}

std::vector<unsigned char> WbxmlEncoder::GetWbxml()
{
    return m_buffer;
}

void WbxmlEncoder::StartTag(EasTagCode::EasTag tag)
{
    StartTag(tag, true);
}

void WbxmlEncoder::StartTag(EasTagCode::EasTag tag, bool hasContent)
{
    unsigned char codePage = EasTagCode::GetCodePage(tag);
    unsigned char tagNum = EasTagCode::GetTagNumber(tag);

    // Switch code page if needed
    if (codePage != m_currentCodePage) {
        m_buffer.push_back(TOKEN_SWITCH_PAGE);
        m_buffer.push_back(codePage);
        m_currentCodePage = codePage;
    }

    // Write tag with content flag
    unsigned char tagByte = tagNum;
    if (hasContent) {
        tagByte |= TAG_HAS_CONTENT;
    }
    m_buffer.push_back(tagByte);

    // Push to stack if has content (needs END token later)
    if (hasContent) {
        m_tagStack.push(tag);
    }
}

void WbxmlEncoder::EndTag(EasTagCode::EasTag tag)
{
    if (!m_tagStack.empty()) {
        m_tagStack.pop();
    }
    m_buffer.push_back(TOKEN_END);
}

void WbxmlEncoder::EmptyTag(EasTagCode::EasTag tag)
{
    StartTag(tag, false);
}

void WbxmlEncoder::Text(EasTagCode::EasTag tag, const std::string& value)
{
    if (value.empty()) {
        // Empty value - write empty tag
        EmptyTag(tag);
        return;
    }

    StartTag(tag, true);
    WriteInlineString(value);
    EndTag(tag);
}

void WbxmlEncoder::WriteInlineString(const std::string& value)
{
    m_buffer.push_back(TOKEN_STR_I);
    for (size_t i = 0; i < value.length(); ++i) {
        m_buffer.push_back(static_cast<unsigned char>(value[i]));
    }
    m_buffer.push_back(0x00);  // Null terminator
}

void WbxmlEncoder::WriteOpaqueData(const unsigned char* data, size_t length)
{
    m_buffer.push_back(TOKEN_OPAQUE);
    WriteMultiByteInt(static_cast<unsigned int>(length));
    for (size_t i = 0; i < length; ++i) {
        m_buffer.push_back(data[i]);
    }
}

void WbxmlEncoder::WriteOpaqueData(const std::vector<unsigned char>& data)
{
    WriteOpaqueData(data.data(), data.size());
}

void WbxmlEncoder::WriteMultiByteInt(unsigned int value)
{
    // Multi-byte integer encoding (most significant bits first)
    // Each byte has 7 bits of data, high bit indicates continuation
    unsigned char bytes[5];
    int count = 0;

    do {
        bytes[count++] = value & 0x7F;
        value >>= 7;
    } while (value > 0);

    // Write in reverse order with continuation bits
    for (int i = count - 1; i >= 0; --i) {
        unsigned char b = bytes[i];
        if (i > 0) {
            b |= 0x80;  // Set continuation bit
        }
        m_buffer.push_back(b);
    }
}

void WbxmlEncoder::Print(const char* debug)
{
    // Debug output - could log to MojLogger
    (void)debug;
}

// Convenience method for integer values (as string)
void WbxmlEncoder::TextInt(EasTagCode::EasTag tag, int value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    Text(tag, std::string(buf));
}
