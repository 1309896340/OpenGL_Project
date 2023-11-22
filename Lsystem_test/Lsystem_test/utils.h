#pragma once

//#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <regex>

typedef enum {
	NUMBER,
	OPERATOR
}ElemType;

char priorComparaLookupTable[][5];
char operatorNumLookupTable[];
extern std::regex expressionExp, replaceToFuzzyReg, inBracket;

//void strip(string& s);
bool isOperator(char c);
bool isNumber(char c);
char opr2id(char c);
void mstrip(std::string& s);

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& data);
