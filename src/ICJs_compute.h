#ifndef ICJS_COMPUTE_H
#define ICJS_COMPUTE_H

#include "ICJs_types.h"
#include <string>
#include <vector>
#include <map>

class Calculator
{
	public:
		Calculator();
		static int calculate(std::string &exp, 
			std::map<std::string, Element> &variables, 
			Element &ret,
			std::vector<Element> &output);
		~Calculator();
};

#endif ICJS_COMPUTE_H