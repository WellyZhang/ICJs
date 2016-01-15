#ifndef ICJS_COMPUTE_H
#define ICJS_COMPUTE_H

#include "ICJs_types.h"
#include "ICJs_parser.h"
#include <string>
#include <vector>
#include <map>

/*
 * Calculator类
 * 用于表达式计算
 * 支持数组，字符串，布尔，数字，函数与递归
 */

class Calculator
{
	public:
		static int calculate(std::string &exp, 
			std::map<std::string, Element> &variables, 
			std::vector<Element> &rets,
			std::vector<Element> &output);
		static int numeric(std::string &exp,
			std::map<std::string, Element> &variables,
			std::vector<Element> &rets,
			std::vector<Element> &output);
		static int isOperator(std::string input);
		static int isFunction(std::string input, std::map<std::string, Element> &variables);
		static int isNumVar(std::string input, std::map<std::string, Element> &variables);
		static int RPNCalc(std::string input, std::map<std::string, Element> &variables, Element &ret);
		static int priority(std::string opt);
		static int isStringVar(std::string input, std::map<std::string, Element> &variables);
		static int isLogic(std::vector<std::string> inputs, std::map<std::string, Element> vars);
		static int isComma(std::string input);
		static int isLogicOperator(std::string input);
		static int isArrayVar(std::string input, std::map<std::string, Element> &variables);

};

#endif ICJS_COMPUTE_H