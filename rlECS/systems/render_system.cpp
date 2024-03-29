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

#include "components/drawable_component.h"
#include "components/transform_component.h"
#include "components/camera_component.h"

#include "systems/render_system.h"
#include "systems/lighting_system.h"

#include "raylib.h"


void RenderSystem::Begin(uint64_t cameraEntityId)
{
    CameraComponent* camera = Entities.MustGetComponent<CameraComponent>(cameraEntityId);
    ViewCam.fovy = 45;

    // a camera entity must have a the transform component, if it doesn't we add one and get the default
    TransformComponent* cameraTransform = camera->MustGetComponent<TransformComponent>();

    // copy the transform vectors to the raylib camera
    ViewCam.position = cameraTransform->GetPosition();
    ViewCam.target = Vector3Add(cameraTransform->GetPosition(), cameraTransform->GetForwardVector());
    ViewCam.up = cameraTransform->GetUpVector();

    BeginMode3D(ViewCam);
}

void RenderSystem::Draw()
{
    // TODO, get the visible set
    Entities.DoForEachEntity<DrawableComponent>([](DrawableComponent* drawable)
        {
            if (drawable->Active)
                drawable->Draw();
        });
}

void RenderSystem::End()
{
    EndMode3D();
}
