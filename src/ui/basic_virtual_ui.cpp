#include "../imgui/imgui.h"
#include "basic_virtual_ui.h"

BasicVirtualDebugState GetBasicVirtualDebugState(Game* game);

void RenderBasicVirtualUI(Game* game, bool* p_open)
{
    if (!game)
        return;

    BasicVirtualDebugState state = GetBasicVirtualDebugState(game);

    if (!ImGui::Begin("Basic Virtual Game Control", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Game Runtime Overview");
    ImGui::Separator();

    ImGui::Text("Time: %.1f", state.nowTime);
    ImGui::Text("CNN Switch: %s", state.cnnSwitch ? "On" : "Off");

    bool cnnEnabled = state.cnnSwitch;
    if (ImGui::Checkbox("Enable CNN Switch", &cnnEnabled))
    {
        SetGameCnnSwitch(game, cnnEnabled);
    }

    ImGui::Separator();
    ImGui::Text("Map Size: %dx%d", state.mapWidth, state.mapHeight);
    ImGui::Text("Static Actors: %d", state.staticActorCount);
    ImGui::Text("Mobile Actors: %d", state.mobileActorCount);

    if (ImGui::TreeNode("Resource Flow"))
    {
        for (int i = 0; i < (int)state.nowCost.size(); ++i)
        {
            float cost = state.nowCost[i];
            float speed = i < (int)state.costSpeed.size() ? state.costSpeed[i] : 0.0f;
            ImGui::BulletText("Owner %d: Cost = %.1f, Speed = %.2f", i, cost, speed);
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::TextWrapped("This UI panel is built with Dear ImGui to inspect basic_virtual.cpp runtime state, following the README style of fast iteration and tool-oriented tools.");
    ImGui::End();
}
