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
	vec3* lightSamplePos{ nullptr };
	bool updateSample{ false };
	DepthMap depthmap;		// 用于存储深度图

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
		width(width), height(height) {}
	~Light() {
		if (lightSamplePos != nullptr)
			delete[] lightSamplePos;
	}
	//void setScene(Scene* scene) {
	//	this->scene = scene;
	//}

	void setTriangleGetter(TriangleGetter* triangleGetter) {
		this->triangleGetter = triangleGetter;
	}

	void genLightSample(unsigned int wNum, unsigned int hNum) {
		if (lightSamplePos != nullptr)
			delete[] lightSamplePos;
		cout << "生成一次光线采样" << endl;

		this->wNum = wNum;
		this->hNum = hNum;
		// 确定分割步长
		float hStep = this->height / hNum;
		float wStep = this->width / wNum;
		// 计算局部坐标系
		vec3 direction = glm::normalize(this->direction);
		vec3 right = glm::normalize(glm::cross(direction, _up));
		vec3 up = glm::normalize(glm::cross(right, direction));
		// 生成光线采样网格
		lightSamplePos = new vec3[wNum * hNum];
		for (unsigned int h = 0; h < hNum; h++) {
			for (unsigned int w = 0; w < wNum; w++) {
				vec3 hPos = (-this->height / 2.0f + hStep / 2.0f + hStep * h) * up;
				vec3 wPos = (-this->width / 2.0f + wStep / 2.0f + wStep * w) * right;
				lightSamplePos[h * wNum + w] = position + hPos + wPos;
			}
		}
		this->updateSample = true;
	}

	DepthMap genDepthMap() {		// 该操作会执行所有采样光线与场景中所有三角面元的求交判断，计算开销较大
		if (lightSamplePos == nullptr) {
			cout << "未生成采样光线" << endl;
			return this->depthmap;
		}
		cout << "生成一次深度图" << endl;

		// 初始化深度图
		if (this->updateSample) {
			if (this->depthmap.ptr != nullptr)
				delete[] this->depthmap.ptr;

			this->depthmap.ptr = new float[this->wNum * this->hNum];
			this->depthmap.width = this->wNum;
			this->depthmap.height = this->hNum;

			this->updateSample = false;
		}
		for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++)
			this->depthmap.ptr[i] = FLT_MAX;		// 初始化为最大值	

		// 计算深度图
		assert(triangleGetter != nullptr);
		vector<Triangle> tribuf;
		triangleGetter->getAllTriangles(tribuf);
		for (auto& triangle : tribuf) {
			for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
				// 判断光线与三角形是否相交
				float depth;
				bool isHit = rayTriangleIntersect(triangle, lightSamplePos[i], this->direction, &depth);
				if (isHit) {
					this->depthmap.ptr[i] = std::min(this->depthmap.ptr[i], depth);
					cout << i << "击中，深度为 " << this->depthmap.ptr[i] << endl;
				}
			}
		}
		return this->depthmap;
	}
	vec3* getLightSamplePos() {
		return this->lightSamplePos;
	}
	DepthMap getDepthMap() {
		return this->depthmap;
	}
};

#endif
