#include "proj.h"

#ifdef TEST_OPENGL

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
	s1.addChild(&c2, Transform(vec3(0.0f), 30.0f, _front));
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
#endif

#ifdef TEST_SOFT_RASTERIZATION

#include "Scene.hpp"
#include "Light.hpp"
#include "Wheat.hpp"
#include "Geometry.hpp"
#include "Camera.hpp"

#include "interaction.h"
#include "utils.h"

Camera* camera{ nullptr };
StatusInfo status;

Mat canvas;		// 为了方便调试声明在全局域

using namespace cv;

int main(int argc, char** argv) {
	Camera camera_m(vec3(0.0f, 2.0f, 5.0f), vec3(0.3f, 0.5f, 0.0f));
	camera = &camera_m;

	Scene scene(camera);
	Leaf leaf_a(2.0f, 0.2f);

	Light light(
		vec3(0.0f, 2.0f, 0.0f), vec3(0.5f, -0.9f, 0.0f),		// 位置，方向
		vec3(1.0f, 1.0f, 1.0f), 1.0f,									// 颜色，强度
		1.2f, 2.4f															// 矩形覆盖范围，宽度，高度
	);

	//scene.add(dynamic_cast<Geometry*>(&leaf_a));

	Cylinder c1(0.08f, 1.0f), c2(0.08f, 2.0f);
	Sphere s1(0.1f);

	//c1.translateTo(vec3(0.5f, 0.0f, 0.0f));
	c1.addChild(&s1, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s1.addChild(&c2);

	c2.rotate(45.0f, _front);

	scene.add(&c1);
	scene.add(&light);

	c2.rotate(90.0f, _up);

	// 生成深度图
	light.genLightSample(20, 40);
	light.genDepthMap();				// 该函数要在light添加到场景中后才有效

	namedWindow("demo", WINDOW_NORMAL);
	setMouseCallback("demo", opencv_mouseCallback, 0);
	int key = 0;
	bool quit = false;

	while (!quit) {
		key = waitKey(10);
		switch (key) {
		case 27:
			quit = true;
			break;
		case 'w':
			camera->move(0.0f, 0.0f, 10.0f);
			break;
		case 's':
			camera->move(0.0f, 0.0f, -10.0f);
			break;
		case 'a':
			camera->move(-10.0f, 0.0f, 0.0f);
			break;
		case 'd':
			camera->move(10.0f, 0.0f, 0.0f);
			break;
		}
		canvas = Mat(HEIGHT, WIDTH, CV_32FC3, Vec3f(1.0f, 1.0f, 1.0f));
		//c2.rotate(13.0f, _up);
		//light.genDepthMap();
		scene.render();
		imshow("demo", canvas);
	}
	cv::destroyAllWindows();

	return 0;
}

#endif
