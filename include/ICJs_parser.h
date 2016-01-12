#ifndef ICJS_PARSER_H
#define ICJS_PARSER_H

#include "ICJS_types.h"
#include "ICJs_compute.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

typedef vector<string>::iterator vsit;
typedef string::iterator sit;

class Parser
{
	public:
		Parser();
		static int parse(std::vector<std::string> &input, 
			std::map<std::string, Element> &variables, 
			std::vector<Element> &output);
		static int run_func(Function &func,
			std::map<std::string, Element> &variables,
			std::vector<Element> parameters,
			std::vector<Element> &ret, 
			std::vector<Element> &output);
		~Parser();
};

void clear_string(string& str);
void string_dblank(string& str);
vsit vector_find_bracket(vector<string> &lines, vsit line);
vsit vector_find_else(vector<string> &lines, vsit line);
vsit vector_find_next_case(vector<string> &lines, vsit line);
int forloop(vsit &bg, vsit&ed,
	map<std::string, Element> &variables,
	vector<Element> &output,
	vector<Element>& fun_ret,
	string var,
	string list);

int _parse(std::vector<std::string> &input,
	std::map<std::string, Element> &variables,
	std::vector<Element> &output,
	bool islocal,
	vector<Element> &fun_ret);

#endif ICJS_PARSER_H

