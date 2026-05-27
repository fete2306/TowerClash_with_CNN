#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <windows.h>
#include <dxgi.h>
#include <wrl/client.h>

#include"basic_virtual.cpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

Draw* g_pDraw=nullptr;

LRESULT WINAPI WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
    if(ImGui_ImplWin32_WndProcHandler(hWnd,msg,wParam,lParam))
        return true;
    switch(msg){
        case WM_SIZE:
            if(g_pDraw && g_pDraw->device && wParam!=SIZE_MINIMIZED){
                g_pDraw->resizeRTV(LOWORD(lParam),HIWORD(lParam));
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int nCmdShow){
    WNDCLASSEXW wc={sizeof(wc)};
    wc.lpfnWndProc=WndProc;
    wc.hInstance=hInstance;
    wc.lpszClassName=L"TowerClash";
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
    RegisterClassExW(&wc);

    int cellSize=64;
    int mapW=19;
    int mapH=13;
    int windowW=mapW*cellSize+300;
    int windowH=mapH*cellSize;

    HWND hwnd=CreateWindowW(L"TowerClash",L"TowerClash - 2D Tower Defense",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,windowW,windowH,
        nullptr,nullptr,hInstance,nullptr);

    DXGI_SWAP_CHAIN_DESC scd={};
    scd.BufferCount=2;
    scd.BufferDesc.Width=windowW;
    scd.BufferDesc.Height=windowH;
    scd.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator=60;
    scd.BufferDesc.RefreshRate.Denominator=1;
    scd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow=hwnd;
    scd.SampleDesc.Count=1;
    scd.Windowed=TRUE;
    scd.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device* device=nullptr;
    ID3D11DeviceContext* context=nullptr;
    IDXGISwapChain* swapChain=nullptr;

    HRESULT hr=D3D11CreateDeviceAndSwapChain(
        nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,
        nullptr,0,D3D11_SDK_VERSION,
        &scd,&swapChain,&device,&featureLevel,&context);

    if(FAILED(hr)){
        hr=D3D11CreateDeviceAndSwapChain(
            nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,
            nullptr,0,D3D11_SDK_VERSION,
            &scd,&swapChain,&device,&featureLevel,&context);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io=ImGui::GetIO();
    io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device,context);

    Game game("assets/MapData/Map.json",0,
        "assets/ActorAttribute/StaticActor.json",
        "assets/ActorAttribute/MobileActor.json",2);
    game.nowCost={0.0f,0.0f};

    Draw draw(&game,device,context,swapChain,hwnd,windowW,windowH);
    game.mainDrawPtr=&draw;
    g_pDraw=&draw;

    ShowWindow(hwnd,nCmdShow);

    MSG msg={};
    bool running=true;
    while(running){
        while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT){running=false;break;}
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(!running)break;

        game.tick();
        game.draw();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    swapChain->Release();
    context->Release();
    device->Release();

    DestroyWindow(hwnd);
    UnregisterClassW(L"TowerClash",hInstance);

    return 0;
}
