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
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// ��z��������Ϊfront���������漰��������Ҫ������������ϵ������cross����ʱ��Ҫ�ر�ע��
glm::vec3 _origin = glm::vec3(0.0f, 0.0f, 0.0f);

StatusInfo status;

Camera* camera = new Camera(glm::vec3(-0.4f, 0.5f, 5.0f), glm::vec3(0.0f, 0.5f, 0.0f));		// �������Ҫ�������߼����ʣ����Զ���Ϊȫ�ֱ���
Shader* defaultShader = nullptr;

Leaf* leaf = nullptr;		// �������ƵĶ���

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

	leaf_b.setShader(scene.shaders["leaf"]);		// ��֪��Ϊʲô�������û����Ч����ʱû���ҵ�ԭ��
	leaf_a.setShader(scene.shaders["leaf"]);

	leaf = &leaf_a;		// �������ƵĶ���

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


		//scene.render(leaf, scene.shaders["normal_v"]);		// ����������Ⱦ����

		gui.render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

