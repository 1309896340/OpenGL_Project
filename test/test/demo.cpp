#include <iostream>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::cout;
using std::endl;
using std::vector;

typedef struct _Vertex {
	vec3 position;
	vec3 normal;
	vec4 color;
}Vertex;


int main(int argc, char** argv) {

	Vertex arr[2]{
		{vec3(1.0f,2.0f,3.0f),vec3(4.0f,5.0f,6.0f),vec4(121.0f,8.0f,9.0f,10.0f)},
		{vec3(31.0f,12.0f,13.0f),vec3(54.0f,15.0f,123.0f),vec4(17.0f,18.0f,19.0f,23.0f)}
	};

	float* ptr = (float*)arr;
	for (unsigned int i = 0; i < 20; i++) {
		cout << ptr[i] << "  ";
	}
	cout << endl;

	return 0;
}
