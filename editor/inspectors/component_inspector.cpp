#include "inspectors/component_inspector.h"

#include "imgui.h"

#include <map>


void ComponentInspector::Inspect(Component* component)
{
    if (Header(component, DisplayName(component)))
    {
        BeginContent(component);
        ShowContent(component);
        EndContent();
    }
}

bool ComponentInspector::Header(Component* component, const char* name)
{
    static char temp[512];
    sprintf(temp, "%s### %s %zu header", name, component->ComponentName(), size_t(component));

    return ImGui::CollapsingHeader(temp, ImGuiTreeNodeFlags_DefaultOpen);
}

void ComponentInspector::BeginContent(Component* component)
{
    ImGui::TreePush(component->ComponentName());
}

void ComponentInspector::EndContent()
{
    ImGui::TreePop();
}

class DefaultInspector : public ComponentInspector
{
};

namespace ComponentInspectorRegistry
{
    std::map<size_t, ComponentInspector*> InspectorCache;

    DefaultInspector Default;

    ComponentInspector* Get(size_t inspectorId)
    {
        std::map<size_t, ComponentInspector*>::iterator itr = InspectorCache.find(inspectorId);
        if (itr == InspectorCache.end())
            return &Default;

        return itr->second;
    }

    void Register(ComponentInspector* inspector)
    {
        InspectorCache[inspector->ComponentTypeId()] = inspector;
    }
}
