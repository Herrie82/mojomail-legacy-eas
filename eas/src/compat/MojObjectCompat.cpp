// MojObjectCompat.cpp - Provides missing MojObject symbols for webOS 3.0.5
// These functions are needed because the newer db8 headers declare them
// but the older libmojocore.so doesn't export them.

#include "core/MojObject.h"

// MojObject::init(const MojObject& obj)
// Copy constructor helper - clones the implementation
void MojObject::init(const MojObject& obj)
{
    release();

    if (obj.impl())
        m_implementation = obj.impl()->clone();
    else
        m_implementation = NULL;
}

// MojObject::release()
// Destructor helper - deletes the implementation
void MojObject::release()
{
    if (m_implementation) {
        delete m_implementation;
        m_implementation = NULL;
    }
}
