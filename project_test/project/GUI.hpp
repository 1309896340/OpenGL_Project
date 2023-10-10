#include "proj.h"
#include "Geometry.hpp"
#include "Shader.hpp"

#include  "imgui.h"
#include  "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#pragma once

class GUI {
private:
	GLFWwindow* window = nullptr;
	Shader* shader = nullptr;
public:
	GUI(GLFWwindow* window, Shader* shader = nullptr) :window(window), shader(shader) {
		if (shader == nullptr)
			shader = DefaultShader::getDefaultShader();

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// 启用键盘控制
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// 启用手柄控制

		ImGui_ImplGlfw_InitForOpenGL(window, true);

		bool flag = ImGui_ImplOpenGL3_Init();
		if (!flag) {
			std::cout << "ImGui init failed!" << flag << std::endl;
			exit(10);
		}
	}
	~GUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}
	void update() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
	}
	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};
