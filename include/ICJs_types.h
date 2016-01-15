#ifndef ICJS_TYPES_H
#define ICJS_TYPES_H

#include <string>
#include <vector>
#include <map>

typedef void * any_t;

/*
 * ȫ�ֱ�����
 * type == 0 / 1 ʱ dataָ���ָ��
 * type == 2 ʱ dataָ��string
 * type == 3 ʱ dataָ��double
 * type == 4 ʱ dataָ��boolean
 * type == 5 ʱ dataָ��һ��vector<Element>
 * type == 6 ʱ dataָ��...
 * type == 7 ʱ dataָ��һ��Functionʵ��
 */

class Global
{
	public:
		static const int _undefined = 0;
		static const int _null = 1;
		static const int _string = 2;
		static const int _number = 3;
		static const int _boolean = 4;
		static const int _array = 5;
		static const int _object = 6;
		static const int _function = 7;
		static const int _ok = 0;
		static const int _fault = -1;
};

/*
 * ������
 * ��¼�����������������뺯����
 */

class Function
{
	public:
		std::string key;
		std::vector<std::string> param_names;
		std::vector<std::string> body;
};

/* 
 * Ԫ����
 * ��¼Ԫ�������������������
 */

class Element
{
	public:
		std::string key;
		int type;
		any_t data;
		Element(){ key = ""; type = Global::_undefined; data = NULL; };
		Element(std::string _key, int _type, any_t _data=NULL){
			key = _key;
			type = _type;
			switch (type){
			case 0:case 1:data = NULL;
				break;
			case 2:data = (std::string*)_data;
				break;
			case 3:data = (double*)_data;
				break;
			case 4:data = (bool*)_data;
				break;
			case 5:data = (std::vector<Element>*)_data;
				break;
			case 6:break;
			case 7:data = (Function*)_data;
				break;
			default:data = NULL;
			}
		}
};


#endif ICJS_TYPES_H
