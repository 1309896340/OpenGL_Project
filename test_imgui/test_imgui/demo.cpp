#include <iostream>
#include <cassert>

#include "imgui.h"
#include "backends\imgui_impl_glfw.h"
#include "backends\imgui_impl_opengl3.h"

#include "glad\glad.h"
#include "glfw\glfw3.h"


using std::cout;
using std::endl;

GLFWwindow* initWindow(int width, int height, const char* title) {
	if (glfwInit() != GLFW_TRUE) {
		cout << "glfw 初始化错误" << endl;
		return nullptr;
	}
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == nullptr) {
		cout << "window 初始化错误" << endl;
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	//==============glad================
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != GL_TRUE) {
		cout << "glad 初始化错误" << endl;
		return nullptr;
	}
	//==============窗口位置中心==================
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int screen_width = mode->width;
	int screen_height = mode->height;
	int window_x = (screen_width - width) / 2;
	int window_y = (screen_height - height) / 2;
	glfwSetWindowPos(window, window_x, window_y);
	return window;
}

bool imgui_init(GLFWwindow* window) {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	bool flag = ImGui_ImplOpenGL3_Init("#version 430");
	if (!flag)
		return false;
	// 设置字体
	ImGuiIO io = ImGui::GetIO();
	const char* fontPath = "C:\\Windows\\Fonts\\msyh.ttc";
	float fontSize = 23.0f;
	ImFont *customFont = io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
	io.FontDefault = customFont;
	return true;
}

void imgui_newFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void imgui_render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imgui_destroy() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

int main(int argc, char** argv) {
	GLFWwindow* window = initWindow(1600, 1200, "test_imgui");
	glEnable(GL_DEPTH);
	if (!imgui_init(window)) {
		cout << "imgui 初始化错误" << endl;
		return -1;
	}


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		imgui_newFrame();

		ImGui::ShowDemoWindow();

		imgui_render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	imgui_destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
