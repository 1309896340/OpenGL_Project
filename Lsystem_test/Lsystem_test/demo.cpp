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

// 将text中匹配到的符合"A(x, y)"模式的文本替换为"A(?,?)"然后到mapper的键中寻找可以匹配的规则
// 匹配的方法是遍历mapper的键，将待替换文本中的?作为通配符，进行模糊匹配，如果匹配成功则按顺序将对应?位置的实际值赋值给mapper值中对应的变量

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
	if (c >= '0' && c <= '9' || c == '.')		// 潜在问题：负号既可以是运算符也可以是数字的一部分
		return true;
	return false;
}

/*
中缀表达式转换为后缀表达式
输出 newExpr 为后缀表达式，elemType为每个元素的类型，0为数字，1为运算符
*/ 
inline void expression2RPN(const string& expression, vector<string>& newExpr, vector<ElemType>& elemType) {
	vector<char> operators;		// 运算符栈
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
			// 数字串压栈，i当前位置更新为扫描到的下一个非数字字符，也就是运算符
			i--;	// 由于for循环结尾会自增，所以这里先减1
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
			operators.pop_back();	// 取出左括号，但不压入newExpr
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
	// 将operators中剩余的运算符压入newExpr
	while (!operators.empty()) {
		char top = operators.back();
		newExpr.push_back(string(1, top));
		elemType.push_back(ElemType::OPERATOR);
		operators.pop_back();
	}
}

/*
对后缀表达式求值
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
	cout << "表达式 \"" << expression << "\" 的结果为：" << res << endl;	// 调试信息
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
	当中缀表达式中存在单操作数的负号时，可以将其转换为用0减去操作数的双操作数形式，如将"3-"换成"03-"，这样统一了操作数为2
	举几个例子
	2*(-3)				转换为		2 0 3 - *
	2*(-3+5)		转换为		2 0 3 - 5 + *
	-2*4-5			转换为		0 2 - 4 * 5 -
	-2*(-4+5)		转换为		0 2 - 0 4 - 5 + *
	另外的一个问题：如何判断负号是单操作数的负号还是双操作数的减号
	根据stackoverflow用户的回答 https://stackoverflow.com/questions/46861254/infix-to-postfix-for-negative-numbers 

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
	regex replaceToFuzzyReg("[+|-]?\\s*[0-9]+(\\.[0-9]+)\\s*");		// 匹配浮点数
	regex inBracket("\\((.*?)\\)");
	smatch m;
	for (auto pos = axiom.cbegin(), end = axiom.cend(); regex_search(pos, end, m, expressionExp); pos = m.suffix().first) {
		string elem = m.str(0);
		// 按顺序取出所有参数
		vector<float> params;
		string paramStr = m.str(2);
		size_t startPos = 0;
		for (size_t endPos = paramStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = paramStr.find(',', startPos)) {
			params.push_back((float)atof(paramStr.substr(startPos, endPos - startPos).c_str()));
		}
		params.push_back((float)atof(paramStr.substr(startPos).c_str()));
		// 在mapper中寻找匹配的规则
		auto mapperElemIter = find_if(mapper.begin(), mapper.end(), [&m](pair<string, string> keyStr) {
			return keyStr.first.substr(0, 1) == m.str(1);
			});
		if (mapperElemIter == mapper.end())	// 没有找到匹配列表，跳过
			continue;
		// 取出映射到的目标表达式，根据其中的计算规则，计算出结果，并将结果重组到原表达式中
		string srcFormat = mapperElemIter->first;				// "A(x, y)"
		string dstFormat = mapperElemIter->second;		// "B(0.2*y, 0.5*x)"
		// 提取srcFormat中的两个字符x和y，动态生成一用于替换的正则表达式
		vector<string> paramsName;
		// 假定paramName只有1个字符，在第0位置，且第1个字符为左括号，则括号内开始字符为第2，结束字符为srcFormat.length() - 1
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
		// 现在paramsName中存放了所有参数的名称（例如这里是"x"和"y"），构造一个map
		map<string, float> paramsMap;
		for (unsigned int i = 0; i < params.size(); i++)
			paramsMap[paramsName[i]] = params[i];
		// 遍历dstFormat并替换其中的参数
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
		cout << "替换完但未计算的表达式：" << replacedStringStream.str() << endl;
		// 将替换完成后的字符串中的表达式计算出来，假定这里只使用不带括号的加减乘除
		string stringNotCompute = replacedStringStream.str();
		// stringNotCompute是替换后的表达式，可能具有多个带参符号，需要再进行一轮遍历
		smatch mm;
		stringstream dstString;		// 用来存储最终的表达式
		string compStr;
		for (auto spos = stringNotCompute.cbegin(), send = stringNotCompute.cend(); regex_search(spos, send, mm, expressionExp); spos = mm.suffix().first) {
			dstString << mm.str(1) << "(";
			// 取出逗号分割的每个表达式并进行计算
			startPos = 0;
			string tmpStr = mm.str(2);
			for (size_t endPos = tmpStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = tmpStr.find(',', startPos)) {
				compStr = tmpStr.substr(startPos, endPos - startPos);
				dstString << to_string(expressionEvaluate(compStr)) << ",";
			}
			compStr = tmpStr.substr(startPos);
			dstString << to_string(expressionEvaluate(compStr)) << ")";
		}
		cout << "最终表达式：" << dstString.str() << endl;
		cout << "===========================================" << endl;
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
