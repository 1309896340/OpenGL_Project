#include <iostream>
#include <string>
#include <map>
#include <regex>

#include "LSystem.hpp"

using namespace std;

// 将text中匹配到的符合"A(x, y)"模式的文本替换为"A(?,?)"然后到mapper的键中寻找可以匹配的规则
// 匹配的方法是遍历mapper的键，将待替换文本中的?作为通配符，进行模糊匹配，如果匹配成功则按顺序将对应?位置的实际值赋值给mapper值中对应的变量
string vreplace(const string& text) {

}

int main(int argc, char** argv) {
	// https://www.cnblogs.com/coolcpp/p/cpp-regex.html
	string axiom = "A(0.1, 0.5)B(1.6, 0.2, 2.1)C(-0.2, 4.1)D(21.32)";
	regex rgx("([A-Z])\\((.*?)\\)");
	smatch m;
	auto pos = axiom.cbegin();
	auto end = axiom.cend();
	float val;
	for (; regex_search(pos, end, m, rgx); pos = m.suffix().first) {
		string symStr = m.str(1);
		string subStr = m.str(2);
		vector<float> param;
		// 解析subStr中逗号分割开的元素
		size_t startPos = 0;
		for (size_t endPos = subStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = subStr.find(',', startPos)) {
			val = atof(subStr.substr(startPos, endPos - startPos).c_str());
			param.push_back(val);

		}
		val = atof(subStr.substr(startPos).c_str());
		param.push_back(val);

		// 字符串解析完毕
		cout << "符号：" << symStr << endl;
		cout << "参数：";
		for (auto& v : param)
			cout << v << " ";
		cout << endl;
		cout << "=======================" << endl;
	}



	/*

	// 代码调试
	string axiom;
	LRule mapper;
	// ======================
	axiom = "AABAB";
	mapper["A"] = "BA";
	mapper["B"] = "AF";
	D0LSystem lsystem1(axiom, mapper);
	lsystem1.test();
	// ======================
	axiom = "A(0.1, 0.5)B(1.6, 0.2)";
	mapper.clear();
	mapper["A(x, y)"] = "B(0.2*y, 0.5*x)F";
	mapper["B(x, y)"] = "GA(x + y, x - y)";			// 迭代1次的结果应该为 "B(0.25, 0.05)FGA(1.8, 1.4)"
	PD0LSystem lsystem2(axiom, mapper);
	lsystem2.test();

	*/



	return 0;
}
