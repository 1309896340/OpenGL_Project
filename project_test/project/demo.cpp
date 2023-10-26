// ���ǽ������������������롰OpenGL���ӻ��������ַ��뿪��

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


vec3 _up = vec3(0.0f, 1.0f, 0.0f);
vec3 _right = vec3(1.0f, 0.0f, 0.0f);
vec3 _front = vec3(0.0f, 0.0f, 1.0f);
vec3 _origin = vec3(0.0f, 0.0f, 0.0f);

void platformCheck() {
	// ���glm��vec�����ڴ�ṹ
	static_assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3, "glm::vec3��֧��");
	static_assert(sizeof(glm::vec4) == sizeof(GLfloat) * 4, "glm::vec4��֧��");
	static_assert(sizeof(Vertex) == sizeof(GLfloat) * 10, "Vertex��֧��");
}

StatusInfo status;
Camera* camera{ nullptr };
Leaf* leaf{ nullptr };

int main(int argc, char** argv) {
	platformCheck();

	GLFWwindow* window = GLFWinit();
	GUI gui(window);
	camera = new Camera(vec3(0.0f, 1.3f, 6.0f), vec3(0.0f, 1.3f, 0.0f));
	Scene scene(camera);

	Cylinder c1(0.04f, 1.0f), c2(0.04f, 1.0f), c3(0.04f, 1.0f);
	Sphere s1(0.06f), s2(0.06f);
	Leaf leaf_a(2.0f, 0.2f);

	leaf = &leaf_a;

	c1.addChild(&s1, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s1.addChild(&c2,Transform(vec3(0.0f),30.0f,_front));
	c2.addChild(&s2, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s2.addChild(&c3);
	c3.addChild(&leaf_a, Transform(vec3(0.04f, 1.0f, 0.0f)));

	scene.add(&c1);

	c3.rotate(-30.0f, _front);


	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		gui.update();
		float deltaTime = scene.step();

		c1.rotate(deltaTime * 20.0f, _up);
		//c2.rotate(-deltaTime * 80.0f, _up);

		scene.render();
		gui.render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

