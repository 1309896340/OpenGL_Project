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

	//Axis* axis = new Axis();

	Geometry* a1 = new Cylinder(0.1f, 1.0f);
	Geometry* a2 = new Cylinder(0.1f, 1.0f);
	Geometry* a3 = new Cylinder(0.1f, 1.0f);
	a2->translate(glm::vec3(0.0f, 1.0f, 0.0f));
	//a3->translate(glm::vec3(0.0f, 2.0f, 0.0f));
	a2->rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//a3->rotate(glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	a1->addChild(a2);
	//a2->addChild(a3);

	//Geometry* cube = new Cube(1.2f, 0.6f, 0.3f);
	//Geometry* sphere = new Sphere(0.6f);
	//Geometry* cylinder = new Cylinder(0.1f, 1.0f);
	//Geometry* cone = new Cone(0.4f, 0.8f);

	//cube->translateTo(glm::vec3(-1.5f, 0.0f, 0.0f));
	//sphere->translateTo(glm::vec3(0.0f, 0.0f, 0.0f));
	//cylinder->translateTo(glm::vec3(1.0f, 0.0f, 0.0f));
	//cone->translateTo(glm::vec3(2.0f, 0.0f, 0.0f));

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
		//scene.render(cube);
		//scene.render(sphere);
		//scene.render(cylinder);
		//scene.render(cone);
		scene.render(a1);
		//scene.render(a2);
		//scene.render(a3);

		gui.render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

