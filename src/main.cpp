#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include <math.h>
#include <map> 
#include <vector>
#include <iostream>
#include "ICJs_compute.h"
#include "ICJs_types.h"
#include "ICJs_util.h"
#include <sstream>

#define FORGREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FORBLACK (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED)
#define FORRED   (FOREGROUND_RED | FOREGROUND_INTENSITY)

using namespace std;

void printHeader(void);
void printCredits(void);
string readIn(int i);
string writeOut(string in);

HANDLE outputHandler = GetStdHandle(STD_OUTPUT_HANDLE);

int main(int argc, char **argv)
{

	printHeader();
	string input = "In ", output = "Out";
	string rawInput = "";
	string result = "nil";
	map<string, Element> variables;
	Element ret;
	vector<Element> rets;
	vector<Element> output_;
	vector<string> statements;

	Element a;
	a.key = "A";
	a.type = Global::_number;
	a.data = any_t(new double(5));
	variables[a.key] = a;
	Element b;
	b.key = "B";
	b.type = Global::_number;
	b.data = any_t(new double(2.5));
	variables[b.key] = b;
	vector<string> funcBody;
	funcBody.push_back("return 10;");
	vector<string> paramNames;
	paramNames.push_back("A");
	paramNames.push_back("B");
	Function foo;
	foo.key = "foo";
	foo.body = funcBody;
	foo.param_names = paramNames;
	Element func;
	func.key = foo.key;
	func.type = Global::_function;
	func.data = any_t(&foo);
	variables[func.key] = func;

	string s = "foo(3.5, 2) + A * B - 4.2";
	int c = Calculator::calculate(s, variables, rets, output_);
	for (int i = 0; i < rets.size(); i++)
	{
		cout << *(double *)rets[i].data << endl;
	}

	system("pause");

	/*
	int i = 0;
	while (true)
	{
		i++;
		SetConsoleTextAttribute(outputHandler, FORGREEN);
		cout << input << "[" << i << "]: ";
		SetConsoleTextAttribute(outputHandler, FORBLACK);
		rawInput = readIn(log10(i));
		if (rawInput == "exit")
		{
			break;
		}
		Util::split(rawInput, "\n", &statements);
		//Calculator::calculate(rawInput, variables, ret, output_);
		
		
		for (int i = 0; i < statements.size(); i++)
		{
			cout << statements[i] << endl;
		}
		
		
		result = writeOut(trim(rawInput));
		if (result != "")
		{
			SetConsoleTextAttribute(outputHandler, FORRED);
			cout << output << "[" << i << "]: ";
			SetConsoleTextAttribute(outputHandler, FORBLACK);
			if (result == "credits")
			{
				printCredits();
			}
			else
			{
				cout << result << endl;
			}
		}
		cout << endl;
	}
	*/
	return 0;
}

void printHeader(void)
{
	time_t now;
	time(&now);
	struct tm tmTmp;
	char stTmp[100];
	localtime_s(&tmTmp, &now);
	asctime_s(stTmp, &tmTmp);
	cout << "Interpretive C++ for JavaScript Console 1.0" << endl;
	cout << "ICJ 1.0 | " << stTmp;
	cout << "Type \"credits\" for mor information" << endl << endl;
	cout << "ICJ -- A lightweight tnterpreter for JavaScript powered by C++" << endl;
	cout << "ICJ is brought to you by Welly Zhang, Tianji Zheng, Qinbin Tan and Qiankun Zhang." << endl << endl;
	return;
}

void printCredits(void)
{
	cout << "We are grateful for users of CNBlog, CPlusPlus website developers, Googlers who share publicly their ideas about C++ ";
	cout << "and anybody who has ever lent a helping hand when we are developing this interactive interpreter without whom the ";
	cout << "project might take way longer to finish than expected." << endl;
	return;
}

string readIn(int i)
{
	string stmt = "";
	char c;
	bool flagSlash = 0;
	while (true)
	{
		c = getchar();
		if (c == '\n' && flagSlash)
		{
			SetConsoleTextAttribute(outputHandler, FORGREEN);
			cout << " ";
			for (int j = 0; j < i + 1; j++)
			{
				cout << " ";
			}

			cout << " " << "...: ";
			SetConsoleTextAttribute(outputHandler, FORBLACK);
			flagSlash = 0;
		}
		else if (c == '\n' && !flagSlash)
		{
			break;
		}
		else if (c == '\\')
		{
			flagSlash = 1;
			stmt += '\n';
		}
		else
		{
			stmt += c;
		}
	}
	return stmt;
}

string writeOut(string in)
{
	vector<string> stmts;
	string out;
	Util::split(in, "\n", &stmts);
	out = in;
	return out;
}

