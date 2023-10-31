#pragma once
#include "Geometry.hpp"

class TriangleGetter {		// 用于Light访问Scene中所有Mesh的接口
public:
	virtual vector<Triangle> getAllTriangles() = 0;
};

