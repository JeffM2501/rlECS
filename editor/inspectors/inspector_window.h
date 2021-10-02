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

#pragma once

#include "application/ui_window.h"
#include "outliner/scene_outliner.h"

#include "scene.h"

#include "raylib.h"
#include "rlImGui.h"
#include "raylib.h"

#include "IconsFontAwesome5.h"

#include <string>

class MainView;

namespace Inspectors
{
    void ShowTextureInspector(const Texture& texture, float width = 0);
    void ShowSetTextureFilter(const Texture& texture);
    void ShowMeshInspector(const Mesh& mesh);
    void ShowMaterialMapInspector(MaterialMap& materialMap);
}

constexpr char InspectorWindowName[] = " Inspector###RaylibInspectorWindow";

class InspectorWindow : public UIWindow
{
public:
    InspectorWindow(SceneData& scene, EntitySelection& selection);
    void GetName(std::string& name, MainView* view) const override;
    const char* GetMenuName() const override;
    void ShowCommonData(MainView* view) const;
    void OnShow(MainView* view) override;

    void Update() override;

    SceneData& Scene;

protected:
    void ShowComponentPicker();

    EntitySelection& Selection;
    std::string Name;

private:
    EntityId_t CurrentSelection = InvalidEntityId;
    size_t ComponentToAdd = 0;
};
