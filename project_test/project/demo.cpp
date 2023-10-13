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

StatusInfo status;
Camera* camera = nullptr;
Leaf* leaf = nullptr;

int main(int argc, char** argv) {
	GLFWwindow* window = GLFWinit();
	GUI gui(window);

	float deltaTime;

	camera = new Camera(glm::vec3(-0.4f, 1.8f, 7.0f), glm::vec3(0.4f, 0.5f, 0.0f));
	Scene scene(camera);

	//Axis* axis = new Axis();

	Geometry* stalk = new Cylinder(0.1f, 1.0f);
	//leaf = new Leaf(0.2f, 3.0f, 5, 30);
	//leaf->translateTo(glm::vec3(0.1f, 0.95f, 0.0f));

	//Combination* com = new Combination();
	//com->add(leaf);
	//com->add(stalk);

	float t = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gui.update();

		deltaTime = scene.step();
		t += deltaTime;
		if (t > 1.0f) {
			t = 0.0f;
			std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
		}

		//scene.render(axis);
		//scene.render(leaf);
		//scene.render(com);
		scene.render(stalk);

		gui.render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

