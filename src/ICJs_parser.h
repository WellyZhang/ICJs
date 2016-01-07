#ifndef ICJS_PARSER_H
#define ICJS_PARSER_H

#include "ICJS_types.h"
#include <string>
#include <vector>
#include <map>

class Parser
{
	public:
		Parser();
		int parse(std::vector<std::string> &input, 
			std::map<std::string, Element> &variables, 
			std::vector<Element> &output);
		int run_func(Function &func,
			std::map<std::string, Element> &variables,
			std::vector<Element> parameters,
			Element &ret, 
			std::vector<Element> &output);
		~Parser();
};

#endif ICJS_PARSER_H

