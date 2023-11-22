#include "utils.h"

using namespace std;

// 运算符分为 '+'  '-'  '*'  '/'  '|'  5种
// 0表示小于，1表示等于，2表示大于
char priorComparaLookupTable[5][5] = {
	{1, 1, 0 ,0 ,0},
	{1, 1, 0, 0, 0},
	{2, 2, 1, 1, 0},
	{2, 2, 1, 1, 0},
	{2, 2, 2, 2, 1}
};

char operatorNumLookupTable[5] = { 2,2,2,2,1 };


regex expressionExp("([A-Z])(?:\\((.*?)\\))?");			// 匹配一个生成单元，例如 A(1.0, 3.5, 2.1)，也可以不带参数，例如 A
regex replaceToFuzzyReg("[+|-]?\\s*[0-9]+(\\.[0-9]+)\\s*");		// 匹配浮点数，如 -1.0
regex inBracket("\\((.*?)\\)");							// 匹配括号中的元组，例如 (1.0, 3.5, 2.1)中的 1.0, 3.5, 2.1

bool isOperator(char c) {
	switch (c) {
	case '+': case '-': case '*': case '/': case '|':
		return true;
		break;
	}
	return false;
}

bool isNumber(char c) {
	if (c >= '0' && c <= '9' || c == '.')		// 潜在问题：负号既可以是运算符也可以是数字的一部分
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
		return -1;		//不应该出现的情况
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
