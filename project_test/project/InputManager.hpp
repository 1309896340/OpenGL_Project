#include "proj.h"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"

#ifndef __WIND_INPUTMANAGER
#define __WIND_INPUTMANAGER
class InputManager {
	// ���ฺ�𱣴浱ǰ�Ľ���״̬��ͬʱ�������Щ״̬�ı���Ӱ��Ķ���
	// ���統����һ��϶�ʱ����Ӱ����֮�󶨵������̬��
	// һ��InputManager�����󶨣�
	// 1. һ��Camera���󣬿��Ƶ�ǰ�������̬��λ��
	// 2. һ��Geometry���������д洢��������Ҫ�����������Geometryָ��

	// ͨ������setXX()����ʱ�л�Camera����Ƶ�ǰѡ�е�Geometry
	// ���⣺Ҫ���Ƶ�ǰ��Geometry��������Ҫ��ȡ�����������
private:
	// ����Ӱ��Ķ���
	GLFWwindow* window{ nullptr };
	Scene* scene{ nullptr };

public:
	// ��ǰ�Ľ���״̬��Ҳ����ֱ��ʹ��ImGui�ṩ�������豸״̬��־λ��
	bool leftMouseButtonPressed{ false };
	bool rightMouseButtonPressed{ false };
	bool shiftPressed{ false };
	vec2 mousePos{ vec2(0.0f,0.0f) };

	InputManager() = delete;
	InputManager(GLFWwindow* window, Scene* scene) :window(window), scene(scene) {
		// ��������GLFWwindow��userdata
		glfwSetWindowUserPointer(window, (void*)this);
		// ���ûص�����
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
				camera->rotate(dx, dy);	// �����Ϊ������ת
			}
			else {
				//// ��ԭ��Ϊ������ת
				//camera->move(-(float)dx, (float)dy, 0.0f);
				//camera->lookAt(vec3(0.0f, 0.0f, 0.0f));
				// ��(0.0,1.0,0.0)Ϊ�ᣬ��ת��������������front����һͬ��ת
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

		if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) {		//�رմ���
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
			case GLFW_KEY_UP:  // ���ƹ�Դ��λ��
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
