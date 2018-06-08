#ifndef _PARTCONTENT_
#define _PARTCONTENT_
#include "content.h"
#define  MAXCENNUM 10
typedef struct CHeightNode///the height of centers
{
	int CenNum;
	double HeightList[MAXCENNUM];
}CHeightNode;
class CPartContent:public CContent
{
public:

	static double* partialMatch(CContent &obj1,CContent &obj2,IplImage *img1,IplImage *img2,int &matchNum);
	static double* partialMatch(vector<PNode> &nodes1,vector<PNode> &nodes2);
	static double* partialMatch(CContent &obj1,CContent &obj2,int *order,int &matchNum,int &costNum);
	static double completeMatch(CContent &obj1,CContent &obj2,double &partSim,int *order,int &matchNum,double* pCost,int &costNum);
	////p regression model
	////valList the list of the values
	///N the size of the valList, p of the p regressin model
	///minVal of Xnow, maxVal of Xnow
	void GetMinMaxVal(double* valList,int N,int p,double &minVal,double &maxVal,double &meanVal);
};
#endif