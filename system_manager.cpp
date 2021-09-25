#include "system_manager.h"

SystemSet::SystemSet(EntitySet& entities)
    :Entitites(entities)
{}

SystemSet::~SystemSet()
{
    for (auto itr : SystemMap)
        delete(itr.second);
}

System* SystemSet::GetSystem(size_t id)
{
    auto itr = SystemMap.find(id);
    if (itr == SystemMap.end())
        return nullptr;

    return itr->second;
}

System* SystemSet::AddSystem(System* system)
{
    auto itr = SystemMap.find(system->Id());
    if (itr != SystemMap.end() && itr->second != system)
        delete (itr->second);

    SystemMap[system->Id()] = system;
    return system;
}