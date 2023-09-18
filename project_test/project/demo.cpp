#include "proj.h"
#include "Geometry.hpp"
#include "Camera.hpp"

typedef struct _StatusInfo{
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
}StatusInfo;

StatusInfo status;
Camera camera(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f));

glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);

inline std::chrono::milliseconds getTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

void framebuff_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS && !status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = true;
		}
		else if (action == GLFW_RELEASE && status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
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
		// 控制视角
		camera.rotate(dx, dy);
		camera.updateViewMatrix();
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) {		//关闭窗口
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

}

int main(int argc, char** argv) {
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "glfw初始化失败" << std::endl;
		exit(1);
	}
	glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwInitHint(GLFW_VERSION_MAJOR, 3);
	glfwInitHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "example", NULL, NULL);
	if (window != NULL) {
		glfwMakeContextCurrent(window);
		//将窗口移动到屏幕中央
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowPos(window, (mode->width - WIDTH) / 2, (mode->height - HEIGHT) / 2);
	}
	else {
		std::cout << "窗口创建失败" << std::endl;
		exit(2);
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "glad初始化失败" << std::endl;
		exit(3);
	}

	glfwSetFramebufferSizeCallback(window, framebuff_size_callback);
	glfwSetMouseButtonCallback(window, mouse_botton_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glViewport(0, 0, WIDTH, HEIGHT);

	//Geometry* obj = new Cube(2.0f, 3.0f, 5.0f, 8, 12, 20);
	//Geometry* obj = new Sphere(2.0f, 40, 20);
	Geometry* obj = new Cylinder(1.0f, 6.0f, 4, 24, 40);
	obj->rotate(glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	camera.addProgram(obj->getProgram());

	float deltaTime = 0.0f;
	float lastTime = 0.0f;
	float currentTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lastTime = currentTime;;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;

		//std::cout<<deltaTime<<std::endl;

		obj->rotate(glm::radians(4*deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		obj->draw();


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
