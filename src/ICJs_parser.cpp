#include "ICJs_parser.h"
#include "ICJs_compute.h"
#include <sstream>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

Parser::Parser()
{

}


Parser::~Parser()
{
}
//去除多余符号，保留操作符
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
//去除空格和制表符
void string_dblank(string& str)
{
	sit it = str.begin();
	while (it!=str.end()){
		if (*it == ' ' || *it == '\t') it = str.erase(it);
		else it++;
	}
}
//找到对应的反大括号
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
//找到对应的else句段
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
	else{
		return it + 1;
	}
}
//找到下一个case句段
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
//foreach语句循环
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
//字符串分词
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
/*
	语句段内部函数
	islocal检测是否需要本地变量
	fun_ret返回可能的函数返回值
*/
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

	while (it < input.end()){
		ret.clear();
		int end = it->find(';');
		if (end == string::npos)
			end = it->size();

		istringstream is(*it);
		is >> oper;
		clear_string(oper);
		/*
			赋值语句
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
			条件选择语句
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
			//Element* element = new Element;
			//ret.push_back(*element);
			int error = Calculator::calculate(cond, variables, ret, output);
			//int error = Calculator::RPNCalc(cond, variables, ret[0]);
			if (error != Global::_ok)
				return error;
			if (ret[0].type != Global::_boolean)
				return Global::_fault;

			if (*((bool*)ret[0].data) == true){
				it++;
			}
			else {
				if (vector_find_else(input, it) < input.end())
					it = vector_find_else(input, it) + 1;
				else
					it = input.end();
			}
		}
		else if (oper == "else"){
			istringstream is(*(it+1));
			is >> oper;
			clear_string(oper);
			if (oper == "{"){
				it = vector_find_bracket(input, it + 1);
			}
			else{
				it = it + 2;
			}
		}
		/*
			foreach循环
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
				it = it + 2;
			}
		}
		/*
			switch-case选择
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

			switch_exp = it->substr(first + 1, last - first - 1);
			ed = vector_find_bracket(input, it + 1);
			it += 2;
		}
		else if (oper == "case"){
			int first = it->find('e');
			int last = it->rfind(':');
			string value = it->substr(first + 1, last - first - 1);
			string_dblank(value);
			exp = switch_exp + " " + "==" + " " +  value ;
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
		/*
			break,continue跳转
		*/
		else if (oper == "break"){
			it = ed;
			ed = input.end();
		}
		else if (oper == "continue"){
			it = ed;
		}
		/*
			while循环
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
		/*
			return语句
		*/
		else if (oper == "return"){
			int first = it->find('n');
			int last = end;
			assert(first != string::npos);
			assert(last != string::npos);
			
			exp = it->substr(first + 1, last - first - 1);
			int error = Calculator::calculate(exp, variables, ret, output);
			if (error != Global::_ok)
				return error;
			fun_ret.clear();
			for (int i = 0; i < ret.size(); i++){
				fun_ret.push_back(ret[i]);
			}
			break;
		}
		/*
			函数定义
			function exp(a, b, c...)
			{
				...
			}
		*/
		else if (oper == "function"){
			int first = it->find('o');
			int last = it->find('(');

			assert(first != string::npos);
			assert(last != string::npos);

			exp = it->substr(first + 2, last - first - 2);
			string_dblank(exp);

			Function *function = new Function;
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
		else if (oper == "{"||oper=="}")
			it++;
		else {
			/*
				赋值语句
				a=exp
				a,b=exp1, exp2
			*/
			if (it->find("=") != string::npos && it->find("==") != it->find("="))
			{
				int first = it->find("=");
				string keylist = it->substr(0, first);
				string_dblank(keylist);
				vector<string> keys;
				split(keylist, ",", keys);

				for (vector<string>::iterator itt = keys.begin(); itt != keys.end(); itt++){
					if (itt->find("[") != string::npos){
						int first = itt->find("[");
						int last = itt->find("]");
						key = itt->substr(0, first);
						if (variables.find(key) == variables.end())
							return Global::_fault;
						if (variables[key].type != Global::_array)
							return Global::_fault;
					}
					else{
						if (variables.find(*itt) == variables.end())
							return Global::_fault;
					}
				}
				exp = it->substr(first + 1, end - first - 1);
				int error = Calculator::calculate(exp, variables, ret, output);
				if (error != Global::_ok)
					return error;
				if (ret.size() != keys.size())
					return Global::_fault;

				for (int i = 0; i < ret.size(); i++){
					if (keys[i].find("[") != string::npos){
						int first = keys[i].find("[");
						int last = keys[i].find("]");
						key = keys[i].substr(0, first);
						string subexp = keys[i].substr(first + 1, last - first - 1);
						vector<Element> subret;
						int error = Calculator::calculate(subexp, variables, subret, output);
						if (error != Global::_ok)
							return error;
						if (subret.size() != 1)
							return Global::_fault;
						if (subret[i].type != Global::_number)
							return Global::_fault;
						(*((vector<Element>*)variables[key].data))[(int)(*(double*)subret[0].data)] = ret[i];
					}
					else {
						ret[i].key = keys[i];
						variables[keys[i]] = ret[i];
					}
				}
				it++;
			}
			/*
				表达式求值
				exp
			*/
			else{
				exp = it->substr(0, end);
				string::iterator itt = exp.begin();
				if (exp == "")
					return Global::_ok;
				while (*itt == '\t' || *itt == ' '){
					itt = exp.erase(itt);
				}
				int error = Calculator::calculate(exp, variables, ret, output);
				if (error != Global::_ok)
					return error;
				for (int i = 0; i < ret.size(); i++)
					output.push_back(ret[i]);
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
//函数运行
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
	//将参数加入变量表
	for (int i = 0; i < parameters.size(); i++){
		parameters[i].key = func.param_names[i];
		localVar.push_back(parameters[i]);
		if (variables.find(func.param_names[i]) != variables.end()){
			coflictVar.push_back(variables[func.param_names[i]]);			
		}
		variables[func.param_names[i]] = parameters[i];
	}
	
	_parse(func.body, variables, output, true, ret);
	//将参数移出变量表
	for (int i = 0; i < localVar.size(); i++){
		variables.erase(localVar[i].key);
	}
	for (int i = 0; i < coflictVar.size(); i++){
		variables[coflictVar[i].key] = coflictVar[i];
	}
	return Global::_ok;
}