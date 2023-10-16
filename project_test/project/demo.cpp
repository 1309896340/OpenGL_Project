#include "Camera.hpp"
#include "Shader.hpp"
#include "Geometry.hpp"
#include "Scene.hpp"
#include "GUI.hpp"
#include "interaction.h"
#include "line.h"
#include "utils.h"
#include "proj.h"


glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// ���ｫz��������Ϊfront���������漰��������Ҫ������������ϵ������cross����ʱ��Ҫ�ر�ע��
glm::vec3 _origin = glm::vec3(0.0f, 0.0f, 0.0f);


StatusInfo status;
Camera* camera = nullptr;
Leaf* leaf = nullptr;

int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();
	GUI gui(window);

	float deltaTime;

	//camera = new Camera(glm::vec3(-0.4f, 1.8f, 7.0f), glm::vec3(0.4f, 0.5f, 0.0f));
	camera = new Camera(glm::vec3(-0.4f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	Scene scene(camera);

	leaf = new Leaf(1.0f, 0.2f);
	Leaf b(3.0f, 0.1f);
	Cylinder stalk(0.1f, 1.0f);
	stalk.addChild(leaf, Transform(glm::vec3(0.0f, 1.0f, 0.0f)));
	stalk.addChild(&b, Transform(glm::vec3(0.0f, 0.5f, 0.0f), 135.0f, _up));

	Axis axis;
	//Arrow axis_x(glm::vec3(0.0f, 0.0f, 0.0f), _right, _right);
	//Arrow axis_y(glm::vec3(0.0f, 0.0f, 0.0f), _up, _up);
	//Arrow axis_z(glm::vec3(0.0f, 0.0f, 0.0f), -_front, -_front);

	float t;

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gui.update();
		deltaTime = scene.step(&t);

		scene.render(&axis);
		scene.render(&stalk);

		gui.render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

