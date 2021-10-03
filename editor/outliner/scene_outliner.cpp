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

#include "outliner/scene_outliner.h"

#include "components/editor_components.h"
#include "entity_manager.h"
#include "application/ui_window.h"

#include "raylib.h"
#include "rlImGui.h"
#include "raylib.h"

bool EntitySelection::IsSelected(EntityId_t id)
{
    return Selection.find(id) != Selection.end();
}

void EntitySelection::Select(EntityId_t id, bool selected, bool add)
{
    if (!add)
        Selection.clear();

    if (IsSelected(id) != selected)
    {
        if (IsSelected(id))
            Selection.erase(id);
        else
            Selection.insert(id);
    }
}

void EntitySelection::Clear()
{
    Selection.clear();
}

const std::set<EntityId_t> EntitySelection::GetSelection()
{
    return Selection;
}

void EntitySelection::DoForEach(std::function<void(EntityId_t)>func)
{
    if (func == nullptr)
        return;

    for (EntityId_t id : Selection)
        func(id);
}

SceneOutliner::SceneOutliner(EntitySet& entities)
    : UIWindow()
    , Entities(entities)
{
    Shown = true;
}

void SceneOutliner::GetName(std::string & name, MainView * view) const
{
    name = SceneOutlinerWindowName;
}

const char* SceneOutliner::GetMenuName() const
{
    return "Outline";
}

void SceneOutliner::ShowEntityNode(EntityId_t entityId)
{
    if (entityId == InvalidEntityId || Entities.HasComponent<EditorHiddenComponent>(entityId))
        return;

    auto* entity = Entities.GetEntity(entityId);
    if (entity == nullptr)
        return;

    std::string displayName = entity->Name;
    if (displayName.empty())
        displayName = TextFormat("Entity-%ul", entityId);

    bool selected = Selection.IsSelected(entityId);

    float x = ImGui::GetCursorPosX();

    bool toggle = ImGui::IsKeyDown(KEY_LEFT_CONTROL) || ImGui::IsKeyDown(KEY_RIGHT_CONTROL);

    bool open = false;
    if (!entity->Children.empty())
    {
        open = ImGui::TreeNodeExV(&entityId, ImGuiTreeNodeFlags_DefaultOpen, "", 0);
        ImGui::SameLine();
    }

    if (ImGui::Selectable(displayName.c_str(), &selected, ImGuiSelectableFlags_None))
        Selection.Select(entityId, selected | !toggle, toggle);

    if (open)
    {
        for (EntityId_t child : Entities.GetEntity(entityId)->Children)
            ShowEntityNode(child);

        ImGui::TreePop();
    }
}

void SceneOutliner::OnShow(MainView * view)
{
    if (ImGui::Button(ICON_FA_PLUS_SQUARE))
    {
        if (CreateEntityCallback != nullptr)
            CreateEntityCallback(GetRootmostParent());
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_USER_PLUS))
    {
        if (CreateEntityCallback != nullptr)
            CreateEntityCallback(GetRootmostEntity());
    }

    if (ImGui::BeginChild("Root", ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvail().y - 30), true))
    {
        bool selected = false;

        if (ImGui::Selectable("Scene Root",selected))
        {
            bool toggle = ImGui::IsKeyDown(KEY_LEFT_CONTROL) || ImGui::IsKeyDown(KEY_RIGHT_CONTROL);
            if (!toggle)
                Selection.Clear();
        }

        ImGui::TreePush();
        Entities.DoForEachRootEntity([this](EntityId_t id) {ShowEntityNode(id); });

        ImGui::TreePop();
 
        ImGui::EndChild();
    }
}

EntityId_t SceneOutliner::GetRootmostParent()
{
    size_t minDepth = size_t(-1);
    EntityId_t minParent = InvalidEntityId;

    for (EntityId_t id : Selection.GetSelection())
    {
        size_t count = Entities.GetParentCount(id);

        // can't get any more root than this
        if (count == 0)
            return InvalidEntityId;

        if (count < minDepth)
        {
            minDepth = count;
            minParent = Entities.GetEntityParent(id);
        }
    }

    return minParent;
}

EntityId_t SceneOutliner::GetRootmostEntity()
{
    size_t minDepth = size_t(-1);
    EntityId_t minParent = InvalidEntityId;

    for (EntityId_t id : Selection.GetSelection())
    {
        size_t count = Entities.GetParentCount(id);

        // can't get any more root than this
        if (count == 0)
            return id;

        if (count < minDepth)
        {
            minDepth = count;
            minParent = id;
        }
    }

    return minParent;
}

