#include "proj.h"
#include "Geometry.hpp"

class Camera : public Geometry {
private:
public:
	Camera() :Geometry(glm::vec3(0.0f, 0.0f, 1.0f)) {
		// 相机需要一个指向目标的向量front，还有一个指向上方的向量up
		// up向量可以通过这种方式得到：先front向量与y-axis向量(0,1,0)做叉乘得到右向量right,再将right向量与front向量做叉乘得到up向量
	
	}
};
