#ifndef _ASSIGNMENT_
#define _ASSIGNMENT_
#include <opencv/highgui.h>

class CAssignment
{
public:
	void ReduceMatrix(CvMat *A);
	void InitialAssign(CvMat *A,CvMat *C,CvMat *U);
	void AssignNewRow(CvMat *A,CvMat *C,CvMat *LC,CvMat *LR,CvMat *U,int l,int r);
	void ReReduceMatrix(CvMat *A,CvMat *CH,CvMat *RH,CvMat *LC,CvMat *LR);
	CvMat* subAssign(CvMat *cost);
	CvMat* Assignment(CvMat *cost1,bool outlier=true);
	double AssignCost(CvMat *cost);
	double AssignCost(CvMat *cost,bool outlier);
	double CostAfterAssign(CvMat *cost,CvMat *C);
	double GetMinCost(CvMat *cost);
};
#endif
