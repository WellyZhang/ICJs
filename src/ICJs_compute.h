#ifndef ICJS_COMPUTE_H
#define ICJS_COMPUTE_H

#include "ICJs_types.h"
#include <string>
#include <vector>
#include <map>

class Calculator
{
	public:
		static int calculate(std::string &exp, 
			std::map<std::string, Element> &variables, 
			std::vector<Element> &rets,
			std::vector<Element> &output);
		static int isOperator(std::string input);
		static int isFunction(std::string input, std::map<std::string, Element> &variables);
		static int isVar(std::string input, std::map<std::string, Element> &variables);
		static int RPNCalc(std::string input, std::map<std::string, Element> &variables, Element &ret);
		static int priority(std::string opt);
};

#endif ICJS_COMPUTE_H