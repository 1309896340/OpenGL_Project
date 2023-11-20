#pragma once
#include <iostream>
#include <string>
#include <map>

#include <cctype>

using namespace std;
typedef map<string, string> LRule;

class LSystem {
private:
	// �����ȹ涨26����д��ĸΪ����Ԫ��
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
	// ���ɺ�������iterate����n�Σ����������ַ���
	string generate(unsigned int n) {
		string tmp(this->axiom);
		for (unsigned int i = 0; i < n; i++)
			tmp = this->iterate(tmp);
		return tmp;
	}
	// ��������ͨ��oldStr����Ӧ��mapper���newStr
	virtual string iterate(const string& oldStr) = 0;

	// ���ڲ���
	void test() {
		cout << "Axiom: " << this->axiom << endl;
		cout << "Rules: " << endl;;
		for (auto& elem : this->mapper)
			cout << "\t" << elem.first << " -> " << elem.second << endl;
		for (unsigned int i = 1; i <= 3; i++) {
			cout << "��" << i << "�ε��������" << endl;
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
			if (tmp == "")			// ����Ҳ�������������
				tmp = oldStr.substr(i, 1);
			for (unsigned int j = 0; j < tmp.size(); j++)
				newStr[ptr++] = tmp[j];
		}
		return newStr;
	}
};

class PD0LSystem :public LSystem {		// ��������D0Lϵͳ
public:
	PD0LSystem(const string& axiom, LRule& mapper) :LSystem(axiom, mapper) {}
	virtual string iterate(const string& oldStr) {
		string newStr;
		
		return newStr;
	}
};

