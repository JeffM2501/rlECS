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

#include "application/application_context.h"
#include "application/application_ui.h"
#include "application/platform_tools.h"
#include "components/automover_component.h"
#include "components/camera_component.h"
#include "components/drawable_component.h"
#include "components/flight_data_component.h"
#include "components/light_component.h"
#include "components/look_at_component.h"
#include "inspectors/common_inspectors.h"
#include "view/main_view.h"
#include "view/scene_view.h"
#include "ui/imgui_dialogs.h"

#include "raylib.h"
#include "rlgl.h"
#include "rlImGui.h"
#include "RLAssets.h"

#include "game_components.h"

#include <deque>

ApplicationContext GlobalContext;

void ApplicationContext::Screenshot()
{
    if (GlobalContext.CopyScreenshot)
    {
        GlobalContext.CopyScreenshot = false;
        unsigned char* imgData = rlReadScreenPixels(GetScreenWidth(), GetScreenHeight());
        Image image = { imgData, GetScreenWidth(), GetScreenHeight(), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

        PlatformTools::CopyImageToClipboard(image);

        RL_FREE(imgData);
        TraceLog(LOG_INFO, "Copied Screenshot to Clipboard");
    }

    if (GlobalContext.TakeScreenshot)
    {
        GlobalContext.TakeScreenshot = false;
        std::string path = PlatformTools::ShowSaveFileDialog(TextFormat("%d.png", GetRandomValue(1, 999999999)));
        if (path.size() > 0)
            ::TakeScreenshot(path.c_str());
    }
}

void ApplicationContext::ChangeView(MainView* newView)
{
    if (View != nullptr)
        View->Shutdown();

    View = newView;
    if (View != nullptr)
        View->Setup();

    if (View != nullptr)
    {
        Prefs.LastView = View->GetName();
        Prefs.Save();
    }
}

MainView* ApplicationContext::FindView(const char* name)
{
    if (OpenScenes.empty())
        return nullptr;

    if (name == nullptr)
        return OpenScenes[0];

    std::string _name = name;
    for (MainView* view : OpenScenes)
    {
        std::string _vName = view->GetName();
        if (_name == _vName)
            return view;
    }

    return OpenScenes[0];
}

void ApplicationStartup();
void ApplicationShutdown();

bool ShowStartupLog = true;
bool Start2D = false;

void DrawOverlay()
{
    if (GlobalContext.View != nullptr)
    {
        ImGui::SetNextWindowPos(ImVec2(GlobalContext.View->LastContentArea.x, GlobalContext.View->LastContentArea.y));
        ImGui::SetNextWindowSize(ImVec2(GlobalContext.View->LastContentArea.width, GlobalContext.View->LastContentArea.height));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        if (ImGui::Begin("###ViewportOverlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
                ImGui::SetWindowFocus();

            GlobalContext.View->OnShowOverlay(GlobalContext.View->LastContentArea);
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
    }
}

void RegisterComponents()
{
    ComponentManager::Register<AutoMoverComponent>();
    ComponentManager::Register<TransformComponent>();
    ComponentManager::Register<CameraComponent>();
    ComponentManager::Register<ShapeComponent>(false);
    ComponentManager::Register<FlightDataComponent>();
    ComponentManager::Register<LookAtComponent>();
    ComponentManager::Register<LightComponent>();
}

#ifdef _WIN32
int wWinMain(void* hInstance, void* hPrevInstance, char* cmdLine, int show)
#else
int main(int argc, char* argv[])
#endif
{
    LogSink::Setup();

    GlobalContext.Prefs.Setup();

    unsigned int flags = FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE;

    if (GlobalContext.Prefs.Maximized)
        flags |= FLAG_WINDOW_MAXIMIZED;

    SetConfigFlags(flags);
    InitWindow(GlobalContext.Prefs.WindowWidth, GlobalContext.Prefs.WindowHeight, "rlECS Editor");
   
    if (GlobalContext.Prefs.Maximized)
        MaximizeWindow();

    ApplicationStartup();

    // register standard components
    RegisterComponents();

    // register anything from the common game lib
    RegisterGameComponents();
    RegisterGameSystems();

    // register editor inspectors
    RegisterCommonInspectors();

    GlobalContext.ChangeView(new SceneView());
    GlobalContext.UI.Startup();

    // Main game loop
    while (!GlobalContext.Quit && !WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsWindowResized())
            GlobalContext.UI.Resized();

        const Rectangle& contentArea = GlobalContext.UI.GetContentArea();

        if (GlobalContext.View != nullptr)
        {
            if (contentArea.x != GlobalContext.View->LastContentArea.x || contentArea.y != GlobalContext.View->LastContentArea.y || contentArea.width != GlobalContext.View->LastContentArea.width || contentArea.height != GlobalContext.View->LastContentArea.height)
            {
                GlobalContext.View->LastContentArea = contentArea;
                GlobalContext.View->ResizeContentArea(GlobalContext.View->LastContentArea);
            }
        }

        GlobalContext.UI.Update();

        if (GlobalContext.View != nullptr)
            GlobalContext.View->Update();

        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (GlobalContext.View != nullptr)
            GlobalContext.View->Show(GlobalContext.View->LastContentArea);

        BeginRLImGui();
        GlobalContext.UI.Show(GlobalContext.View);
        DrawOverlay();
        ImGui::UpdateDialogs();
        EndRLImGui();

        EndDrawing();

        if (!GlobalContext.ScreenshotView)
            ApplicationContext::Screenshot();
    }

    GlobalContext.Prefs.Save();

    GlobalContext.ChangeView(nullptr);
    GlobalContext.UI.Shutdown();

    ApplicationShutdown();

    ShutdownRLImGui();
    CloseWindow();
}

void ApplicationStartup()
{
    if (!ShowStartupLog)
        LogSink::Flush();

    TraceLog(LOG_INFO, "Editor Startup");

    rlas_SetAssetRootPath("resources/",false);

    InitRLGLImGui();
    // load fonts here
    AddRLImGuiIconFonts(12,true);
    FinishRLGLImguSetup();
}

void ApplicationShutdown()
{
    rlas_Cleanup();
    LogSink::Flush();
}

namespace LogSink
{
    std::deque<LogItem> LogLines;

    void GetLogLevelPrefix(int logLevel, LogItem& item)
    {
        item.Prefix = GetLogLevelName(logLevel);
        item.Prefix += ": ";
        switch (logLevel)
        {
        default:            item.Prefix.clear(); item.Color = ImGuiColors::Convert(WHITE); break;
        case LOG_TRACE:     item.Color = ImGuiColors::Convert(GRAY); break;
        case LOG_DEBUG:     item.Color = ImGuiColors::Convert(SKYBLUE); break;
        case LOG_INFO:      item.Color = ImGuiColors::Convert(GREEN); break;
        case LOG_WARNING:   item.Color = ImGuiColors::Convert(YELLOW); break;
        case LOG_ERROR:     item.Color = ImGuiColors::Convert(ORANGE); break;
        case LOG_FATAL:     item.Color = ImGuiColors::Convert(RED); break;
        }
    }

    void TraceLog(int logLevel, const char* text, va_list args)
    {
        static char logText[2048] = { 0 };
        LogItem item;
        item.Level = logLevel;
        GetLogLevelPrefix(logLevel, item);
        vsprintf(logText, text, args);
        item.Text += logText;
        LogLines.push_back(item);
    }

    void Setup()
    {
        SetTraceLogCallback(TraceLog);
    }

    bool PopLogLine(LogItem& line)
    {
        if (LogLines.size() == 0)
            return false;

        line = LogLines.front();
        LogLines.pop_front();
        return true;
    }

    void Flush()
    {
        LogLines.clear();
    }
}