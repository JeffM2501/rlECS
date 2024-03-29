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

#include "view/main_view.h"
#include "graphics/drawing_utils.h"
#include "inspectors/inspector_window.h"
#include "application/platform_tools.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "RaylibColors.h"

MainView::MainView()
{
}

void MainView::Shutdown()
{

}

void MainView::Update()
{

}

void MainView::Show(const Rectangle& contentArea)
{
    OnShow(contentArea);
    LastMousePos = GetMousePosition();
}

void MainView::OnShowOverlay(const Rectangle& contentArea)
{

}

void MainView::ResizeContentArea(const Rectangle& contentArea)
{
}

void MainView::OnShow(const Rectangle& contentArea)
{ 
    ClearBackground(BLACK);
}

bool MainView::OpenFileMenu(std::string& filename)
{
    if (ImGui::MenuItem("Open..."))
    {
        filename = PlatformTools::ShowOpenFileDialog(filename.c_str(), OpenFileExtensions);
        return filename.size() > 0;
    }
    return false;
}

// 3d view
void ThreeDView::Setup()
{
    Camera.HideCursor = false;
    Camera.Setup(45, Vector3{ 0,1,0 });
    Camera.MoveSpeed = Vector3{ 10,10,10 };

    Camera.ControlsKeys[FPCamera::CameraControls::MOVE_UP] = KEY_SPACE;
    Camera.ControlsKeys[FPCamera::CameraControls::MOVE_DOWN] = KEY_LEFT_CONTROL;

    rlEnableSmoothLines();

    OnSetup();
    SetupSkybox();
}

void ThreeDView::Shutdown()
{
    OnShutdown();
    if (SkyboxTexture.id != 0)
        UnloadTexture(SkyboxTexture);

    if (Skybox.meshCount > 0)
        UnloadModel(Skybox);
}

void ThreeDView::OnStartFrameCamera(const Rectangle& contentArea)
{
    if (IsMouseButtonPressed(1))
        Camera.UseKeyboard = Camera.UseMouseX = Camera.UseMouseY = (IsMouseButtonDown(1) && CheckCollisionPointRec(GetMousePosition(), contentArea));
    else if (!IsMouseButtonDown(1))
        Camera.UseKeyboard = Camera.UseMouseX = Camera.UseMouseY = false;

    Camera.Update();
    Camera.BeginMode3D();
}

void ThreeDView::OnEndFrameCamera()
{
    Camera.EndMode3D();
}

void ThreeDView::Show(const Rectangle& contentArea)
{
    if (SceneTexture.texture.id == 0)
        ResizeContentArea(contentArea);

    if (SceneTexture.texture.id == 0)
        return;

    BeginTextureMode(SceneTexture);
    if (GlobalContext.ScreenshotView)
    {
        ApplicationContext::Screenshot();
        GlobalContext.ScreenshotView = false;
    }

    ClearBackground(BLACK);
    OnStartFrameCamera(contentArea);

    DrawDefaultScene();
    OnShow(contentArea);

    OnEndFrameCamera();
    EndTextureMode();

    DrawTexturePro(SceneTexture.texture,
        Rectangle{ 0, 0, (float)SceneTexture.texture.width, (float)-SceneTexture.texture.height },
        contentArea,
        Vector2Zero(),
        0,
        WHITE);
}

void ThreeDView::ResizeContentArea(const Rectangle& contentArea)
{
    if (SceneTexture.id != 0)
        UnloadRenderTexture(SceneTexture);

    if (contentArea.width == 0 || contentArea.height == 0)
        return;

    SceneTexture = LoadRenderTexture((int)contentArea.width, (int)contentArea.height);
    BeginTextureMode(SceneTexture);
    ClearBackground(BLACK);
    EndTextureMode();
}

void ThreeDView::ShowInspectorContents(const InspectorWindow& window)
{
    window.ShowCommonData(this);

    Vector3 camPos = Camera.GetCameraPosition();
    ImGui::Text("X %.2f Y %.2f Z %.2f", camPos.x, camPos.y, camPos.z);
    Vector2 camAngles = Camera.GetViewAngles();
    ImGui::Text("Yaw%.2f Pitch%.2f", camAngles.y, camAngles.x);
    ImGui::Separator();
    ImGui::Separator();
    OnShowInspector(window);
}

void ThreeDView::DrawAxis(float scale)
{
    DrawLine3D(Vector3{ 0,0,0 }, Vector3{ scale,0,0 }, RED);
    DrawLine3D(Vector3{ scale,0,0 }, Vector3{ scale * 0.75f,0,scale * 0.125f }, RED);

    DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,scale,0 }, GREEN);
    DrawLine3D(Vector3{ 0,scale,0 }, Vector3{ scale * 0.125f,scale * 0.75f, 0 }, GREEN);

    DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,0,scale }, BLUE);
    DrawLine3D(Vector3{ 0,0,scale }, Vector3{ scale * 0.125f, 0,scale * 0.75f }, BLUE);
}

void DrawArrow(float scale, Color& color)
{
    DrawCylinder(Vector3Zero(), scale * 0.05f, scale * 0.05f, scale * 0.75f, 10, color);
    DrawCylinder(Vector3{ 0,scale * .75f,0 }, scale * 0, scale * 0.125f, scale * 0.25f, 10, color);
}

void ThreeDView::DrawGizmo(float scale)
{
    rlPushMatrix();
    rlRotatef(90, 0, 0, 1);
    DrawArrow(scale, RED);
    rlPopMatrix();

    rlPushMatrix();
    //rlRotatef(90, 0, 0, 1);
    DrawArrow(scale, GREEN);
    rlPopMatrix();

    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    DrawArrow(scale, BLUE);
    rlPopMatrix();

    DrawSphere(Vector3Zero(), scale * 0.1f, WHITE);
}

void ThreeDView::SetupSkybox()
{
    if (!ShowSkybox)
        return;

    Image skyImg = LoadImage(SkyboxResource.c_str());
    SkyboxTexture = LoadTextureCubemap(skyImg, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(skyImg);

    Skybox = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    int val = MATERIAL_MAP_CUBEMAP;
    // Load skybox shader and set options in required locations
    SetModelMaterialShader(&Skybox, 0, LoadShaderSet("shaders", "skybox"));
    SetModelMaterialShaderValue(&Skybox, 0, "environmentMap", &val, SHADER_UNIFORM_INT);
    val = 1;
    SetModelMaterialShaderValue(&Skybox, 0, "noGamma", &val, SHADER_UNIFORM_INT);
    SetModelMaterialTexture(&Skybox, 0, MATERIAL_MAP_CUBEMAP, SkyboxTexture);
}

void ThreeDView::DrawSkybox()
{
    if (!ShowSkybox)
        return;

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube
    rlDisableDepthMask();			// Disable depth writes
    rlDisableDepthTest();			// Disable depth test for speed

    DrawModel(Skybox, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

    rlEnableBackfaceCulling();     // enable things we turned off
    rlEnableDepthMask();
    rlEnableDepthTest();
}

void ThreeDView::DrawDefaultScene()
{
    DrawSkybox();

    float planeSize = 50.0f;

    rlDisableDepthTest();

    if (ShowGround)
    {
        DrawPlane(Vector3Zero(), Vector2{ (float)planeSize, (float)planeSize }, Colors::Beige);
    }

    if (ShowOrigin)
    {
        DrawUtils::DrawGridXZ(Vector3Zero(), planeSize, 1, ColorAlpha(Colors::RayWhite, 0.75f), ColorAlpha(Colors::DarkGray, 0.5f));
        rlSetLineWidth(2.0f);
        rlDrawRenderBatchActive();

        rlSetLineWidth(3.0f);
        DrawLine3D(Vector3{ 0,0,0 }, Vector3{ planeSize,0,0 }, RED);
        DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,0,planeSize }, BLUE);
    }

    rlDrawRenderBatchActive();

    if (ShowOrigin)
    {
        DrawAxis(2.0f);
        rlDrawRenderBatchActive();
    }
    rlSetLineWidth(1.0f);
    rlEnableDepthTest();
}

// 2dView
void TwoDView::Setup()
{
    Camera.zoom = 1;
    OnSetup();
}

void TwoDView::Shutdown()
{
    OnShutdown();
    UnloadRenderTexture(SceneTexture);
}

void TwoDView::Update()
{
    OnUpdate();

    if (IsMouseButtonPressed(1))
    {
        Dragging = true;
        ClickPos = GetMousePosition();
        ClickTarget = Camera.target;
    }

    if (IsMouseButtonDown(1))
    {
        Vector2 delta = Vector2Scale(Vector2Subtract(ClickPos, GetMousePosition()), 1.0f / ZoomLevels[ZoomLevel]);
        Camera.target = Vector2Add(ClickTarget, delta);
    }
}

void TwoDView::Show(const Rectangle& contentArea)
{
    if (SceneTexture.texture.id == 0)
        ResizeContentArea(contentArea);

    if (SceneTexture.texture.id == 0)
        return;

    if (CheckCollisionPointRec(GetMousePosition(), contentArea))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT))
        {
            Camera.zoom += GetMouseWheelMove() * 0.05f;
            if (Camera.zoom <= 0)
                Camera.zoom = 0.05f;
        }
        else
        {
            if (GetMouseWheelMove() < 0)
            {
                ZoomLevel--;
                if (ZoomLevel < 0)
                    ZoomLevel = 0;
            }
            else if (GetMouseWheelMove() > 0)
            {
                ZoomLevel++;
                if (ZoomLevel >= MaxZoomLevels)
                    ZoomLevel = MaxZoomLevels - 1;
            }

            if (GetMouseWheelMove() != 0)
                Camera.zoom = ZoomLevels[ZoomLevel];

        }
    }

    BeginTextureMode(SceneTexture);
    if (GlobalContext.ScreenshotView)
    {
        ApplicationContext::Screenshot();
        GlobalContext.ScreenshotView = false;
    }
    ClearBackground(Colors::DarkGray);
    BeginMode2D(Camera);
    DrawUtils::DrawGrid2D(Vector2Zero(), (int)RectTools::MaxSize(contentArea) / 2, 100, Colors::Gray, Colors::DarkBlue);

    OnShow(contentArea);

    EndMode2D();

    DrawText(TextFormat("Zoom:%.1f%%", Camera.zoom * 100), 0, (int)contentArea.height - 20, 20, Colors::White);
    EndTextureMode();

    DrawTexturePro(SceneTexture.texture,
        Rectangle{ 0, 0, (float)SceneTexture.texture.width, (float)-SceneTexture.texture.height },
        contentArea,
        Vector2Zero(),
        0,
        Colors::White);
}

void TwoDView::ResizeContentArea(const Rectangle& contentArea)
{
    if (SceneTexture.id != 0)
        UnloadRenderTexture(SceneTexture);

    if (contentArea.width == 0 || contentArea.height == 0)
        return;

    Camera.offset = RectTools::CenterSize(contentArea);

    SceneTexture = LoadRenderTexture((int)contentArea.width, (int)contentArea.height);
    BeginTextureMode(SceneTexture);
    ClearBackground(Colors::Black);
    EndTextureMode();
}

void TwoDView::ShowInspectorContents(const InspectorWindow& window)
{
    if (ImGui::Button("Center"))
    {
        Camera.target = Vector2Zero();
        Camera.offset = RectTools::CenterSize(LastContentArea);
    }

    ImGui::SameLine();
    if (ImGui::Button("Zero"))
        Camera.offset = Vector2Zero();

    ImGui::SameLine();
    if (ImGui::Button("1:1"))
        Camera.zoom = 1;

    window.ShowCommonData(this);

    ImGui::Text("X %.2f Y %.2f", Camera.target.x, Camera.target.y);
    ImGui::Text("Rotation%.2f", Camera.rotation);
    ImGui::Separator();
    ImGui::Separator();
    OnShowInspector(window);
}

// shader tools

Shader SetModelMaterialShader(Model* model, int materialIndex, Shader shader)
{
    model->materials[materialIndex].shader = shader;
    return shader;
}

void SetModelMaterialShaderValue(Model* model, int materialIndex, const char* location, const void* value, int uniformType)
{
    Shader shader = model->materials[materialIndex].shader;
    SetShaderValue(shader, GetShaderLocation(shader, location), value, uniformType);
}

void SetModelMaterialShaderValueV(Model* model, int materialIndex, const char* location, const void* value, int uniformType, int count)
{
    Shader shader = model->materials[materialIndex].shader;
    SetShaderValueV(shader, GetShaderLocation(shader, location), value, uniformType, count);
}

void SetModelMaterialTexture(Model* model, int materialIndex, int maptype, Texture2D texture)
{
    SetMaterialTexture(&model->materials[materialIndex], maptype, texture);
}

RLAPI Shader LoadShaderSet(const char* resourcePath, const char* name)
{
    static char vsTemp[512];
    static char fsTemp[512];

#if defined(PLATFORM_WEB)
    static const char* glsl = "glsl110";
#else
    static const char* glsl = "glsl330";
#endif
    sprintf(vsTemp, "%s/%s/%s.vs", resourcePath, glsl, name);
    sprintf(fsTemp, "%s/%s/%s.fs", resourcePath, glsl, name);

    return LoadShader(vsTemp, fsTemp);
}

RLAPI Shader LoadShaders(const char* resourcePath, const char* vsName, const char* fsName)
{
    static char vsTemp[512];
    static char fsTemp[512];

#if defined(PLATFORM_WEB)
    static const char* glsl = "glsl110";
#else
    static const char* glsl = "glsl330";
#endif
    sprintf(vsTemp, "%s/%s/%s.vs", resourcePath, glsl, vsName);
    sprintf(fsTemp, "%s/%s/%s.fs", resourcePath, glsl, fsName);

    return LoadShader(vsTemp, fsTemp);
}