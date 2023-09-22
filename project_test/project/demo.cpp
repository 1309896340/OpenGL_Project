#include "proj.h"
#include "Geometry.hpp"
#include "Camera.hpp"
#include "Shader.hpp"

typedef struct _StatusInfo {
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
	unsigned int lastKey = 0;
}StatusInfo;

StatusInfo status;
Camera camera(glm::vec3(1.0f, 6.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f));

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
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	//std::cout << "key: " << key << "  scancode: " << scancode << "  action: " << action << "  modes: " << mods << std::endl;

	if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) {		//关闭窗口
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_W:
			camera.move(0.0f, 0.1f);
			break;
		case GLFW_KEY_S:
			camera.move(0.0f, -0.1f);
			break;
		case GLFW_KEY_A:
			camera.move(-0.1f, 0.0f);
			break;
		case GLFW_KEY_D:
			camera.move(0.1f, 0.0f);
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		status.lastKey = GLFW_KEY_UNKNOWN;
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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_FILL);
	glViewport(0, 0, WIDTH, HEIGHT);


	// Shader的初始化
	Shader* shader = new Shader("vertexShader.vs", "fragmentShader.fs");	//默认着色器

	std::vector<Geometry*> objs;

	Geometry* obj1 = new Cube(2.0f, 3.0f, 5.0f, 8, 12, 20, shader);
	Geometry* obj2 = new Sphere(2.0f, 40, 20, shader);
	Geometry* obj3 = new Cylinder(1.0f, 6.0f, 4, 24, 40, shader);
	Geometry* obj4 = new Cone(2.0f, 3.0f, 10, 30, 60, shader);


	//Geometry* obj5 = new Sphere(0.05f, 36, 18, shader);
	//obj5->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  obj5->moveTo(glm::vec3(0.2f, 0.2f, 0.2f));
	//obj5->setColor(glm::vec4(1.0f,0.0f,0.0f,1.0f));
	//objs.push_back(obj5);


	initLineDrawing(shader);


	obj1->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  obj1->moveTo(glm::vec3(8.0f, 0.0f, 0.0f));
	obj2->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  obj2->moveTo(glm::vec3(3.0f, 0.0f, 0.0f));
	obj3->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  obj3->moveTo(glm::vec3(0.0f, 0.0f, -3.0f));
	obj4->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  obj4->moveTo(glm::vec3(-3.0f, 0.0f, 0.0f));
	//obj->rotate(glm::radians(20.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	objs.push_back(obj1);
	objs.push_back(obj2);
	objs.push_back(obj3);
	objs.push_back(obj4);

	Arrow* axis_x = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), shader);
	Arrow* axis_y = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), shader);
	Arrow* axis_z = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), shader);


	float deltaTime = 0.0f;
	float lastTime = 0.0f;
	float currentTime = glfwGetTime();

	shader->use();
	shader->setMat4("projection", camera.getProjectionMatrix());
	shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setVec3("lightPos", glm::vec3(0.1f, 0.1f, 0.1f));


	//obj4->setColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;


		shader->use();
		shader->setMat4("view", camera.getViewMatrix());
		shader->setVec3("viewPos", camera.getPosition());

		for (int i = 0; i < objs.size(); i++) {
			objs[i]->rotate((32 + i * 3) * glm::radians(deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
			objs[i]->draw();
		}

		axis_x->draw();
		axis_y->draw();
		axis_z->draw();

		//drawLine(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, 1, 1), glm::vec3(0.0f, 1.0f, 1.0f), 1.5f, shader);
		//drawLine(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1, 0, 1), glm::vec3(1.0f, 0.0f, 1.0f), 1.5f, shader);
		//drawLine(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1, 1, 0), glm::vec3(1.0f, 1.0f, 0.0f), 1.5f, shader);

		showLines();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
