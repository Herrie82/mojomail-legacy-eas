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

#ifndef WBXMLENCODER_H_
#define WBXMLENCODER_H_

#include "wbxml/EasTagCode.h"
#include <string>
#include <vector>
#include <stack>

/**
 * WbxmlEncoder - Encodes XML to WBXML for EAS protocol
 *
 * Exchange ActiveSync uses WBXML (WAP Binary XML) for all communications.
 * This encoder creates WBXML-formatted byte arrays from structured data.
 *
 * Reference: [MS-ASWBXML] Exchange ActiveSync: WAP Binary XML Algorithm
 *
 * Usage:
 *   WbxmlEncoder encoder;
 *   encoder.StartTag(EasTagCode::FolderHierarchy_FolderSync);
 *   encoder.Text(EasTagCode::FolderHierarchy_SyncKey, "0");
 *   encoder.EndTag(EasTagCode::FolderHierarchy_FolderSync);
 *   std::vector<unsigned char> wbxml = encoder.GetWbxml();
 */
class WbxmlEncoder
{
public:
    WbxmlEncoder();
    virtual ~WbxmlEncoder();

    /**
     * Initialize encoder and write WBXML header
     */
    void Init();

    /**
     * Reset encoder state
     */
    void Reset();

    /**
     * Get the encoded WBXML data
     * @return Vector of bytes containing WBXML data
     */
    std::vector<unsigned char> GetWbxml();

    /**
     * Start an element tag with content
     * @param tag Combined tag ID from EasTagCode
     */
    void StartTag(EasTagCode::EasTag tag);

    /**
     * Start an element tag, optionally without content
     * @param tag Combined tag ID from EasTagCode
     * @param hasContent true if element will have content/children
     */
    void StartTag(EasTagCode::EasTag tag, bool hasContent);

    /**
     * End the current element
     * @param tag The tag being closed (for verification)
     */
    void EndTag(EasTagCode::EasTag tag);

    /**
     * Write an empty (self-closing) element
     * @param tag Combined tag ID from EasTagCode
     */
    void EmptyTag(EasTagCode::EasTag tag);

    /**
     * Write an element with text content
     * @param tag Combined tag ID from EasTagCode
     * @param value Text content
     */
    void Text(EasTagCode::EasTag tag, const std::string& value);

    /**
     * Write an element with integer content (as string)
     * @param tag Combined tag ID from EasTagCode
     * @param value Integer value
     */
    void TextInt(EasTagCode::EasTag tag, int value);

    /**
     * Write inline string content (within current element)
     * @param value Text content
     */
    void WriteInlineString(const std::string& value);

    /**
     * Write opaque binary data
     * @param data Pointer to data
     * @param length Length of data
     */
    void WriteOpaqueData(const unsigned char* data, size_t length);

    /**
     * Write opaque binary data
     * @param data Vector of bytes
     */
    void WriteOpaqueData(const std::vector<unsigned char>& data);

    /**
     * Debug output
     */
    void Print(const char* debug);

private:
    void WriteMultiByteInt(unsigned int value);

    std::vector<unsigned char>      m_buffer;
    std::stack<EasTagCode::EasTag>  m_tagStack;
    unsigned char                   m_currentCodePage;
};

#endif /* WBXMLENCODER_H_ */
