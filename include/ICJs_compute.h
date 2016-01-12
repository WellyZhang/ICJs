#ifndef ICJS_COMPUTE_H
#define ICJS_COMPUTE_H

#include "ICJs_types.h"
#include "ICJs_parser.h"
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
		static int isLogic(std::vector<std::string> inputs);
		static int isComma(std::string input);

};

#endif ICJS_COMPUTE_H