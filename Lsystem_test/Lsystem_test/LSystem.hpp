#pragma once
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <cctype>

#include "utils.h"

using namespace std;
typedef map<string, string> LRule;

class LSystem {
private:
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
		for (unsigned int i = 0; i < n; i++) {
			tmp = this->iterate(tmp);
		}
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
private:
public:
	PD0LSystem(const string& axiom, LRule& mapper) :LSystem(axiom, mapper) {}
	virtual string iterate(const string& oldStr) {
		stringstream newStr;

		// 输入axiom和mapper
		smatch m;
		for (auto pos = oldStr.cbegin(), end = oldStr.cend(); regex_search(pos, end, m, expressionExp); pos = m.suffix().first) {
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
				return keyStr.first.substr(0, 1) == m.str(1);		// 只比较第一个字符
				});
			if (mapperElemIter == mapper.end()) {// 没有找到匹配列表，将符号直接放入，并跳过
				newStr << elem;
				continue;
			}
			// 取出映射到的目标表达式，根据其中的计算规则，计算出结果，并将结果重组到原表达式中
			string srcFormat = mapperElemIter->first;				// "A(x, y)"
			string dstFormat = mapperElemIter->second;		// "B(y, x, x+y)F"
			// 提取srcFormat中的两个字符x和y，动态生成一用于替换的正则表达式
			vector<string> paramsName;
			// 假定paramName只有1个字符，在第0位置，且第1个字符为左括号，则括号内开始字符为第2，结束字符为srcFormat.length() - 1
			string subSrcFormat = srcFormat.substr(2, srcFormat.length() - 3);
			startPos = 0;
			for (size_t endPos = subSrcFormat.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = subSrcFormat.find(',', startPos)) {
				string pname = subSrcFormat.substr(startPos, endPos - startPos);
				mstrip(pname);
				paramsName.push_back(pname.substr(0, 1));
			}
			string pname = subSrcFormat.substr(startPos);
			mstrip(pname);
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
					replacedStringStream << src;			// 找不到就直接放入，不做替换
				}
				else {
					replacedStringStream << to_string(paramsMap[src]);
				}
			}
			//cout << "文本替换后：" << replacedStringStream.str() << endl;
			// 将替换完成后的字符串中的表达式计算出来，假定这里只使用不带括号的加减乘除
			string stringNotCompute = replacedStringStream.str();
			// stringNotCompute是替换后的表达式，可能具有多个带参符号，需要再进行一轮遍历
			smatch mm;
			stringstream dstString;		// 用来存储最终的表达式
			string compStr;
			for (auto spos = stringNotCompute.cbegin(), send = stringNotCompute.cend(); regex_search(spos, send, mm, expressionExp); spos = mm.suffix().first) {
				dstString << mm.str(1);
				// 判断是否带参数
				if (!mm.str(2).empty()) {
					dstString << "(";
					// 取出逗号分割的每个表达式并进行计算
					startPos = 0;
					string tmpStr = mm.str(2);
					for (size_t endPos = tmpStr.find(',', startPos); endPos != string::npos; startPos = endPos + 1, endPos = tmpStr.find(',', startPos)) {
						compStr = tmpStr.substr(startPos, endPos - startPos);
						mstrip(compStr);
						dstString << to_string(expressionEvaluate(compStr)) << ",";
					}
					compStr = tmpStr.substr(startPos);
					mstrip(compStr);
					dstString << to_string(expressionEvaluate(compStr)) << ")";
				}
				else{
					// 不带参数，留空
				}
			}
			newStr << dstString.str();
			//cout << "最终表达式：" << dstString.str() << endl;
			//cout << "===========================================" << endl;
		}
		return newStr.str();
	}
	
	// 中缀表达式转换为后缀表达式，newExpr 为后缀表达式，elemType为每个元素的类型，0为数字，1为运算符
	static inline void expression2RPN(const string& expression, vector<string>& newExpr, vector<ElemType>& elemType) {
		vector<char> operators;		// 运算符栈
		newExpr.clear();
		elemType.clear();
		for (unsigned int i = 0; i < expression.size(); i++) {
			char c = expression[i];
			if (c == ' ')		// 跳过空格
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
				// 处理负号歧义问题
				// c是第0个元素，或者c前面是左括号，或者c前面是运算符，那么c就是负号。否则是普通减号
				if (c == '-' && (i == 0 || expression[i - 1] == '(' || isOperator(expression[i - 1]))) {
					// 如果是单目负号，将其处理替换为'|'以区分
					c = '|';
				}
				// =========================
				if (operators.empty())
					operators.push_back(c);
				else {
					char top = operators.back();
					if (top == '(')
						operators.push_back(c);
					else {
						// 处理连续单目负号的问题
						if (c == '|' && top == '|') {
							// 移除operators中top位置的'|'，同时不处理当前c的'|'
							operators.pop_back();
							continue;
						}
						
						// 比较优先级
						char compRes = priorComparaLookupTable[opr2id(c)][opr2id(top)];
						if (compRes <= 1) {
							// 当前优先级低于栈顶运算符优先级
							newExpr.push_back(string(1, top));
							elemType.push_back(ElemType::OPERATOR);
							operators.pop_back();
							operators.push_back(c);
						}
						else if (compRes == 2) {
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
		//cout << "原始表达式：" << endl << expression << endl;
		//cout << "生成的后缀表达式：" << endl << newExpr << endl;
	}
	// 对后缀表达式求值
	static inline float evalRPN(vector<string>& rpn, vector<ElemType>& elemType) {
		vector<float> stack;
		for (unsigned int i = 0; i < rpn.size(); i++) {
			ElemType& type = elemType[i];
			if (type == ElemType::NUMBER) {
				stack.push_back((float)atof(rpn[i].c_str()));
			}
			else if (type == ElemType::OPERATOR) {
				float params[2];
				char opr = rpn[i][0];
				float res;
				for (unsigned int k = 0; k < (unsigned int)operatorNumLookupTable[opr2id(opr)]; k++) {
					params[k] = stack.back();
					stack.pop_back();
				}
				switch (opr) {
				case '+':
					res = params[1] + params[0];
					break;
				case '-':
					res = params[1] - params[0];
					break;
				case '*':
					res = params[1] * params[0];
					break;
				case '/':
					res = params[1] / params[0];
					break;
				case '|':
					res = -params[0];
					break;
				default:
					assert(0);
					break;
				}
				stack.push_back(res);
			}
		}
		return stack[0];
	}
	// 转换为后缀表达式并求值
	static inline float expressionEvaluate(const string& expression) {
		string expr = expression;
		mstrip(expr);
		vector<string> RPNExpr;
		vector<ElemType> elemType;
		expression2RPN(expr, RPNExpr, elemType);
		float res = evalRPN(RPNExpr, elemType);
		//cout << "表达式 \"" << expression << "\" 的结果为：" << res << endl;	// 调试信息
		return res;
	}
};

