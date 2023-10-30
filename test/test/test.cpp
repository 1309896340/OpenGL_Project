#include <iostream>
#include <vector>

using namespace std;

typedef struct _vec3 {
	float x;
	float y;
	float z;
}vec3;

class Mesh {
	vec3 a{ 0.0f,1.0f,2.0f };
	float b{ 10.0f };
public:
	Mesh(vec3 a, float b) :a(a), b(b) {}
};

vector<Mesh> func() {
	vector<Mesh> a;
	a.push_back(Mesh({ 11.0f,12.0f,13.0f }, 123.0f));
	a.push_back(Mesh({ 21.0f,22.0f,23.0f }, 313.0f));
	a.push_back(Mesh({ 31.0f,32.0f,33.0f }, 513.0f));
	a.push_back(Mesh({ 31.0f,32.0f,33.0f }, 513.0f));
	a.push_back(Mesh({ 31.0f,32.0f,33.0f }, 513.0f));

	return a;
}

int main(void) {

	vector<Mesh> a = func();

	cout << "a size: " << sizeof(a) << endl;
	cout << "Mesh size: " << sizeof(Mesh({ 0.0f,0.0f,0.0f }, 0.0f)) << endl;

	return 0;
}
