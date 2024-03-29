/**********************************************************************************************
*
*   raylib_ECS_sample * a sample Entity Component System using raylib
*
*   LICENSE: ZLIB
*
*   Copyright (c) 2021 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

#include "entity_manager.h"

#include <map>

class System
{
public:
    System(EntitySet& entities)
        :Entities(entities)
    {
    }
    virtual void OnCreate() {}
    virtual size_t Id() { return 0; }
    virtual const char* SystemName() { return nullptr; }

protected:
    EntitySet& Entities;
};

#define DEFINE_SYSTEM(TYPE) \
    TYPE(EntitySet& entities) : System(entities) { OnCreate(); } \
    static size_t GetSystemId() { return reinterpret_cast<size_t>(#TYPE); } \
    size_t Id() override { return TYPE::GetSystemId(); } \
    const char* SystemName() override { return #TYPE; }

class SystemSet
{
public:
    SystemSet(EntitySet& entities);
    virtual ~SystemSet();

    System* GetSystem(size_t id);
    System* AddSystem(System* system);

    template<class T>
    inline T* GetSystem()
    {
        T* system = static_cast<T*>(GetSystem(T::GetSystemId()));
        if (system != nullptr)
            return system;

        return static_cast<T*>(AddSystem(new T(this->Entitites)));
    }

private:
    EntitySet& Entitites;
    std::map<size_t, System*> SystemMap;
};