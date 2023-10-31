#include <iostream>
#include <vector>

using namespace std;

int main(void) {

	vector<int> arr1,arr2;
	for (int i = 0; i < 10; i++) {
		arr1.push_back(i);
		arr2.push_back(i+10);
	}
	vector<int> arr;
	arr.insert(arr.end(), arr1.begin(), arr1.end());
	arr.insert(arr.end(), arr2.begin(), arr2.end());
		
	return 0;
}
