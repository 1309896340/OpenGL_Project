#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

#include "LSystem.hpp"

using namespace std;

typedef enum {
	NUMBER,
	OPERATOR
}ElemType;

// ��text��ƥ�䵽�ķ���"A(x, y)"ģʽ���ı��滻Ϊ"A(?,?)"Ȼ��mapper�ļ���Ѱ�ҿ���ƥ��Ĺ���
// ƥ��ķ����Ǳ���mapper�ļ��������滻�ı��е�?��Ϊͨ���������ģ��ƥ�䣬���ƥ��ɹ���˳�򽫶�Ӧ?λ�õ�ʵ��ֵ��ֵ��mapperֵ�ж�Ӧ�ı���

inline void strip(string& s) {
	if (!s.empty()) {
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}
}

bool isOperator(char c) {
	switch (c) {
	case '+': case '-': case '*': case '/':
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

/*
��׺���ʽת��Ϊ��׺���ʽ
��� newExpr Ϊ��׺���ʽ��elemTypeΪÿ��Ԫ�ص����ͣ�0Ϊ���֣�1Ϊ�����
*/ 
inline void expression2RPN(const string& expression, vector<string>& newExpr, vector<ElemType>& elemType) {
	vector<char> operators;		// �����ջ
	newExpr.clear();
	elemType.clear();
	for (unsigned int i = 0; i < expression.size(); i++) {
		char c = expression[i];
		if (c == ' ')
			continue;
		if (isNumber(c)) {
			unsigned int startPos = i;
			while (isNumber(expression[++i]));
			newExpr.push_back(expression.substr(startPos, i - startPos));
			elemType.push_back(ElemType::NUMBER);
			// ���ִ�ѹջ��i��ǰλ�ø���Ϊɨ�赽����һ���������ַ���Ҳ���������
			i--;	// ����forѭ����β�����������������ȼ�1
		}
		else if (c == '(') {
			//newExpr.push_back(expression.substr(i, 1));
			operators.push_back(c);
		}
		else if (c == ')') {
			char top = operators.back();
			while (top != '(') {
				newExpr.push_back(string(1, top));
				elemType.push_back(ElemType::OPERATOR);
				operators.pop_back();
				top = operators.back();
			}
			operators.pop_back();	// ȡ�������ţ�����ѹ��newExpr
		}
		else if (isOperator(c)) {
			if (operators.empty())
				operators.push_back(c);
			else {
				char top = operators.back();
				if (top == '(')
					operators.push_back(c);
				else {
					if (c == '+' || c == '-') {
						if (top == '*' || top == '/') {
							newExpr.push_back(string(1, top));
							elemType.push_back(ElemType::OPERATOR);
							operators.pop_back();
							operators.push_back(c);
						}
						else {
							operators.push_back(c);
						}
					}
					else if (c == '*' || c == '/') {
						operators.push_back(c);
					}
				}
			}
		}
	}
	// ��operators��ʣ��������ѹ��newExpr
	while (!operators.empty()) {
		char top = operators.back();
		newExpr.push_back(string(1, top));
		elemType.push_back(ElemType::OPERATOR);
		operators.pop_back();
	}
}

/*
�Ժ�׺���ʽ��ֵ
*/
inline float evalRPN(vector<string>& rpn, vector<ElemType> &elemType) {
	vector<float> stack;
	for (unsigned int i = 0; i < rpn.size();i++) {
		ElemType& type = elemType[i];
		if (type == ElemType::NUMBER) {
			stack.push_back((float)atof(rpn[i].c_str()));
		}
		else if (type == ElemType::OPERATOR) {
			float v2 = stack.back();
			stack.pop_back();
			float v1 = stack.back();
			stack.pop_back();
			char opr = rpn[i][0];
			float res = 0.0f;
			switch (opr) {
			case '+':
				res = v1 + v2;
				break;
			case '-':
				res = v1 - v2;
				break;
			case '*':
				res = v1 * v2;
				break;
			case '/':
				res = v1 / v2;
				break;
			}
			stack.push_back(res);
		}
	}
	return stack[0];
}


inline float expressionEvaluate(const string& expression) {
	vector<string> RPNExpr;
	vector<ElemType> elemType;
	expression2RPN(expression, RPNExpr, elemType);
	float res = evalRPN(RPNExpr, elemType);
	cout << "���ʽ \"" << expression << "\" �Ľ��Ϊ��" << res << endl;	// ������Ϣ
	return res;
}

template<typename T>
ostream& operator<<(ostream& out, const vector<T>& data) {
	for (unsigned int i = 0; i < data.size(); i++)
		out << data[i] << " ";
	return out;
}

int main(int argc, char** argv) {
	vector<string> rpnElem;
	vector<ElemType> elemType;
	float res;
	//expression2RPN("(1.0 + 2.4)*5.6", rpnElem, elemType);
	//res = evalRPN(rpnElem, elemType);
	//cout << rpnElem << endl;
	//cout << "res = " << res << endl;
	//cout << "======================" << endl;
	//expression2RPN("1.0 + 2.4 * 5.6", rpnElem, elemType);
	//cout << rpnElem << endl;
	//res = evalRPN(rpnElem, elemType);
	//cout << "res = " << res << endl;
	//cout << "======================" << endl;
	//expression2RPN("((2.41+3.93)*5.64+6.12)*2.32-5.21", rpnElem, elemType);
	//cout << rpnElem << endl;
	//res = evalRPN(rpnElem, elemType);
	//cout << "res = " << res << endl;
	//cout << "======================" << endl;


	/*
	����׺���ʽ�д��ڵ��������ĸ���ʱ�����Խ���ת��Ϊ��0��ȥ��������˫��������ʽ���罫"3-"����"03-"������ͳһ�˲�����Ϊ2
	�ټ�������
	2*(-3)				ת��Ϊ		2 0 3 - *
	2*(-3+5)		ת��Ϊ		2 0 3 - 5 + *
	-2*4-5			ת��Ϊ		0 2 - 4 * 5 -
	-2*(-4+5)		ת��Ϊ		0 2 - 0 4 - 5 + *
	�����һ�����⣺����жϸ����ǵ��������ĸ��Ż���˫�������ļ���
	����stackoverflow�û��Ļش� https://stackoverflow.com/questions/46861254/infix-to-postfix-for-negative-numbers 

	*/
	expression2RPN("(-1.0 + 2.4)*(-5.6)", rpnElem, elemType);
	cout << rpnElem << endl;
	res = evalRPN(rpnElem, elemType);
	cout << "res = " << res << endl;
	cout << "======================" << endl;

	// https://www.cnblogs.com/coolcpp/p/cpp-regex.html
	string axiom = "A(0.1, 0.5)B(1.6, 0.2, 2.1)C(-0.2, 4.1)D(21.32)";
	//LRule mapper = {
	//	{"A(x, y)", "B(0.2*y, 0.5*x)F"},
	//	{"B(x, y, z)", "GA(x + y, x - y + 0.5*z)F"},
	//	{"C(x, y)", "B(0.2*y, 0.5*x)C(0.1*x, 0.5*y)"},
	//	{"D(x)", "C(0.5*y, 0.2*x)F"}
	//};
	LRule mapper = {
		{"A(x, y)", "B(0.2*y, 0.5*x)"},
		{"B(x, y, z)", "A(x + y, x - y + 0.5*z)"},
		{"C(x, y)", "B(0.2*y, 0.5*x)C(0.1*x, 0.5*y)"},
		{"D(x)", "C(0.5, 0.2*x)"}
	};

	regex expressionExp("([A-Z])\\((.*?)\\)");
	regex replaceToFuzzyReg("[+|-]?\\s*[0-9]+(\\.[0-9]+)\\s*");		// ƥ�両����
	regex inBracket("\\((.*?)\\)");
	smatch m;
	for (auto pos = axiom.cbegin(), end = axiom.cend(); regex_search(pos, end, m, expressionExp); pos = m.suffix().first) {
		string elem = m.str(0);
		// ��˳��ȡ�����в���
		vector<float> params;
		string paramStr = m.str(2);
		size_t startPos = 0;
		for (size_t endPos = paramStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = paramStr.find(',', startPos)) {
			params.push_back((float)atof(paramStr.substr(startPos, endPos - startPos).c_str()));
		}
		params.push_back((float)atof(paramStr.substr(startPos).c_str()));
		// ��mapper��Ѱ��ƥ��Ĺ���
		auto mapperElemIter = find_if(mapper.begin(), mapper.end(), [&m](pair<string, string> keyStr) {
			return keyStr.first.substr(0, 1) == m.str(1);
			});
		if (mapperElemIter == mapper.end())	// û���ҵ�ƥ���б�����
			continue;
		// ȡ��ӳ�䵽��Ŀ����ʽ���������еļ�����򣬼�������������������鵽ԭ���ʽ��
		string srcFormat = mapperElemIter->first;				// "A(x, y)"
		string dstFormat = mapperElemIter->second;		// "B(0.2*y, 0.5*x)"
		// ��ȡsrcFormat�е������ַ�x��y����̬����һ�����滻��������ʽ
		vector<string> paramsName;
		// �ٶ�paramNameֻ��1���ַ����ڵ�0λ�ã��ҵ�1���ַ�Ϊ�����ţ��������ڿ�ʼ�ַ�Ϊ��2�������ַ�ΪsrcFormat.length() - 1
		string subSrcFormat = srcFormat.substr(2, srcFormat.length() - 3);
		startPos = 0;
		for (size_t endPos = subSrcFormat.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = subSrcFormat.find(',', startPos)) {
			string pname = subSrcFormat.substr(startPos, endPos - startPos);
			strip(pname);
			paramsName.push_back(pname.substr(0, 1));
		}
		string pname = subSrcFormat.substr(startPos);
		strip(pname);
		paramsName.push_back(pname.substr(0, 1));
		// ����paramsName�д�������в��������ƣ�����������"x"��"y"��������һ��map
		map<string, float> paramsMap;
		for (unsigned int i = 0; i < params.size(); i++)
			paramsMap[paramsName[i]] = params[i];
		// ����dstFormat���滻���еĲ���
		stringstream replacedStringStream;
		for (unsigned int i = 0; i < dstFormat.length(); i++) {
			string src = dstFormat.substr(i, 1);
			if (paramsMap.count(src) <= 0) {
				replacedStringStream << src;
			}
			else {
				replacedStringStream << to_string(paramsMap[src]);
			}
		}
		cout << "�滻�굫δ����ı��ʽ��" << replacedStringStream.str() << endl;
		// ���滻��ɺ���ַ����еı��ʽ����������ٶ�����ֻʹ�ò������ŵļӼ��˳�
		string stringNotCompute = replacedStringStream.str();
		// stringNotCompute���滻��ı��ʽ�����ܾ��ж�����η��ţ���Ҫ�ٽ���һ�ֱ���
		smatch mm;
		stringstream dstString;		// �����洢���յı��ʽ
		string compStr;
		for (auto spos = stringNotCompute.cbegin(), send = stringNotCompute.cend(); regex_search(spos, send, mm, expressionExp); spos = mm.suffix().first) {
			dstString << mm.str(1) << "(";
			// ȡ�����ŷָ��ÿ�����ʽ�����м���
			startPos = 0;
			string tmpStr = mm.str(2);
			for (size_t endPos = tmpStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = tmpStr.find(',', startPos)) {
				compStr = tmpStr.substr(startPos, endPos - startPos);
				dstString << to_string(expressionEvaluate(compStr)) << ",";
			}
			compStr = tmpStr.substr(startPos);
			dstString << to_string(expressionEvaluate(compStr)) << ")";
		}
		cout << "���ձ��ʽ��" << dstString.str() << endl;
		cout << "===========================================" << endl;
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
