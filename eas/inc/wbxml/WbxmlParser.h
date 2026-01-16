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

#ifndef WBXMLPARSER_H_
#define WBXMLPARSER_H_

#include "core/MojLog.h"
#include "wbxml/EasTagCode.h"
#include <string>
#include <vector>
#include <stack>

/**
 * WbxmlContentHandler - Callback interface for WBXML parsing events
 *
 * Implement this interface to receive parsing events from WbxmlParser.
 * This matches the binary's WbxmlContentHandler interface.
 */
class WbxmlContentHandler
{
public:
    virtual ~WbxmlContentHandler() {}

    /**
     * Called when a start element tag is encountered
     * @param tag Combined tag ID (code page + tag number)
     */
    virtual void StartElement(EasTagCode::EasTag tag) = 0;

    /**
     * Called when an end element tag is encountered
     * @param tag Combined tag ID (code page + tag number)
     */
    virtual void EndElement(EasTagCode::EasTag tag) = 0;

    /**
     * Called when text content is encountered
     * @param text The text content
     */
    virtual void Text(const std::string& text) = 0;

    /**
     * Called when opaque binary data is encountered
     * @param data The binary data
     */
    virtual void OpaqueData(const std::vector<unsigned char>& data) = 0;
};

/**
 * WbxmlParser - Parses WBXML responses from EAS server
 *
 * Exchange ActiveSync uses WBXML (WAP Binary XML) for all communications.
 * This parser converts WBXML byte arrays into structured data via callbacks.
 *
 * Reference: [MS-ASWBXML] Exchange ActiveSync: WAP Binary XML Algorithm
 */
class WbxmlParser
{
public:
    WbxmlParser(WbxmlContentHandler& handler);
    virtual ~WbxmlParser();

    /**
     * Parse WBXML data
     * @param data Pointer to WBXML data
     * @param length Length of data in bytes
     * @return true if parsing succeeded
     */
    bool HandleInput(const char* data, unsigned int length);

    /**
     * Reset parser state
     */
    void Reset();

    /**
     * Set current code page
     */
    void SetCodePage(unsigned char codePage);

    /**
     * Get current code page
     */
    unsigned char GetCurrentCodePage() const { return m_currentCodePage; }

    // Methods matching binary symbols
    void OpenTag(unsigned char tag);
    void CloseTag();
    bool ParseStateless(unsigned char byte);
    bool ParseByte(unsigned char byte);
    void ParseInlineStringByte(unsigned char byte);

protected:
    static MojLogger s_log;

private:
    bool ParseHeader();
    bool ParseElement();
    bool ParseTag(unsigned char tagByte);

    unsigned char ReadByte();
    unsigned int ReadMultiByteInt();
    std::string ReadInlineString();

    WbxmlContentHandler&        m_handler;
    const unsigned char*        m_data;
    size_t                      m_dataLength;
    size_t                      m_position;
    unsigned char               m_currentCodePage;
    std::stack<EasTagCode::EasTag> m_tagStack;
    MojLogger&                  m_log;
};

#endif /* WBXMLPARSER_H_ */
