#include "proj.h"
#include "Geometry.hpp"

inline std::chrono::milliseconds getTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

void framebuff_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {}

int main(int argc, char** argv) {
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "glfw初始化失败" << std::endl;
		exit(1);
	}
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwInitHint(GLFW_VERSION_MAJOR, 3);
	glfwInitHint(GLFW_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "example", NULL, NULL);
	if (window != NULL) {
		glfwMakeContextCurrent(window);
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

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glViewport(0, 0, WIDTH, HEIGHT);

	//Geometry* obj = new Cube(2.0f, 3.0f, 5.0f, 8, 12, 20);
	//Geometry* obj = new Sphere(2.0f, 40, 20);
	Geometry* obj = new Cylinder(1.0f, 6.0f, 4, 24, 40);
	obj->rotate(glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 view, projection;
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 40.0f);
	view = glm::lookAt(
		glm::vec3(0.0, 0.0, 8.0),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0)
	);
	obj->rotate(glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 1.0f));


	// projection和view矩阵或许应该封装在camera类中，这里先这样写
	glUseProgram(obj->getProgram());
	updateUniformMatrix4fv(obj->getProgram(), "view", view);
	updateUniformMatrix4fv(obj->getProgram(), "projection", projection);
	glUseProgram(0);

	long long startTimestamp = getTimestamp().count();
	long long t0, t1;

	unsigned int frameRate = 120;
	unsigned int frameTime = (unsigned int)(1000.0 / frameRate);

	unsigned int cnt = 0;

	while (!glfwWindowShouldClose(window)) {
		t0 = getTimestamp().count();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		obj->rotate(glm::radians(0.2f), glm::vec3(0.0f, 1.0f, 0.0f));
		obj->draw();

		t1 = getTimestamp().count();

		cnt++;
		if (cnt % frameRate == 0)
			std::cout << "time stamp: " << getTimestamp().count() - startTimestamp << std::endl;

		if ((t1 - t0) < frameTime) {
			Sleep(frameTime - (t1 - t0));
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
