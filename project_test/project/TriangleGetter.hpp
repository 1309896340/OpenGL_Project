#pragma once
#include "Geometry.hpp"

class TriangleGetter {		// ����Light����Scene������Mesh�Ľӿ�
public:
	virtual vector<Triangle> getAllTriangles() = 0;
};

