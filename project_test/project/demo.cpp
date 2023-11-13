//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "proj.h"

#ifdef TEST_OPENGL

#define STB_IMAGE_IMPLEMENTATION

#include "Scene.hpp"
#include "InputManager.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "GUI.hpp"

#include "interaction.h"
#include "utils.h"
#include "proj.h"

void platformCheck() {
	// 检查glm的vec类型内存结构
	static_assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3, "glm::vec3不支持");
	static_assert(sizeof(glm::vec4) == sizeof(GLfloat) * 4, "glm::vec4不支持");
	static_assert(sizeof(Vertex) == sizeof(GLfloat) * 10, "Vertex不支持");
}

//StatusInfo status;
//Camera* camera{ nullptr };
//Leaf* leaf{ nullptr };
int main(int argc, char** argv) {
	platformCheck();

	GLFWwindow* window = GLFWinit();
	Camera* camera = new Camera(vec3(0.0f, 1.3f, 6.0f), vec3(0.0f, 1.3f, 0.0f));
	Scene scene(camera);

	//Axis axis;

	Cylinder c1(0.04f, 1.0f), c2(0.04f, 1.0f), c3(0.04f, 1.0f);
	Sphere s1(0.06f), s2(0.06f);
	Leaf leaf_a(2.0f, 0.2f), leaf_b(2.0f, 0.2f);

	Light light(vec3(6.0f, 6.0f, 0.0f), vec3(0.0f, 1.3f, 0.0f));
	light.setFieldSize(8.0f, 6.0f, 0.001f, 20.0f);
	light.setResolution(320, 240);

	InputManager inputmanager(window, &scene);
	GUI gui(window);

	//leaf = &leaf_a;

	c1.addChild(&s1, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s1.addChild(&c2, Transform(vec3(0.0f), 30.0f, _front));
	c2.addChild(&s2, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s2.addChild(&c3);

	c1.addChild(&leaf_a, Transform(vec3(0.0f, 1.0f, 0.0f)));
	c2.addChild(&leaf_b, Transform(vec3(0.04f, 1.0f, 0.0f), 180.0f, _up));

	//scene.add(&axis);
	scene.add(&c1);
	scene.addLight(&light);

	c3.rotate(-30.0f, _front);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gui.update();
		float deltaTime = scene.step();

		c1.rotate(deltaTime * 20.0f, _up);


		scene.render();
		//scene.updateDepthMap();	//更新深度图
		//float flux = scene.computeRadiantFlux();	//计算辐射通量
		//cout << "当前辐射通量为  " << flux << endl;

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
	//Leaf leaf_a(2.0f, 0.2f);

	Light light(
		vec3(0.0f, 2.0f, 0.0f), vec3(0.5f, -0.9f, 0.0f),		// 位置，方向
		vec3(1.0f, 1.0f, 1.0f), 1.0f,									// 颜色，强度
		1.2f, 2.4f															// 矩形覆盖范围，宽度，高度
	);

	//scene.add(dynamic_cast<Geometry*>(&leaf_a));

	Cylinder c1(0.12f, 1.0f, 10, 10, 12), c2(0.12f, 1.0f, 10, 10, 12);
	Sphere s1(0.1f, 12, 10);

	//c1.translateTo(vec3(0.5f, 0.0f, 0.0f));
	c1.addChild(&s1, Transform(vec3(0.0f, 1.0f, 0.0f)));
	s1.addChild(&c2);

	c2.rotate(-75.0f, _front);

	scene.add(&c1);
	scene.add(&light);

	c2.rotate(40.0f, _up);

	//// 生成深度图
	//light.genLightSample(30, 60);
	//light.genDepthMap();				// 该函数要在light添加到场景中后才有效
	light.genDepthMap(400, 300);

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
