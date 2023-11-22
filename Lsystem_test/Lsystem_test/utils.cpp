#include "utils.h"

using namespace std;

// �������Ϊ '+'  '-'  '*'  '/'  '|'  5��
// 0��ʾС�ڣ�1��ʾ���ڣ�2��ʾ����
char priorComparaLookupTable[5][5] = {
	{1, 1, 0 ,0 ,0},
	{1, 1, 0, 0, 0},
	{2, 2, 1, 1, 0},
	{2, 2, 1, 1, 0},
	{2, 2, 2, 2, 1}
};

char operatorNumLookupTable[5] = { 2,2,2,2,1 };


regex expressionExp("([A-Z])(?:\\((.*?)\\))?");			// ƥ��һ�����ɵ�Ԫ������ A(1.0, 3.5, 2.1)��Ҳ���Բ������������� A
regex replaceToFuzzyReg("[+|-]?\\s*[0-9]+(\\.[0-9]+)\\s*");		// ƥ�両�������� -1.0
regex inBracket("\\((.*?)\\)");							// ƥ�������е�Ԫ�飬���� (1.0, 3.5, 2.1)�е� 1.0, 3.5, 2.1

bool isOperator(char c) {
	switch (c) {
	case '+': case '-': case '*': case '/': case '|':
		return true;
		break;
	}
	return false;
}

bool isNumber(char c) {
	if (c >= '0' && c <= '9' || c == '.')		// Ǳ�����⣺���żȿ����������Ҳ���������ֵ�һ����
		return true;
	return false;
}

char opr2id(char c) {
	switch (c) {
	case '+':
		return 0;
	case '-':
		return 1;
	case '*':
		return 2;
	case '/':
		return 3;
	case '|':
		return 4;
	default:
		return -1;		//��Ӧ�ó��ֵ����
	}
}


void mstrip(string& s) {
	if (!s.empty()) {
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}
}

template<typename T>
ostream& operator<<(ostream& out, const vector<T>& data) {
	for (unsigned int i = 0; i < data.size(); i++)
		out << data[i] << " ";
	return out;
}
