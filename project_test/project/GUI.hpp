#include "proj.h"

#ifdef TEST_OPENGL

#ifndef __WIND_GUI
#define __WIND_GUI

#include "proj.h"
#include "Geometry.hpp"
#include "Wheat.hpp"

#include  "imgui.h"
#include  "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"


extern StatusInfo status;
extern Leaf *leaf;

class GUI {
private:
	GLFWwindow* window{ nullptr };
public:
	GUI(GLFWwindow* window) :window(window) {

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// 启用键盘控制
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// 启用手柄控制
		ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 30.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		ImGui::GetIO().FontDefault = font;

		bool flag = ImGui_ImplGlfw_InitForOpenGL(window, true);
		flag &= ImGui_ImplOpenGL3_Init();
		if (!flag) {
			std::cout << "ImGui init failed!" << flag << std::endl;
			exit(10);
		}
	}
	~GUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	void update() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();
		// 绘制自定义的控件
		
		bool isChanged = false;

		ImGui::Begin(u8"交互窗口");
		if (ImGui::SliderFloat(u8"叶长", &status.leafstatus.length, 1.0f, 4.5f)) {
			leaf->setLength(status.leafstatus.length);
		}
		if (ImGui::SliderFloat(u8"叶宽因子", &status.leafstatus.theta, 0.0f, 0.5f)) {
			leaf->setTheta(status.leafstatus.theta);
		}
		if (ImGui::SliderFloat(u8"茎叶夹角", &status.leafstatus.SLAngle, 0.1f, 120.0f)) {
			leaf->setSLAngle(status.leafstatus.SLAngle);
		}
		if (ImGui::SliderFloat(u8"弯曲系数", &status.leafstatus.k, 0.0f, 30.0f)) {
			leaf->setK(status.leafstatus.k);
		}
		if (ImGui::SliderFloat(u8"主叶脉扭曲角", &status.leafstatus.MVAngle, -540.0f, 540.0f)) {
			leaf->setMVAngle(status.leafstatus.MVAngle);
		}
		// 显示camera的所有状态
		ImGui::Text(u8"Camera状态");
		ImGui::ShowDemoWindow();
		ImGui::End();
	}
	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

#endif
#endif