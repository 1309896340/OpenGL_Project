
#include "interaction.h"
#ifdef TEST_OPENGL_A   // 暂时不使用
//extern StatusInfo status;
extern Camera* camera;

extern Leaf* leaf;

void framebuff_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS && !status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = true;
			glfwGetCursorPos(window, &status.mousePos[0], &status.mousePos[1]);
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
			//// 以原点为中心旋转
			//camera->move(-(float)dx, (float)dy, 0.0f);
			//camera->lookAt(vec3(0.0f, 0.0f, 0.0f));
			// 以(0.0,1.0,0.0)为轴，旋转相机，包括相机的front向量一同旋转
			camera->rotateByAxis((float)dx, (float)dy);
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
			//camera->move(-(float)dx, (float)dy, 0.0f);
		}
		else {
			camera->move(-(float)dx, (float)dy, 0.0f);
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
	else if (action == GLFW_RELEASE) {
	}
}
#endif

#ifdef TEST_SOFT_RASTERIZATION

extern StatusInfo status;
extern Camera* camera;

void opencv_mouseCallback(int event, int x, int y, int flags, void* userdata) {
	if (event == cv::EVENT_RBUTTONDOWN || event == cv::EVENT_LBUTTONDOWN) {
		status.mousePos[0] = (double)x;
		status.mousePos[1] = (double)y;
	}
	else if (event == cv::EVENT_MOUSEMOVE && flags == cv::EVENT_FLAG_LBUTTON) {
		double dx = status.mousePos[0] - (double)x;
		double dy = status.mousePos[1] - (double)y;
		status.mousePos[0] = (double)x;
		status.mousePos[1] = (double)y;
		camera->move((float)dx, -(float)dy);
	}
	else if (event == cv::EVENT_MOUSEMOVE && flags == cv::EVENT_FLAG_RBUTTON) {
		double dx = status.mousePos[0] - (double)x;
		double dy = status.mousePos[1] - (double)y;
		status.mousePos[0] = (double)x;
		status.mousePos[1] = (double)y;
		camera->rotate((float)dx, (float)dy);
	}
	else if (event == cv::EVENT_MOUSEMOVE && flags == (cv::EVENT_FLAG_RBUTTON | cv::EVENT_FLAG_SHIFTKEY)) {
		double dx = status.mousePos[0] - (double)x;
		double dy = status.mousePos[1] - (double)y;
		status.mousePos[0] = (double)x;
		status.mousePos[1] = (double)y;
		camera->rotateByAxis(-(float)dx, -(float)dy);	// 绕着圆心轴旋转，第三个参数写得有点问题
}
}
#endif