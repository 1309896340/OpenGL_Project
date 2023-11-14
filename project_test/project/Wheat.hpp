#ifndef __WIND_WHEAT
#define __WIND_WHEAT
#include "proj.h"
#include "Geometry.hpp"


class LeafMesh : public Mesh {
	// 实现专用于Leaf的网格变化，在其参数改变后更新顶点数据，但还没有更新VBO
	// 因此当网络改变时，将isChanged设置为true，以便Scene通过检查该标志来决定是否更新VBO
private:
	unsigned int wSliceNum, hSliceNum;

	float wFunc(float h) {
		return (-1.0f / powf(height, 2.0f) * h * h + 2.0f * theta / height * h + (1.0f - 2.0f * theta)) * width / powf(1.0f - theta, 2.0f);
	}
public:
	float width{ 1.0f }, height{ 1.0f };
	float k = 4.4f, SLAngle = 30.0f, MVAngle = 0.0f, theta = 0.4f;

	LeafMesh(float height, float width, unsigned int uSize = 2, unsigned int vSize = 2) :
		Mesh(uSize, vSize), height(height), width(width), wSliceNum(uSize), hSliceNum(vSize) {
	}
	virtual void updateVertex() {
		float x_accum = 0.0f, y_accum = 0.0f;
		float a = -k * width / height, b = tanf(PI / 2.0f - SLAngle * PI / 180.0f);
		float ds = height / hSliceNum;
		float tmp, frac, costheta, sintheta;
		float x_rt, x, z, z_rt;
		vec3 _offset, _axis;
		mat4 trans(1.0f);
		vec4 tp;

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
				vertex[v * (uSize + 1) + u].position = { tp.x, tp.y, tp.z };
				// 法线旋转
				tp = transpose(inverse(trans)) * vec4(-sintheta, costheta, 0.0f, 0.0f);
				vertex[v * (uSize + 1) + u].normal = { tp.x, tp.y, tp.z };
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
	// 由管理该mesh的Geometry调用，用于设置带生成网格顶点的外围属性，调用过后还需要调用updateVertex计算并更新
	void setLength(float length) {
		this->height = length;
		changeFlag = true;
	}
	void setTheta(float theta) {
		this->theta = theta;
		changeFlag = true;
	}
	void addTheta(float delta) {
		this->theta += delta;
		if (this->theta > 0.5f - 0.01f) {
			this->theta = 0.5f - 0.01f;
		}
		else if (this->theta < 0.0f) {
			this->theta = 0.0f;
		}
		changeFlag = true;
	}
	void setK(float k) {
		this->k = k;
		changeFlag = true;
	}
	void addK(float delta) {
		this->k = delta;
		this->k += delta;
		if (this->k > 20.0f) {
			this->k = 20.0f;
		}
		else if (this->k < 0.0f) {
			this->k = 0.0f;
		}
		changeFlag = true;
	}
	void setSLAngle(float angle) {
		this->SLAngle = angle;
		changeFlag = true;
	}
	void addSLAngle(float delta) {
		this->SLAngle += delta;
		if (this->SLAngle > 90.0f) {
			this->SLAngle = 90.0f;
		}
		else if (this->SLAngle < 0.1f) {
			this->SLAngle = 0.1f;
		}
		changeFlag = true;
	}
	void setMVAngle(float angle) {
		this->MVAngle = angle;
		changeFlag = true;
	}
};

// 要在Scene中正确渲染Geometry的Leaf子类，需要解决Geometry.addChild()、Scene.addOne()、Scene.add()三个方法的问题
// 如果不进行重载，则会自动转型为Geometry，也就无法识别Leaf的属性
// 考虑到渲染端不需要知道Geometry的具体子类，只需要在渲染其每一个Mesh时调用其isChanged()方法来判断是否需要更新，再调用updateVertex()方法来更新顶点数据即可
// 对具体子类的属性修改，只发生在客户端未add入Scene之前的子类指针，因此无需重载Geometry.addChild()、Scene.addOne()、Scene.add()，渲染端不用关心具体子类
class Leaf : public Geometry {
private:
	static inline unsigned int subIdCount = 0;
public:
	Leaf(float height, float width, unsigned int hSliceNum = 20, unsigned int wSliceNum = 5) {
		this->subId = subIdCount++;
		this->name = "Leaf_" + std::to_string(subId);
		this->type = LEAF;
		needCalFlux = true;		// 叶子网格需要计算辐射通量

		// 暂时没有考虑height、width、hSliceNum、wSliceNum改变的情况
		// 这里Geometry和其meshes[0]都有一份height、width、hSliceNum、wSliceNum的拷贝，如何保证一致性？
		// meshes[0]需要这些信息来计算顶点坐标，而Geometry需要这些信息来计算包围盒，考虑移除Leaf类中的拷贝，重载获取这些属性的方法，将它们从meshes[0]中获取
		meshes.push_back(new LeafMesh(height, width, wSliceNum, hSliceNum));
		meshes[0]->connect();

		meshes[0]->updateVertex();		// 同时更新坐标位置和法向量

		// 默认绿色
		//attribute = { false, vec4(0.0f,1.0f,0.0f,1.0f) };
		//attribute = { false, vec4(0.0f,1.0f,1.0f,1.0f) };
		// 设置纹理
		//meshes[0]->setTexture("texture/leaf.jpg");

		model.reset();
		pose();
	}

	virtual void pose() {}

	// 将对Leaf这个Geometry对象的修改应用到其meshes[0]上，这些方法作为适配
	void setLength(float length) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->setLength(length);
	}
	void setTheta(float theta) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->setTheta(theta);
	}
	void addTheta(float delta) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->addTheta(delta);
	}
	void setK(float k) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->setK(k);
	}
	void addK(float delta) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->addK(delta);
	}
	void setSLAngle(float angle) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->setSLAngle(angle);
	}
	void addSLAngle(float delta) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->addSLAngle(delta);
	}
	void setMVAngle(float angle) {
		LeafMesh* mesh = dynamic_cast<LeafMesh*>(meshes[0]);
		mesh->setMVAngle(angle);
	}
};

#endif

