#ifndef __WIND_LIGHT
#define __WIND_LIGHT
#include "proj.h"
//#include "Geometry.hpp"

class Light {
	// Ŀǰ�ݶ���Ϊƽ�й�
private:
	// ����genLightSample()�Ż�����
	vec3* lightSamples{ nullptr };
public:
	vec3 position{ 0.0f,1.5f,0.0f };
	vec3 direction{ 0.0f,-1.0f,0.0f };
	vec3 color{ 1.0f,1.0f,1.0f };
	float intensity{ 1.0f };	// ����ָ�����նȣ���λΪW/m^2������ƽ�й⣬����λ�÷��ն����
	float width{ 1.0f }, height{ 1.0f };

	Light() {}
	Light(vec3 position, vec3 direction, vec3 color, float intensity, float width, float height) :
		position(position), direction(normalize(direction)), color(color), intensity(intensity),
		width(width), height(height) {}
	~Light() {
		delete[] lightSamples;
		lightSamples = nullptr;
	}

	void genLightSample(unsigned int wNum, unsigned int hNum) {
		// ȷ���ָ��
		float hStep = this->height / hNum;
		float wStep = this->width / wNum;
		// ����ֲ�����ϵ
		vec3 direction = glm::normalize(this->direction);
		vec3 right = glm::normalize(glm::cross(direction, _up));
		vec3 up = glm::normalize(glm::cross(right, direction));
		// ���ɹ��߲�������
		lightSamples = new vec3[wNum * hNum];
		for (unsigned int h = 0; h < hNum; h++) {
			for (unsigned int w = 0; w < wNum; w++) {
				vec3 hPos = (-this->height / 2.0f + hStep / 2.0f + hStep * h) * up;
				vec3 wPos = (-this->width / 2.0f + wStep / 2.0f + wStep * w) * right;
				lightSamples[h * wNum + w] = position + hPos + wPos;
			}
		}
	}
	vec3* getLightSamples() {
		return lightSamples;
	}
};

#endif
