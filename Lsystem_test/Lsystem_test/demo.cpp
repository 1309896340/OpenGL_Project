#include <iostream>
#include <string>
#include <map>
#include <regex>

#include "LSystem.hpp"

using namespace std;

// ��text��ƥ�䵽�ķ���"A(x, y)"ģʽ���ı��滻Ϊ"A(?,?)"Ȼ��mapper�ļ���Ѱ�ҿ���ƥ��Ĺ���
// ƥ��ķ����Ǳ���mapper�ļ��������滻�ı��е�?��Ϊͨ���������ģ��ƥ�䣬���ƥ��ɹ���˳�򽫶�Ӧ?λ�õ�ʵ��ֵ��ֵ��mapperֵ�ж�Ӧ�ı���
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
		// ����subStr�ж��ŷָ��Ԫ��
		size_t startPos = 0;
		for (size_t endPos = subStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = subStr.find(',', startPos)) {
			val = atof(subStr.substr(startPos, endPos - startPos).c_str());
			param.push_back(val);

		}
		val = atof(subStr.substr(startPos).c_str());
		param.push_back(val);

		// �ַ����������
		cout << "���ţ�" << symStr << endl;
		cout << "������";
		for (auto& v : param)
			cout << v << " ";
		cout << endl;
		cout << "=======================" << endl;
	}



	/*

	// �������
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
	mapper["B(x, y)"] = "GA(x + y, x - y)";			// ����1�εĽ��Ӧ��Ϊ "B(0.25, 0.05)FGA(1.8, 1.4)"
	PD0LSystem lsystem2(axiom, mapper);
	lsystem2.test();

	*/



	return 0;
}
