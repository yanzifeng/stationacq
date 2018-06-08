
#include "StringDataReader.h"
// add xukun 20131022
// txt translator file->string

CStringDataReader::CStringDataReader(string strData)
	: data(strData)
{
}
CStringDataReader::~CStringDataReader(void)
{
}

bool CStringDataReader::endOfData()
{
	return (data.length()==0 || (data.length()==1&&data.front()==0x02));
}

string CStringDataReader::getLine()
{
	string dataLine("");
	char ch = '0';
	int count = 0;
	while(1)
	{
		ch = data[count++];
		if(ch == '\r' || ch == '\n')
		{
			data.erase(0, count+1);
			return dataLine;
		}
		dataLine += ch;
	}	
}
