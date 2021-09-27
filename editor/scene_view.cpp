/**********************************************************************************************
*
*   raylibExtras * Utilities and Shared Components for Raylib
*
*   Testframe - a Raylib/ImGui test framework
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

#include "scene_view.h"

#include "drawing_utils.h"

#include "components/automover_component.h"
#include "components/camera_component.h"
#include "components/drawable_component.h"
#include "components/flight_data_component.h"
#include "components/light_component.h"
#include "components/look_at_component.h"
#include "components/transform_component.h"

#include "systems/free_flight_controller.h"
#include "systems/lighting_system.h"
#include "systems/render_system.h"

#include "raylib.h"

void SceneView::OnSetup()
{
    Outliner = std::make_shared<SceneOutliner>(Entities);
    GlobalContext.UI.AddWindow(Outliner);

    ShowGround = false;

    // scene defaults
    // 
    // editor camera
    TransformComponent* camera = Entities.AddComponent<TransformComponent>();
    camera->GetEntity().Name = "Editor Camera";
    Entities.AddComponent<CameraComponent>(camera);
    Entities.AddComponent<FlightDataComponent>(camera);

    camera->SetPosition(0, 1.5f, -3);

    EditorCamera = camera->EntityId;

    Systems.GetSystem<LightingSystem>()->Setup();

    // create a light
    auto* light = Entities.AddComponent<LightComponent>();
    light->GetEntity().Name = "Default Light";
    auto* lightTransform = light->MustGetComponent<TransformComponent>();
    lightTransform->SetPosition(10, 10, 10);
    lightTransform->MustGetComponent<ShapeComponent>()->ObjectShape = DrawShape::Sphere;

    // test data
    // 
    // basic entity
    TransformComponent* testEntity = Entities.AddComponent<TransformComponent>();
    testEntity->GetEntity().Name = "Test Entity";
    testEntity->SetPosition(0, 0.5f, 0);

    // give it some geometry
    // body
    ShapeComponent* drawable = Entities.AddComponent<ShapeComponent>(testEntity);
    drawable->ObjectColor = Colors::Green;
    drawable->ObjectSize = Vector3{ 1,1,1 };

    AutoMoverComponent* mover = drawable->MustGetComponent<AutoMoverComponent>();
    mover->AngularSpeed.y = 90;

    // child
    TransformComponent* child = testEntity->AddChildComponent<TransformComponent>();
    child->GetEntity().Name = "Child";
    child->SetPosition(0, 1.0f, 0);
    // body
    drawable = Entities.AddComponent<ShapeComponent>(child);
    drawable->ObjectColor = Colors::DarkBlue;
    drawable->ObjectShape = DrawShape::Sphere;
    drawable->ObjectSize = Vector3{ 0.5f,0.5f,0.5f };
}

void SceneView::OnStartFrameCamera(const Rectangle& contentArea)
{
    Systems.GetSystem<RenderSystem>()->Begin(EditorCamera);
}

void SceneView::OnEndFrameCamera()
{
    Systems.GetSystem<RenderSystem>()->End();
}

void SceneView::OnUpdate()
{
    Entities.Update();
    Systems.GetSystem<LightingSystem>()->UpdateLights();

    Systems.GetSystem<FreeFlightController>()->Update(Entities.GetComponent<TransformComponent>(EditorCamera));

    Systems.GetSystem<LightingSystem>()->Update(EditorCamera);
}

void SceneView::OnShutdown()
{
    GlobalContext.UI.RemoveWindow(Outliner);
}

void SceneView::OnShow(const Rectangle& contentArea)
{
    Systems.GetSystem<RenderSystem>()->Draw();
}
