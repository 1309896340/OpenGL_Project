/*
����˵�����ù������ڳ�����CPU��ʵ�ֽ��շ��������㣬ȫ�̲�ʹ��OpenGL
�Ȳ�������Ӱ������һ��ƽ̹��ƽ������Ϊ����ʵ�ּ򵥵ļ���
������һ��z=f(x,y)�����⺯������ʵ�ֽ��շ���������
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
glm::vec3 _front = glm::vec3(0.0f, 0.0f, -1.0f);			// ��z��������Ϊfront���������漰��������Ҫ������������ϵ������cross����ʱ��Ҫ�ر�ע��
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
	float intensity;		// ����ǿ�ȣ���λΪW/sr�������������������õ�������
	// ������ radiance = intensity / (4 * pi * radius^2)
}Light;

// ����ʵ��һ����ƽ�����������
void generateMesh(float width, float height, unsigned int uSize, unsigned int vSize, vector<Vertex>& vertex, vector<unsigned int>& index, function<glm::vec3(unsigned int, unsigned int)> zFunc, function<glm::vec3(unsigned int, unsigned int)> zGrad) {
	// uSize��vSize�ֱ��ʾu��v�����ϵ��������
	vertex.resize(uSize * vSize, { glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f) });
	index.resize((uSize - 1) * (vSize - 1) * 6, 0);

	float du = width / (uSize - 1);
	float dv = height / (vSize - 1);
	for (unsigned int u = 0; u < uSize; u++) {
		for (unsigned int v = 0; v < vSize; v++) {
			vertex[u * vSize + v].position = zFunc(u, v);
			vertex[u * vSize + v].normal = zGrad(u, v);
			if (u != uSize - 1 && v != vSize - 1) {
				unsigned int* ptr = &index[(u * (vSize - 1) + v) * 6];		// ��ʱ����ƾ�����Ԫ������������
				ptr[0] = u * vSize + v;							// ���Ͻ�
				ptr[1] = u * vSize + v + 1;					// ���½�	
				ptr[2] = (u + 1) * vSize + v + 1;			// ���½�
				ptr[3] = u * vSize + v;							// ���Ͻ�
				ptr[4] = (u + 1) * vSize + v + 1;			// ���½�
				ptr[5] = (u + 1) * vSize;						// ���Ͻ�
			}
		}
	}
}

#define SAMPLE_NUM 10000

// ���ڴ�����������Ԫ�ĺ����������������㣬�Լ���Դλ�ã��������������Ԫ�Ľ��շ�����
// ���費�����ڵ����ҹ�ԴΪ���Դ
float calculateRadiationByTriangle(const Vertex& a, const Vertex& b, const Vertex& c, const Light& light) {
	glm::vec3 aa(a.position), bb(b.position), cc(c.position);
	// ��ʱ�����Ƕ�������ķ���������������������Ԫ�ķ���������
	// a b cΪ��ʱ��˳�򣬼��㳯�ϵķ�����
	glm::vec3 tmp = glm::cross(bb - aa, cc - aa);
	glm::vec3 N = glm::normalize(tmp);
	// ������Ԫ������
	glm::vec3 pos = (aa + bb + cc) / 3.0f;
	// ���������
	float sumbuf = 0.0f;
	float radiance = light.intensity / (4 * PI * powf(light.radius, 2.0f));
	// �������Դ���л��֣�������Ŀ����ϵķ���ͨ��
	// ��������о��Ȳ���
	float dS = 4.0f * PI * powf(light.radius, 2.0f) / SAMPLE_NUM;
	for (unsigned int i = 0; i < SAMPLE_NUM; i++) {
		float theta = acosf(1.0f - (float)rand() / RAND_MAX);		// acosf(1 - 2 * (float)rand() / RAND_MAX);	����ȡ�ϰ�������theta�ķ�ΧΪ[0, pi/2]
		float phi = 2.0f * PI * (float)rand() / RAND_MAX;
		// // ����һ���Թ�Դ����Ϊԭ�㽨���ֲ����꣬�����up��right��front�����ᣬȻ������L
		//glm::vec3 front = glm::normalize(light.position - pos);
		//glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		//glm::vec3 up = glm::normalize(glm::cross(right, -front));
		//glm::vec3 L = glm::normalize(light.radius * sinf(theta) * cosf(phi) * right + light.radius * sinf(theta) * sinf(phi) * up + light.radius * cosf(theta) * L);
		// // �������������front��_up�ļнǣ����㴹ֱ��front��_up����ƽ�����ת�ᣬ�����ϰ�����ת��front����Ȼ������L
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
	// ����������Ԫ�㹻С����������Դ����Ϊһ�㣬���ֻ�Ըõ���֡����õ��Ծ���΢С������������Ҫ�������
	float tri_area = glm::length(tmp) / 2.0f;
	return sumbuf * tri_area;
}

int main(int argc, char** argv) {
	vector<Vertex> meshVertex;
	vector<unsigned int> meshIdx;

	unsigned int uSize = 40, vSize = 40;

	Light light{
		glm::vec3(3000.0f,1000.0f * sqrtf(3.0f),0.0f),		// ����6000Զ����10����˿��Խ���Ϊ���Դ��������ͨ��������Ӧ��Լ����50
		glm::vec3(0.0f,0.0f,0.0f),
		1.0f,
		4.0f * PI
	};			// ��Դλ�ã���Դ���򣬹�Դ�뾶������ǿ�ȡ� �����Ȳ����ǹ�Դ���򣬲���뾶Ϊ1������ǿ��Ϊ4pi������������radiance��Ϊ1


	generateMesh(10.0f, 10.0f, 40, 40, meshVertex, meshIdx,
		[uSize, vSize](unsigned int u, unsigned int v) {
			return glm::vec3((float)u / (uSize - 1), 0.0f, (float)v / (vSize - 1));		// һ��y=0�ϵ�ƽ�棬��������OpenGL������ϵ�淶
		},
		[uSize, vSize](unsigned int u, unsigned int v) {										// ͳһ���ϵķ�����
			return glm::vec3(0.0f, 1.0f, 0.0f);
		}
		);

	float sumRadiation = 0.0f;

	// �����������еľ�����Ԫ
	for (unsigned int i = 0; i < (uSize - 1) * (vSize - 1); i++) {
		unsigned int* ptr = &meshIdx[i * 6];
		// ����������Ԫ�е�������������Ԫ
		for (unsigned int k = 0; k < 2; k++) {
			float rd = calculateRadiationByTriangle(meshVertex[ptr[k * 3 + 0]], meshVertex[ptr[k * 3 + 1]], meshVertex[ptr[k * 3 + 2]], light);		// ����������Ԫ�������㴫�봦����
			sumRadiation += rd;
		}
		cout << "radiation : " << sumRadiation << endl;
	}


	cout << "radiation : " << sumRadiation << endl;
	return 0;
}

