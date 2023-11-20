#include <iostream>
#include <string>
#include <map>

using namespace std;

typedef map<string, string> LRule;


void parallelReplace(const string& oldStr, string& newStr, LRule& mapper) {
	// 需要将oldStr中的字符并行替换，替换规则在mapper中
	// 考虑先计算出最终字符串的长度
	unsigned int newStrLen = 0;
	for (unsigned int i = 0; i < oldStr.size(); i++) {
		newStrLen += (unsigned int)mapper[oldStr.substr(i, 1)].size();
	}
	cout << "新字符串长度为" << newStrLen << endl;
	newStr.resize(newStrLen);
	unsigned int ptr = 0;
	for (unsigned int i = 0; i < oldStr.size(); i++) {
		string tmp = mapper[oldStr.substr(i, 1)];
		for (unsigned int j = 0; j < tmp.size(); j++)
			newStr[ptr++] = tmp[j];
	}
}


int main(int argc, char** argv) {
	string atom = "AABAB";
	LRule mapper;
	mapper["A"] = "BA";
	mapper["B"] = "AF";		// 结果应该为 BA BA AF BA AF
	string newStr;
	parallelReplace(atom, newStr, mapper);
	cout << newStr << endl;

	return 0;
}
