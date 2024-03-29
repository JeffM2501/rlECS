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
#include "common_utils.h"
#include "inspectors/inspector_window.h"
#include "outliner/scene_outliner.h"
#include "application/platform_tools.h"
#include "application/ui_window.h"
#include "view/main_view.h"
#include "view/scene_view.h"

#include "RLAssets.h"
#include "raylib.h"
#include "rlgl.h"
#include "rlImGui.h"
#include "imgui_internal.h"

void UIManager::Startup()
{
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Right;

    ImGui::StyleColorsDark();

    ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.75f);

    AddWindow(std::make_shared<LogWindow>());
}

void UIManager::Shutdown()
{
    for (auto& window : Windows)
        window->Shutdown();

    Windows.clear();
}

void UIManager::RemoveWindow(std::shared_ptr<UIWindow> window)
{
    std::vector<std::shared_ptr<UIWindow>>::iterator itr = std::find(Windows.begin(), Windows.end(), window);
    if (itr != Windows.end())
        Windows.erase(itr); 
}

void UIManager::Resized()
{
    for (auto& window : Windows)
        window->Resize();
}

void UIManager::Show(MainView* view)
{
    ImVec2 screenSize((float)GetScreenWidth(), (float)GetScreenHeight());

    ImGui::SetNextWindowSize(screenSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

    if (ImGui::Begin("MainFrame",
        nullptr,
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::PopStyleVar();

        DockspaceId = ImGui::DockSpace(ImGui::GetID("MainWindowDock"), ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);

        if (ImGui::IsWindowAppearing())
            SetupUI();

        for (auto& window : Windows)
            window->Show(view); 

        auto* rootNode = ImGui::DockBuilderGetNode(DockspaceId);
        if (rootNode != nullptr && rootNode->CentralNode != nullptr)
        {
            ContentArea.x = rootNode->CentralNode->Pos.x;
            ContentArea.y = rootNode->CentralNode->Pos.y;
            ContentArea.width = rootNode->CentralNode->Size.x;
            ContentArea.height = rootNode->CentralNode->Size.y;
        }
        ShowDebugWindows();
        ShowMenu();
 
    }
    else
    {
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

void UIManager::Update()
{
    for (auto& window : Windows)
        window->Update();
}

void UIManager::SetupUI()
{
    auto* rootNode = ImGui::DockBuilderGetNode(DockspaceId);
    if (rootNode == nullptr || rootNode->ChildNodes[0] == nullptr)
    {
        ImGuiID centralNode = DockspaceId;
        auto leftId = ImGui::DockBuilderSplitNode(centralNode, ImGuiDir_Left, 0.25f, nullptr, &centralNode);
        auto rightId = ImGui::DockBuilderSplitNode(centralNode, ImGuiDir_Right, 0.25f, nullptr, &centralNode);

        auto childId = ImGui::DockBuilderSplitNode(centralNode, ImGuiDir_Down, 0.25f, nullptr, nullptr);

        ImGui::DockBuilderDockWindow(SceneOutlinerWindowName, leftId);
        ImGui::DockBuilderDockWindow(InspectorWindowName, rightId);
        ImGui::DockBuilderDockWindow(LogWindowName, childId);
    }
}

void UIManager::ShowMenu()
{
    bool openAbout = false;
    bool copyScreenshot = false;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                SceneView* scene = new SceneView();
                scene->Setup();
                GlobalContext.OpenScenes.push_back(scene);
                GlobalContext.View = scene;
            }

            if (GlobalContext.View != nullptr)
                GlobalContext.View->OnFileMenu();

            if (ImGui::MenuItem("Exit", "Alt+F4"))
                GlobalContext.Quit = true;

            ImGui::EndMenu();
        }

        if (GlobalContext.View != nullptr)
            GlobalContext.View->OnMenuBar();

        if (!GlobalContext.OpenScenes.empty())
        {
            if (ImGui::BeginMenu("Views"))
            {
                for (MainView* view : GlobalContext.OpenScenes)
                {
                    bool selected = view == GlobalContext.View;

                    if (ImGui::MenuItem(view->GetName(), nullptr, &selected, !selected))
                    {
                        GlobalContext.ChangeView(view);
                    }
                }

                ImGui::EndMenu();
            }
        }

        if (ImGui::BeginMenu("Window"))
        {
            for (auto& window : Windows)
            {
                ImGui::MenuItem(window->GetMenuName(), nullptr, &window->Shown);
            }
            
            ImGui::EndMenu();
        } 
        
        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::BeginMenu("ImGui"))
            {
                if (ImGui::MenuItem("Item Picker"))
                    ImGui::DebugStartItemPicker();

                ImGui::MenuItem("Style Editor", nullptr, &ShowStyleEditor);
                ImGui::MenuItem("Metrics", nullptr, &ShowMetricsWindow);
                ImGui::MenuItem("Demo", nullptr, &ShowDemoWindow);
                if (ImGui::MenuItem("About"))
                    ShowAboutImGuiWindow = true;

                ImGui::EndMenu();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu("Screenshot"))
            {
                if (ImGui::MenuItem("Save Screenshot..."))
                    GlobalContext.TakeScreenshot = true;

                if (ImGui::MenuItem("Copy Screenshot", "F11"))
                    GlobalContext.CopyScreenshot = true;

                ImGui::Separator();
                if (ImGui::MenuItem("Save View Screenshot..."))
                    GlobalContext.ScreenshotView = GlobalContext.TakeScreenshot = true;

                if (ImGui::MenuItem("Copy View Screenshot", "Shift+F11"))
                    GlobalContext.ScreenshotView = GlobalContext.CopyScreenshot = true;

                ImGui::EndMenu();
            }
            if (GlobalContext.View != nullptr)
                GlobalContext.View->OnToolsMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
                openAbout = true;

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();

        if (openAbout)
            ImGui::OpenPopup("About Testframe");
        
        if (ImGui::BeginPopupModal("About Testbed", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("Raylib Testframe Copyright 2021 Jeffery Myers");
            ImGui::TextUnformatted("A Test frame for trying out things in raylib and Dear ImGui");
            if (ImGui::Button("Ok"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    if (IsKeyPressed(KEY_F11))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            GlobalContext.ScreenshotView = true;

        GlobalContext.CopyScreenshot = true;
    }
}

void UIManager::ShowDebugWindows()
{
    if (ShowStyleEditor)
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::Begin("Style Editor", &ShowStyleEditor))
        {
            ImGui::ShowStyleEditor(&style);
            
        }
        ImGui::End();
    }

    if (ShowMetricsWindow)
        ImGui::ShowMetricsWindow(&ShowMetricsWindow);

    if (ShowDemoWindow)
        ImGui::ShowDemoWindow(&ShowDemoWindow);

    if (ShowAboutImGuiWindow)
        ImGui::ShowAboutWindow(&ShowAboutImGuiWindow);
}