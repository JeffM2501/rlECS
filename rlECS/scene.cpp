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


#include "scene.h"

#include "components/automover_component.h"
#include "components/camera_component.h"
#include "components/drawable_component.h"
#include "components/editor_components.h"
#include "components/flight_data_component.h"
#include "components/light_component.h"
#include "components/transform_component.h"

#include "systems/lighting_system.h"

#include "raylib.h"

void SceneData::SetupEditorBaseScene()
{
    // scene defaults
   // 
   // editor camera
    TransformComponent* camera = Entities.AddComponent<TransformComponent>();
    camera->GetEntity().Name = "Editor Camera";
    camera->AddComponent<CameraComponent>();
    camera->AddComponent<FlightDataComponent>();
    camera->SetPosition(0, 1.5f, -3);

    camera->AddComponent<EditorCameraComponent>();
    camera->AddComponent<EditorHiddenComponent>();

    Systems.GetSystem<LightingSystem>()->Setup();

    // create a light
    auto* light = Entities.AddComponent<LightComponent>();
    light->GetEntity().Name = "Default Light";
    auto* lightTransform = light->MustGetComponent<TransformComponent>();
    lightTransform->SetPosition(10, 10, 10);
    lightTransform->MustGetComponent<ShapeComponent>()->ObjectShape = DrawShape::Sphere;
}

void SceneData::SetupDefaultEntities()
{
    // test data
    // 
    // basic entity
    TransformComponent* testEntity = Entities.AddComponent<TransformComponent>();
    testEntity->GetEntity().Name = "Test Entity";
    testEntity->SetPosition(0, 0.5f, 0);

    // give it some geometry
    // body
    ShapeComponent* drawable = Entities.AddComponent<ShapeComponent>(testEntity);
    drawable->ObjectColor = GREEN;
    drawable->ObjectSize = Vector3{ 1,1,1 };

    AutoMoverComponent* mover = drawable->MustGetComponent<AutoMoverComponent>();
    mover->AngularSpeed.y = 90;

    // child
    TransformComponent* child = testEntity->AddChildComponent<TransformComponent>();
    child->GetEntity().Name = "Child";
    child->SetPosition(0, 1.0f, 0);
    // body
    drawable = Entities.AddComponent<ShapeComponent>(child);
    drawable->ObjectColor = DARKBLUE;
    drawable->ObjectShape = DrawShape::Sphere;
    drawable->ObjectSize = Vector3{ 0.5f,0.5f,0.5f };
}