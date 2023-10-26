// 考虑将“几何形体描述”与“OpenGL可视化”两部分分离开来

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
	// 检查glm的vec类型内存结构
	static_assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3, "glm::vec3不支持");
	static_assert(sizeof(glm::vec4) == sizeof(GLfloat) * 4, "glm::vec4不支持");
	static_assert(sizeof(Vertex) == sizeof(GLfloat) * 10, "Vertex不支持");
}

StatusInfo status;
Camera* camera = new Camera(vec3(0.0f, 0.0f, 6.0f), vec3(0.0f, 0.0f, 0.0f));


int main(int argc, char** argv) {
	platformCheck();

	GLFWwindow* window = GLFWinit();
	GUI gui(window);
	Scene scene(camera);

	Cylinder c1(0.2f, 1.0f), c2(0.2f, 1.0f), c3(0.2f, 1.0f);
	Sphere s1(0.3f), s2(0.3f);

	c1.addChild(&s1, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s1.addChild(&c2);
	c2.addChild(&s2, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s2.addChild(&c3);

	scene.add(&c1);


	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		gui.update();
		float deltaTime = scene.step();

		scene.render();
		gui.render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

