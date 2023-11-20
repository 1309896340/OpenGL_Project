#include <iostream>
#include <string>
#include <map>

using namespace std;

typedef map<string, string> LRule;


void parallelReplace(const string& oldStr, string& newStr, LRule& mapper) {
	// ��Ҫ��oldStr�е��ַ������滻���滻������mapper��
	// �����ȼ���������ַ����ĳ���
	unsigned int newStrLen = 0;
	for (unsigned int i = 0; i < oldStr.size(); i++) {
		newStrLen += (unsigned int)mapper[oldStr.substr(i, 1)].size();
	}
	cout << "���ַ�������Ϊ" << newStrLen << endl;
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
	mapper["B"] = "AF";		// ���Ӧ��Ϊ BA BA AF BA AF
	string newStr;
	parallelReplace(atom, newStr, mapper);
	cout << newStr << endl;

	return 0;
}
