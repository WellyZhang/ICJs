#include "ICJs_compute.h"
#include "ICJs_util.h"
#include "ICJs_types.h"
#include "ICJs_parser.h"
#include <stack>
#include <cmath>
#include <ctype.h>
#include <sstream>
#include <iostream>

const int numericMode = Global::_number;
const int stringMode = Global::_string;

/*
 * calculate函数
 * 输入表达式，变量，返回值及输出
 * 此函数处理数组，逗号表达式并将具体所需的计算传入一下层numeric函数
 */

int Calculator::calculate(std::string &exp,
	std::map<std::string, Element> &variables,
	std::vector<Element> &rets,
	std::vector<Element> &output)
{
	Util::trim(exp);
	size_t leftBkt;
	size_t rightBkt;
	std::vector<Element> *elemArray = new std::vector<Element>;
		
	// 处理数组并获取数组[]内的逗号表达式
	// 当做逗号表达式一起处理
	leftBkt = exp.find_first_of("[");
	rightBkt = exp.find_last_of("]");
	int commaExp = 0;
	if (!isStringVar(exp, variables))
		commaExp = isComma(exp);
	int isArrayDef = 1;
	std::string arrayName;
	if ((leftBkt != std::string::npos && rightBkt != std::string::npos) || commaExp)
	{
		std::string inArray;
		std::vector<std::string> aryElements;
		std::vector<Element> tempRets;

		isArrayDef = (leftBkt == 0);

		for (int k = leftBkt - 1; k >= 0; k--)
		{
			if (exp.at(k) == ' ' || k == 0)
			{
				if (k == 0)
					k -= 1;
				arrayName = exp.substr(k + 1, leftBkt - k - 1);
				break;
			}
		}

		if (!isArrayDef && !isArrayVar(arrayName, variables) && !commaExp)
			return Global::_fault;

		if (!commaExp)
		{
			if (rightBkt < leftBkt)
				return Global::_fault;
			if (!isArrayDef)
				rightBkt = exp.find_first_of("]");
			inArray = exp.substr(leftBkt + 1, rightBkt - leftBkt - 1);
		}
		else
		{
			inArray = exp;
		}
		Util::split(inArray, ",", &aryElements, false);
		std::vector<std::string> arrayElements;
		std::vector<std::string> quoteElements;
		std::vector<std::string> fusedElements;
		
		// 由于采用了“，”和“ ”作为分割，可能将字符串内的字符分离
		// 因此在此处需要将它们重新联合起来
		bool isSep = false;
		std::string toBeFused = "";
		int numOfParenth = 0;
		int localParenth = 0;
		int numOfBracket = 0;
		int localBracket = 0;
		int numOfQuote = 0;

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
					arrayElements.push_back(aryElements[i]);
				}
				if (localParenth < 0)
				{
					toBeFused += aryElements[i];
					arrayElements.push_back(toBeFused);
					toBeFused = "";
				}
			}
		}

		for (int i = 0; i < arrayElements.size(); i++)
		{
			localBracket = 0;
			localBracket += Util::numOfChar(arrayElements[i], '[');
			localBracket -= Util::numOfChar(arrayElements[i], ']');
			numOfBracket += localBracket;
			if (numOfBracket < 0)
			{
				return Global::_fault;
			}
			if (numOfBracket > 0)
			{
				toBeFused += arrayElements[i];
				toBeFused += ", ";
			}
			if (numOfBracket == 0)
			{
				if (localBracket == 0)
				{
					quoteElements.push_back(arrayElements[i]);
				}
				if (localBracket < 0)
				{
					toBeFused += arrayElements[i];
					quoteElements.push_back(toBeFused);
					toBeFused = "";
				}
			}
		}

		isSep = false;

		for (int i = 0; i < quoteElements.size(); i++)
		{
			numOfQuote = Util::numOfChar(quoteElements[i], '\"');
			if (numOfQuote % 2 == 0 && isSep)
			{
				toBeFused += quoteElements[i];
				toBeFused += ",";
			}
			else if (numOfQuote % 2 == 0 && !isSep)
				fusedElements.push_back(quoteElements[i]);
			else if (numOfQuote % 2 != 0 && isSep)
			{
				toBeFused += ",";
				toBeFused += quoteElements[i];
				fusedElements.push_back(toBeFused);
				toBeFused = "";
				isSep = false;
			}
			else
			{
				isSep = true;
				toBeFused += quoteElements[i];
			}
		}
		
		// 计算每个逗号表达式成分
		int flag;
		for (int i = 0; i < fusedElements.size(); i++)
		{
			Element *elem = new Element;
			flag = calculate(fusedElements[i], variables, tempRets, output);
			if (flag == Global::_fault)
				return Global::_fault;
			(*elem).type = tempRets[0].type;
			(*elem).data = tempRets[0].data;
			(*elemArray).push_back(*elem);
			tempRets.clear();
		}

		// 每个返回值必须是new出来的
		if (isArrayDef || commaExp)
		{
			if (!commaExp)
			{
				Element *toRet = new Element;
				(*toRet).type = Global::_array;
				(*toRet).data = any_t(elemArray);
				rets.push_back(*toRet);
			}
			else
			{
				for (int i = 0; i < (*elemArray).size(); i++)
				{
					rets.push_back((*elemArray)[i]);
				}
			}
			return Global::_ok;
		}
		else
		{
			// 若是一个数组调用，如a[2]
			// 需要采用特殊方法处理
			if ((*elemArray).size() != 1 ||
				(*elemArray)[0].type != Global::_number)
				return Global::_fault;
			double index = *(double *)((*elemArray)[0].data);
			if (index == (int)index)
			{
				std::ostringstream os;
				std::string s;
				Element e;
				e = (*((std::vector<Element> *)(variables[arrayName].data)))[int(index)];
				switch (e.type)
				{
				case Global::_string:
					os << "\"";
					os << *(std::string *)(e.data);
					os << "\"";
					exp.replace(leftBkt - arrayName.length(), rightBkt - leftBkt + arrayName.length() + 1, os.str());
					break;
				case Global::_number:
					os << *(double *)(e.data);
					exp.replace(leftBkt - arrayName.length(), rightBkt - leftBkt + arrayName.length() + 1, os.str());
					break;
				case Global::_array:
					os << "[";
					os << Util::arrayToString(*((std::vector<Element> *)(e.data)));
					os << "]";
					exp.replace(leftBkt - arrayName.length(), rightBkt - leftBkt + arrayName.length() + 1, os.str());
					break;
				}
				return calculate(exp, variables, rets, output);
			}
			else return Global::_fault;
		}
	}
	else if ((leftBkt == std::string::npos && rightBkt != std::string::npos )|| 
		(leftBkt != std::string::npos && rightBkt == std::string::npos))
		return Global::_fault;
	else
	{
		// 若直接是表达式则可以放入下一层计算
		return numeric(exp, variables, rets, output);
	}
}

/* 
 * numeric函数
 * 输入表达式，变量值，返回位置及输出
 * 先处理表达式中的函数成分和()成分
 * 将成分算出后修改表达式
 * 将修改后的表达式放入RPNCalc中计算 
 */

int Calculator::numeric(std::string &exp,
	std::map<std::string, Element> &variables,
	std::vector<Element> &rets,
	std::vector<Element> &output)
{
	size_t index;
	size_t start;
	int counter = 0;

	Util::trim(exp);

	// 处理逻辑，字符串和算数运算
	while ((start = exp.find_first_of("(")) != std::string::npos)
	{
		// 优先处理最深的括号
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

		// 计算括号中内容
		component = exp.substr(start + 1, end - start - 1);
		flag = numeric(component, variables, tempRets, output);

		if (flag == Global::_fault)
			return Global::_fault;

		// 处理括号返回值
		// 将字符串修改
		for (int i = 0; i < tempRets.size(); i++)
		{
			switch(tempRets[i].type)
			{
				case Global::_boolean:
					os << *(bool *)(tempRets[i].data);
					break;
				case Global::_number:
					os << *(double *)(tempRets[i].data);
					break;
				case Global::_string:
					os << *(std::string *)(tempRets[i].data);
					break;
			}
			if (i != tempRets.size() - 1)
				os << ",";
		}

		// 处理函数调用
		if (start != 0 && isalnum(exp.at(start - 1)))
		{
			int j;
			int flag;
			std::string funcName;
			std::vector<Element> funcRets;

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

			Parser::run_func(*((Function *)(variables[funcName].data)), variables, tempRets, funcRets, output);
			
			std::ostringstream newOS;
			newOS << "";
			if (!funcRets.empty())
			{
				switch (funcRets[0].type)
				{
				case Global::_boolean:
					newOS << *(bool *)funcRets[0].data;
					break;
				case Global::_number:
					newOS << *((double *)funcRets[0].data);
					break;
				case Global::_string:
					newOS << "\"";
					newOS << *((std::string *)funcRets[0].data);
					newOS << "\"";
					break;
				}
				exp.replace(j + 1, end - j, newOS.str());
			}
		}
		else
		{
			exp.replace(start, end - start + 1, os.str());
		}
	}

	// 现在所有的括号和函数都被修改为真实值
	// 剩下的部分处理具体计算
	std::vector<std::string> commaSeps;
	std::vector<std::string> tempSeps;

	Element toCommaRet;
	int flag;
	
	Util::split(exp, ",", &tempSeps, false);

	std::string toBeFused = "";
	size_t quoteIdx;
	int numOfQuote = 0;

	bool isSep = false;
	
	for (int i = 0; i < tempSeps.size(); i++)
	{
		numOfQuote = Util::numOfChar(tempSeps[i], '\"');
		if (numOfQuote % 2 == 0 && isSep)
		{
			toBeFused += tempSeps[i];
			toBeFused += ",";
		}
		else if (numOfQuote % 2 == 0 && !isSep)
			commaSeps.push_back(tempSeps[i]);
		else if (numOfQuote % 2 != 0 && isSep)
		{
			toBeFused += ",";
			toBeFused += tempSeps[i];
			commaSeps.push_back(toBeFused);
			toBeFused = "";
			isSep = false;
		}
		else
		{
			isSep = true;
			toBeFused += tempSeps[i];
		}
	}

	// 计算每个成分
	for (int i = 0; i < commaSeps.size(); i++)
	{
		if (isArrayVar(commaSeps[i], variables))
		{
			rets.push_back(variables[commaSeps[i]]);
			return Global::_ok;
		}
		flag = RPNCalc(commaSeps[i], variables, toCommaRet);
		if (flag == Global::_fault)
			return Global::_fault;
		rets.push_back(toCommaRet);
	}

	return Global::_ok;
}

/*
 * isOperator函数
 * 输入字符串
 * 判断是否是运算符
 */

int Calculator::isOperator(std::string input)
{
	if (input == "**" || input == "*" || input == "/" || input == "+" || input == "-" || input == "mod" || 
		input == "and" || input == "or" || input == "not" || input == ">=" || input == "<=" ||
		input == "==" || input == "!=" || input == ">" || input == "<")
		return 1;
	else 
		return 0;
}

/*
 * priority函数
 * 输入字符串
 * 定义运算优先级
 */

int Calculator::priority(std::string opt)
{
	if (opt == "**")
		return 6;
	if (opt == "not")
		return 5;
	if (opt == "*" || opt == "/" || opt == "mod")
		return 4;
	if (opt == "+" || opt == "-")
		return 3;
	if (opt == ">=" || opt == "<=" || opt == ">" || opt == "<")
		return 2;
	if (opt == "==" || opt == "!=")
		return 1;
	if (opt == "and" || opt == "or")
		return 0;
	return 0;
}

/*
 * isFunction函数
 * 输入字符串及变量映射
 * 判断给定的函数名是否已经有函数定义
 */

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

/*
 * isNumVar函数
 * 输入字符串及变量映射
 * 判断变量或常量是否是一个number
 */

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

/* 
 * isStringVar函数
 * 输入字符串及变量映射
 * 判断一个变量或常量是否是一个string
 */

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
		int start, end;
		start = input.find_first_of("\"", 0);
		end = input.find_first_of("\"", start + 1);
		if (start == 0 && end == input.length() - 1)
			return 1;
	}
	return 0;
}

/*
 * isArrayVar函数
 * 输入字符串及变量映射
 * 判断一个变量或常量是否是一个数组
 */

int Calculator::isArrayVar(std::string input, std::map<std::string, Element> &variables)
{
	std::map<std::string, Element>::iterator it;
	it = variables.find(input);
	if (it != variables.end())
	{
		if (it->second.type == Global::_array)
			return 1;
	}
	return 0;
}

/*
 * isLogic函数
 * 输入字符串向量及变量映射
 * 判断一个输入是否是逻辑表达
 */

int Calculator::isLogic(std::vector<std::string> inputs, std::map<std::string, Element> vars)
{
	std::map<std::string, Element>::iterator it;
	it = vars.find(inputs[0]);
	if ((it != vars.end()) && it->second.type == Global::_boolean)
		return 1;
	for (int i = 0; i < inputs.size(); i++)
	{
		
		if (inputs[i] == "true" || inputs[i] == "false")
			return 1;
		if (inputs[i] == ">=" || inputs[i] == "<=" || inputs[i] == "==" || inputs[i] == "<" || 
			inputs[i] == ">" || inputs[i] == "!=")
			return 1;
		if (inputs[i] == "and" || inputs[i] == "or" || inputs[i] == "not")
			return 1;
	}
	return 0;
}

/*
 * isComma函数
 * 输入字符串
 * 判断一个输入是否是逗号表达式
 */

int Calculator::isComma(std::string input)
{
	int counterP = 0;
	int counterQ = 0;
	
	if (input.empty())
		return 0;
	if (input.at(0) == '[' && input.at(input.length() - 1) == ']')
		return 0;
	for (int i = 0; i < input.length(); i++)
	{
		if (input.at(i) == '(')
			counterP++;
		if (input.at(i) == '\"')
			counterQ++;
		if (input.at(i) == ')')
			counterP--;
		if (counterP == 0 && (counterQ % 2 == 0) && input.at(i) == ',')
			return 1;
	}
	return 0;
}

/*
 * isLogicOperator函数
 * 输入字符串
 * 判断一个输入是否是逻辑运算符
 */

int Calculator::isLogicOperator(std::string input)
{
	if (input == "==" || input == "!=" || input == ">=" || input == "<=" || input == ">" ||
		input == "<" || input == "and" || input == "or" || input == "not")
		return 1;
	else
		return 0;
}

/*
 * RPNCalc函数
 * 输入字符串，变零映射及返回地址
 * 通过逆波兰表达式计算表达式的值
 */

int Calculator::RPNCalc(std::string input, std::map<std::string, Element> &variables, Element &ret)
{
	Util::trim(input);
	std::vector<std::string> ops;
	std::stack<std::string> optStack;
	std::stack<std::string> expStack;
	std::stack<std::string> reverse;
	std::stack<double> numericStack;
	std::stack<std::string> stringStack;

	double num;
	std::string s;
	int mode = numericMode;

	if (!isStringVar(input, variables))
	{
		Util::split(input, " ", &ops, true);
	}
	else
	{
		ops.push_back(input);
	}

	if (isStringVar(ops[0], variables))
	{
		mode = stringMode;
	}

	int log = isLogic(ops, variables);

	int indicator;

	// 分为字符串与数值计算两类
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
					if (!log)
						num = *(double *)it->second.data;
					else{
						if (it->second.type == Global::_boolean){
							num = (double)(*(bool *)(it->second.data));
						}
						else
							num = (double)(*(double *)(it->second.data));
					
					}
					std::ostringstream os;
					os << num;
					expStack.push(os.str());
				}
				else if (ops[i] == "true")
					expStack.push("1");
				else if (ops[i] == "false")
					expStack.push("0");
				else if (ops[i] == "not")
				{
					i++;
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
					else if (ops[i] == "true")
						expStack.push("1");
					else if (ops[i] == "false")
						expStack.push("0");
					if (mode == stringMode && ops[i] != "+" && ops[i] != "==" && ops[i] != "!=")
						return Global::_fault;
					while (optStack.size() != 0)
					{
						std::string tmp = optStack.top();
						if (priority("not") > priority(tmp))
						{
							break;
						}
						else
						{
							optStack.pop();
							expStack.push(tmp);
						}
					}
					optStack.push("not");
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
				if (mode == stringMode && ops[i] != "+" && ops[i] != "==" && ops[i] != "!=")
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

	// 根据运算符确定运算类型
	if (mode == numericMode)
	{
		double op1, op2;
		std::string temp = "";
		while (reverse.size() != 0)
		{
			temp = reverse.top();
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
					if (temp == "mod")
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
					if (temp == "or")
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
		// 根据是否是逻辑表达式判断说是否返回布尔类型
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
				if (isLogicOperator(temp))
				{
					ret.type = Global::_boolean;
					ret.data = any_t(new bool(numericStack.top() != 0));
				}
				else
				{
					if (reverse.size() == 0)
					{
						ret.type = Global::_boolean;
						ret.data = any_t(new bool(numericStack.top() != 0));
					}
					else
					{
						ret.type = Global::_number;
						ret.data = any_t(new double(numericStack.top()));
					}
				}
				return Global::_ok;
			}
		}
	}
	if (mode == stringMode)
	{
		std::string s1, s2;
		double tOrF = 0;
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
						tOrF = 1.0;
					else
						tOrF = 0.0;
					break;
				}
				else if (temp == "!=")
				{
					if (s1 != s2)
						tOrF = 1.0;
					else
						tOrF = 0.0;
					break;
				}
				else
					return Global::_fault;
			}
		}
		if ((stringStack.size() != 0 && stringStack.size() != 1) || reverse.size() != 0)
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
				ret.data = any_t(new bool(tOrF == 1.0));
				return Global::_ok;
			}
		}
	}

	return Global::_ok;
	
}