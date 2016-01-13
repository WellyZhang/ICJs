#include "ICJs_util.h"
#include "ICJs_compute.h"
#include <string>
#include <vector>
#include <sstream>

void Util::split(std::string& s, std::string delim, std::vector<std::string> *ret, bool preserveBlank)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	std::vector<std::string> temp;
	std::string tempS = "";

	while (index != std::string::npos)
	{
		temp.push_back(trim(s.substr(last, index - last)));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		temp.push_back(trim(s.substr(last, index - last)));
	}
	
	if (preserveBlank)
	{
		int blankFound = 0;
		for (int i = 0; i < temp.size(); i++)
		{
			if (Calculator::isOperator(temp[i]))
			{

				if (tempS != "")
				{
					ret->push_back(tempS);
					tempS = "";
				}
				ret->push_back(temp[i]);
			}
			else
			{
				if (temp[i] == "")
				{
					blankFound = true;
					tempS += " ";
				}
				else if (temp[i] == "\"" && blankFound)
				{
					tempS += " ";
					tempS += temp[i];
					blankFound = false;
				}
				else
					tempS += temp[i];
			}
		}
		if (tempS != "")
			ret->push_back(tempS);
	}
	else
	{
		for (int i = 0; i < temp.size(); i++)
		{
			ret->push_back(temp[i]);
		}
	}

}

std::string &Util::trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

int Util::numOfChar(std::string &s, char c)
{
	int counter = 0;
	for (int i = 0; i < s.length(); i++)
	{
		if (s.at(i) == c)
		{
			counter++;
		}
	}
	return counter;
}

std::string Util::arrayToString(std::vector<Element> ary)
{
	std::ostringstream os;
	for (int i = 0; i < ary.size(); i++)
	{
		switch (ary[i].type)
		{
		case Global::_number:
			os << *(double *)(ary[i].data);
			if (i != ary.size() - 1)
				os << ", ";
			break;
		case Global::_boolean:
			os << *(bool *)(ary[i].data);
			if (i != ary.size() - 1)
				os << ", ";
			break;
		case Global::_string:
			os << *(string *)(ary[i].data);
			if (i != ary.size() - 1)
				os << ", ";
			break;
		case Global::_array:
			os << "[";
			os << arrayToString(*(vector<Element> *)ary[i].data);
			os << "]";
			if (i != ary.size() - 1)
				os << ", ";
			break;
		}
	}
	return os.str();
}

/*
void Util::numBlankPos(std::string &s, std::vector<int> pos)
{
	int length;
	int start;
	int end;
	bool isIn = false;
	for (int i = 0; i < s.length(); i++)
	{
		if (s.at(i) == '"')
		{
			isIn = ~isIn;
		}
		if (s.at(i) == ' ' && isIn
	}
}

void Util::rmBlankInParenth(std::string &s)
{
	size_t start = s.find_first_of("(");
	if (start == std::string::npos)
	{
		return;
	}
	int counter = 1;
	int blankStart = 0, blankEnd = 0, blankLength = 0;
	bool blankFound = false;
	for (int i = start + 1; i < s.length(); i++)
	{
		if (s.at(i) == '(')
		{
			counter++;
			continue;
		}
		else if (s.at(i) == ')')
		{
			counter--;
			continue;
		}

		if (counter == 0)
		{
			continue;
		}

		if (s.at(i) == ' ' && !blankFound)
		{
			blankFound = true;
			blankStart = i;
		}
		else if (blankFound && s.at(i) != ' ')
		{
			blankFound = false;
			blankEnd = i - 1;
			blankLength = blankEnd - blankStart + 1;
			s.replace(blankStart, blankLength, "");
			i -= blankLength;
		}
	}
}
*/