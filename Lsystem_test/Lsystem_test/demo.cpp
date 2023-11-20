#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

#include "LSystem.hpp"

using namespace std;

// 将text中匹配到的符合"A(x, y)"模式的文本替换为"A(?,?)"然后到mapper的键中寻找可以匹配的规则
// 匹配的方法是遍历mapper的键，将待替换文本中的?作为通配符，进行模糊匹配，如果匹配成功则按顺序将对应?位置的实际值赋值给mapper值中对应的变量

inline void strip(string& s) {
	if (!s.empty()) {
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}
}

// 比如 "8+7*2-9/3"
inline float expressionEvaluate(const string& expression) {
	float res = 0.0f;
	// 暂未实现

	cout << "表达式 \"" << expression << "\" 的结果为：" << res << endl;	// 调试信息
	return res;
}

int main(int argc, char** argv) {

	expressionEvaluate("3.01 +  21.412");
	expressionEvaluate("3.0 *  -4.2");
	expressionEvaluate("1.2 + 3.0 *  -4.2");


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
