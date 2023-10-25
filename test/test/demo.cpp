#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::cout;
using std::endl;

typedef struct _Vertex {
	vec3 position;
	vec3 normal;
	vec4 color;
	// 后续可以增加其他属性
}Vertex;


int main(int argc, char** argv) {

	Vertex** ptr;
	ptr = new Vertex * [10] ;
	for (int i = 0; i < 10; i++) {
		cout << (void *)ptr[i] << endl;
	}

	return 0;
}
