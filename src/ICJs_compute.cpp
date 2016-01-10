#include "ICJs_compute.h"
#include "ICJs_util.h"
#include "ICJs_types.h"
#include "ICJs_parser.h"
#include <stack>
#include <cmath>
#include <ctype.h>
#include <sstream>

int Calculator::calculate(std::string &exp,
	std::map<std::string, Element> &variables,
	std::vector<Element> &rets,
	std::vector<Element> &output)
{
	Util::trim(exp);
	size_t leftBkt;
	size_t rightBkt;
	std::vector<Element> tempElements;
	
	leftBkt = exp.find_first_of("[");
	rightBkt = exp.find_first_of("]");
	if (leftBkt != std::string::npos)
	{
		std::string inArray;
		std::vector<std::string> aryElements;
		std::vector<Element> tempRets;

		if (rightBkt == std::string::npos || rightBkt != exp.length() - 1 || rightBkt < leftBkt)
			return Global::_fault;
		inArray = exp.substr(leftBkt + 1, rightBkt - leftBkt - 1);
		Util::split(inArray, ",", &aryElements);
		for (int i = 0; i < aryElements.size(); i++)
		{
			numeric(aryElements[i], variables, tempRets, output);
			tempRets.clear();
		}

	}
	else if (rightBkt == std::string::npos)
		return Global::_fault;
	
	return Global::_ok;
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
			//p.run_func(f, variables, tempRets, tempRet, output);

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
	if (input == "**" || input == "*" || input == "/" || input == "+" || input == "-" || input == "%")
		return 1;
	else 
		return 0;
}

int Calculator::priority(std::string opt)
{
	if (opt == "**")
		return 3;
	else if (opt == "*" || opt == "/" || opt == "%")
		return 2;
	else
		return 1;
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

int Calculator::isVar(std::string input, std::map<std::string, Element> &variables)
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

int Calculator::RPNCalc(std::string input, std::map<std::string, Element> &variables, Element &ret)
{
	Util::trim(input);
	std::vector<std::string> ops;
	Util::split(input, " ", &ops);
	std::stack<std::string> optStack;
	std::stack<std::string> expStack;
	std::stack<std::string> reverse;
	std::stack<double> resultStack;

	double num;
	int indicator;

	if (ops.size() % 2 == 0)
		return Global::_fault;
	for (int i = 0; i < ops.size(); i++)
	{
		if (i % 2 == 0)
		{
			indicator = isVar(ops[i], variables);
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
		else
		{
			if (isOperator(ops[i]))
			{
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

	double op1, op2;
	while (reverse.size() != 0)
	{
		std::string temp = reverse.top();
		reverse.pop();
		if (!isOperator(temp))
		{
			std::istringstream(temp) >> op1;
			resultStack.push(op1);
		}
		else
		{
			op2 = resultStack.top();
			resultStack.pop();
			op1 = resultStack.top();
			resultStack.pop();
			if (temp == "**")
				resultStack.push(pow(op1, op2));
			if (temp == "*")
				resultStack.push(op1 * op2);
			if (temp == "/")
				resultStack.push(op1 / op2);
			if (temp == "+")
				resultStack.push(op1 + op2);
			if (temp == "-")
				resultStack.push(op1 - op2);
			if (temp == "%")
				resultStack.push((int)op1 % (int)op2);
		}
	}
	if (resultStack.size() != 1 || reverse.size() != 0)
		return Global::_fault;
	else
	{
		ret.type = Global::_number;
		ret.data = any_t(new double(resultStack.top()));
		return Global::_ok;
	}
	
}