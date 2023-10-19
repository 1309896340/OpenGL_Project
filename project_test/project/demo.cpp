#define STB_IMAGE_IMPLEMENTATION

#include "Scene.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "GUI.hpp"

#include "interaction.h"
#include "utils.h"
#include "proj.h"

glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// 将z负方向作为front方向，所以涉及它的运算要采用左手坐标系，在做cross运算时需要特别注意
glm::vec3 _origin = glm::vec3(0.0f, 0.0f, 0.0f);

StatusInfo status;

Camera* camera = new Camera(glm::vec3(-0.4f, 0.5f, 5.0f), glm::vec3(0.0f, 0.5f, 0.0f));		// 摄像机需要被交互逻辑访问，所以定义为全局变量
Shader* defaultShader = nullptr;

Leaf* leaf = nullptr;		// 交互控制的对象

int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();
	GUI gui(window);

	Scene scene;
	scene.setCamera(camera);

	defaultShader = new Shader("shader/shader.gvs", "shader/shader.gfs");

	Axis axis;
	Leaf leaf_a(1.0f, 0.1f);
	Leaf leaf_b(2.0f, 0.1f);
	Cylinder stalk(0.04f, 1.0f);

	stalk.translateTo(0.5, 0.0f, 0.5f);

	leaf_b.setShader(scene.shaders["leaf"]);		// 不知道为什么这句像是没有生效，暂时没有找到原因
	leaf_a.setShader(scene.shaders["leaf"]);

	leaf = &leaf_a;		// 交互控制的对象

	stalk.addChild(&leaf_a, Transform(glm::vec3(0.0f, 1.0f, 0.0f)));
	stalk.addChild(&leaf_b, Transform(glm::vec3(0.0f, 0.5f, 0.0f), 180.0f, _up));

	float t;
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gui.update();
		float deltaTime = scene.step(&t);

		//stalk.rotate(deltaTime * 20.0f, _up);

		scene.render(&axis);
		scene.render(&stalk);


		//scene.render(leaf, scene.shaders["normal_v"]);		// 将法向量渲染出来

		gui.render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

