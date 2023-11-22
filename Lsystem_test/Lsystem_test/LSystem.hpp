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
	// ���ɺ�������iterate����n�Σ����������ַ���
	string generate(unsigned int n) {
		string tmp(this->axiom);
		for (unsigned int i = 0; i < n; i++) {
			tmp = this->iterate(tmp);
		}
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
private:
public:
	PD0LSystem(const string& axiom, LRule& mapper) :LSystem(axiom, mapper) {}
	virtual string iterate(const string& oldStr) {
		stringstream newStr;

		// ����axiom��mapper
		smatch m;
		for (auto pos = oldStr.cbegin(), end = oldStr.cend(); regex_search(pos, end, m, expressionExp); pos = m.suffix().first) {
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
				return keyStr.first.substr(0, 1) == m.str(1);		// ֻ�Ƚϵ�һ���ַ�
				});
			if (mapperElemIter == mapper.end()) {// û���ҵ�ƥ���б�������ֱ�ӷ��룬������
				newStr << elem;
				continue;
			}
			// ȡ��ӳ�䵽��Ŀ����ʽ���������еļ�����򣬼�������������������鵽ԭ���ʽ��
			string srcFormat = mapperElemIter->first;				// "A(x, y)"
			string dstFormat = mapperElemIter->second;		// "B(y, x, x+y)F"
			// ��ȡsrcFormat�е������ַ�x��y����̬����һ�����滻��������ʽ
			vector<string> paramsName;
			// �ٶ�paramNameֻ��1���ַ����ڵ�0λ�ã��ҵ�1���ַ�Ϊ�����ţ��������ڿ�ʼ�ַ�Ϊ��2�������ַ�ΪsrcFormat.length() - 1
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
			// ����paramsName�д�������в��������ƣ�����������"x"��"y"��������һ��map
			map<string, float> paramsMap;
			for (unsigned int i = 0; i < params.size(); i++)
				paramsMap[paramsName[i]] = params[i];
			// ����dstFormat���滻���еĲ���
			stringstream replacedStringStream;
			for (unsigned int i = 0; i < dstFormat.length(); i++) {
				string src = dstFormat.substr(i, 1);
				if (paramsMap.count(src) <= 0) {
					replacedStringStream << src;			// �Ҳ�����ֱ�ӷ��룬�����滻
				}
				else {
					replacedStringStream << to_string(paramsMap[src]);
				}
			}
			//cout << "�ı��滻��" << replacedStringStream.str() << endl;
			// ���滻��ɺ���ַ����еı��ʽ����������ٶ�����ֻʹ�ò������ŵļӼ��˳�
			string stringNotCompute = replacedStringStream.str();
			// stringNotCompute���滻��ı��ʽ�����ܾ��ж�����η��ţ���Ҫ�ٽ���һ�ֱ���
			smatch mm;
			stringstream dstString;		// �����洢���յı��ʽ
			string compStr;
			for (auto spos = stringNotCompute.cbegin(), send = stringNotCompute.cend(); regex_search(spos, send, mm, expressionExp); spos = mm.suffix().first) {
				dstString << mm.str(1);
				// �ж��Ƿ������
				if (!mm.str(2).empty()) {
					dstString << "(";
					// ȡ�����ŷָ��ÿ�����ʽ�����м���
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
					// ��������������
				}
			}
			newStr << dstString.str();
			//cout << "���ձ��ʽ��" << dstString.str() << endl;
			//cout << "===========================================" << endl;
		}
		return newStr.str();
	}
	
	// ��׺���ʽת��Ϊ��׺���ʽ��newExpr Ϊ��׺���ʽ��elemTypeΪÿ��Ԫ�ص����ͣ�0Ϊ���֣�1Ϊ�����
	static inline void expression2RPN(const string& expression, vector<string>& newExpr, vector<ElemType>& elemType) {
		vector<char> operators;		// �����ջ
		newExpr.clear();
		elemType.clear();
		for (unsigned int i = 0; i < expression.size(); i++) {
			char c = expression[i];
			if (c == ' ')		// �����ո�
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
				// ��������������
				// c�ǵ�0��Ԫ�أ�����cǰ���������ţ�����cǰ�������������ôc���Ǹ��š���������ͨ����
				if (c == '-' && (i == 0 || expression[i - 1] == '(' || isOperator(expression[i - 1]))) {
					// ����ǵ�Ŀ���ţ����䴦���滻Ϊ'|'������
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
						// ����������Ŀ���ŵ�����
						if (c == '|' && top == '|') {
							// �Ƴ�operators��topλ�õ�'|'��ͬʱ������ǰc��'|'
							operators.pop_back();
							continue;
						}
						
						// �Ƚ����ȼ�
						char compRes = priorComparaLookupTable[opr2id(c)][opr2id(top)];
						if (compRes <= 1) {
							// ��ǰ���ȼ�����ջ����������ȼ�
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
		// ��operators��ʣ��������ѹ��newExpr
		while (!operators.empty()) {
			char top = operators.back();
			newExpr.push_back(string(1, top));
			elemType.push_back(ElemType::OPERATOR);
			operators.pop_back();
		}
		//cout << "ԭʼ���ʽ��" << endl << expression << endl;
		//cout << "���ɵĺ�׺���ʽ��" << endl << newExpr << endl;
	}
	// �Ժ�׺���ʽ��ֵ
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
	// ת��Ϊ��׺���ʽ����ֵ
	static inline float expressionEvaluate(const string& expression) {
		string expr = expression;
		mstrip(expr);
		vector<string> RPNExpr;
		vector<ElemType> elemType;
		expression2RPN(expr, RPNExpr, elemType);
		float res = evalRPN(RPNExpr, elemType);
		//cout << "���ʽ \"" << expression << "\" �Ľ��Ϊ��" << res << endl;	// ������Ϣ
		return res;
	}
};

