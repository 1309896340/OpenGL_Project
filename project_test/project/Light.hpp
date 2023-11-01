#include "proj.h"

#ifndef __WIND_LIGHT
#define __WIND_LIGHT

#include "Geometry.hpp"
#include "TriangleGetter.hpp"

typedef struct _DepthMap {
	float* ptr{ nullptr };	// 以h*width+w的索引顺序进行存储
	unsigned int width{ 0 };
	unsigned int height{ 0 };
}DepthMap;

class Light {
	// 目前暂定其为平行光
private:
	// 调用genLightSample()才会生成
	//Scene* scene{ nullptr };
	TriangleGetter* triangleGetter{ nullptr };
	//vec3* lightSamplePos{ nullptr };					// 记录生成的光线采样网格
	//bool updateSample{ false };							// 标记是否需要更新光线采样网格
	DepthMap depthmap;		// 用于存储深度图

	mat4 view{ identity<mat4>() };
	mat4 projection{ identity<mat4>() };

	// 判断光线相交算法
	bool rayTriangleIntersect(const Triangle& t, const vec3& lightPos, const vec3& lightDir, float* tnear) {
		vec3 S = lightPos - t.vertex[0].position;
		vec3 E1 = t.vertex[1].position - t.vertex[0].position;
		vec3 E2 = t.vertex[2].position - t.vertex[0].position;
		vec3 S1 = cross(lightDir, E2);
		vec3 S2 = cross(S, E1);
		float S1E1 = dot(S1, E1);
		float tt = dot(S2, E2) / S1E1;
		float b1 = dot(S1, S) / S1E1;
		float b2 = dot(S2, lightDir) / S1E1;
		if (tt >= 0.0f && b1 >= 0.0f && b2 >= 0.0f && (1 - b1 - b2) >= 0.0f) {
			*tnear = tt;	// 击中光线的长度
			return true;
		}
		return false;
	}
public:
	vec3 position{ 0.0f,1.5f,0.0f };
	vec3 direction{ 0.0f,-1.0f,0.0f };
	vec3 color{ 1.0f,1.0f,1.0f };
	float intensity{ 1.0f };	// 这里指辐射照度，单位为W/m^2。对于平行光，所有位置辐照度相等
	float width{ 1.0f }, height{ 1.0f };
	unsigned int wNum{ 1 }, hNum{ 1 };
	Light() = default;
	Light(vec3 position, vec3 direction, vec3 color, float intensity, float width, float height) :
		position(position), direction(normalize(direction)), color(color), intensity(intensity),
		width(width), height(height) {
		this->view = glm::lookAt(this->position, this->position + this->direction, _up);
		this->projection = glm::ortho(-this->width / 2.0f, this->width / 2.0f, -this->height / 2.0f, this->height / 2.0f, 0.1f, 100.0f);
	}

	~Light() {
		//if (lightSamplePos != nullptr)
		//	delete[] lightSamplePos;
	}
	//void setScene(Scene* scene) {
	//	this->scene = scene;
	//}

	void setTriangleGetter(TriangleGetter* triangleGetter) {
		this->triangleGetter = triangleGetter;
	}

	vec3 world2screen(vec3& pos) {		// 将世界坐标系下的点转换到光源的视角坐标系下，并保留z值
		vec4 tmp = (this->projection) * (this->view) * vec4(pos, 1.0f);
		tmp /= tmp.w;
		return vec3(tmp.x, tmp.y, tmp.z);
	}

	//void genLightSample(unsigned int wNum, unsigned int hNum) {
	//	if (lightSamplePos != nullptr)
	//		delete[] lightSamplePos;
	//	cout << "生成一次光线采样" << endl;

	//	this->wNum = wNum;
	//	this->hNum = hNum;
	//	// 确定分割步长
	//	float hStep = this->height / hNum;
	//	float wStep = this->width / wNum;
	//	// 计算局部坐标系
	//	vec3 direction = glm::normalize(this->direction);
	//	vec3 right = glm::normalize(glm::cross(direction, _up));
	//	vec3 up = glm::normalize(glm::cross(right, direction));
	//	// 生成光线采样网格
	//	lightSamplePos = new vec3[wNum * hNum];
	//	for (unsigned int h = 0; h < hNum; h++) {
	//		for (unsigned int w = 0; w < wNum; w++) {
	//			vec3 hPos = (-this->height / 2.0f + hStep / 2.0f + hStep * h) * up;
	//			vec3 wPos = (-this->width / 2.0f + wStep / 2.0f + wStep * w) * right;
	//			lightSamplePos[h * wNum + w] = position + hPos + wPos;
	//		}
	//	}
	//	this->updateSample = true;
	//}

	//DepthMap genDepthMap() {		// 该操作会执行所有采样光线与场景中所有三角面元的求交判断，计算开销较大
	//	if (lightSamplePos == nullptr) {
	//		cout << "未生成采样光线" << endl;
	//		return this->depthmap;
	//	}
	//	cout << "生成一次深度图" << endl;
	//	// 初始化深度图
	//	if (this->updateSample) {
	//		if (this->depthmap.ptr != nullptr)
	//			delete[] this->depthmap.ptr;
	//		this->depthmap.ptr = new float[this->wNum * this->hNum];
	//		this->depthmap.width = this->wNum;
	//		this->depthmap.height = this->hNum;
	//		this->updateSample = false;
	//	}
	//	for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++)
	//		this->depthmap.ptr[i] = FLT_MAX;		// 初始化为最大值	
	//	// 计算深度图
	//	assert(triangleGetter != nullptr);
	//	vector<Triangle> tribuf;
	//	triangleGetter->getAllTriangles(tribuf);
	//	for (auto& triangle : tribuf) {
	//		for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
	//			unsigned int w = i % (this->depthmap.width), h = i / (this->depthmap.height);	// 图像当前像素的坐标
	//			vec3 curPos = lightSamplePos[i];	// 当前采样点的实际坐标
	//		}
	//	}
	//	return this->depthmap;
	//}

	DepthMap genDepthMap(unsigned int wNum, unsigned int hNum) {
		// 通过视角变换+软光栅的方式记录深度
		// 需要传入待生成图片的分辨率，对应在世界坐标下的范围在Light实例化时就已经初始化为this->width和this->height
		this->wNum = wNum;
		this->hNum = hNum;
		// 分割步长
		float dw = this->width / wNum;
		float dh = this->height / hNum;
		// 初始化深度图
		if (this->depthmap.ptr != nullptr)
			delete[] this->depthmap.ptr;
		this->depthmap.ptr = new float[wNum * hNum];
		this->depthmap.width = wNum;
		this->depthmap.height = hNum;

		// 计算深度图
		assert(triangleGetter != nullptr);
		vector<Triangle> tribuf;
		triangleGetter->getAllTriangles(tribuf);
		for (auto& triangle : tribuf) {
			for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
				unsigned int w = i % (this->depthmap.width), h = i / (this->depthmap.height);	// 图像当前像素的坐标
				float wCur = -(this->width) / 2.0f + dw / 2.0f + w * dw;		// 采样点
				float hCur = -(this->height) / 2.0f + dh / 2.0f + h * dh;
				// 将三角形的三个顶点坐标变换到光源的视角下，然后做正交变换
				vec3 vPos[3];
				for (unsigned int k = 0; k < 3; k++) 
					vPos[k] = this->world2screen(triangle.vertex[k].position);
				// 判断(wCur,hCur)是否在vPos[0]、vPos[1]、vPos[2]的xy坐标同侧
				// 暂时没想清楚要怎么实现
			}
		}
		return this->depthmap;
	}

	//vec3* getLightSamplePos() {
	//	return this->lightSamplePos;
	//}
	DepthMap getDepthMap() {
		return this->depthmap;
	}
};

#endif
