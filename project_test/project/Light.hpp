#include "proj.h"

#ifndef __WIND_LIGHT
#define __WIND_LIGHT

#include "Geometry.hpp"
#include "TriangleGetter.hpp"

typedef struct _DepthMap {
	float* ptr{ nullptr };	// ��h*width+w������˳����д洢
	unsigned int width{ 0 };
	unsigned int height{ 0 };
}DepthMap;

class Light {
	// Ŀǰ�ݶ���Ϊƽ�й�
private:
	// ����genLightSample()�Ż�����
	//Scene* scene{ nullptr };
	TriangleGetter* triangleGetter{ nullptr };
	//vec3* lightSamplePos{ nullptr };					// ��¼���ɵĹ��߲�������
	//bool updateSample{ false };							// ����Ƿ���Ҫ���¹��߲�������
	DepthMap depthmap;		// ���ڴ洢���ͼ

	mat4 view{ identity<mat4>() };
	mat4 projection{ identity<mat4>() };

	// �жϹ����ཻ�㷨
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
			*tnear = tt;	// ���й��ߵĳ���
			return true;
		}
		return false;
	}
public:
	vec3 position{ 0.0f,1.5f,0.0f };
	vec3 direction{ 0.0f,-1.0f,0.0f };
	vec3 color{ 1.0f,1.0f,1.0f };
	float intensity{ 1.0f };	// ����ָ�����նȣ���λΪW/m^2������ƽ�й⣬����λ�÷��ն����
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

	vec3 world2screen(vec3& pos) {		// ����������ϵ�µĵ�ת������Դ���ӽ�����ϵ�£�������zֵ
		vec4 tmp = (this->projection) * (this->view) * vec4(pos, 1.0f);
		tmp /= tmp.w;
		return vec3(tmp.x, tmp.y, tmp.z);
	}

	//void genLightSample(unsigned int wNum, unsigned int hNum) {
	//	if (lightSamplePos != nullptr)
	//		delete[] lightSamplePos;
	//	cout << "����һ�ι��߲���" << endl;

	//	this->wNum = wNum;
	//	this->hNum = hNum;
	//	// ȷ���ָ��
	//	float hStep = this->height / hNum;
	//	float wStep = this->width / wNum;
	//	// ����ֲ�����ϵ
	//	vec3 direction = glm::normalize(this->direction);
	//	vec3 right = glm::normalize(glm::cross(direction, _up));
	//	vec3 up = glm::normalize(glm::cross(right, direction));
	//	// ���ɹ��߲�������
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

	//DepthMap genDepthMap() {		// �ò�����ִ�����в��������볡��������������Ԫ�����жϣ����㿪���ϴ�
	//	if (lightSamplePos == nullptr) {
	//		cout << "δ���ɲ�������" << endl;
	//		return this->depthmap;
	//	}
	//	cout << "����һ�����ͼ" << endl;
	//	// ��ʼ�����ͼ
	//	if (this->updateSample) {
	//		if (this->depthmap.ptr != nullptr)
	//			delete[] this->depthmap.ptr;
	//		this->depthmap.ptr = new float[this->wNum * this->hNum];
	//		this->depthmap.width = this->wNum;
	//		this->depthmap.height = this->hNum;
	//		this->updateSample = false;
	//	}
	//	for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++)
	//		this->depthmap.ptr[i] = FLT_MAX;		// ��ʼ��Ϊ���ֵ	
	//	// �������ͼ
	//	assert(triangleGetter != nullptr);
	//	vector<Triangle> tribuf;
	//	triangleGetter->getAllTriangles(tribuf);
	//	for (auto& triangle : tribuf) {
	//		for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
	//			unsigned int w = i % (this->depthmap.width), h = i / (this->depthmap.height);	// ͼ��ǰ���ص�����
	//			vec3 curPos = lightSamplePos[i];	// ��ǰ�������ʵ������
	//		}
	//	}
	//	return this->depthmap;
	//}

	DepthMap genDepthMap(unsigned int wNum, unsigned int hNum) {
		// ͨ���ӽǱ任+���դ�ķ�ʽ��¼���
		// ��Ҫ���������ͼƬ�ķֱ��ʣ���Ӧ�����������µķ�Χ��Lightʵ����ʱ���Ѿ���ʼ��Ϊthis->width��this->height
		this->wNum = wNum;
		this->hNum = hNum;
		// �ָ��
		float dw = this->width / wNum;
		float dh = this->height / hNum;
		// ��ʼ�����ͼ
		if (this->depthmap.ptr != nullptr)
			delete[] this->depthmap.ptr;
		this->depthmap.ptr = new float[wNum * hNum];
		this->depthmap.width = wNum;
		this->depthmap.height = hNum;

		// �������ͼ
		assert(triangleGetter != nullptr);
		vector<Triangle> tribuf;
		triangleGetter->getAllTriangles(tribuf);
		for (auto& triangle : tribuf) {
			for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
				unsigned int w = i % (this->depthmap.width), h = i / (this->depthmap.height);	// ͼ��ǰ���ص�����
				float wCur = -(this->width) / 2.0f + dw / 2.0f + w * dw;		// ������
				float hCur = -(this->height) / 2.0f + dh / 2.0f + h * dh;
				// �������ε�������������任����Դ���ӽ��£�Ȼ���������任
				vec3 vPos[3];
				for (unsigned int k = 0; k < 3; k++) 
					vPos[k] = this->world2screen(triangle.vertex[k].position);
				// �ж�(wCur,hCur)�Ƿ���vPos[0]��vPos[1]��vPos[2]��xy����ͬ��
				// ��ʱû�����Ҫ��ôʵ��
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
