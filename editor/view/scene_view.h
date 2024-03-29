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

#include "view/main_view.h"
#include "outliner/scene_outliner.h"
#include "scene.h"
#include "scene_editor.h"

#include <memory>

class SceneView : public ThreeDView
{
protected:

public:
    SceneView()
        : Editor(Scene)
    {}

    inline const char* GetName() override { return "Scene View"; }

    void OnSetup() override;
    void OnUpdate() override;
    void OnShutdown() override;
    void OnShow(const Rectangle& contentArea) override;
    void OnShowOverlay(const Rectangle& contentArea) override;

protected:
    uint64_t EditorCamera = 0;

    SceneData Scene;
    SceneEditor Editor;

    std::shared_ptr<SceneOutliner> Outliner;

protected:
    void OnStartFrameCamera(const Rectangle& contentArea) override;
    void OnEndFrameCamera() override;
};