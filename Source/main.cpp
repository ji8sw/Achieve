#include "Windows.h"
#include "dwmapi.h"
#include "d3d11.h"
#include "string"
#include "iomanip"
#include <iostream>
#include <fstream>
#include "algorithm"
#include "vector"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
		return 0L;
    
    if (Message == WM_DESTROY)
    {
        PostQuitMessage(0);
		return 0L;
    }

    return DefWindowProc(Window, Message, WParam, LParam);
}

INT APIENTRY WinMain(HINSTANCE Instance, HINSTANCE, PSTR, INT ShowCMD)
{
    WNDCLASSEXW WC{};

    WC.cbSize = sizeof(WNDCLASSEXW);
    WC.style = CS_HREDRAW || CS_VREDRAW;
    WC.lpfnWndProc = WndProc;
    WC.hInstance = Instance;
    WC.lpszClassName = L"Overlook";

    RegisterClassExW(&WC);

    const HWND Window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        WC.lpszClassName,
        L"Overlook",
        WS_POPUP,
        0,
        0,
        1920,
        1080,
        nullptr,
        nullptr,
        WC.hInstance,
        nullptr
    );

    SetLayeredWindowAttributes(Window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(Window, &client_area);

        RECT window_area{};
        GetWindowRect(Window, &window_area);

        POINT diff{};
        ClientToScreen(Window, &diff);

        const MARGINS margins{
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom
        };

        DwmExtendFrameIntoClientArea(Window, &margins);
    }

    DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1U;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.SampleDesc.Count = 1U;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2U;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL FeatureLevels[]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

    ID3D11Device* Device{nullptr};
    ID3D11DeviceContext* DeviceContext{nullptr};
    IDXGISwapChain* SwapChain{nullptr};
    ID3D11RenderTargetView* RenderTargetView{nullptr};
    D3D_FEATURE_LEVEL FeatureLevel{};

    D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		FeatureLevels,
		2U,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&SwapChain,
		&Device,
		&FeatureLevel,
		&DeviceContext
	);

    ID3D11Texture2D* BackBuffer{nullptr};
    SwapChain->GetBuffer(0U, IID_PPV_ARGS(&BackBuffer));

    if (BackBuffer)
    {
        Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
        BackBuffer->Release();
    }
    else
        return 1;

    ShowWindow(Window, ShowCMD);
    UpdateWindow(Window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX11_Init(Device, DeviceContext);

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.7f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.02f, 0.7f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.6f, 0.6f, 0.7f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.12f, 0.12f, 0.7f);

    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7f, 0.7f, 0.7f, 0.7f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.7f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.6f, 0.6f, 0.6f, 0.7f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 0.7f);

    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.7f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.12f, 0.12f, 0.12f, 0.7f);

    struct Achievement {
        std::string Name;        // Name of the achievement
        std::string Description; // Description if i decide to add them
        std::string Type;		// Type of achievement (e.g. toggle, value, etc)
        bool Completed;			// If the achievement has been completed
        int Value;				// Value of the achievement (e.g. 100 kills, 1000 kills, etc)

        Achievement(const std::string& _name, const std::string& _description, const std::string& _type, const bool& _completed, const int& _value)
            : Name(_name), Description(_description), Type(_type), Completed(_completed), Value(_value) {}
    };

    bool Running = true;
    bool Menu = true;

    bool QuickDisplay = false;

    std::vector<Achievement> Achievements;
    std::vector<Achievement> QuickDisplayAchievements;

    int VK_Hotkey = VK_INSERT;

    SetWindowLongPtr(Window, GWL_EXSTYLE, GetWindowLongPtr(Window, GWL_EXSTYLE) & ~WS_EX_LAYERED); // Remove WS_EX_LAYERED

    while (Running)
    {
       MSG Message;
       while (PeekMessage(&Message, nullptr, 0U, 0U, PM_REMOVE))
       {
			TranslateMessage(&Message);
			DispatchMessage(&Message);

			if (Message.message == WM_QUIT)
				Running = false;
		}

       if (!Running)
		   break;   

       ImGui_ImplDX11_NewFrame();
	   ImGui_ImplWin32_NewFrame();

	   ImGui::NewFrame();

       if (GetAsyncKeyState(VK_Hotkey))
       {
           Menu = !Menu;
           Sleep(150);

           if (!Menu)
               SetWindowLongPtr(Window, GWL_EXSTYLE, GetWindowLongPtr(Window, GWL_EXSTYLE) | WS_EX_LAYERED); // Add WS_EX_LAYERED
           else
               SetWindowLongPtr(Window, GWL_EXSTYLE, GetWindowLongPtr(Window, GWL_EXSTYLE) & ~WS_EX_LAYERED); // Remove WS_EX_LAYERED

           SetWindowPos(Window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
       }


       if (Menu)
       {
           ImGui::Begin("Achieve", &Menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
           ImVec2 WindowSize = ImVec2(400.f, 100.f);

           static char buffer[256];
           bool NewAchievement = false;

           ImGui::PushItemWidth(100.f);
           ImGui::InputText(" ", buffer, sizeof(buffer)); 
           
           ImGui::SameLine();

           if (ImGui::Button("Add Toggle", { 100.f, 20.f })) {
               if (buffer[0] == '\0')
                   Achievements.push_back(Achievement("New Achievement", "", "toggle", false, 0));
               else
                Achievements.push_back(Achievement(buffer, "", "toggle", false, 0));
           }
           ImGui::SameLine();
           if (ImGui::Button("Add Value", { 100.f, 20.f })) {
               if (buffer[0] == '\0')
                   Achievements.push_back(Achievement("New Achievement", "", "value", false, 0));
               else
               Achievements.push_back(Achievement(buffer, "", "value", false, 0));
           }

           for (int i = 0; i < Achievements.size(); i++)
           {
               if (Achievements[i].Type == "toggle")
               {
				   ImGui::Checkbox(Achievements[i].Name.c_str(), &Achievements[i].Completed);
				   ImGui::SameLine();
                   if (ImGui::Button("Remove"))
					   Achievements.erase(Achievements.begin() + i);
                   ImGui::SameLine();
                   if (ImGui::Button("Quick Display"))
                   {
                       QuickDisplayAchievements.push_back(Achievements[i]);
                       Achievements.erase(Achievements.begin() + i);
                   }
			   }
               else if (Achievements[i].Type == "value")
               {
                   ImGui::Text(Achievements[i].Name.c_str());
                   ImGui::SameLine();

                   if (ImGui::Button("-", { 20.f, 20.f }))
                       Achievements[i].Value--;
                   ImGui::SameLine();

                   ImGui::Text("%s", std::to_string(Achievements[i].Value).c_str());
                   ImGui::SameLine();

                   if (ImGui::Button("+", { 20.f, 20.f }))
                       Achievements[i].Value++;
                   ImGui::SameLine();

                   if (ImGui::Button("Remove"))
                       Achievements.erase(Achievements.begin() + i);
                   ImGui::SameLine();

                   if (ImGui::Button("Quick Display"))
                   {
                       QuickDisplayAchievements.push_back(Achievements[i]);
                       Achievements.erase(Achievements.begin() + i);
                   }
			   }

               WindowSize.y += 24.f;
		   }


           ImGui::Checkbox("Quick Display", &QuickDisplay);

           if (ImGui::Button("Exit", { 100.f, 20.f }))
               Running = false;
           
           ImGui::SetWindowSize({ WindowSize });
           ImGui::End();
       }

       if (QuickDisplay)
       {
           ImGui::Begin("Quick Display", &QuickDisplay, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
           ImVec2 WindowSize = ImVec2(400.f, 30.f);

           for (int i = 0; i < QuickDisplayAchievements.size(); i++)
           {
               if (QuickDisplayAchievements[i].Type == "toggle")
               {
                   ImGui::Checkbox(QuickDisplayAchievements[i].Name.c_str(), &QuickDisplayAchievements[i].Completed);
                   ImGui::SameLine();
               }
               else if (QuickDisplayAchievements[i].Type == "value")
               {
                   ImGui::Text(QuickDisplayAchievements[i].Name.c_str());
                   ImGui::SameLine();

                   if (ImGui::Button("+", { 20.f, 20.f }))
                       QuickDisplayAchievements[i].Value++;
                   ImGui::SameLine();

                   ImGui::Text("%s", std::to_string(QuickDisplayAchievements[i].Value).c_str());
                   ImGui::SameLine();

                   if (ImGui::Button("-", { 20.f, 20.f }))
                       QuickDisplayAchievements[i].Value--;
                   ImGui::SameLine();
               }

               if (ImGui::Button("Remove"))
               {
                   Achievements.push_back(QuickDisplayAchievements[i]);
                   QuickDisplayAchievements.erase(QuickDisplayAchievements.begin() + i);
               }

               WindowSize.y += 24.f;
           }

		   ImGui::SetWindowSize({ WindowSize });
		   ImGui::End();
       }

       ImGui::Render();

       constexpr float Colour[4]{0.f, 0.f, 0.f, 0.f};

	   DeviceContext->OMSetRenderTargets(1U, &RenderTargetView, nullptr);
	   DeviceContext->ClearRenderTargetView(RenderTargetView, Colour);

	   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

       SwapChain->Present(1U, 0U);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (SwapChain)
		SwapChain->Release();
    if (DeviceContext)
        DeviceContext->Release();
    if (Device)
        Device->Release();
    if (RenderTargetView)
        RenderTargetView->Release();

    DestroyWindow(Window);
    UnregisterClassW(WC.lpszClassName, WC.hInstance);

    return 0;
}
