#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include <math.h>
#include <map> 
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ICJs_compute.h"
#include "ICJs_types.h"
#include "ICJs_util.h"
#include "ICJs_parser.h"


#define FORGREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FORBLACK (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED)
#define FORRED   (FOREGROUND_RED | FOREGROUND_INTENSITY)

using namespace std;

void printHeader(void);
void printCredits(void);
string readIn(int i);
string writeOut(string in);
void print(vector<Element> out, bool isrecursion);

HANDLE outputHandler = GetStdHandle(STD_OUTPUT_HANDLE);

int main(int argc, char **argv)
{

	printHeader();
	char buf[256];
	string input = "In ", output = "Out";
	string rawInput = "";
	string result = "nil";
	map<string, Element> variables;
	Element ret;
	vector<Element> rets;
	vector<Element> output_;
	vector<string> statements;

	int i = 0;
	while (true)
	{
		output_.clear();
		i++;
		SetConsoleTextAttribute(outputHandler, FORGREEN);
		cout << input << "[" << i << "]: ";
		SetConsoleTextAttribute(outputHandler, FORBLACK);
		rawInput = readIn(int(log10(i)));
		
		//退出语句
		if (rawInput.find("exit") != string::npos)
		{
			break;
		}
		Util::split(rawInput, "\n", &statements, false);
		//加载文件
		if (rawInput.find("%load") != string::npos){
			statements.clear();
			string oper, filename;
			istringstream is(rawInput);
			is >> oper >> filename;
			ifstream file;
			file.open(filename);
			while (file.getline(buf, 256)){
				statements.push_back(string(buf));
			}
			file.close();
		}

		int error = Parser::parse(statements, variables, output_);

		//result = writeOut(Util::trim(output_));
		
		if (rawInput == "credits" || !output_.empty() || error != Global::_ok){
			SetConsoleTextAttribute(outputHandler, FORRED);
			cout << output << "[" << i << "]: ";
			SetConsoleTextAttribute(outputHandler, FORBLACK);
			//I/O输出
			print(output_, false);
			//输出credits
			if (rawInput == "credits")
			{
				printCredits();
				continue;
			}
			if (error != Global::_ok)
				cout << "Error!";
			cout << endl;
		}

		
		statements.clear();
		cout << endl;
	}
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
	cout << "ICJs 1.0 | " << stTmp;
	cout << "Type \"credits\" for mor information" << endl << endl;
	cout << "ICJs -- A lightweight interpreter for JavaScript powered by C++" << endl;
	cout << "ICJs is brought to you by Welly Zhang, Tianji Zheng, Qinbin Tan and Qiankun Zhang." << endl << endl;
	return;
}

void printCredits(void)
{
	cout << "We are grateful for users of CNBlog, CPlusPlus website developers, Googlers who share publicly their ideas about C++ ";
	cout << "and anybody who has ever lent a helping hand when we are developing this interactive interpreter, without whom the ";
	cout << "project might take way longer to finish than expected." << endl;
	cout << endl;
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
	Util::split(in, "\n", &stmts, false);
	out = in;
	return out;
}

void print(vector<Element> rets, bool isrecursion)
{
	for (int i = 0; i < rets.size(); i++)
	{
		switch (rets[i].type)
		{
		case Global::_number:
			cout << *(double *)(rets[i].data);
			if (isrecursion && i != rets.size() - 1)
				cout << ", ";
			break;
		case Global::_boolean:
			if (*(bool *)(rets[i].data))
				cout << "true";
			else
				cout << "false";
			if (isrecursion && i != rets.size() - 1)
				cout << ", ";
			break;
		case Global::_string:
			cout << "\"" <<*(string *)(rets[i].data) << "\"";
			if (isrecursion && i != rets.size() - 1)
				cout << ", ";
			break;
		case Global::_array:
			cout << "[";
			print(*(vector<Element> *)rets[i].data, true);
			cout << "]";
			if (isrecursion && i != rets.size() - 1)
				cout << ", ";
			break;
		}
		if (!isrecursion)
			cout << endl;
	}
}