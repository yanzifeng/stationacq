#ifndef _DATACONTENT_
#define _DATACONTENT_
#include "PartContent.h"

class CDataContent:public CPartContent
{
public:
	////for the database
	///get the value of point p
	//double getPointVal(CvPoint);
	bool readCSV(string str);
	void loadstandparams();
//	bool readCSV(string str,double factor);
    bool readTXT(string oriData);////
	IplImage* show3DEMM(int num);///ÓÃÓÚÏÔÊ¾
	void createRG(string reebData);
	void createRG(){CContent::createRG();};
	void createSubRG(string &str,ReebGraph *head);
	void createSubRG(istringstream &input,ReebGraph *head);
    static bool getData(string path,string &oriData,string &reeData,string &centerData);

	// add xukun 20131022
	// txt translator file->string
	bool readCSVString(string str);
	//
private:
	void printRGStr(ReebGraph *ptr);
	////for the MFC
	string oriStr,reebStr,centerStr;
	double _factor;
    bool _selected;
};
#endif
