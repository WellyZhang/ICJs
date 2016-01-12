#include "ICJs_parser.h"
#include "ICJs_compute.h"
#include <sstream>
#include <algorithm>
#include <cassert>

using namespace std;

Parser::Parser()
{

}


Parser::~Parser()
{
}

void clear_string(string& str)
{
	sit it = str.begin();
	while (it != str.end()){
		if (*it == '(' || *it == ':' || *it == ';'){
			str.erase(it, str.end());
			break;
		}
		it++;
	}
}

void string_dblank(string& str)
{
	sit it = str.begin();
	while ((it = find(it, str.end(), ' ')) != str.end()){
		str.erase(it);
		if (it == str.end()) break;
	}
}

vsit vector_find_bracket(vector<string> &lines, vsit line)
{
	vsit it = line + 1;
	int first = 1;
	int last = 0;
	while (first != last){
		string oper;
		istringstream is(*it);
		is >> oper;
		if (oper == "{")
			first++;
		if (oper == "}")
			last++;
		it++;
	}
	return it;
}

vsit vector_find_else(vector<string> &lines, vsit line)
{
	vsit it = line + 1;
	string oper;
	istringstream is(*it);
	is >> oper;
	clear_string(oper);

	/*
		if ()
		{

		}
		else
	*/
	if (oper == "{")
		return vector_find_bracket(lines, it);
	/*
		if ()
			..
		else
	*/
	else
		return it + 2;
}

vsit vector_find_next_case(vector<string> &lines, vsit line)
{
	vsit it = line + 1;
	string oper;
	while (it != lines.end())
	{
		istringstream is(*it);
		is >> oper;
		clear_string(oper);
		if (oper == "case" || oper == "default" || oper == "}")
			return it;
		it++;
	}
	return it;
}

int forloop(vsit &bg, vsit&ed,
	map<std::string, Element> &variables,
	vector<Element> &output,
	vector<Element> &fun_ret,
	string var,
	string list)
{
	vector<string> lines = vector<string>(bg, ed);
	vector<Element> *_list = (vector<Element>*)(variables[list].data);
	Element conflictVar;
	bool isconflict = false;
	if (variables.find(var) != variables.end()){
		conflictVar = variables[var];
		isconflict = true;
	}
	for (vector<Element>::iterator it = _list->begin(); it != _list->end(); it++){
		variables[var] = *it;
		int error = _parse(lines, variables, output, true, fun_ret);
		if (error != Global::_ok)
			return error;
	}
	variables.erase(var);
	if (isconflict)
		variables[var] = conflictVar;
	return Global::_ok;
}

int Parser::parse(vector<string> &input,
	map<std::string, Element> &variables,
	vector<Element> &output)
{
	vector<Element> fun_ret;
	return _parse(input, variables, output, false, fun_ret);
}

void split(string& s, string delim, vector<string> &ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != string::npos)
	{
		ret.push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret.push_back(s.substr(last, index - last));
	}
}

int _parse(vector<string> &input,
	map<std::string, Element> &variables,
	vector<Element> &output,
	bool islocal,
	vector<Element> &fun_ret)
{
	vsit it = input.begin();
	vsit ed = input.end();
	vsit bg = input.begin();
	string oper;
	string key;
	string cond;
	string exp;
	string var;
	string list;
	string switch_exp;
	vector<Element> ret;
	vector<Element> localVar;
	vector<Element> cofflictVar;

	while (it != input.end()){
		ret.clear();
		int end = it->find(';');
		if (end == string::npos)
			end = it->size() - 1;

		istringstream is(*it);
		is >> oper;
		clear_string(oper);
		/*
			var a=1+2;
			var a;
		*/
		if (oper == "var"){
			int last = it->find('=');
			int first = it->find('r');

			if (last != string::npos){
				string keylist = it->substr(first + 1, last - first - 1);
				string_dblank(keylist);
				vector<string> keys;
				split(keylist, ",", keys);
				exp = it->substr(last + 1, end - last - 1);

				int error = Calculator::calculate(exp, variables, ret, output);
				if (error != Global::_ok){
					return Global::_fault;
				}
				if (ret.size() != keys.size())
					return Global::_fault;
				for (int i = 0; i < keys.size(); i++){
					ret[i].key = keys[i];
					if (islocal){
						if (variables.find(keys[i]) != variables.end()){
							cofflictVar.push_back(variables[keys[i]]);
							variables.erase(keys[i]);
						}
						localVar.push_back(ret[i]);
					}
					variables[keys[i]] = ret[i];
				}
				it++;
			}
			else{
				string keylist = it->substr(first + 1, end - first -1);
				string_dblank(keylist);
				vector<string> keys;
				split(keylist, ",", keys);

				for (int i = 0; i < keys.size(); i++){
					if (islocal){
						if (variables.find(keys[i]) != variables.end()){
							cofflictVar.push_back(variables[keys[i]]);
							variables.erase(keys[i]);
						}
						localVar.push_back(Element(keys[i], Global::_undefined));
					}
					variables[keys[i]] = Element(keys[i], Global::_undefined);
				}
				it++;
			}
		}
		/*
			if (cond)
				....
			else
		*/
		else if (oper == "if"){
			int first = it->find('(');
			int last = it->rfind(')');

			assert(first != string::npos);
			assert(last != string::npos);
			
			cond = it->substr(first + 1, last - first - 1);
			int error = Calculator::calculate(cond, variables, ret, output);
			//int error = Calculator::RPNCalc(cond, variables, ret, output);
			if (error != Global::_ok)
				return error;
			if (ret[0].type != Global::_boolean)
				return Global::_fault;

			if (*((bool*)ret[0].data) == true){
				it++;
			}
			else {
				it = vector_find_else(input, it);
			}
		}
		else if (oper == "else"){
			it++;
		}
		/*
			for (var a : list)
				{

				}
		*/
		else if (oper == "for"){
			int first = it->find('a')+2;
			int last = it->find(':');
			assert(first != string::npos);
			assert(last != string::npos);
			var = it->substr(first + 1, last - first - 1);
			string_dblank(var);
			
			first = last;
			last = it->rfind(')');
			assert(last != string::npos);
			list = it->substr(first + 1, last - first - 1);
			string_dblank(list);

			if (variables.find(list) == variables.end())
				return Global::_fault;

			string oper;
			istringstream is(*(it+1));
			is >> oper;
			clear_string(oper);
			if (oper == "{"){
				int error = forloop(it + 2, vector_find_bracket(input, it + 1) - 1, 
					variables, 
					output, 
					fun_ret, 
					var, list);
				if (error != Global::_ok)
					return error;
				it = vector_find_bracket(input, it + 1);
			}
			else{
				int error = forloop(it + 1, it + 2, variables, output, 
					fun_ret,
					var, list);
				if (error != Global::_ok)
					return error;

			}
		}
		/*
			switch (exp)
			{
				case n:
				...
				...
				case ss:
				...
				...
				default:
			}
		*/
		else if (oper == "switch"){
			int first = it->find('(');
			int last = it->rfind(')');

			assert(first != string::npos);
			assert(last != string::npos);

			switch_exp = it->substr(first, last - first +1);
			ed = vector_find_bracket(input, it + 1);
			it += 2;
		}
		else if (oper == "case"){
			int first = it->find('e');
			int last = it->rfind(':');
			exp = switch_exp + "==" + "(" + it->substr(first + 1, last - first - 1) + ")";
			int error = Calculator::calculate(exp, variables, ret, output);
			assert(ret[0].type == Global::_boolean);

			if (*((bool*)ret[0].data) == true){
				it++;
			}
			else {
				it = vector_find_next_case(input, it);
			}
		}
		else if (oper == "default"){
			it++;
		}
		else if (oper == "break"){
			it = ed;
			ed = input.end();
		}
		else if (oper == "continue"){
			it = ed;
		}
		/*
			while ()
			{
				..
			}
		*/
		else if (oper == "while"){
			int first = it->find('(');
			int last = it->rfind(')');

			assert(first != string::npos);
			assert(last != string::npos);

			cond = it->substr(first + 1, last - first - 1);
			int error = Calculator::calculate(cond, variables, ret, output);
			if (error != Global::_ok)
				return error;
			if (ret[0].type != Global::_boolean)
				return Global::_fault;

			string oper;
			istringstream is(*(it + 1));
			is >> oper;
			clear_string(oper);

			if (*((bool*)ret[0].data) == true){
				if (oper == "{"){
					int error = _parse(vector<string>(it + 2, vector_find_bracket(input, it + 1)),
						variables,
						output,
						true,
						fun_ret);
					if (error != Global::_ok)
						return error;
				}
				else{
					int error = _parse(vector<string>(it + 1, it + 2),
						variables,
						output,
						true,
						fun_ret);
					if (error != Global::_ok)
						return error;
				}
			}
			else {
				if (oper == "{")
					it = vector_find_bracket(input, it + 1);
				else
					it = it + 2;
			}
		}
		else if (oper == "return"){
			int first = it->find('n');
			int last = it->rfind(';');
			assert(first != string::npos);
			assert(last != string::npos);
			
			exp = it->substr(first + 1, last - first - 1);
			int error = Calculator::calculate(exp, variables, ret, output);
			if (error != Global::_ok)
				return error;
			fun_ret = ret;
			break;
		}
		else if (oper == "function"){
			int first = it->find('o');
			int last = it->find('(');

			assert(first != string::npos);
			assert(last != string::npos);

			exp = it->substr(first + 2, last - first - 2);
			string_dblank(exp);

			Function *function=new Function;
			function->key = exp;

			first = last;
			last = it->rfind(')');
			split(it->substr(first + 1, last - first - 1), ",", function->param_names);
			for (vsit itt = function->param_names.begin(); itt != function->param_names.end(); itt++)
				string_dblank(*itt);
			
			function->body = vector<string>(it + 2, vector_find_bracket(input, it + 1) - 1);

			Element *element = new Element(function->key, Global::_function, function);

			variables[function->key] = *element;
			it = vector_find_bracket(input, it + 1);
		}
		else {
			if (it->find("=") != string::npos || it->find("==") == string::npos)
			{
				exp = it->substr(0, end);
				int error = Calculator::calculate(exp, variables, ret, output);
				if (error != Global::_ok)
					return error;
				output.push_back(ret[0]);
				it++;
			}
			else{
				int first = it->find("=");
				string keylist = it->substr(0, first);
				string_dblank(keylist);
				vector<string> keys;
				split(keylist, ",", keys);
				
				for (vector<string>::iterator itt = keys.begin(); itt != keys.end(); itt++){
					if (variables.find(*itt) == variables.end())
						return Global::_fault;
				}
				exp = it->substr(first + 1, end - first - 1);
				int error = Calculator::calculate(exp, variables, ret, output);
				if (error != Global::_ok)
					return error;
				if (ret.size() != keys.size())
					return Global::_fault;

				for (int i = 0; i < ret.size(); i++){
					ret[i].key = keys[i];
					variables[key] = ret[i];
				}
				it++;
			}
		}
	}
	for (vector<Element>::iterator it = localVar.begin(); it != localVar.end(); it++){
		variables.erase(it->key);
	}
	for (vector<Element>::iterator it = cofflictVar.begin(); it != cofflictVar.end(); it++){
		variables[it->key] = *it;
	}
	return Global::_ok;
}

int Parser::run_func(Function &func,
	std::map<std::string, Element> &variables,
	std::vector<Element> parameters,
	std::vector<Element> &ret,
	std::vector<Element> &output)
{
	vector<Element> localVar;
	vector<Element> coflictVar;

	if (func.param_names.size() != parameters.size())
		return Global::_fault;

	for (int i = 0; i < parameters.size(); i++){
		parameters[i].key = func.param_names[i];
		localVar.push_back(parameters[i]);
		if (variables.find(func.param_names[i]) != variables.end()){
			coflictVar.push_back(variables[func.param_names[i]]);			
		}
		variables[func.param_names[i]] = parameters[i];
	}
	
	_parse(func.body, variables, output, true, ret);
	
	for (int i = 0; i < localVar.size(); i++){
		variables.erase(localVar[i].key);
	}
	for (int i = 0; i < coflictVar.size(); i++){
		variables[coflictVar[i].key] = coflictVar[i];
	}
	return Global::_ok;
}