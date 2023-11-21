#include <iostream>
#include <string>
#include <vector>

#include "LSystem.hpp"

using namespace std;


int main(int argc, char** argv) {



	// https://www.cnblogs.com/coolcpp/p/cpp-regex.html
	string axiom = "A(0.1, 0.5)B(1.6, 0.2, 2.1)C(-0.2, 4.1)D(21.32)";
	LRule mapper = {
		{"A(x, y)", "B(0.2*y, 0.5*x, x+y)"},
		{"B(x, y, z)", "A(x + y, x - y + 0.5*z)"},
		{"C(x, y)", "B(0.2*x, 0.5*y, x)C(0.1*x, 0.5*y)"},
		{"D(x)", "C(0.5, 0.2*x)"}
	};

	PD0LSystem lsystem(axiom, mapper);
	lsystem.test();

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
