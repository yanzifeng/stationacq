#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
// add xukun 20131022
// txt translator file->string
using namespace std;
class CStringDataReader
{
public:
	CStringDataReader(string strData);
	~CStringDataReader(void);

	bool endOfData();
	string getLine();

private:
	string data;
};

