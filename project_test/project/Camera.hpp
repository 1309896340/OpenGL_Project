#include "proj.h"
#include "Geometry.hpp"

class Camera : public Geometry {
private:
public:
	Camera() :Geometry(glm::vec3(0.0f, 0.0f, 1.0f)) {
		// �����Ҫһ��ָ��Ŀ�������front������һ��ָ���Ϸ�������up
		// up��������ͨ�����ַ�ʽ�õ�����front������y-axis����(0,1,0)����˵õ�������right,�ٽ�right������front��������˵õ�up����
	
	}
};
