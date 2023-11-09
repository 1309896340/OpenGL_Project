#include "proj.h"

#ifndef __WIND_LIGHT
#define __WIND_LIGHT

#ifdef TEST_SOFT_RASTERIZATION

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
	bool checkInTriangle(const vec3& point, const vector<vec3>& wPos, float* depth) {
		// �жϵ��Ƿ��������ڲ�������ڣ��򷵻�true����ͨ��depth���ؼ���õ������ֵ
		float v1 = cross(wPos[1] - wPos[0], point - wPos[0]).z;
		float v2 = cross(wPos[2] - wPos[1], point - wPos[1]).z;
		float v3 = cross(wPos[0] - wPos[2], point - wPos[2]).z;
		bool isHit = false;
		if (v1 > 0.0f && v2 > 0.0f && v3 > 0.0f)
			isHit = true;
		if (v1 < 0.0f && v2 < 0.0f && v3 < 0.0f)
			isHit = true;
		if (isHit) {
			// �������ֵ
			float Adet = dot(wPos[0], cross(wPos[1], wPos[2]));
			float A2det = dot(wPos[0], cross(point, wPos[2]));
			float A3det = dot(wPos[0], cross(wPos[1], point));
			float k1 = A2det / Adet;
			float k2 = A3det / Adet;
			*depth = (1 - k1 - k2) * wPos[0].z + k1 * wPos[1].z + k2 * wPos[2].z;
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
		this->projection = glm::ortho(-this->width / 2.0f, this->width / 2.0f, -this->height / 2.0f, this->height / 2.0f, 0.01f, 3.0f);
	}

	~Light() {
	}

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

		for (unsigned int i = 0; i < hNum * wNum; i++)
			this->depthmap.ptr[i] = FLT_MAX;		// ��ʼ��Ϊ���ֵ


		// �������ͼ
		assert(triangleGetter != nullptr);
		vector<Triangle> tribuf;
		triangleGetter->getAllTriangles(tribuf);
		for (unsigned int t = 0; t < tribuf.size(); t++) {
			auto& triangle = tribuf[t];
			cout << "��" << t << "/" << tribuf.size() << "��������" << endl;
			// �������ε�������������任����Դ���ӽ��£�Ȼ���������任
			vector<vec3> vPos(3);
			for (unsigned int k = 0; k < 3; k++)
				vPos[k] = this->world2screen(triangle.vertex[k].position);
			// �������ǵļ򵥰�Χ��
			float minX = std::min(vPos[0].x, std::min(vPos[1].x, vPos[2].x));
			float maxX = std::max(vPos[0].x, std::max(vPos[1].x, vPos[2].x));
			float minY = std::min(vPos[0].y, std::min(vPos[1].y, vPos[2].y));
			float maxY = std::max(vPos[0].y, std::max(vPos[1].y, vPos[2].y));

			for (unsigned int i = 0; i < this->depthmap.width * this->depthmap.height; i++) {
				unsigned int w = i % (this->depthmap.width), h = i / (this->depthmap.height);	// ͼ��ǰ���ص�����
				float wCur = -(this->width) / 2.0f + dw / 2.0f + w * dw;		// ������
				float hCur = -(this->height) / 2.0f + dh / 2.0f + h * dh;
				// �ж�(wCur,hCur)�Ƿ���vPos[0]��vPos[1]��vPos[2]��ͬ��
				// ��ʱ�Ⱥ���z���꣬��ֻ����(wCur, hCur, 0)��(vPos.x, vPos.y, 0)
				// �ڵõ�(wCur, hCur, 0)���������ڲ����ٽ����ֵ�������ԭ���ͼ��Ӧλ��Ԫ�رȽϣ�ȡ��Сֵ
				vec3 checkPoint(wCur, hCur, 0.0f);
				float depth = FLT_MAX;
				if (wCur<minX || wCur>maxX || hCur<minY || hCur > maxY) {
					//cout << "(" << w << "," << h << ")" << "û�л���" << endl;		// ������Ϣ
					continue;
				}

				if (checkInTriangle(checkPoint, vPos, &depth)) {
					// checkPoint���������ڲ�����¼���ֵ
					depthmap.ptr[h * wNum + w] = std::min(depthmap.ptr[h * wNum + w], depth);
					cout << "(" << w << "," << h << ")" << "������ȣ�" << depth << endl;		// ������Ϣ
				}
				else {
					//cout << "(" << w << "," << h << ")" << "û�л���" << endl;		// ������Ϣ
				}
			}
		}
		// �������ͼ���
		Mat depthimg(hNum, wNum, CV_8UC1);
		std::stringstream imgPath;
		imgPath << "C:\\Users\\windwhisper\\Desktop\\test_image\\data" << ".jpg";
		for (unsigned int h = 0; h < depthmap.height; h++)
			for (unsigned int w = 0; w < depthmap.width; w++) {
				float tmp = depthmap.ptr[h * wNum + w];
				unsigned char ttp = 0;
				if (tmp != FLT_MAX) {
					ttp = (unsigned char)((tmp + 1.0f) / 2.0f * 255.0f);
				}
				depthimg.at<unsigned char>(hNum - 1 - h, w) = ttp;
			}
		cv::imwrite(imgPath.str(), depthimg);
		cout << "�������ͼ���������ͼƬ���ļ� " << imgPath.str() << endl;
		exit(0);

		return this->depthmap;
	}

	//vec3* getLightSamplePos() {
	//	return this->lightSamplePos;
	//}
	DepthMap getDepthMap() {
		return this->depthmap;
	}
};
#endif	// !LIGHT_H


#ifdef TEST_OPENGL
class Light {
	// �ٶ�Ϊƽ�й⣬����λ�á����򡢹ⳡ�Ŀ�ߡ��ֱ���
private:
	vec3 position{ vec3(0.0f,0.0f,0.0f) };
	vec3 direction{ -_front };

	vec3 color{ vec3(1.0f,1.0f,1.0f) };
	float intensity{ 1.0f };

	float width{ 2.0f };
	float height{ 2.0f };
	float nearPlane{ 0.001f };
	float farPlane{ 14.0f };

	// ����������ķֱ���
	unsigned int wSliceNum{ 100 };
	unsigned int hSliceNum{ 100 };

	bool changeFlag{ false };
public:
	Light() = delete;
	Light(vec3 position, vec3 target, float intensity = 1.0f) :position(position), direction(normalize(target - position)), intensity(intensity) {
	}

	void setResolution(unsigned int wRes, unsigned int hRes) {
		wSliceNum = wRes;
		hSliceNum = hRes;
	}

	void setFieldSize(float width, float height, float near, float far) {
		this->width = width;
		this->height = height;
		this->nearPlane = near;
		this->farPlane = far;
		setChangeFlag();
	}

	void setPosition(vec3 position) {
		this->position = position;
		setChangeFlag();
	}

	void setDirection(vec3 direction) {
		this->direction = normalize(direction);
		setChangeFlag();
	}

	void getResolution(unsigned int* wRes, unsigned int* hRes) {
		*wRes = wSliceNum;
		*hRes = hSliceNum;
	}

	void getFieldSize(float* width, float* height, float* near, float* far) {
		*width = this->width;
		*height = this->height;
		*near = this->nearPlane;
		*far = this->farPlane;
	}

	vec3 getPosition() {
		return this->position;
	}
	vec3 getDirection() {
		return this->direction;
	}
	vec3 getColor() {
		return this->color;
	}
	float getIntensity() {
		return this->intensity;
	}
	

	// �õ�����������ϵ�����ӽ��µı�׼�ü�����ϵ�ı任����
	// �÷�����Ҫ�����ṩ��Scene�������ͼ
	mat4 getProjectionViewMatrix() {
		mat4 projection = glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, nearPlane, farPlane);
		mat4 view = glm::lookAt(this->position, this->position + this->direction, _up);
		return projection * view;
	}

	// ����Scene�ж��Ƿ�������ͼ�Ľӿ�
	bool isChange() {
		return this->changeFlag;
	}
	void resetChangeFlag() {
		this->changeFlag = false;
	}
	void setChangeFlag() {
		this->changeFlag = true;
	}
};

#endif

#endif
