#ifndef __WIND_WHEAT
#define __WIND_WHEAT
#include "proj.h"
#include "Geometry.hpp"

class Leaf : public Geometry {
private:
	float width, height{ 1.0f };
	unsigned int wSliceNum, hSliceNum;

	float k = 4.4f, SLAngle = 30.0f, MVAngle = 0.0f, theta = 0.4f;
	bool isChanged = false;

	float wFunc(float h) {
		return (-1.0f / powf(height, 2.0f) * h * h + 2.0f * theta / height * h + (1.0f - 2.0f * theta)) * width / powf(1.0f - theta, 2.0f);
	}
public:
	Leaf(float height, float width, unsigned int hSliceNum = 20, unsigned int wSliceNum = 5) :height(height), width(width), hSliceNum(hSliceNum), wSliceNum(wSliceNum) {
		type = LEAF;

		meshes.push_back(new Mesh(wSliceNum, hSliceNum));
		meshes[0]->connect();

		updateVertex();		// 同时更新坐标位置和法向量

		// 默认绿色
		//attribute = { false, vec4(0.0f,1.0f,0.0f,1.0f) };
		//attribute = { false, vec4(0.0f,1.0f,1.0f,1.0f) };
		// 设置纹理
		//meshes[0]->setTexture("texture/leaf.jpg");

		model.reset();
		pose();
	}

	void updateVertex() {
		float x_accum = 0.0f, y_accum = 0.0f;
		float a = -k * width / height, b = tanf(PI / 2.0f - SLAngle * PI / 180.0f);
		float ds = height / hSliceNum;
		float tmp, frac, costheta, sintheta;
		float x_rt, x, z, z_rt;
		vec3 _offset, _axis;
		mat4 trans(1.0f);
		vec4 tp;

		Vertex* ptr = meshes[0]->getVertexPtr();
		unsigned int uSize = meshes[0]->getUSize(), vSize = meshes[0]->getVSize();

		tmp = b;
		frac = sqrtf(1.0f + tmp * tmp);
		costheta = 1.0f / frac;
		sintheta = tmp / frac;
		for (unsigned int v = 0; v <= hSliceNum; v++) {
			x_rt = (float)v / hSliceNum;
			x = x_rt * height;
			z_rt = wFunc(x);
			for (unsigned int u = 0; u <= wSliceNum; u++) {
				z = (u - wSliceNum / 2.0f) / (wSliceNum / 2.0f) * z_rt;
				// 进行主脉旋转
				tp = trans * vec4(x_accum, y_accum, z, 1.0f);			// 坐标旋转
				ptr[v * (uSize + 1) + u].position = { tp.x, tp.y, tp.z };
				// 法线旋转
				tp = transpose(inverse(trans)) * vec4(-sintheta, costheta, 0.0f, 0.0f);
				ptr[v * (uSize + 1) + u].normal = { tp.x, tp.y, tp.z };
			}
			tmp = 2.0f * a * x + b;
			frac = sqrtf(1.0f + tmp * tmp);
			costheta = 1.0f / frac;
			sintheta = tmp / frac;
			x_accum += ds * costheta;
			y_accum += ds * sintheta;
			// 相同长度分位点、不同宽度分位点的节点，使用相同的旋转矩阵进行变换，不知道这里是否应该用compute shader实现更好
			_offset = vec3(x_accum, y_accum, 0.0f);
			_axis = vec3(costheta, sintheta, 0.0f);
			trans = glm::translate(mat4(1.0f), _offset);
			trans = glm::rotate(trans, radians(MVAngle * x_rt), _axis);
			trans = glm::translate(trans, -_offset);
		}
	}
	virtual void pose() {}

	void setLength(float length) {
		this->height = length;
		isChanged = true;
	}
	void setTheta(float theta) {
		this->theta = theta;
		isChanged = true;
	}
	void addTheta(float delta) {
		theta += delta;
		if (theta > 0.5f - 0.01f) {
			theta = 0.5f - 0.01f;
		}
		else if (theta < 0.0f) {
			theta = 0.0f;
		}
		isChanged = true;
	}
	void setK(float k) {
		this->k = k;
		isChanged = true;
	}
	void addK(float delta) {
		k += delta;
		if (k > 20.0f) {
			k = 20.0f;
		}
		else if (k < 0.0f) {
			k = 0.0f;
		}
		isChanged = true;
	}
	void setSLAngle(float angle) {
		this->SLAngle = angle;
		isChanged = true;
	}
	void addSLAngle(float delta) {
		SLAngle += delta;
		if (SLAngle > 90.0f) {
			SLAngle = 90.0f;
		}
		else if (SLAngle < 0.1f) {
			SLAngle = 0.1f;
		}
		isChanged = true;
	}
	void setMVAngle(float angle) {
		this->MVAngle = angle;
		isChanged = true;
	}
	bool isMeshChanged() {
		return isChanged;
	}
};

#endif

