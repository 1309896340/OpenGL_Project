#ifndef __WIND_GUI
#define __WIND_GUI

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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// ���ü��̿���
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// �����ֱ�����
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
		// �����Զ���Ŀؼ�

		ImGui::Begin(u8"��ť����");
		if (ImGui::Button(u8"��ť")) {
			ImGui::Text(u8"�ı��򱻼���");
			std::cout << "��ť������" << std::endl;
		}
		ImGui::End();
	}
	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

#endif