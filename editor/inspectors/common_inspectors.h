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

        Vector3 pos = transform->GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
            transform->SetPosition(pos);

        Vector3 rot = QuaternionToEuler(transform->GetOrientation());
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.25f, -180.0f, 180.0f))
            transform->SetOrientation(rot);
    }
};

class ShapesInspector : public ComponentInspector
{
public:
    DEFINE_COMPONENT_INSPECTOR(ShapeComponent);

    const char* GetShapeName(DrawShape shape)
    {
        switch (shape)
        {
        case DrawShape::Box:
            return "Box";

        case DrawShape::Sphere:
            return "Sphere";

        case DrawShape::Cylinder:
            return "Cylinder";

        case DrawShape::Plane:
            return "Plane";
        }

        return "Shape";
    }

    const char* DisplayName(Component* component) override
    {
        return "Shape";
    }

    void ShowContent(Component* component) override
    {
        ShapeComponent* shape = static_cast<ShapeComponent*>(component);

        if (ImGui::BeginCombo("Shape", GetShapeName(shape->ObjectShape), ImGuiComboFlags_PopupAlignLeft))
        {
            if (ImGui::Selectable(GetShapeName(DrawShape::Box), shape->ObjectShape == DrawShape::Box))
                shape->ObjectShape = DrawShape::Box;

            if (ImGui::Selectable(GetShapeName(DrawShape::Sphere), shape->ObjectShape == DrawShape::Sphere))
                shape->ObjectShape = DrawShape::Sphere;

            if (ImGui::Selectable(GetShapeName(DrawShape::Cylinder), shape->ObjectShape == DrawShape::Cylinder))
                shape->ObjectShape = DrawShape::Cylinder;

            if (ImGui::Selectable(GetShapeName(DrawShape::Plane), shape->ObjectShape == DrawShape::Plane))
                shape->ObjectShape = DrawShape::Plane;

            ImGui::EndCombo();
        }

        ImGui::DragFloat3("Position", &shape->ObjectOrigin.x, 0.1f);
        ImGui::DragFloat3("Rotation", &shape->ObjectOrientationShift.x, 0.25f, -180.0f, 180.0f);
        ImGui::DragFloat3("Size", &shape->ObjectSize.x, 0.1f);
    }
};

inline void RegisterCommonInspectors()
{
    ComponentInspectorRegistry::Register<TransformInspector>();
    ComponentInspectorRegistry::Register<ShapesInspector>();
}