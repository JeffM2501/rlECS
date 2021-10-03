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

#include "entity_manager.h"

#include <algorithm>
#include <map>


std::map<size_t, ComponentInfo> ComponentFactories;

namespace ComponentManager
{
    void Register(size_t typeId, const char* name, ComponentFactory factory, bool unquie)
    {
        ComponentFactories[typeId] = ComponentInfo{ typeId, name, factory, unquie };
    }

    const std::map<size_t, ComponentInfo>& GetComponentList()
    {
        return ComponentFactories;
    }

    Component* Create(size_t typeId, EntityId_t entityId, EntitySet& entities)
    {
        std::map<size_t, ComponentInfo>::iterator itr = ComponentFactories.find(typeId);
        if (itr == ComponentFactories.end())
            return nullptr;

        Component* comp = itr->second.Factory(entityId, entities);
        entities.StoreComponent(comp->Id(),comp);
        return comp;
    }

    Component* Create(const char* typeName, EntityId_t entityId, EntitySet& entities)
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

EntityId_t EntitySet::CreateEntity()
{
    while (EntityMap.find(NextEntity) != EntityMap.end())
        NextEntity++;

    EntityMap.emplace(NextEntity, Entity{ NextEntity });
    RootNodes.insert(NextEntity);
    NextEntity++;

    return NextEntity - 1;
}

void EntitySet::RemoveFromParent(Entity* entity)
{
    if (entity == nullptr)
        return;

    auto* parent = GetEntity(entity->Parent);
    if (parent != nullptr)
    {
        auto itr = std::find(parent->Children.begin(), parent->Children.end(), entity->Id);
        if (itr != parent->Children.end())
            parent->Children.erase(itr);
    }
}

void EntitySet::RemoveEntity(EntityId_t entityId, bool removeChildren)
{
    auto itr = EntityMap.find(entityId);
    if (itr == EntityMap.end())
        return;

    Entity& entity = itr->second;

    if (!removeChildren)
    {
        for (EntityId_t childId : entity.Children)
        {
            ReparentEntity(childId, entity.Parent);
        }
    }
    else
    {
        for (EntityId_t childId : entity.Children)
        {
            RemoveEntity(childId, true);
        }
    }

    for (auto componentTable : ComponentDB)
    {
        EraseAllComponents(componentTable.first, entityId);
    }
    if (entity.Parent == InvalidEntityId)
        RootNodes.erase(entity.Parent);

    RemoveFromParent(&entity);

    EntityMap.erase(itr);
}

Entity* EntitySet::GetEntity(EntityId_t id)
{
    auto itr = EntityMap.find(id);
    if (itr == EntityMap.end())
        return nullptr;

    return &(itr->second);
}

const char* EntitySet::GetEntityName(EntityId_t id)
{
    static std::string emptyName;

    auto itr = EntityMap.find(id);
    if (itr == EntityMap.end())
        return emptyName.c_str();

    return itr->second.Name.c_str();
}

EntityId_t EntitySet::GetEntityParent(EntityId_t id)
{
    auto itr = EntityMap.find(id);
    if (itr == EntityMap.end())
        return InvalidEntityId;

    return itr->second.Parent;
}

EntityId_t EntitySet::AddChild(EntityId_t id)
{
    Entity* entity = GetEntity(id);
    
    EntityId_t childId = CreateEntity();
    entity->Children.push_back(childId);

    Entity* child = GetEntity(childId);
    if (child->Parent == InvalidEntityId)
        RootNodes.erase(childId);

    child->Parent = id;

    return childId;
}

void EntitySet::ReparentEntity(EntityId_t id, EntityId_t newParent)
{
    Entity* entity = GetEntity(id);
    if (entity == nullptr || entity->Parent == newParent)
        return;

    if (entity->Parent == InvalidEntityId)
        RootNodes.erase(id);

    RemoveFromParent(entity);
    entity->Parent = newParent;

    if (entity->Parent == InvalidEntityId)
        RootNodes.insert(id);

    Entity* parent = GetEntity(newParent);
    if (parent != nullptr)
        parent->Children.push_back(id);
}

size_t EntitySet::GetParentCount(EntityId_t id)
{
    Entity* entity = GetEntity(id);
    if (entity == nullptr)
        return 0;

    if (entity->Parent == InvalidEntityId)
        return 0;

    return GetParentCount(entity->Parent) + 1;
}

void EntitySet::Update()
{
    for (auto* component : ComponentUpdateCache)
    {
        if (component->Active)
            component->OnUpdate();
    }
}

void EntitySet::DoForEachEntity(std::function<void(EntityId_t)> func, EntityId_t startWith)
{
    if (startWith != InvalidEntityId)
    {
        Entity* entity = GetEntity(startWith);
        if (entity == nullptr)
            return;

        func(entity->Id);
        for (EntityId_t child : entity->Children)
            DoForEachEntity(func, child);
    }
    else
    {
        for (auto entity : EntityMap)
            func(entity.first);
    }
}

void EntitySet::DoForEachRootEntity(std::function<void(EntityId_t)> func)
{
    for (EntityId_t entity : RootNodes)
        func(entity);
}

void EntitySet::DoForEachSiblingEntity(EntityId_t entityId, std::function<void(EntityId_t)> func)
{
    Entity* entity = GetEntity(entityId);
    if (entity == nullptr || func == nullptr)
        return;

    if (entity->Parent == InvalidEntityId)
    {
        for (EntityId_t entity : RootNodes)
        {
            if (entity != entityId)
                func(entity);
        }
        return;
    }

    Entity* parent = GetEntity(entity->Parent);
    if (parent != nullptr)
    {
        for (EntityId_t entity : parent->Children)
        {
            if (entity != entityId)
                func(entity);
        }
    }
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

Component* EntitySet::FindComponent(size_t compId, EntityId_t entityId)
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

bool EntitySet::HasComponent(size_t componentId, EntityId_t entityId)
{
    auto componentTableItr = ComponentDB.find(componentId);
    if (componentTableItr == ComponentDB.end())
        return false;

    ComponentTable& componentTable = componentTableItr->second;

    auto entityCacheItr = componentTable.Entities.find(entityId);
    return entityCacheItr != componentTable.Entities.end() && !entityCacheItr->second.empty();
}

static std::vector<Component*> EmptyComponentList;

const std::vector<Component*>& EntitySet::FindComponents(size_t compId, EntityId_t entityId)
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

void EntitySet::EraseAllComponents(size_t compId, EntityId_t entityId)
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

void EntitySet::DoForEachComponentInEntity(EntityId_t entityId, std::function<void(Component*)> func)
{
     for (auto& componentTable : ComponentDB)
     {
         auto entityItr = componentTable.second.Entities.find(entityId);
         if (entityItr == componentTable.second.Entities.end())
             continue;

         for (Component* component : entityItr->second)
             func(component);
     }
}
