#pragma once

#include "components/transform_component.h"
#include "component_inspector.h"

#include "imgui.h"

#include "raymath.h"

class TransformInspector : public ComponentInspector
{
public:
    DEFINE_COMPONENT_INSPECTOR(TransformComponent);

    const char* DisplayName(Component*) override { return "Transform"; }

    void ShowContent(Component* component) override
    {
        TransformComponent* transform = static_cast<TransformComponent*>(component);

        float width = ImGui::GetContentRegionAvailWidth();

        float itemSize = width / 3;
        itemSize -= ImGui::GetStyle().ItemInnerSpacing.x;

        Vector3 pos = transform->GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
            transform->SetPosition(pos);

        Vector3 rot = QuaternionToEuler(transform->GetOrientation());
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.25f, -180.0f, 180.0f))
            transform->SetOrientation(rot);
    }
};

inline void RegisterCommonInspectors()
{
    ComponentInspectorRegistry::Register<TransformInspector>();
}