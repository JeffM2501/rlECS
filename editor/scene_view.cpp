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

#include "automover_component.h"
#include "camera_component.h"
#include "drawable_component.h"
#include "flight_data_component.h"
#include "light_component.h"
#include "look_at_component.h"
#include "transform_component.h"

#include "free_flight_controller.h"
#include "lighting_system.h"
#include "render_system.h"

#include "raylib.h"

void SceneView::OnSetup()
{
    ShowGround = false;

    Systems.GetSystem<LightingSystem>()->Setup();

    // create a light
    auto* light = Entities.AddComponent<LightComponent>();
    auto* lightTransform = light->MustGetComponent<TransformComponent>();
    lightTransform->SetPosition(10, 10, 10);
    lightTransform->MustGetComponent<ShapeComponent>()->ObjectShape = DrawShape::Sphere;

    // editor camera
    TransformComponent* camera = Entities.AddComponent<TransformComponent>();
    Entities.AddComponent<CameraComponent>(camera);
    Entities.AddComponent<FlightDataComponent>(camera);

    camera->SetPosition(0, 1.5f, -3);
    
    EditorCamera = camera->EntityId;

    // basic entity
    TransformComponent* testEntity = Entities.AddComponent<TransformComponent>();

    testEntity->SetPosition(0, 0.5f, 0);

    // give it some geometry
    // body
    ShapeComponent* drawable = Entities.AddComponent<ShapeComponent>(testEntity);
    drawable->ObjectColor = Colors::Green;
    drawable->ObjectSize = Vector3{ 1,1,1 };
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

}

void SceneView::OnShow(const Rectangle& contentArea)
{
    Systems.GetSystem<RenderSystem>()->Draw();
}
