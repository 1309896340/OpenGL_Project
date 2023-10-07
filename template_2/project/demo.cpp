#include "proj.h"
#include "interaction.h"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Geometry.hpp"
#include "Scene.hpp"

StatusInfo status;
Camera* camera = nullptr;

glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);


GLFWwindow* GLFWinit() {
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "glfw初始化失败" << std::endl;
		exit(1);
	}
	glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwInitHint(GLFW_VERSION_MAJOR, 4);
	glfwInitHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "example", NULL, NULL);
	if (window != NULL) {
		glfwMakeContextCurrent(window);
		//将窗口移动到屏幕中央
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
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
	//glEnable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_FILL);
	glViewport(0, 0, WIDTH, HEIGHT);
	return window;
}


int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();

	float deltTime;

	camera = new Camera(glm::vec3(-0.4f, 0.8f, 3.0f), glm::vec3(0.4f, 0.5f, 0.0f));
	Scene scene(camera);
	Shader* shader = new DefaultShader();

	Drawable* axis = new Axis(shader);
	Geometry* obj1 = new Cylinder(0.2f, 1.0f, 4, 20, 32, shader);
	Geometry* obj2 = new Cylinder(0.2f, 1.0f, 4, 20, 32, shader);

	obj1->rotate(-glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	obj1->applyTransform();
	obj2->rotate(-glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	obj2->translate(glm::vec3(0.0f, 0.5f, 0.0f));
	obj2->applyTransform();

	obj2->translate(glm::vec3(0.0f, 0.5f, 0.0f));

	Combination *com = new Combination();
	com->add(obj1);
	com->add(obj2);


	scene.addShader(shader);
	scene.add(axis);
	scene.add(com);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		deltTime = scene.step();

		// obj->rotate(glm::radians(30 * deltTime), glm::vec3(0.0f, 0.0f, 1.0f));
		com->rotate(glm::radians(30 * deltTime), glm::vec3(0.0f, 0.0f, 1.0f));
		// 由于在Combination的draw()中忽略了obj自身的model，因此这句无效。目前的做法只能修改obj2在Combination中的model，也就是objModel
		obj2->rotate(glm::radians(70 * deltTime), glm::vec3(1.0f, 0.0f, 0.0f));

		scene.render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

