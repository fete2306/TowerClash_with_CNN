#pragma once

#include <vector>

class Game;

struct BasicVirtualDebugState {
    float nowTime = 0.0f;
    bool cnnSwitch = false;
    int mapWidth = 0;
    int mapHeight = 0;
    int staticActorCount = 0;
    int mobileActorCount = 0;
    std::vector<float> nowCost;
    std::vector<float> costSpeed;
};

BasicVirtualDebugState GetBasicVirtualDebugState(Game* game);
void SetGameCnnSwitch(Game* game, bool enabled);
void RenderBasicVirtualUI(Game* game, bool* p_open = nullptr);
