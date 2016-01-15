#include "ICJs_util.h"
#include "ICJs_compute.h"
#include <string>
#include <vector>
#include <sstream>

/*
 * split函数
 * 输入字符串，分隔符，返回向量地址及一个布尔类型参数
 * 根据分隔符将字符分分割并放入向量中
 * 布尔确定是否需要保存字符串中的空格
 */

void Util::split(std::string& s, std::string delim, std::vector<std::string> *ret, bool preserveBlank)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	std::vector<std::string> temp1, temp;
	std::string tempS = "";

	while (index != std::string::npos)
	{
		temp1.push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		temp1.push_back(s.substr(last, index - last));
	}
	
	std::string toBeFused = "";
	size_t quoteIdx;
	int numOfQuote = 0;

	bool isSep = false;

	for (int i = 0; i < temp1.size(); i++)
	{
		numOfQuote = Util::numOfChar(temp1[i], '\"');
		if (numOfQuote % 2 == 0 && isSep)
		{
			toBeFused += delim;
			toBeFused += temp1[i];
		}
		else if (numOfQuote % 2 == 0 && !isSep)
			temp.push_back(temp1[i]);
		else if (numOfQuote % 2 != 0 && isSep)
		{
			toBeFused += delim;
			toBeFused += temp1[i];
			temp.push_back(toBeFused);
			toBeFused = "";
			isSep = false;
		}
		else
		{
			isSep = true;
			toBeFused += temp1[i];
		}
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

/*
 * trim函数
 * 输入一个字符串
 * 修正字符串
 * 将头尾的空格去掉
 */

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

/*
 * numOfChar函数
 * 输入字符串和一个字符
 * 统计字符串中字符出现的个数
 */

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

/*
 * arrayToString函数
 * 输入一个元素向量
 * 将这个元素向量转化为字符串输出
 */
 
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