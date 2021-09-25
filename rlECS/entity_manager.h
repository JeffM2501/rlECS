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

#include <stdint.h>
#include <functional>
#include <set>
#include <vector>
#include <map>

class EntitySet;
class Component;


using ComponentList = std::vector<Component*>;

class ComponentTable
{
public:
    std::map<uint64_t, ComponentList> Entities;

    virtual ~ComponentTable();

    bool Add(Component* component);
};

class EntitySet
{
private:
    std::set<uint64_t> EntityMap;
    uint64_t NextEntity = 0;

    std::map<size_t, ComponentTable> ComponentDB;
    std::vector<Component*> ComponentUpdateCache;

private:
    Component* StoreComponent(size_t componentId, Component* component);
    void EraseAllComponents(size_t componentId, uint64_t entityId);
    void EraseComponent(size_t componentId, Component* component);
    Component* FindComponent(size_t componentId, uint64_t entityId);
    const std::vector<Component*>& FindComponents(size_t componentId, uint64_t entityId);

public:
    uint64_t CreateEntity();
    void RemoveEntity(uint64_t entityId);
    void Update();

    /// <summary>
    /// Iterate all entities by Id
    /// </summary>
    /// <param name="func">Callback to run for every entity</param>
    void DoForEachEntity(std::function<void(uint64_t)> func);

    /// <summary>
    /// Iterate all the entities with a component
    /// </summary>
    /// <param name="componentId">The component ID</param>
    /// <param name="func">Callback to be run for every entity with the component</param>
    void DoForEachEntity(size_t componentId, std::function<void(Component*)> func);

    /// <summary>
    /// Iterate all components for an entity
    /// </summary>
    /// <param name="entityId">The entity to itterate</param>
    /// <param name="func">the callback to run for every component on an entity</param>
    void DoForEachComponentInEntity(uint64_t entityId, std::function<void(Component*)> func);

    /// <summary>
    /// Iterate all the entities with a component
    /// </summary>
    /// <typeparam name="T">Component to iterate</typeparam>
    /// <param name="func">callback to call with each entity that has a component</param>
    template<class T>
    inline void DoForEachEntity(std::function<void(T*)> func)
    {
        DoForEachEntity(T::GetComponentId(), [func](Component* comp) {func(static_cast<T*>(comp)); });
    }

    template<class T>
    inline T* AddComponent(uint64_t entityId)
    {
        T* component = ComponentManager::Create<T>(entityId, *this);
        return static_cast<T*>(StoreComponent(component->Id(), component));
    }

    template<class T>
    inline T* AddComponent()
    {
        T* component = ComponentManager::Create<T>(EntitySet::CreateEntity(), *this);
        if (component == nullptr)
            return nullptr;

        return static_cast<T*>(StoreComponent(component->Id(), component));
    }

    template<class T>
    inline T* AddComponent(Component* component)
    {
        if (component == nullptr)
            return AddComponent<T>();

        T* newComponent = ComponentManager::Create<T>(component->EntityId, *this);

        return static_cast<T*>(StoreComponent(newComponent->Id(), newComponent));
    }

    template<class T>
    inline void RemoveComponents(uint64_t entityId)
    {
        EraseAllComponents(T::GetComponentId(), entityId);
    }

    template<class T>
    inline void RemoveComponent(Component* component)
    {
        if (component == nullptr)
            return;

        EraseComponent(component->Id(), component);
    }

    template<class T>
    inline T* GetComponent(uint64_t entityId)
    {
        return static_cast<T*>(FindComponent(T::GetComponentId(), entityId));
    }

    template<class T>
    inline T* GetComponent(Component* component)
    {
        return static_cast<T*>(FindComponent(T::GetComponentId(), component->EntityId));
    }

    template<class T>
    inline T* MustGetComponent(uint64_t entityId)
    {
        T* newComponent = static_cast<T*>(FindComponent(T::GetComponentId(), entityId));
        if (newComponent != nullptr)
            return newComponent;

        return AddComponent<T>(entityId);
    }

    template<class T>
    inline T* MustGetComponent(Component* component)
    {
        T* newComponent = static_cast<T*>(FindComponent(T::GetComponentId(), component->EntityId));
        if (newComponent != nullptr)
            return newComponent;

        return AddComponent<T>(component->EntityId);
    }
};

class Component
{
protected:
    bool NeedUpdate = false;
    EntitySet& Entities;

public:
    uint64_t EntityId;
    bool Active = true;

public:
    Component(uint64_t id, EntitySet& entities) 
    : EntityId(id) 
    , Entities(entities)
    {}

    virtual ~Component() = default;
    virtual size_t Id() { return 0; }
    virtual size_t TypeId() { return 0; }
    virtual const char* ComponentName() { return nullptr; }

    virtual void OnCreate() {}
    virtual void OnDestory() {}
    virtual void OnUpdate() {}

    inline bool WantUpdate() { return NeedUpdate; }

    template<class T>
    inline T* GetComponent()
    {
        return Entities.GetComponent<T>(this);
    }

    template<class T>
    inline T* MustGetComponent()
    {
        return Entities.MustGetComponent<T>(this);
    }

    template<class T>
    inline T* MustGetComponent(uint64_t id)
    {
        return Entities.MustGetComponent<T>(id);
    }
};

#define DEFINE_COMPONENT(TYPE) \
    TYPE(uint64_t id, EntitySet& entities) : Component(id, entities) {} \
    static size_t GetComponentId() { return reinterpret_cast<size_t>(#TYPE); } \
    static const char* GetComponentName() { return #TYPE; } \
    size_t Id() override { return TYPE::GetComponentId(); } \
    const char* ComponentName() override { return #TYPE; } \
    static TYPE* Factory(uint64_t id, EntitySet& entities) { return new TYPE(id, entities); }

#define DEFINE_DERIVED_COMPONENT(TYPE, BASETYPE) \
    TYPE(uint64_t id, EntitySet& entities) : BASETYPE(id, entities) {} \
    static size_t GetComponentId() { return reinterpret_cast<size_t>(#BASETYPE); } \
    static const char* GetComponentName() { return #TYPE; } \
    size_t Id() override { return TYPE::GetComponentId(); } \
    const char* ComponentName() override { return #TYPE; } \
    static TYPE* Factory(uint64_t id, EntitySet& entities) { return new TYPE(id, entities); }

using ComponentFactory = std::function<Component* (uint64_t, EntitySet&)>;

namespace ComponentManager
{
    void Register(size_t typeId, const char* name, ComponentFactory factory);
    Component* Create(size_t typeId, uint64_t entityId, EntitySet& manager);
    Component* Create(const char* typeName, uint64_t entityId, EntitySet& manager);

    template<class T>
    inline void Register()
    {
        Register(T::GetComponentId(), T::GetComponentName(), T::Factory);
    }

    template<class T>
    inline T* Create(uint64_t entityId, EntitySet& entities)
    {
        Component* comp = Create(T::GetComponentId(), entityId, entities);
        if (comp == nullptr)
        {
            // auto register
            Register<T>();
            comp = Create(T::GetComponentId(), entityId, entities);
        }

        if (comp == nullptr)
            return nullptr;

        return static_cast<T*>(comp);
    }
}
