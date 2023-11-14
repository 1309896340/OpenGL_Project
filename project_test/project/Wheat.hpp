#ifndef __WIND_WHEAT
#define __WIND_WHEAT
#include "proj.h"
#include "Geometry.hpp"


class LeafMesh : public Mesh {
	// ʵ��ר����Leaf������仯����������ı����¶������ݣ�����û�и���VBO
	// ��˵�����ı�ʱ����isChanged����Ϊtrue���Ա�Sceneͨ�����ñ�־�������Ƿ����VBO
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
				// ����������ת
				tp = trans * vec4(x_accum, y_accum, z, 1.0f);			// ������ת
				vertex[v * (uSize + 1) + u].position = { tp.x, tp.y, tp.z };
				// ������ת
				tp = transpose(inverse(trans)) * vec4(-sintheta, costheta, 0.0f, 0.0f);
				vertex[v * (uSize + 1) + u].normal = { tp.x, tp.y, tp.z };
			}
			tmp = 2.0f * a * x + b;
			frac = sqrtf(1.0f + tmp * tmp);
			costheta = 1.0f / frac;
			sintheta = tmp / frac;
			x_accum += ds * costheta;
			y_accum += ds * sintheta;
			// ��ͬ���ȷ�λ�㡢��ͬ��ȷ�λ��Ľڵ㣬ʹ����ͬ����ת������б任����֪�������Ƿ�Ӧ����compute shaderʵ�ָ���
			_offset = vec3(x_accum, y_accum, 0.0f);
			_axis = vec3(costheta, sintheta, 0.0f);
			trans = glm::translate(mat4(1.0f), _offset);
			trans = glm::rotate(trans, radians(MVAngle * x_rt), _axis);
			trans = glm::translate(trans, -_offset);
		}
	}
	// �ɹ����mesh��Geometry���ã��������ô��������񶥵����Χ���ԣ����ù�����Ҫ����updateVertex���㲢����
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

// Ҫ��Scene����ȷ��ȾGeometry��Leaf���࣬��Ҫ���Geometry.addChild()��Scene.addOne()��Scene.add()��������������
// ������������أ�����Զ�ת��ΪGeometry��Ҳ���޷�ʶ��Leaf������
// ���ǵ���Ⱦ�˲���Ҫ֪��Geometry�ľ������ֻ࣬��Ҫ����Ⱦ��ÿһ��Meshʱ������isChanged()�������ж��Ƿ���Ҫ���£��ٵ���updateVertex()���������¶������ݼ���
// �Ծ�������������޸ģ�ֻ�����ڿͻ���δadd��Scene֮ǰ������ָ�룬�����������Geometry.addChild()��Scene.addOne()��Scene.add()����Ⱦ�˲��ù��ľ�������
class Leaf : public Geometry {
private:
	static inline unsigned int subIdCount = 0;
public:
	Leaf(float height, float width, unsigned int hSliceNum = 20, unsigned int wSliceNum = 5) {
		this->subId = subIdCount++;
		this->name = "Leaf_" + std::to_string(subId);
		this->type = LEAF;
		needCalFlux = true;		// Ҷ��������Ҫ�������ͨ��

		// ��ʱû�п���height��width��hSliceNum��wSliceNum�ı�����
		// ����Geometry����meshes[0]����һ��height��width��hSliceNum��wSliceNum�Ŀ�������α�֤һ���ԣ�
		// meshes[0]��Ҫ��Щ��Ϣ�����㶥�����꣬��Geometry��Ҫ��Щ��Ϣ�������Χ�У������Ƴ�Leaf���еĿ��������ػ�ȡ��Щ���Եķ����������Ǵ�meshes[0]�л�ȡ
		meshes.push_back(new LeafMesh(height, width, wSliceNum, hSliceNum));
		meshes[0]->connect();

		meshes[0]->updateVertex();		// ͬʱ��������λ�úͷ�����

		// Ĭ����ɫ
		//attribute = { false, vec4(0.0f,1.0f,0.0f,1.0f) };
		//attribute = { false, vec4(0.0f,1.0f,1.0f,1.0f) };
		// ��������
		//meshes[0]->setTexture("texture/leaf.jpg");

		model.reset();
		pose();
	}

	virtual void pose() {}

	// ����Leaf���Geometry������޸�Ӧ�õ���meshes[0]�ϣ���Щ������Ϊ����
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

