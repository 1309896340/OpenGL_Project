#pragma once
#include <iostream>
#include <string>
#include <map>

#include <cctype>

using namespace std;
typedef map<string, string> LRule;

class LSystem {
private:
	// 暂且先规定26个大写字母为生成元素
	//bool isElement(const string& str) {
	//	if (std::isupper(str[0]))
	//		return true;
	//	return false;
	//}
protected:
	string axiom;
	LRule mapper;
public:
	LSystem(const string& axiom, LRule& mapper) {
		this->axiom = axiom;
		this->mapper = mapper;
	}
	// 生成函数调用iterate函数n次，返回最终字符串
	string generate(unsigned int n) {
		string tmp(this->axiom);
		for (unsigned int i = 0; i < n; i++)
			tmp = this->iterate(tmp);
		return tmp;
	}
	// 迭代函数通过oldStr返回应用mapper后的newStr
	virtual string iterate(const string& oldStr) = 0;

	// 用于测试
	void test() {
		cout << "Axiom: " << this->axiom << endl;
		cout << "Rules: " << endl;;
		for (auto& elem : this->mapper)
			cout << "\t" << elem.first << " -> " << elem.second << endl;
		for (unsigned int i = 1; i <= 3; i++) {
			cout << "第" << i << "次迭代结果：" << endl;
			cout << "\t" << this->generate(i) << endl;
		}
	}
};


class D0LSystem :public LSystem {
public:
	D0LSystem(const string& axiom, LRule& mapper) :LSystem(axiom, mapper) {}
	virtual string iterate(const string& oldStr) {
		string newStr;
		unsigned int newStrLen = 0;
		for (unsigned int i = 0; i < oldStr.size(); i++) {
			string tmp = mapper[oldStr.substr(i, 1)];
			if (tmp == "")
				newStrLen++;
			else
				newStrLen += (unsigned int)tmp.size();
		}
		newStr.resize(newStrLen);
		unsigned int ptr = 0;
		for (unsigned int i = 0; i < oldStr.size(); i++) {
			string tmp = mapper[oldStr.substr(i, 1)];
			if (tmp == "")			// 如果找不到，则保留下来
				tmp = oldStr.substr(i, 1);
			for (unsigned int j = 0; j < tmp.size(); j++)
				newStr[ptr++] = tmp[j];
		}
		return newStr;
	}
};

class PD0LSystem :public LSystem {		// 带参数的D0L系统
public:
	PD0LSystem(const string& axiom, LRule& mapper) :LSystem(axiom, mapper) {}
	virtual string iterate(const string& oldStr) {
		string newStr;
		
		return newStr;
	}
};

