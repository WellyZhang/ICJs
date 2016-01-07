#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include <math.h>
#include <map> 
#include <vector>
#include <iostream>


#define FORGREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FORBLACK (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED)
#define FORRED   (FOREGROUND_RED | FOREGROUND_INTENSITY)

using namespace std;

void printHeader(void);
void printCredits(void);
string readIn(int i);
string writeOut(string in);
string &trim(string &in);
void split(string& s, string delim, vector<string> *ret);

HANDLE outputHandler = GetStdHandle(STD_OUTPUT_HANDLE);

int main(int argc, char **argv)
{

	printHeader();
	string input = "In ", output = "Out";
	string rawInput = "";
	string result = "nil";

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
	split(in, "\n", &stmts);
	out = in;
	return out;
}

string &trim(string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

void split(string& s, string delim, vector<string> *ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != string::npos)
	{
		ret->push_back(trim(s.substr(last, index - last)));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret->push_back(trim(s.substr(last, index - last)));
	}
}

