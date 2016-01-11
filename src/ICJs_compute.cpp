#include "ICJs_compute.h"
#include "ICJs_util.h"
#include "ICJs_types.h"
#include "ICJs_parser.h"
#include <stack>
#include <cmath>
#include <ctype.h>
#include <sstream>

const int numericMode = Global::_number;
const int stringMode = Global::_string;

int Calculator::calculate(std::string &exp,
	std::map<std::string, Element> &variables,
	std::vector<Element> &rets,
	std::vector<Element> &output)
{
	Util::trim(exp);
	size_t leftBkt;
	size_t rightBkt;
	std::vector<Element> *elemArray = new std::vector<Element>;
		
	leftBkt = exp.find_first_of("[");
	rightBkt = exp.find_first_of("]");
	if (leftBkt != std::string::npos && rightBkt != std::string::npos)
	{
		std::string inArray;
		std::vector<std::string> aryElements;
		std::vector<Element> tempRets;

		if (rightBkt == std::string::npos || rightBkt != exp.length() - 1 || rightBkt < leftBkt)
			return Global::_fault;
		inArray = exp.substr(leftBkt + 1, rightBkt - leftBkt - 1);
		Util::split(inArray, ",", &aryElements);
		std::vector<std::string> fusedElements;
		
		bool isSep = false;
		std::string toBeFused = "";
		int numOfParenth = 0;
		int localParenth = 0;
		for (int i = 0; i < aryElements.size(); i++)
		{
			localParenth = 0;
			localParenth += Util::numOfChar(aryElements[i], '(');
			localParenth -= Util::numOfChar(aryElements[i], ')');
			numOfParenth += localParenth;
			if (numOfParenth < 0)
			{
				return Global::_fault;
			}
			if (numOfParenth > 0)
			{
				toBeFused += aryElements[i];
				toBeFused += ",";
			}
			if (numOfParenth == 0)
			{
				if (localParenth == 0)
				{		
					fusedElements.push_back(aryElements[i]);
				}
				if (localParenth < 0)
				{
					toBeFused += aryElements[i];
					fusedElements.push_back(toBeFused);
					toBeFused = "";
				}
			}
		}
		

		for (int i = 0; i < fusedElements.size(); i++)
		{
			Element *elem = new Element;
			numeric(fusedElements[i], variables, tempRets, output);
			(*elem).type = tempRets[0].type;
			(*elem).data = tempRets[0].data;
			(*elemArray).push_back(*elem);
			tempRets.clear();
		}
		Element *toRet = new Element;
		(*toRet).type = Global::_array;
		(*toRet).data = any_t(elemArray);

		rets.push_back(*toRet);

		return Global::_ok;
	}
	else if ((leftBkt == std::string::npos && rightBkt != std::string::npos )|| 
		(leftBkt != std::string::npos && rightBkt == std::string::npos))
		return Global::_fault;
	else
	{
		return numeric(exp, variables, rets, output);
	}
}

int Calculator::numeric(std::string &exp,
	std::map<std::string, Element> &variables,
	std::vector<Element> &rets,
	std::vector<Element> &output)
{
	size_t index;
	size_t start;
	int counter = 0;

	Util::trim(exp);

	while ((start = exp.find_first_of("(")) != std::string::npos)
	{
		int flag;
		size_t end;
		std::string component;
		std::vector<Element> tempRets;
		std::ostringstream os;

		counter++;
		index = start + 1;

		while (index < exp.length())
		{
			if (exp.at(index) == '(')
				counter++;
			if (exp.at(index) == ')')
				counter--;
			if (counter == 0)
				break;
			index++;
		}

		end = index;
		if (end >= exp.length())
			return Global::_fault;

		component = exp.substr(start + 1, end - start - 1);
		flag = numeric(component, variables, tempRets, output);

		if (flag == Global::_fault)
			return Global::_fault;

		for (int i = 0; i < tempRets.size(); i++)
		{
			os << *(double *)(tempRets[i].data);
			if (i != tempRets.size() - 1)
				os << ",";
		}

		if (start != 0 && isalnum(exp.at(start - 1)))
		{
			int j;
			int flag;
			std::string funcName;
			Element tempRet;
			std::ostringstream os;

			exp.replace(start + 1, end - start - 1, os.str());
			for (end = start + 1; end < exp.length(); end++)
				if (exp.at(end) == ')')
					break;
			
			for (j = start - 1; j >= 0; j--)
			{
				if (exp.at(j) == ' ')
					break;
			}

			funcName = exp.substr(j + 1, start - j - 1);
			flag = isFunction(funcName, variables);
			if (flag == 0)
				return Global::_fault;

			// TODO
			tempRet.data = any_t(new double(*(double *)tempRets[0].data + *(double *)tempRets[1].data));
			//Parser::run_func(*((Function *)(variables[funcName].data)), variables, tempRets, tempRet, output);

			os << *(double *)(tempRet.data);
			exp.replace(j + 1, end - j, os.str());
		}
		else
		{
			exp.replace(start, end - start + 1, os.str());
		}
	}

	std::vector<std::string> commaSeps;
	Element toCommaRet;
	int flag;
	
	Util::split(exp, ",", &commaSeps);
	
	for (int i = 0; i < commaSeps.size(); i++)
	{
		flag = RPNCalc(commaSeps[i], variables, toCommaRet);
		if (flag == Global::_fault)
			return Global::_fault;
		rets.push_back(toCommaRet);
	}

	return Global::_ok;
}

int Calculator::isOperator(std::string input)
{
	if (input == "**" || input == "*" || input == "/" || input == "+" || input == "-" || input == "%" || 
		input == "and" || input == "or" || input == "not" || input == ">=" || input == "<=" ||
		input == "==" || input == "!=" || input == ">" || input == "<")
		return 1;
	else 
		return 0;
}

int Calculator::priority(std::string opt)
{
	if (opt == "**")
		return 6;
	if (opt == "not")
		return 5;
	if (opt == "*" || opt == "/" || opt == "%")
		return 4;
	if (opt == "+" || opt == "-")
		return 3;
	if (opt == ">=" || opt == "<=" || opt == ">" || opt == "<")
		return 2;
	if (opt == "==" || opt == "!=")
		return 1;
	if (opt == "and" || opt == "or")
		return 0;
}

int Calculator::isFunction(std::string input, std::map<std::string, Element> &variables)
{
	std::map<std::string, Element>::iterator it;
	it = variables.find(input);
	if (it == variables.end())
	{
		return 0;
	}
	else if (it->second.type == Global::_function)
	{
		return 1;
	}
	else
		return 0;
}

int Calculator::isNumVar(std::string input, std::map<std::string, Element> &variables)
{
	std::map<std::string, Element>::iterator it;
	it = variables.find(input);
	if (it != variables.end())
		return 2;

	double num;
	std::istringstream iss(input);
	iss >> num;
	if (!iss)
		return 0;

	return (iss.rdbuf()->in_avail() == 0);
		
}

int Calculator::isStringVar(std::string input, std::map<std::string, Element> &variables)
{
	std::map<std::string, Element>::iterator it;
	it = variables.find(input);
	if (it != variables.end())
	{
		if (it->second.type == Global::_string)
			return 2;
	}
	else
	{
		if (input.at(0) == '"' && input.at(input.length() - 1) == '"')
			return 1;
	}
	return 0;
}

int Calculator::isLogic(std::string input)
{
	size_t indicator;
	indicator = input.find_first_of("==");
	if (indicator != std::string::npos)
		return 1;
	indicator = input.find_first_of("!=");
	if (indicator != std::string::npos)
		return 1;
	indicator = input.find_first_of(">=");
	if (indicator != std::string::npos)
		return 1;
	indicator = input.find_first_of("<=");
	if (indicator != std::string::npos)
		return 1;
	indicator = input.find_first_of("<");
	if (indicator != std::string::npos)
		return 1;
	indicator = input.find_first_of(">");
	if (indicator != std::string::npos)
		return 1;
	return 0;
}

int Calculator::RPNCalc(std::string input, std::map<std::string, Element> &variables, Element &ret)
{
	Util::trim(input);
	std::vector<std::string> ops;
	Util::split(input, " ", &ops);
	std::stack<std::string> optStack;
	std::stack<std::string> expStack;
	std::stack<std::string> reverse;
	std::stack<double> numericStack;
	std::stack<bool> logicalStack;
	std::stack<std::string> stringStack;

	double num;
	std::string s;
	int mode = numericMode;

	bool log = isLogic(input);
	
	if (isStringVar(ops[0], variables))
	{
		mode = stringMode;
	}

	int indicator;

	if (ops.size() % 2 == 0)
		return Global::_fault;

	for (int i = 0; i < ops.size(); i++)
	{
		if (i % 2 == 0)
		{
			if (mode == numericMode)
			{
				indicator = isNumVar(ops[i], variables);
				if (indicator == 1)
				{
					expStack.push(ops[i]);
				}
				else if (indicator == 2)
				{
					std::map<std::string, Element>::iterator it;
					it = variables.find(ops[i]);
					num = *(double *)it->second.data;
					std::ostringstream os;
					os << num;
					expStack.push(os.str());
				}
				else
					return Global::_fault;
			}
			if (mode == stringMode)
			{
				indicator = isStringVar(ops[i], variables);
				if (indicator == 1)
				{
					expStack.push(ops[i].substr(1, ops[i].length() - 2));
				}
				else if (indicator == 2)
				{
					std::map<std::string, Element>::iterator it;
					it = variables.find(ops[i]);
					s = *(std::string *)it->second.data;
					expStack.push(s);
				}
				else
					return Global::_fault;
			}
		}
		else
		{
			if (isOperator(ops[i]))
			{
				if (mode == stringMode && ops[i] != "+")
					return Global::_fault;
				while (optStack.size() != 0)
				{
					std::string tmp = optStack.top();
					if (priority(ops[i]) > priority(tmp))
					{
						break;
					}
					else
					{
						optStack.pop();
						expStack.push(tmp);
					}
				}
				optStack.push(ops[i]);
				
			}
			else
			{
				return Global::_fault;
			}
		}
	}
	while (optStack.size() != 0)
	{
		std::string temp = optStack.top();
		optStack.pop();
		expStack.push(temp);
	}
	while (expStack.size() != 0)
	{
		std::string temp = expStack.top();
		expStack.pop();
		reverse.push(temp);
	}

	if (mode == numericMode)
	{
		double op1, op2;
		while (reverse.size() != 0)
		{
			std::string temp = reverse.top();
			reverse.pop();
			if (!isOperator(temp))
			{
				std::istringstream(temp) >> op1;
				numericStack.push(op1);
			}
			else
			{
				op2 = numericStack.top();
				numericStack.pop();
				if (temp != "not")
				{
					op1 = numericStack.top();
					numericStack.pop();
					if (temp == "**")
						numericStack.push(pow(op1, op2));
					if (temp == "*")
						numericStack.push(op1 * op2);
					if (temp == "/")
						numericStack.push(op1 / op2);
					if (temp == "+")
						numericStack.push(op1 + op2);
					if (temp == "-")
						numericStack.push(op1 - op2);
					if (temp == "%")
						numericStack.push((int)op1 % (int)op2);
					if (temp == "==")
						numericStack.push((double)(op1 == op2));
					if (temp == ">=")
						numericStack.push((double)(op1 >= op2));
					if (temp == "<=")
						numericStack.push((double)(op1 <= op2));
					if (temp == "!=")
						numericStack.push((double)(op1 != op2));
					if (temp == ">")
						numericStack.push((double)(op1 > op2));
					if (temp == "<")
						numericStack.push((double)(op1 < op2));
					if (temp == "and")
						numericStack.push((double)(op1 && op2));
					if (temp == "||")
						numericStack.push((double)(op1 || op2));
				}
				else
				{
					if (op2 != 0)
						numericStack.push(0.0);
					else
						numericStack.push(1.0);
				}
			}
		}
		if (numericStack.size() != 1 || reverse.size() != 0)
			return Global::_fault;
		else
		{
			if (!log)
			{
				ret.type = Global::_number;
				ret.data = any_t(new double(numericStack.top()));
				return Global::_ok;
			}
			else
			{
				ret.type = Global::_boolean;
				ret.data = any_t(new bool(numericStack.top()));
				return Global::_ok;
			}
		}
	}
	if (mode == stringMode)
	{
		std::string s1, s2;
		bool tOrF;
		while (reverse.size() != 0)
		{
			std::string temp = reverse.top();
			reverse.pop();
			if (!isOperator(temp))
			{
				stringStack.push(temp);
			}
			else
			{
				s2 = stringStack.top();
				stringStack.pop();
				s1 = stringStack.top();
				stringStack.pop();
				if (temp == "+")
					stringStack.push(s1 + s2);
				else if (temp == "==")
				{
					if (s1 == s2)
						tOrF = true;
					else
						tOrF = false;
					break;
				}
				else if (temp == "!=")
				{
					if (s1 != s2)
						tOrF = true;
					else
						tOrF = false;
					break;
				}
				else
					return Global::_fault;
			}
		}
		if (stringStack.size() != 1 || reverse.size() != 0)
			return Global::_fault;
		else
		{
			if (!log)
			{
				ret.type = Global::_string;
				ret.data = any_t(new std::string(stringStack.top()));
				return Global::_ok;
			}
			else
			{
				ret.type = Global::_boolean;
				ret.data = any_t(new bool(tOrF));
				return Global::_ok;
			}
		}
	}
	
}