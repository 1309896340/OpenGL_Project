#pragma once
#include "Geometry.hpp"

class TriangleGetter {		// ����Light����Scene������Mesh�Ľӿ�
public:
	virtual void getAllTriangles(vector<Triangle>& outTrianglesVector) = 0;
};

