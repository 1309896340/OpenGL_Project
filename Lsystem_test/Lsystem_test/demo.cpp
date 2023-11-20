#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

#include "LSystem.hpp"

using namespace std;

// ��text��ƥ�䵽�ķ���"A(x, y)"ģʽ���ı��滻Ϊ"A(?,?)"Ȼ��mapper�ļ���Ѱ�ҿ���ƥ��Ĺ���
// ƥ��ķ����Ǳ���mapper�ļ��������滻�ı��е�?��Ϊͨ���������ģ��ƥ�䣬���ƥ��ɹ���˳�򽫶�Ӧ?λ�õ�ʵ��ֵ��ֵ��mapperֵ�ж�Ӧ�ı���

inline void strip(string& s) {
	if (!s.empty()) {
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}
}

// ���� "8+7*2-9/3"
inline float expressionEvaluate(const string& expression) {
	float res = 0.0f;
	// ��δʵ��

	cout << "���ʽ \"" << expression << "\" �Ľ��Ϊ��" << res << endl;	// ������Ϣ
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
