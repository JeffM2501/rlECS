#include "ui/imgui_buttons.h"

#include "imgui.h"
#include "imgui_internal.h"


namespace ImGui
{
    bool ConditionalButton(const std::string& id, bool enabled, const ImVec2& size)
    {
        bool returnValue = false;

        if (enabled == 0)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        returnValue = ImGui::Button(id.data(), size);

        if (enabled == 0)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        return returnValue;
    }
}