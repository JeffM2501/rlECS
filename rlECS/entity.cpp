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

#include "entity.h"

#include <algorithm>
#include <map>

struct ComponentInfo
{
    size_t Id = 0;
    const char* Name = nullptr;
    ComponentFactory Factory = nullptr;
};

std::map<size_t, ComponentInfo> ComponentFactories;

// template<class T>
// inline T* Component::GetComponent()
// {
//     return Entitites.GetComponent<T>(this);
// }
// 
// template<class T>
// inline T* Component::MustGetComponent()
// {
//     return Entitites.MustGetComponent<T>(this);
// }
// 
// template<class T>
// inline T* Component::MustGetComponent(uint64_t id)
// {
//     return Entitites.MustGetComponent<T>(id);
// }


namespace ComponentManager
{
    void Register(size_t typeId, const char* name, ComponentFactory factory)
    {
        ComponentFactories[typeId] = ComponentInfo{ typeId, name, factory };
    }

    Component* Create(size_t typeId, uint64_t entityId, EntitySet& entities)
    {
        std::map<size_t, ComponentInfo>::iterator itr = ComponentFactories.find(typeId);
        if (itr == ComponentFactories.end())
            return nullptr;

        return itr->second.Factory(entityId, entities);
    }

    Component* Create(const char* typeName, uint64_t entityId, EntitySet& entities)
    {
        for (auto itr : ComponentFactories)
        {
            if (strcmp(itr.second.Name, typeName) == 0)
            {
                return Create(itr.first, entityId, entities);
            }
        }

        return nullptr;
    }
}

ComponentTable::~ComponentTable()
{
    for (auto& r : Entities)
    {
        ComponentList& components = r.second;
        for (auto c : components)
            delete(c);

        components.clear();
    }
    Entities.clear();
}

bool ComponentTable::Add(Component* component)
{
    ComponentList& components = Entities[component->EntityId];
    if (components.empty())
    {
        components.push_back(component);
        return true;
    }

    auto itr = std::find(components.begin(), components.end(), component);
    if (itr != components.end())
        return false;

    components.push_back(component);
    return true;
}

uint64_t EntitySet::CreateEntity()
{
    while (EntityMap.find(NextEntity) != EntityMap.end())
        NextEntity++;

    EntityMap.emplace(NextEntity);
    NextEntity++;

    return NextEntity - 1;
}

void EntitySet::RemoveEntity(uint64_t entityId)
{
    for (auto componentTable : ComponentDB)
    {
        EraseAllComponents(componentTable.first, entityId);
    }

    auto itr = EntityMap.find(entityId);
    if (itr == EntityMap.end())
        return;

    EntityMap.erase(itr);
}

void EntitySet::Update()
{
    for (auto* component : ComponentUpdateCache)
    {
        if (component->Active)
            component->OnUpdate();
    }
}

void EntitySet::DoForEachEntity(std::function<void(uint64_t)> func)
{
    for (uint64_t entity : EntityMap)
        func(entity);
}

Component* EntitySet::StoreComponent(size_t compId, Component* component)
{
    ComponentTable& componentTable = ComponentDB[compId];

    if (!componentTable.Add(component))
        return component;

    component->OnCreate();

    if (component->WantUpdate())
        ComponentUpdateCache.push_back(component);

    return component;
}

Component* EntitySet::FindComponent(size_t compId, uint64_t entityId)
{
    auto componentTableItr = ComponentDB.find(compId);
    if (componentTableItr == ComponentDB.end())
        return nullptr;

    ComponentTable& componentTable = componentTableItr->second;

    auto entityCacheItr = componentTable.Entities.find(entityId);
    if (entityCacheItr == componentTable.Entities.end() || entityCacheItr->second.empty())
        return nullptr;

    return entityCacheItr->second[0];
}

static std::vector<Component*> EmptyComponentList;

const std::vector<Component*>& EntitySet::FindComponents(size_t compId, uint64_t entityId)
{
    auto componentTableItr = ComponentDB.find(compId);
    if (componentTableItr == ComponentDB.end())
        return EmptyComponentList;

    ComponentTable& componentTable = componentTableItr->second;

    std::map<uint64_t, ComponentList>::iterator entityCacheItr = componentTable.Entities.find(entityId);
    if (entityCacheItr == componentTable.Entities.end())
        return EmptyComponentList;

    return entityCacheItr->second;
}

void EntitySet::EraseAllComponents(size_t compId, uint64_t entityId)
{
    auto componentTableItr = ComponentDB.find(compId);
    if (componentTableItr == ComponentDB.end())
        return;

    ComponentTable& componentTable = componentTableItr->second;

    auto entityCacheItr = componentTable.Entities.find(entityId);
    if (entityCacheItr != componentTable.Entities.end())
    {
        ComponentList& components = entityCacheItr->second;
        for (Component* component : components)
        {
            component->OnDestory();
            if (component->WantUpdate())
                ComponentUpdateCache.erase(std::find(ComponentUpdateCache.begin(), ComponentUpdateCache.end(), component));

            delete(component);
        }

        componentTable.Entities.erase(entityCacheItr);
    }
}

void EntitySet::EraseComponent(size_t compId, Component* component)
{
    auto componentTableItr = ComponentDB.find(compId);
    if (componentTableItr == ComponentDB.end())
        return;

    ComponentTable& componentTable = componentTableItr->second;

    auto entityCacheItr = componentTable.Entities.find(component->EntityId);
    if (entityCacheItr != componentTable.Entities.end())
    {
        ComponentList& components = entityCacheItr->second;

        ComponentList::iterator itr = std::find(components.begin(), components.end(), component);

        component->OnDestory();
        if (component->WantUpdate())
            ComponentUpdateCache.erase(std::find(ComponentUpdateCache.begin(), ComponentUpdateCache.end(), component));

        delete(component);
        components.erase(itr);
    }
}

void EntitySet::DoForEachEntity(size_t compId, std::function<void(Component*)> func)
{
    auto componentTableItr = ComponentDB.find(compId);
    if (componentTableItr == ComponentDB.end())
        return;

    ComponentTable& componentTable = componentTableItr->second;

    for (auto& entity : componentTable.Entities)
    {
        for (Component* component : entity.second)
            func(component);
    }
}

void EntitySet::DoForEachComponentInEntity(uint64_t entityId, std::function<void(Component*)> func)
{
    for (auto componentTable : ComponentDB)
    {
        auto entityItr = componentTable.second.Entities.find(entityId);
        if (entityItr == componentTable.second.Entities.end())
            continue;

        for (Component* component : entityItr->second)
            func(component);
    }
}
