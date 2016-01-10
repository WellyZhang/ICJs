#include "ICJs_parser.h"
#include "ICJs_compute.h"

Parser::Parser()
{
}

int run_func(Function &func,
	std::map<std::string, Element> &variables,
	std::vector<Element> parameters,
	Element &ret,
	std::vector<Element> &output)
{
	ret.data = any_t(new double(*(double *)parameters[0].data + *(double *)parameters[2].data));
	ret.type = Global::_number;
	return 1;
}

Parser::~Parser()
{
}

