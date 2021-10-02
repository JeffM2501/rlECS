#pragma once

#include "entity_manager.h"

class ComponentInspector
{
public:
    virtual void Inspect(Component* component);
    virtual const char* DisplayName(Component* component) { return component->ComponentName(); }
    virtual void ShowContent(Component* component) {}

    virtual size_t ComponentTypeId() { return 0; }

protected:
    bool Header(Component* component, const char* name);
    void BeginContent(Component* component);
    void EndContent();
};

#define DEFINE_COMPONENT_INSPECTOR(COMPONENT_TYPE) \
    size_t ComponentTypeId() override { return COMPONENT_TYPE::GetComponentTypeId(); }

namespace ComponentInspectorRegistry
{
    ComponentInspector* Get(size_t compoentId);
    void Register(ComponentInspector* inspector);

    template<class T>
    inline void Register()
    {
        Register(new T());
    }
}
