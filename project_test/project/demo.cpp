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
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// 这里将z负方向作为front方向，所以涉及它的运算要采用左手坐标系，在做cross运算时需要特别注意
glm::vec3 _origin = glm::vec3(0.0f, 0.0f, 0.0f);


StatusInfo status;
Camera* camera = nullptr;
Leaf* leaf = nullptr;
float deltaTime;

int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();
	GUI gui(window);

	camera = new Camera(glm::vec3(-0.4f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	Scene scene(camera);

	Axis axis;
	Leaf leaf_a(1.0f, 0.2f);
	Leaf leaf_b(2.0f, 0.2f);
	Cylinder stalk(0.1f, 1.0f);

	leaf = &leaf_a;		// 交互控制的对象

	stalk.addChild(&leaf_a, Transform(glm::vec3(0.0f, 1.0f, 0.0f)));
	stalk.addChild(&leaf_b, Transform(glm::vec3(0.0f, 0.5f, 0.0f), 180.0f, _up));

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

