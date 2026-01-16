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

#include "wbxml/WbxmlParser.h"
#include "wbxml/EasTagCode.h"

MojLogger WbxmlParser::s_log("com.palm.eas.wbxml.parser");

// WBXML token constants
namespace {
    const unsigned char TOKEN_SWITCH_PAGE = 0x00;
    const unsigned char TOKEN_END = 0x01;
    const unsigned char TOKEN_ENTITY = 0x02;
    const unsigned char TOKEN_STR_I = 0x03;
    const unsigned char TOKEN_LITERAL = 0x04;
    const unsigned char TOKEN_EXT_I_0 = 0x40;
    const unsigned char TOKEN_EXT_I_1 = 0x41;
    const unsigned char TOKEN_EXT_I_2 = 0x42;
    const unsigned char TOKEN_PI = 0x43;
    const unsigned char TOKEN_LITERAL_C = 0x44;
    const unsigned char TOKEN_EXT_T_0 = 0x80;
    const unsigned char TOKEN_EXT_T_1 = 0x81;
    const unsigned char TOKEN_EXT_T_2 = 0x82;
    const unsigned char TOKEN_STR_T = 0x83;
    const unsigned char TOKEN_LITERAL_A = 0x84;
    const unsigned char TOKEN_EXT_0 = 0xC0;
    const unsigned char TOKEN_EXT_1 = 0xC1;
    const unsigned char TOKEN_EXT_2 = 0xC2;
    const unsigned char TOKEN_OPAQUE = 0xC3;
    const unsigned char TOKEN_LITERAL_AC = 0xC4;

    const unsigned char TAG_HAS_CONTENT = 0x40;
    const unsigned char TAG_HAS_ATTRS = 0x80;
    const unsigned char TAG_MASK = 0x3F;
}

WbxmlParser::WbxmlParser(WbxmlContentHandler& handler)
    : m_handler(handler)
    , m_data(NULL)
    , m_dataLength(0)
    , m_position(0)
    , m_currentCodePage(0)
    , m_log(s_log)
{
}

WbxmlParser::~WbxmlParser()
{
}

void WbxmlParser::Reset()
{
    m_data = NULL;
    m_dataLength = 0;
    m_position = 0;
    m_currentCodePage = 0;
    while (!m_tagStack.empty()) {
        m_tagStack.pop();
    }
}

bool WbxmlParser::HandleInput(const char* data, unsigned int length)
{
    if (!data || length == 0) {
        return false;
    }

    m_data = reinterpret_cast<const unsigned char*>(data);
    m_dataLength = length;
    m_position = 0;

    // Parse WBXML header
    if (!ParseHeader()) {
        MojLogError(m_log, "Failed to parse WBXML header");
        return false;
    }

    // Parse body
    while (m_position < m_dataLength) {
        if (!ParseElement()) {
            return false;
        }
    }

    return true;
}

bool WbxmlParser::ParseHeader()
{
    // Need at least 4 bytes for minimal header
    if (m_dataLength < 4) {
        MojLogError(m_log, "WBXML data too short for header");
        return false;
    }

    // Version (should be 0x03 for WBXML 1.3)
    unsigned char version = ReadByte();
    if (version != 0x03) {
        MojLogWarning(m_log, "Unexpected WBXML version: %d", version);
    }

    // Public ID (multi-byte int or literal)
    unsigned int publicId = ReadMultiByteInt();
    (void)publicId;  // We don't use this

    // Charset (multi-byte int)
    unsigned int charset = ReadMultiByteInt();
    (void)charset;  // Assumed UTF-8

    // String table length
    unsigned int stringTableLen = ReadMultiByteInt();
    if (stringTableLen > 0) {
        // Skip string table (we don't use it)
        m_position += stringTableLen;
    }

    return m_position <= m_dataLength;
}

bool WbxmlParser::ParseElement()
{
    if (m_position >= m_dataLength) {
        return true;  // End of data
    }

    unsigned char byte = ReadByte();

    // Check for global tokens
    switch (byte) {
        case TOKEN_SWITCH_PAGE:
            SetCodePage(ReadByte());
            return true;

        case TOKEN_END:
            CloseTag();
            return true;

        case TOKEN_STR_I:
            // Inline string content
            {
                std::string text = ReadInlineString();
                m_handler.Text(text);
            }
            return true;

        case TOKEN_OPAQUE:
            // Opaque data
            {
                unsigned int length = ReadMultiByteInt();
                std::vector<unsigned char> data(length);
                for (unsigned int i = 0; i < length && m_position < m_dataLength; ++i) {
                    data[i] = ReadByte();
                }
                m_handler.OpaqueData(data);
            }
            return true;

        case TOKEN_ENTITY:
        case TOKEN_EXT_I_0:
        case TOKEN_EXT_I_1:
        case TOKEN_EXT_I_2:
        case TOKEN_PI:
        case TOKEN_EXT_T_0:
        case TOKEN_EXT_T_1:
        case TOKEN_EXT_T_2:
        case TOKEN_STR_T:
        case TOKEN_EXT_0:
        case TOKEN_EXT_1:
        case TOKEN_EXT_2:
            // These tokens are not used in EAS
            MojLogWarning(m_log, "Unexpected WBXML token: 0x%02x", byte);
            return true;

        default:
            // Must be a tag
            return ParseTag(byte);
    }
}

bool WbxmlParser::ParseTag(unsigned char tagByte)
{
    bool hasContent = (tagByte & TAG_HAS_CONTENT) != 0;
    bool hasAttrs = (tagByte & TAG_HAS_ATTRS) != 0;
    unsigned char tagNum = tagByte & TAG_MASK;

    // Build full tag ID
    EasTagCode::EasTag tag = EasTagCode::BuildId(m_currentCodePage, tagNum);

    // Skip attributes if present (not used in EAS)
    if (hasAttrs) {
        // Read until END token
        while (m_position < m_dataLength) {
            unsigned char b = ReadByte();
            if (b == TOKEN_END) {
                break;
            }
        }
    }

    // Notify handler of start element
    OpenTag(tagNum);
    m_handler.StartElement(tag);

    if (hasContent) {
        // Push to stack for matching END
        m_tagStack.push(tag);
    } else {
        // Empty element - immediately close
        m_handler.EndElement(tag);
    }

    return true;
}

void WbxmlParser::SetCodePage(unsigned char codePage)
{
    m_currentCodePage = codePage;
}

void WbxmlParser::OpenTag(unsigned char tag)
{
    // Track open tag (tag number only, code page tracked separately)
    (void)tag;
}

void WbxmlParser::CloseTag()
{
    if (!m_tagStack.empty()) {
        EasTagCode::EasTag tag = m_tagStack.top();
        m_tagStack.pop();
        m_handler.EndElement(tag);
    }
}

unsigned char WbxmlParser::ReadByte()
{
    if (m_position >= m_dataLength) {
        return 0;
    }
    return m_data[m_position++];
}

unsigned int WbxmlParser::ReadMultiByteInt()
{
    unsigned int value = 0;

    while (m_position < m_dataLength) {
        unsigned char b = ReadByte();
        value = (value << 7) | (b & 0x7F);
        if ((b & 0x80) == 0) {
            break;  // Last byte
        }
    }

    return value;
}

std::string WbxmlParser::ReadInlineString()
{
    std::string result;

    while (m_position < m_dataLength) {
        unsigned char c = ReadByte();
        if (c == 0) {
            break;  // Null terminator
        }
        result += static_cast<char>(c);
    }

    return result;
}

bool WbxmlParser::ParseStateless(unsigned char byte)
{
    // Stateless parsing of a single byte (for streaming)
    return ParseByte(byte);
}

bool WbxmlParser::ParseByte(unsigned char byte)
{
    // This would be used for streaming parsing
    // For now, use HandleInput for batch parsing
    (void)byte;
    return true;
}

void WbxmlParser::ParseInlineStringByte(unsigned char byte)
{
    // Handle a byte during inline string parsing
    (void)byte;
}
