#include "proj.h"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"

#ifndef __WIND_INPUTMANAGER
#define __WIND_INPUTMANAGER
class InputManager {
	// 该类负责保存当前的交互状态，同时负责绑定这些状态改变所影响的对象
	// 例如当鼠标右击拖动时将会影响与之绑定的相机姿态角
	// 一个InputManager对象会绑定：
	// 1. 一个Camera对象，控制当前相机的姿态和位置
	// 2. 一个Geometry容器，其中存储着所有需要交互的物体的Geometry指针

	// 通过调用setXX()来随时切换Camera或控制当前选中的Geometry
	// 问题：要控制当前的Geometry，可能需要获取它具体的子类
private:
	// 交互影响的对象
	GLFWwindow* window{ nullptr };
	Scene* scene{ nullptr };

public:
	// 当前的交互状态（也可以直接使用ImGui提供的输入设备状态标志位）
	bool leftMouseButtonPressed{ false };
	bool rightMouseButtonPressed{ false };
	bool shiftPressed{ false };
	vec2 mousePos{ vec2(0.0f,0.0f) };

	InputManager() = delete;
	InputManager(GLFWwindow* window, Scene* scene) :window(window), scene(scene) {
		// 将自身传入GLFWwindow的userdata
		glfwSetWindowUserPointer(window, (void*)this);
		// 设置回调函数
		glfwSetFramebufferSizeCallback(window, framebuff_size_callback);
		glfwSetMouseButtonCallback(window, mouse_botton_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetKeyCallback(window, key_callback);
	}

	Camera* getCamera() { return scene->getCamera(); }
	Scene* getScene() { return scene; }

	static void framebuff_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {
		InputManager* ctx = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS && !(ctx->leftMouseButtonPressed)) {
				ctx->leftMouseButtonPressed = true;
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				ctx->mousePos.x = (float)xpos;
				ctx->mousePos.y = (float)ypos;
			}
			else if (action == GLFW_RELEASE && ctx->leftMouseButtonPressed) {
				ctx->leftMouseButtonPressed = false;
			}
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS && !(ctx->rightMouseButtonPressed)) {
				ctx->rightMouseButtonPressed = true;
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				ctx->mousePos.x = (float)xpos;
				ctx->mousePos.y = (float)ypos;
			}
			else if (action == GLFW_RELEASE && ctx->rightMouseButtonPressed) {
				ctx->rightMouseButtonPressed = false;
			}
		}
	}
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
		InputManager* ctx = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
		Camera* camera = ctx->getCamera();
		float dx = 0.0f, dy = 0.0f;
		if (ctx->rightMouseButtonPressed == true) {
			dx = (float)xpos - ctx->mousePos.x;
			dy = (float)ypos - ctx->mousePos.y;
			ctx->mousePos.x = (float)xpos;
			ctx->mousePos.y = (float)ypos;
			if (camera == nullptr)
				return;
			if (ctx->shiftPressed == false) {
				camera->rotate(dx, dy);	// 以相机为中心旋转
			}
			else {
				//// 以原点为中心旋转
				//camera->move(-(float)dx, (float)dy, 0.0f);
				//camera->lookAt(vec3(0.0f, 0.0f, 0.0f));
				// 以(0.0,1.0,0.0)为轴，旋转相机，包括相机的front向量一同旋转
				camera->rotateByAxis(dx, dy);
			}
		}
		if (ctx->leftMouseButtonPressed == true) {
			dx = (float)xpos - ctx->mousePos.x;
			dy = (float)ypos - ctx->mousePos.y;
			ctx->mousePos.x = (float)xpos;
			ctx->mousePos.y = (float)ypos;
			if (!camera)
				return;
			if (ctx->shiftPressed == false) {
				//camera->move(-(float)dx, (float)dy, 0.0f);
			}
			else {
				camera->move(-dx, dy, 0.0f);
			}
		}
	}
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		InputManager* ctx = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
		Camera* camera = ctx->getCamera();

		//std::cout << "key: " << key << "  scancode: " << scancode << "  action: " << action << "  modes: " << mods << std::endl;

		if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) {		//关闭窗口
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		if (key == GLFW_KEY_LEFT_SHIFT) {
			if (action == GLFW_PRESS) {
				ctx->shiftPressed = true;
			}
			else if (action == GLFW_RELEASE) {
				ctx->shiftPressed = false;
			}
		}

		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (!camera)
				return;
			switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			case GLFW_KEY_W:
				camera->move(0.0f, 0.0f, 10.0f);
				break;
			case GLFW_KEY_S:
				camera->move(0.0f, 0.0f, -10.0f);
				break;
			case GLFW_KEY_A:
				camera->move(-10.0f, 0.0f, 0.0f);
				break;
			case GLFW_KEY_D:
				camera->move(10.0f, 0.0f, 0.0f);
				break;
			case GLFW_KEY_UP:  // 控制光源的位置
				break;
			case GLFW_KEY_DOWN:
				break;
			case GLFW_KEY_LEFT:
				break;
			case GLFW_KEY_RIGHT:
				break;
			case GLFW_KEY_H:
				break;
			case GLFW_KEY_G:
				break;
			}
		}
	}
};

#endif
