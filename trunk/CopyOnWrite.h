#pragma once

#include "RefCounted.h"

// Dirty hack... Private part class should be called Private and
// implement Clone method.

#define IMPLEMENT_COPY_ON_WRITE(ClassName)                              \
private:                                                                \
    class Private;                                                      \
    Private* _ptr;                                                      \
    void MakePrivate()                                                  \
    {                                                                   \
        ASSERT(IsValid());                                              \
        if (IsPrivate())                                                \
            return;                                                     \
        Private* copy = _ptr->Clone();                                  \
        _ptr->Release();                                                \
        _ptr = copy;                                                    \
    }                                                                   \
public:                                                                 \
    ClassName() : _ptr(NULL) {}                                         \
    ClassName(const ClassName& obj) : _ptr(NULL) {  *this = obj; }      \
    ~ClassName() {  if (_ptr) _ptr->Release(); }                        \
    ClassName& operator=(const ClassName& obj)                          \
    {                                                                   \
        if (obj._ptr == _ptr)                                           \
            return *this;                                               \
        if (obj._ptr)                                                   \
            obj._ptr->Acquire();                                        \
        if (_ptr)                                                       \
            _ptr->Release();                                            \
        _ptr = obj._ptr;                                                \
        return *this;                                                   \
    }                                                                   \
    bool IsValid() const {  return _ptr != NULL; }                      \
    bool IsPrivate() const { ASSERT(IsValid()); return _ptr->GetRefs() == 1; } \
private:
