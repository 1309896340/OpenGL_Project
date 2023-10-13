#ifndef __WIND_GUI
#define __WIND_GUI

#include "proj.h"
#include "Geometry.hpp"
#include "Shader.hpp"

#include  "imgui.h"
#include  "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"


extern StatusInfo status;
extern Leaf *leaf;

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
		
		bool isChanged = false;

		ImGui::Begin("��������");
		if (ImGui::SliderFloat("Ҷ������", &status.leafstatus.theta, 0.0f, 0.5f)) {
			leaf->setTheta(status.leafstatus.theta);
		}
		if (ImGui::SliderFloat("��Ҷ�н�", &status.leafstatus.SLAngle, 0.1f, 120.0f)) {
			leaf->setSLAngle(status.leafstatus.SLAngle);
		}
		if (ImGui::SliderFloat("����ϵ��", &status.leafstatus.k, 0.0f, 30.0f)) {
			leaf->setK(status.leafstatus.k);
		}
		if (ImGui::SliderFloat("��Ҷ��Ť����", &status.leafstatus.MVAngle, -360.0f, 360.0f)) {
			leaf->setMVAngle(status.leafstatus.MVAngle);
		}
		ImGui::End();
	}
	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

#endif