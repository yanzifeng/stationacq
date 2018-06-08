#ifndef _CONTENT_
#define _CONTENT_
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
//#include <CommDlg.h>
#include "assignment.h"
//#include <minmax.h>
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define TRANS_XOR_CHAR	8
using namespace std;
const double SCALE=1.5;

typedef struct ReebGraph
{
	int num;////子结点的个数
	int sum;////当前结点的面积
	CvPoint2D32f center;
	CvBox2D minBBox;///minimum bounding box形状信息
	int nextSum;////后面所有结点的面积之和
	ReebGraph* ptr;////指向子结点
	int level;
	ReebGraph()
	{
		num=0;sum=0;center=cvPoint2D32f(0,0);
		nextSum=0;ptr=NULL;level=0;
		minBBox.center=cvPoint2D32f(0,0);minBBox.angle=0;minBBox.size=cvSize2D32f(0,0);
	}
}ReebGraph;
typedef struct PNode///node for partial match
{
	int sum;
	CvPoint2D32f center;
	double intensity;
	CvBox2D minBBox;
}PNode;
class CContent
{
public:
	CContent()
	{
		height=0;
		width=0;
		data=0;
		flag=0;
		RG=0;
		MRG=0;
		showImg=0;
		oriImage=0;
		centerVector.clear();
		nodeVector.clear();
		centerHeight=0;
		a[0]=0;a[1]=0;b[0]=0;b[1]=0;
	}
	~CContent()
	{
		deleteContent();
	}
	void deleteContent()
	{
		if (data!=0)
		{
			for (int i=0;i<hData;i++)
			{
				delete[] (data[i]);
			}
			delete []data;
		}
		if (flag!=0)
		{
			for (int i=0;i<height;i++)
			{
				delete[](flag[i]);
			}
			delete []flag;
		}
		if (RG!=0)
		{
			for (int i=0;i<height;i++)
			{
				for (int j=0;j<width;j++)
				{
					delete[](RG[i][j]);
				}
				delete[](RG[i]);
			}
			delete []RG;
		}
		if (MRG)
		{
			releaseLink(MRG);
			delete MRG;
		}
		if (showImg)
		{
			cvReleaseImage(&showImg);
		}
		if (oriImage)
		{
			cvReleaseImage(&oriImage);
		}
		height=0;
		width=0;
		data=0;
		flag=0;
		RG=0;
		MRG=0;
		showImg=0;
		oriImage=0;
		centerVector.clear();
		nodeVector.clear();
		centerHeight=0;
	}
	void DeleteData()
	{
		if (data!=0)
		{
			for (int i=0;i<hData;i++)
			{
				delete[] (data[i]);
			}
			delete []data;
		}
		data=0;
	}
	void releaseLink(ReebGraph *ptr)
	{
		for (int i=0;i<ptr->num;i++)
		{
			releaseLink(ptr->ptr+i);
		}
		if(ptr->num>=1)
		{
			delete [](ptr->ptr);
		}
	}
public:
    double getPointVal(CvPoint);
    bool readCSV(string str);
	IplImage* draw3DEMM();
	IplImage* GetPeakImg(IplImage* src);
	IplImage* getSpecificHeight(double sHeight);
	void createRG();//build the Reeb Graph
	void setCenters(ReebGraph *ptr);
	void createTree(ReebGraph* lHead,ReebGraph* rHead);
	static double compareRG(ReebGraph* RG1,ReebGraph* RG2);
	static void compareCenter(vector<CvPoint2D32f> &list1,vector<CvPoint2D32f> &list2);
	
	// add xukun 20130522
	// txt translator
	string fileDecoder(string fname);
	void delFile(string fname);
	//

	friend double compareRG(CContent &obj1,CContent &obj2)
	{
		ReebGraph *RG1=obj1.MRG;
		ReebGraph *RG2=obj2.MRG;
		double dis;
		dis=compareRG(RG1,RG2);
		return dis;
	}
	int computeNextSum(ReebGraph *head,int pSum);//to calculate the number of blocks
	void changeData();//删除两条线两边没有用的数据
//	friend void cvText(IplImage* img, const char* text, int x, int y);
public:
	double** data;
	IplImage *oriImage;
	int height,width;
	double maxHeight;
	ReebGraph* MRG;
	CvPoint2D32f oriPoint,endPoint;///真正的荧光波长坐标
	vector<CvPoint2D32f> centerVector;////the centers of the data///按图像xy方向的0-x 0-y的坐标
	vector<PNode> nodeVector;///node for the partial match
	double centerHeight;

	static int N;
	static double RGSum1,RGSum2;
//	ReebGraph RGHead;
protected:
	int hData;////data实际分配的高度
	bool **flag;
	int ***RG;
	ReebGraph** cMRG;//new MRG structure for comparation;
	double a[2],b[2];
	IplImage *showImg;
	CvPoint temp;/////////
protected:
	void sortCenters();///将峰的中心按从左到右，从下到上编号
	ReebGraph* nCreateRG(int level);
	static double nCompareRG(ReebGraph* leftHead,ReebGraph* rightHead);
	void setFlag(double sHeight);///判断大于sheight的块
	void setRG(int sLevel,CvPoint p,int order,int &sum,CvPoint2D32f &center,CvBox2D &minBBox);////判断各个块属于哪个点
	void getNumbers(const string line,double* (&numbers),int &num);
	void printRG(ReebGraph *ptr);
	void initNode(ReebGraph *ptr)
	{
		ptr->center=cvPoint2D32f(0,0);ptr->level=0;ptr->minBBox.center=cvPoint2D32f(0,0);
		ptr->minBBox.angle=0;ptr->minBBox.size=cvSize2D32f(0,0);ptr->nextSum=0;ptr->num=0;ptr->ptr=NULL;ptr->sum=0;
	}
	// add xukun 20131022
	// txt translator file->string
	string fileDecoderToString(string fname);
	//
};

void cvText(IplImage* img, const char* text, int x, int y);

#endif
