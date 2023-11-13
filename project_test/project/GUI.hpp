#include "proj.h"

#ifdef TEST_OPENGL

#ifndef __WIND_GUI
#define __WIND_GUI

#include "proj.h"
#include "InputManager.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

#include  "imgui.h"
#include  "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"


//extern StatusInfo status;
//extern Leaf *leaf;

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

		InputManager* ctx = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));

		ImGui::Begin(u8"交互窗口");

		// 直接修改Leaf的属性，这里写死了InputManager的geometry必须是Leaf
		Leaf* leaf = static_cast<Leaf*>(ctx->getGeometry());
		LeafMesh *mesh = static_cast<LeafMesh *>(leaf->getMeshes()[0]);

		if (ImGui::SliderFloat(u8"叶长", &(mesh->height), 1.0f, 4.5f)) {
			mesh->setChangeFlag();
		}
		if (ImGui::SliderFloat(u8"叶宽因子", &(mesh->theta), 0.0f, 0.5f)) {
			mesh->setChangeFlag();
		}
		if (ImGui::SliderFloat(u8"茎叶夹角", &(mesh->SLAngle), 0.1f, 120.0f)) {
			mesh->setChangeFlag();
		}
		if (ImGui::SliderFloat(u8"弯曲系数", &(mesh->k), 0.0f, 30.0f)) {
			mesh->setChangeFlag();
		}
		if (ImGui::SliderFloat(u8"主叶脉扭曲角", &(mesh->MVAngle), -540.0f, 540.0f)) {
			mesh->setChangeFlag();
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