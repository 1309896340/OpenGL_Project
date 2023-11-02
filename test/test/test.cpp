#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace std;
using namespace cv;


using glm::vec3;
using glm::length;
using glm::cross;
using glm::dot;
using glm::normalize;

bool checkInTriangle(const vec3& point, const vector<vec3>& wPos) {
	float v1 = cross(wPos[1] - wPos[0], point - wPos[0]).z;
	float v2 = cross(wPos[2] - wPos[1], point - wPos[1]).z;
	float v3 = cross(wPos[0] - wPos[2], point - wPos[2]).z;
	if (v1 > 0.0f && v2 > 0.0f && v3 > 0.0f)
		return true;
	if (v1 < 0.0f && v2 < 0.0f && v3 < 0.0f)
		return true;
	return false;
}

int main(void) {
	vector<vec3> arr;
	arr.push_back(vec3(0.0f, 0.0f, 1.0f));
	arr.push_back(vec3(3.0f, 0.0f, 0.0f));
	arr.push_back(vec3(1.5f, 1.0f, -1.0f));

	unsigned int width = 1200, height = 400;
	unsigned int scale = 300;

	namedWindow("demo", WINDOW_NORMAL);
	Mat img(height + 1, width + 1, CV_8UC3, Scalar(0, 0, 0));

	for (unsigned int h = 0; h <= height; h++) {
		for (unsigned int w = 0; w <= width; w++) {
			float tmpH = h / (float)scale;
			float tmpW = w / (float)scale;
			if (checkInTriangle(vec3(tmpW, tmpH, 0.0f), arr)) {
				img.at<Vec3b>(height - h, w) = Vec3b(0, 0, 255);	// 在里面是红色
			}
			else {
				img.at<Vec3b>(height - h, w) = Vec3b(255, 0, 0);	// 在外面是蓝色
			}
		}
	}
	imshow("demo", img);
	waitKey(0);

	return 0;
}
