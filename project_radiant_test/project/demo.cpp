/*
工程说明：该工程用于尝试在CPU上实现接收辐射量计算，全程不使用OpenGL
先不考虑阴影，且以一个平坦的平面网格为对象实现简单的计算
而后以一个z=f(x,y)的任意函数曲面实现接收辐射量计算
*/


//#define STB_IMAGE_IMPLEMENTATION
//
//#include "Scene.hpp"
//#include "Geometry.hpp"
//#include "Wheat.hpp"
//#include "Shader.hpp"
//#include "Camera.hpp"
//#include "GUI.hpp"
//
//#include "interaction.h"
//#include "utils.h"
#include "proj.h"

glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// 将z负方向作为front方向，所以涉及它的运算要采用左手坐标系，在做cross运算时需要特别注意
glm::vec3 _origin = glm::vec3(0.0f, 0.0f, 0.0f);

using namespace std;

typedef struct {
	glm::vec3 position;
	glm::vec3 normal;
} Vertex;

typedef struct _Light {
	glm::vec3 position;
	glm::vec3 direction;
	float radius;
	float intensity;		// 辐射强度，单位为W/sr。将其除以球面面积，得到辐射率
	// 辐射率 radiance = intensity / (4 * pi * radius^2)
}Light;

// 首先实现一个简单平面网格的生成
void generateMesh(float width, float height, unsigned int uSize, unsigned int vSize, vector<Vertex>& vertex, vector<unsigned int>& index, function<glm::vec3(unsigned int, unsigned int)> zFunc, function<glm::vec3(unsigned int, unsigned int)> zGrad) {
	// uSize和vSize分别表示u和v方向上的网格点数
	vertex.resize(uSize * vSize, { glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f) });
	index.resize((uSize - 1) * (vSize - 1) * 6, 0);

	float du = width / (uSize - 1);
	float dv = height / (vSize - 1);
	for (unsigned int u = 0; u < uSize; u++) {
		for (unsigned int v = 0; v < vSize; v++) {
			vertex[u * vSize + v].position = zFunc(u, v);
			vertex[u * vSize + v].normal = zGrad(u, v);
			if (u != uSize - 1 && v != vSize - 1) {
				unsigned int* ptr = &index[(u * (vSize - 1) + v) * 6];		// 逆时针绘制矩形面元，法向量朝上
				ptr[0] = u * vSize + v;							// 左上角
				ptr[1] = u * vSize + v + 1;					// 左下角	
				ptr[2] = (u + 1) * vSize + v + 1;			// 右下角
				ptr[3] = u * vSize + v;							// 左上角
				ptr[4] = (u + 1) * vSize + v + 1;			// 右下角
				ptr[5] = (u + 1) * vSize;						// 右上角
			}
		}
	}
}

#define SAMPLE_NUM 10000

// 用于处理三角形面元的函数，传入三个顶点，以及光源位置，计算这个三角面元的接收辐射量
// 假设不考虑遮挡，且光源为点光源
float calculateRadiationByTriangle(const Vertex& a, const Vertex& b, const Vertex& c, const Light& light) {
	glm::vec3 aa(a.position), bb(b.position), cc(c.position);
	// 暂时不考虑顶点自身的法向量，而是用三角形面元的法向量代替
	// a b c为逆时针顺序，计算朝上的法向量
	glm::vec3 tmp = glm::cross(bb - aa, cc - aa);
	glm::vec3 N = glm::normalize(tmp);
	// 三角面元的重心
	glm::vec3 pos = (aa + bb + cc) / 3.0f;
	// 计算辐射率
	float sumbuf = 0.0f;
	float radiance = light.intensity / (4 * PI * powf(light.radius, 2.0f));
	// 对球面光源进行积分，计算在目标点上的辐射通量
	// 对球面进行均匀采样
	float dS = 4.0f * PI * powf(light.radius, 2.0f) / SAMPLE_NUM;
	for (unsigned int i = 0; i < SAMPLE_NUM; i++) {
		float theta = acosf(1.0f - (float)rand() / RAND_MAX);		// acosf(1 - 2 * (float)rand() / RAND_MAX);	由于取上半球，所以theta的范围为[0, pi/2]
		float phi = 2.0f * PI * (float)rand() / RAND_MAX;
		// // 方案一：以光源中心为原点建立局部坐标，计算出up、right、front三个轴，然后计算出L
		//glm::vec3 front = glm::normalize(light.position - pos);
		//glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		//glm::vec3 up = glm::normalize(glm::cross(right, -front));
		//glm::vec3 L = glm::normalize(light.radius * sinf(theta) * cosf(phi) * right + light.radius * sinf(theta) * sinf(phi) * up + light.radius * cosf(theta) * L);
		// // 方案二：计算出front到_up的夹角，计算垂直于front和_up所成平面的旋转轴，将球上半面旋转到front方向，然后计算出L
		glm::vec3 front = glm::normalize(light.position - pos);
		float angle = acosf(glm::dot(_up, front));
		glm::vec3 axis = glm::normalize(glm::cross(_up, front));
		float x = light.radius * sinf(theta) * cosf(phi);
		float y = light.radius * sinf(theta) * sinf(phi);
		float z = light.radius * cosf(theta);
		glm::vec3 samplePos = glm::rotate(glm::vec3(x, y, z), angle, axis) + light.position;
		glm::vec3 L = glm::normalize(samplePos - pos);
		float test = glm::dot(N, L);
		sumbuf += radiance * glm::dot(N, L) * dS;
	}
	// 假设三角面元足够小，相对球面光源近似为一点，因此只对该点积分。但该点仍具有微小的面积，因此需要乘以面积
	float tri_area = glm::length(tmp) / 2.0f;
	return sumbuf * tri_area;
}

int main(int argc, char** argv) {
	vector<Vertex> meshVertex;
	vector<unsigned int> meshIdx;

	unsigned int uSize = 40, vSize = 40;

	Light light{
		glm::vec3(3000.0f,1000.0f * sqrtf(3.0f),0.0f),		// 由于6000远大于10，因此可以近似为点光源，最后辐射通量计算结果应该约等于50
		glm::vec3(0.0f,0.0f,0.0f),
		1.0f,
		4.0f * PI
	};			// 光源位置，光源方向，光源半径，光照强度。 这里先不考虑光源方向，并令半径为1，辐射强度为4pi，这样辐射率radiance就为1


	generateMesh(10.0f, 10.0f, 40, 40, meshVertex, meshIdx,
		[uSize, vSize](unsigned int u, unsigned int v) {
			return glm::vec3((float)u / (uSize - 1), 0.0f, (float)v / (vSize - 1));		// 一个y=0上的平面，这里沿用OpenGL的坐标系规范
		},
		[uSize, vSize](unsigned int u, unsigned int v) {										// 统一朝上的法向量
			return glm::vec3(0.0f, 1.0f, 0.0f);
		}
		);

	float sumRadiation = 0.0f;

	// 遍历其中所有的矩形面元
	for (unsigned int i = 0; i < (uSize - 1) * (vSize - 1); i++) {
		unsigned int* ptr = &meshIdx[i * 6];
		// 遍历矩形面元中的两个三角形面元
		for (unsigned int k = 0; k < 2; k++) {
			float rd = calculateRadiationByTriangle(meshVertex[ptr[k * 3 + 0]], meshVertex[ptr[k * 3 + 1]], meshVertex[ptr[k * 3 + 2]], light);		// 将三角形面元三个顶点传入处理函数
			sumRadiation += rd;
		}
		cout << "radiation : " << sumRadiation << endl;
	}


	cout << "radiation : " << sumRadiation << endl;
	return 0;
}

