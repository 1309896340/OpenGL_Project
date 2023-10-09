#include "proj.h"
#include "Camera.hpp"

//#include "Geometry.hpp"

extern StatusInfo status;
extern Camera* camera;


void framebuff_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS && !status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = true;
			status.startShoot = true;
			glfwGetCursorPos(window, &status.mousePos[0], &status.mousePos[1]);
			//std::cout << "发出射线:(" << status.shootPos[0] << "," << status.shootPos[1] << ")" << std::endl;
		}
		else if (action == GLFW_RELEASE && status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = false;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS && !status.rightMouseButtonPressed) {
			status.rightMouseButtonPressed = true;
			glfwGetCursorPos(window, &status.mousePos[0], &status.mousePos[1]);
		}
		else if (action == GLFW_RELEASE && status.rightMouseButtonPressed) {
			status.rightMouseButtonPressed = false;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static double dx = 0.0, dy = 0.0;
	if (status.rightMouseButtonPressed == true) {
		dx = xpos - status.mousePos[0];
		dy = ypos - status.mousePos[1];
		status.mousePos[0] = xpos;
		status.mousePos[1] = ypos;
		if (!camera)
			return;
		if (status.shiftPressed == false) {
			camera->rotate((float)dx, (float)dy);	// 以相机为中心旋转
		}
		else {
			// 以原点为中心旋转
			camera->move(-(float)dx, (float)dy, 0.0f);
			camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
	if (status.leftMouseButtonPressed == true) {
		dx = xpos - status.mousePos[0];
		dy = ypos - status.mousePos[1];
		status.mousePos[0] = xpos;
		status.mousePos[1] = ypos;
		if (!camera)
			return;
		if (status.shiftPressed == false) {
			camera->move(-(float)dx, (float)dy, 0.0f);
		}
		else {
			// 控制叶片轴向扭曲？
			//if (leaf != nullptr) {
			//	leaf->addTheta((float)dx);
			//	leaf->addK((float)dy);
			//}
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	//std::cout << "key: " << key << "  scancode: " << scancode << "  action: " << action << "  modes: " << mods << std::endl;

	if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) {		//关闭窗口
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS) {
			status.shiftPressed = true;
		}
		else if (action == GLFW_RELEASE) {
			status.shiftPressed = false;
		}
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (!camera)
			return;
		switch (key) {
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
			status.lightPos.z -= 0.2f;
			break;
		case GLFW_KEY_DOWN:
			status.lightPos.z += 0.2f;
			break;
		case GLFW_KEY_LEFT:
			status.lightPos.x -= 0.2f;
			break;
		case GLFW_KEY_RIGHT:
			status.lightPos.x += 0.2f;
			break;
		case GLFW_KEY_H:
			status.lightPos.y += 0.2f;
			break;
		case GLFW_KEY_G:
			status.lightPos.y -= 0.2f;
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		status.lastKey = GLFW_KEY_UNKNOWN;
	}
}
