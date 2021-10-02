/**********************************************************************************************
*
*   raylibExtras * Utilities and Shared Components for Raylib
*
*   rlECS- a simple ECS in raylib with editor
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

#include "view/scene_view.h"

#include "graphics/drawing_utils.h"

#include "components/automover_component.h"
#include "components/camera_component.h"
#include "components/drawable_component.h"
#include "components/editor_components.h"
#include "components/flight_data_component.h"
#include "components/light_component.h"
#include "components/look_at_component.h"
#include "components/transform_component.h"

#include "inspectors/inspector_window.h"

#include "systems/free_flight_controller.h"
#include "systems/lighting_system.h"
#include "systems/render_system.h"

#include "raylib.h"
#include "IconsForkAwesome.h"

void SceneView::OnSetup()
{
    Outliner = std::make_shared<SceneOutliner>(Scene.Entities);
    GlobalContext.UI.AddWindow(Outliner);

    GlobalContext.UI.AddWindow(std::make_shared<InspectorWindow>(Scene, Outliner->Selection));

    ShowGround = false;

    Scene.SetupEditorBaseScene();
    Scene.SetupDefaultEntities();

    Scene.Entities.DoForEachEntity<EditorCameraComponent>([this](EditorCameraComponent* camera) {EditorCamera = camera->EntityId; });

    Scene.Systems.GetSystem<FreeFlightController>()->AllowMovement = []() { return IsMouseButtonDown(1); };

    Scene.Run = false;
}

void SceneView::OnStartFrameCamera(const Rectangle& contentArea)
{
    Scene.Systems.GetSystem<RenderSystem>()->Begin(EditorCamera);
}

void SceneView::OnEndFrameCamera()
{
    Scene.Systems.GetSystem<RenderSystem>()->End();
}

void SceneView::OnUpdate()
{
    if (Scene.Run)
    {
        Scene.Entities.Update();
    }

    Scene.Systems.GetSystem<LightingSystem>()->UpdateLights();
    Scene.Systems.GetSystem<FreeFlightController>()->Update(Scene.Entities.GetComponent<TransformComponent>(EditorCamera));
    Scene.Systems.GetSystem<LightingSystem>()->Update(EditorCamera);
}

void SceneView::OnShutdown()
{
    GlobalContext.UI.RemoveWindow(Outliner);
}

void SceneView::OnShow(const Rectangle& contentArea)
{
    Scene.Systems.GetSystem<RenderSystem>()->Draw();
}

void SceneView::OnShowOverlay(const Rectangle& contentArea)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.5f));
    auto& style = ImGui::GetStyle();

    if (ImGui::BeginChild("###Toolbar", ImVec2(contentArea.width, ImGui::GetTextLineHeight() + (style.FramePadding.y * 4)), false))
    {
        ImGui::SetCursorPosY(style.FramePadding.y);

        if (Scene.Run)
        {
            if (ImGui::Button(ICON_FA_STOP " Stop"))
                Scene.Run = false;
        }
        else
        {
            if (ImGui::Button(ICON_FA_PLAY " Play"))
                Scene.Run = true;
        }
        
        ImGui::EndChild();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}
