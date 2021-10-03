#pragma once

#include <string>

#include "imgui.h"

namespace ImGui
{
    bool ConditionalButton(const std::string& id, bool enabled, const ImVec2& size = ImVec2(0, 0));
}