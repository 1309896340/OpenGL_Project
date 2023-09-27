#include "proj.h"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Geometry.hpp"

typedef struct _StatusInfo {
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
	unsigned int lastKey = 0;
	bool startShoot = false;
	bool shiftPressed = false;
	double shootPos[2];
	glm::vec3 lightPos = glm::vec3(0.1f, 0.1f, 0.1f);
	float lastTime, currentTime, deltaTime;
}StatusInfo;

StatusInfo status;
Camera camera(glm::vec3(-0.4f, 0.8f, 3.0f), glm::vec3(0.4f, 0.5f, 0.0f));

glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);


void framebuff_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS && !status.leftMouseButtonPressed) {
			status.leftMouseButtonPressed = true;
			status.startShoot = true;
			glfwGetCursorPos(window, &status.shootPos[0], &status.shootPos[1]);
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
		if (status.shiftPressed == false) {
			camera.rotate(dx, dy);	// 以相机为中心旋转
		}
		else {
			// 以原点为中心旋转
			camera.move(-dx / 100.0f, dy / 100.0f, 0.0f);
			camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
	if (status.leftMouseButtonPressed == true) {
		// 移动射线准心
		status.shootPos[0] = xpos;
		status.shootPos[1] = ypos;
		std::cout << "移动射线:(" << xpos << "," << ypos << ")" << std::endl;
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
		switch (key) {
		case GLFW_KEY_W:
			camera.move(0.0f, 0.0f, 0.1f);
			break;
		case GLFW_KEY_S:
			camera.move(0.0f, 0.0f, -0.1f);
			break;
		case GLFW_KEY_A:
			camera.move(-0.1f, 0.0f, 0.0f);
			break;
		case GLFW_KEY_D:
			camera.move(0.1f, 0.0f, 0.0f);
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

GLFWwindow* GLFWinit() {
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

	status.lastTime = 0.0f;
	status.currentTime = (float)glfwGetTime();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LINE_SMOOTH);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_FILL);
	glViewport(0, 0, WIDTH, HEIGHT);
	return window;
}


int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();

	Shader* normShader = new Shader("nshader.gvs", "nshader.ggs", "nshader.gfs");
	Shader* shader = new Shader("shader.gvs", "shader.gfs");

	Drawable* axis = new Axis(shader);
	Geometry* obj = new Cone(1.0f, 2.0f, 4, 8, 36, normShader);
	obj->rotate(-glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


	shader->use();
	(*shader)["projection"] = camera.getProjectionMatrix();
	normShader->use();
	(*normShader)["projection"] = camera.getProjectionMatrix();

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader->use();
		(*shader)["view"] = camera.getViewMatrix();
		normShader->use();
		(*normShader)["view"] = camera.getViewMatrix();

		status.lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;

		//leaf->draw();
		obj->draw();
		axis->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}


//int main(int argc, char** argv) {
//
//	GLFWwindow* window = GLFWinit();
//
//	Shader* sd = new Shader("lightShader.gvs", "lightShader.gfs");	//默认着色器
//	Shader& shader = *sd;
//
//	std::vector<Geometry*> objs;
//
//	//objs.push_back(new Cube(2.0f, 3.0f, 5.0f, 8, 12, 20, shader));
//	//objs.push_back(new Sphere(2.0f, 40, 20, shader));
//	//objs.push_back(new Cylinder(1.0f, 6.0f, 4, 24, 40, shader));
//	//objs.push_back(new Cone(2.0f, 3.0f, 10, 30, 60, shader));
//	objs.push_back(new Surface(-4.0f, 4.0f, -4.0f, 4.0f, SurfaceFunc, SurfaceGrad, 320, 320, sd));
//	objs[0]->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//	objs[0]->translate(glm::vec3(0.0f, -2.0f, 0.0f));
//
//	objs.push_back(new Sphere(0.02f, 20, 40, sd));
//	objs.back()->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
//
//	Drawable * axis = new Axis(sd);
//
//	initLineDrawing(sd);
//
//	float deltaTime = 0.0f;
//	float lastTime = 0.0f;
//	float currentTime = glfwGetTime();
//
//	shader.use();
//	shader["projection"] = camera.getProjectionMatrix();
//	shader["lightColor"] = glm::vec3(1.0f, 1.0f, 1.0f);
//	shader["lightPos"] = status.lightPos;
//
//	while (!glfwWindowShouldClose(window)) {
//		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		lastTime = currentTime;
//		currentTime = glfwGetTime();
//		deltaTime = currentTime - lastTime;
//
//		shader.use();
//		shader["view"] = camera.getViewMatrix();
//		shader["lightPos"] = status.lightPos;
//		objs[1]->moveTo(status.lightPos);
//
//		for (int i = 0; i < objs.size(); i++) {
//			//objs[i]->rotate((32 + i * 3) * glm::radians(deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
//			objs[i]->draw();
//		}
//
//		axis->draw();
//
//		showLines();
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	glfwTerminate();
//	return 0;
//}

