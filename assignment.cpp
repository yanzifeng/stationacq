#include"assignment.h"
#define MAXVALUE 10000
#include <opencv/highgui.h>
#include <opencv/cv.h>

using namespace std;
void CAssignment::ReduceMatrix(CvMat *A)
{
	int n=A->width-1;
	double minValue;
	for(int i=0;i<A->height;i++)
	{
		minValue=MAXVALUE;
		for(int j=0;j<n;j++)
		{
			if(minValue>cvGetReal2D(A,i,j))
				minValue=cvGetReal2D(A,i,j);
		}
		if(minValue==0)
			continue;
		for(int j=0;j<n;j++)
		{
			double a=cvGetReal2D(A,i,j)-minValue;
			cvSetReal2D(A,i,j,a);
		}
	}
	for(int j=0;j<n;j++)
	{
		minValue=MAXVALUE;
		for(int i=0;i<A->height;i++)
		{
			if(minValue>cvGetReal2D(A,i,j))
				minValue=cvGetReal2D(A,i,j);
		}
		if(minValue==0)
			continue;
		for(int i=0;i<A->height;i++)
		{
			double a=cvGetReal2D(A,i,j)-minValue;
			cvSetReal2D(A,i,j,a);
		}
	}
	for(int i=0;i<n;i++)
	{
		int k1=n;
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(A,i,j)==0)
			{
				cvSetReal2D(A,i,k1,-(j+1));
				k1=j;
			}
		}
	}
}
void CAssignment::InitialAssign(CvMat *A, CvMat *C, CvMat *U)
{
	for(int i=0;i<C->width;i++)
		cvSetReal2D(C,0,i,0);
	for(int i=0;i<U->width;i++)
		cvSetReal2D(U,0,i,0);
	int n=A->width-1;
	CvMat *LZ=cvCreateMat(1,n,CV_32FC1);
	CvMat *NZ=cvCreateMat(1,n,CV_32FC1);
	for(int i=0;i<n;i++)
	{
		int lj=n+1;
		int j=-((int)(cvGetReal2D(A,i,lj-1)));
		while(cvGetReal2D(C,0,j-1)!=0)
		{
			lj=j;
			j=-((int)(cvGetReal2D(A,i,lj-1)));
			if(j==0)
				break;
		}
		if(j!=0)
		{
			cvSetReal2D(C,0,j-1,i+1);
			double a=cvGetReal2D(A,i,j-1);
			cvSetReal2D(A,i,lj-1,a);
			cvSetReal2D(NZ,0,i,-a);
			cvSetReal2D(LZ,0,i,lj);
			cvSetReal2D(A,i,j-1,0);
		}
		else
		{
			lj=n+1;
			j=-((int)(cvGetReal2D(A,i,lj-1)));
			while(j!=0)
			{
				int r=cvGetReal2D(C,0,j-1)-1;
				int lm=cvGetReal2D(LZ,0,r);
				int m=cvGetReal2D(NZ,0,r);
				while(m!=0)
				{
					if(cvGetReal2D(C,0,m-1)==0)
						break;
					lm=m;
					m=-((int)(cvGetReal2D(A,r,lm-1)));
				}
				if(m==0)
				{
					lj=j;j=-((int)(cvGetReal2D(A,i,lj-1)));
				}
				else
				{
					cvSetReal2D(A,r,lm-1,-j);
					double a=cvGetReal2D(A,r,m-1);
					cvSetReal2D(A,r,j-1,a);
					cvSetReal2D(NZ,0,r,-a);
					cvSetReal2D(LZ,0,r,j);
					cvSetReal2D(A,r,m-1,0);
					cvSetReal2D(C,0,m-1,r+1);
					a=cvGetReal2D(A,i,j-1);
					cvSetReal2D(A,i,lj-1,a);
					cvSetReal2D(NZ,0,i,-a);
					cvSetReal2D(LZ,0,i,lj);
					cvSetReal2D(A,i,j-1,0);
					cvSetReal2D(C,0,j-1,i+1);
					break;
				}//if(m==0)
			}
		}//if(j!=0)
	}//for
	bool *temp=new bool[n];
	for(int i=0;i<n;i++)
	{
		temp[i]=false;
	}
	for(int i=0;i<n;i++)
	{
		int k=(int)(cvGetReal2D(C,0,i))-1;
		if(k>=0)
		{
			temp[k]=true;
		}
	}
	int k1=n;
	for(int i=0;i<n;i++)
	{
		if(!temp[i])
		{
			cvSetReal2D(U,0,k1,i+1);
			k1=i;
		}
	}
}
void CAssignment::AssignNewRow(CvMat *A, CvMat *C, CvMat *LC, CvMat *LR, CvMat *U, int l, int r)
{
	int n=A->width-1;
	while(1)
	{
		cvSetReal2D(C,0,l-1,r+1);
		int m;
		for(int j=0;j<=n;j++)
		{
			if(cvGetReal2D(A,r,j)==-l)
			{
				m=j;
				break;
			}
		}
		int a=(int)(cvGetReal2D(A,r,l-1));
		cvSetReal2D(A,r,m,a);
		cvSetReal2D(A,r,l-1,0);
		if(cvGetReal2D(LR,0,r)<0)
		{
			int a=cvGetReal2D(U,0,r);
			cvSetReal2D(U,0,n,a);
			cvSetReal2D(U,0,r,0);
			return;
		}
		else
		{
			l=cvGetReal2D(LR,0,r);
			int a=cvGetReal2D(A,r,n);
			cvSetReal2D(A,r,l-1,a);
			cvSetReal2D(A,r,n,-l);
			r=cvGetReal2D(LC,0,l-1)-1;
		}
	}
}
void CAssignment::ReReduceMatrix(CvMat *A,CvMat *CH,CvMat *RH,CvMat *LC,CvMat *LR)
{
	double minValue=MAXVALUE;
	int n=A->width-1;
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(LR,0,i)!=0&&cvGetReal2D(LC,0,j)==0)
			{
				if(minValue>cvGetReal2D(A,i,j))
					minValue=cvGetReal2D(A,i,j);
			}
		}
	}
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(LR,0,i)!=0&&cvGetReal2D(LC,0,j)==0)
			{
				double a=cvGetReal2D(A,i,j);
				cvSetReal2D(A,i,j,a-minValue);
			}
		}
	}
	for(int i=0;i<n;i++)
	{
		if(cvGetReal2D(LR,0,i)==0)
			continue;
		int lj=-1;
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(LC,0,j)!=0)
				continue;
			if(cvGetReal2D(A,i,j)==0)
			{
				if(cvGetReal2D(RH,0,i)==0)
				{
					int a=cvGetReal2D(RH,0,n);
					cvSetReal2D(RH,0,i,a);
					cvSetReal2D(RH,0,n,i+1);
					cvSetReal2D(CH,0,i,j+1);
				}
				if(lj<0)
				{
					int sum=0;
					for(int k=0;k<=n;k++)
					{
						int a=(int)(cvGetReal2D(A,i,k));
						if(a<0)
						{
							sum++;
							if(cvGetReal2D(A,i,-a-1)==0)
							{
								lj=-a-1;
								break;
							}
						}
					}
					if(sum==0)
						lj=n;
				}
				cvSetReal2D(A,i,lj,-(j+1));
				lj=j;
			}
		}
	}
	for(int i=0;i<n;i++)
	{
		if(cvGetReal2D(LR,0,i)!=0)
			continue;
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(LC,0,j)==0)
				continue;
			double a=cvGetReal2D(A,i,j);
			//cvSetReal2D(A,i,j,a+minValue);
			if(cvGetReal2D(A,i,j)>0)
				continue;
			int lj=-1;
			for(int k=0;k<=n;k++)
			{
				if(cvGetReal2D(A,i,k)==-(j+1))
				{
					lj=k;
					break;
				}
			}
			if(lj>=0)
			{
				int a=cvGetReal2D(A,i,j);
				cvSetReal2D(A,i,lj,a);
				cvSetReal2D(A,i,j,0);
			}
		}
	}
	for(int i=0;i<n;i++)
	{
		if(cvGetReal2D(LR,0,i)!=0)
			continue;
		for(int j=0;j<n;j++)
		{
			if(cvGetReal2D(LC,0,j)==0)
				continue;
			double a=cvGetReal2D(A,i,j);
			cvSetReal2D(A,i,j,a+minValue);
		}
	}
}
CvMat* CAssignment::subAssign(CvMat *cost)
{
	int n=cost->width;
	CvMat *A=cvCreateMat(n,n+1,CV_32FC1);
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			double a=cvGetReal2D(cost,i,j);
			cvSetReal2D(A,i,j,a);
		}
	}
	ReduceMatrix(A);
	CvMat *C=cvCreateMat(1,n,CV_32FC1);
	CvMat *U=cvCreateMat(1,n+1,CV_32FC1);
	InitialAssign(A,C,U);
	while(cvGetReal2D(U,0,n)!=0)
	{
		CvMat *LR=cvCreateMat(1,n,CV_32FC1);
		CvMat *LC=cvCreateMat(1,n,CV_32FC1);
		CvMat *CH=cvCreateMat(1,n,CV_32FC1);
		CvMat *RH=cvCreateMat(1,n+1,CV_32FC1);
		cvZero(LR);cvZero(LC);cvZero(CH);cvZero(RH);
		cvSetReal2D(RH,0,n,-1);
		int r=cvGetReal2D(U,0,n)-1;
		int l;
		cvSetReal2D(LR,0,r,-1);
		while(1)
		{
			if(cvGetReal2D(A,r,n)!=0)
			{
				l=-((int)(cvGetReal2D(A,r,n)));
				if(cvGetReal2D(A,r,l-1)!=0&&cvGetReal2D(RH,0,r)==0)
				{
					int a=cvGetReal2D(RH,0,n);
					cvSetReal2D(RH,0,r,a);
					cvSetReal2D(RH,0,n,r+1);
					a=-((int)(cvGetReal2D(A,r,l-1)));
					cvSetReal2D(CH,0,r,a);
				}
			}
			else
			{
				if(cvGetReal2D(RH,0,n)<=0)
				{
					ReReduceMatrix(A,CH,RH,LC,LR);
				}
				r=cvGetReal2D(RH,0,n)-1;
				l=cvGetReal2D(CH,0,r);
				int a=-((int)(cvGetReal2D(A,r,l-1)));
				cvSetReal2D(CH,0,r,a);
				if(cvGetReal2D(A,r,l-1)==0)
				{
					int a=cvGetReal2D(RH,0,r);
					cvSetReal2D(RH,0,n,a);
					cvSetReal2D(RH,0,r,0);
				}
			}//if
			while(cvGetReal2D(LC,0,l-1)!=0)
			{
				if(cvGetReal2D(RH,0,r)==0)
				{
					if(cvGetReal2D(RH,0,n)<=0)
					{
						ReReduceMatrix(A,CH,RH,LC,LR);
					}
					r=cvGetReal2D(RH,0,n)-1;
				}
				l=cvGetReal2D(CH,0,r);
				int a=-((int)(cvGetReal2D(A,r,l-1)));
				cvSetReal2D(CH,0,r,a);
				if(cvGetReal2D(A,r,l-1)==0)
				{
					int a=cvGetReal2D(RH,0,r);
					cvSetReal2D(RH,0,n,a);
					cvSetReal2D(RH,0,r,0);
				}

			}//while(cvGetReal2D(LC,0,l-1)!=0)
			if(cvGetReal2D(C,0,l-1)==0)
			{
				AssignNewRow(A,C,LC,LR,U,l,r);
				break;
			}
			else
			{
				cvSetReal2D(LC,0,l-1,r+1);
				r=cvGetReal2D(C,0,l-1)-1;
				cvSetReal2D(LR,0,r,l);
			}
		}//while(1)
	}
	return C;
}
CvMat* CAssignment::Assignment(CvMat *cost1,bool outlier)
{
	CvMat *cost=cvCloneMat(cost1);
	cvNormalize(cost,cost,0,1,CV_MINMAX);
	CvMat *C;
	int n=max(cost->height,cost->width);
	CvMat *temp;
	if(outlier)
	{
		temp=cvCreateMat(n*1.25,n*1.25,CV_32FC1);
	}
	else
	{
		temp=cvCreateMat(n,n,CV_32FC1);
	}
	for(int i=0;i<cost->height;i++)
	{
		for(int j=0;j<cost->width;j++)
		{
			double a=cvGetReal2D(cost,i,j);
			cvSetReal2D(temp,i,j,a);
		}
	}
	for(int i=0;i<temp->height;i++)
	{
		for(int j=cost->width;j<temp->width;j++)
		{
			if (outlier)
			{
				cvSetReal2D(temp,i,j,0.8);
			}
			else
			{
				cvSetReal2D(temp,i,j,1.2);
			}
		}
	}
	for (int i=cost->height;i<temp->height;i++)
	{
		for (int j=0;j<temp->width;j++)
		{
			if (outlier)
			{
				cvSetReal2D(temp,i,j,0.8);
			}
			else
			{
				cvSetReal2D(temp,i,j,1.2);
			}
		}
	}
	C=subAssign(temp);
	CvMat *mat=cvCreateMat(1,cost->width,CV_32FC1);
	for(int j=0;j<cost->width;j++)
	{
		int i=cvGetReal2D(C,0,j);
		if(i>cost->height)
			cvSetReal2D(mat,0,j,0);
		else
			cvSetReal2D(mat,0,j,i);

	}
	cvReleaseMat(&C);
	return mat;
}
double CAssignment::AssignCost(CvMat *cost)
{
	CvMat *C=Assignment(cost);
	double a=CostAfterAssign(cost,C);
	cvReleaseMat(&C);
	return a;
}
double CAssignment::CostAfterAssign(CvMat *cost,CvMat *C)
{
	double sum=0;
	int a=0;
	for(int j=0;j<cost->width;j++)
	{
		int i=(int)(cvGetReal2D(C,0,j))-1;
		if(i>=0)
		{
			sum+=cvGetReal2D(cost,i,j);
			a++;
		}
	}
	sum=sum*cost->width/a;
	return sum;
}
double CAssignment::AssignCost(CvMat *cost,bool outlier)
{
	if(!outlier)
		return (AssignCost(cost));
	CvMat *C=Assignment(cost,outlier);
	double sum=CostAfterAssign(cost,C);
	cvReleaseMat(&C);
	return sum;

}
double CAssignment::GetMinCost(CvMat *cost)
{
	double sum1=0,sum2=0;
	for(int i=0;i<cost->height;i++)
	{
		double minValue=10000;
		for(int j=0;j<cost->width;j++)
		{
			if(minValue>cvGetReal2D(cost,i,j))
				minValue=cvGetReal2D(cost,i,j);
		}
		sum1+=minValue;
	}
	sum1/=cost->height;
	for(int j=0;j<cost->width;j++)
	{
		double minValue=10000;
		for(int i=0;i<cost->height;i++)
		{
			if(minValue>cvGetReal2D(cost,i,j))
				minValue=cvGetReal2D(cost,i,j);
		}
		sum2+=minValue;
	}
	sum2/=cost->width;
	double sum=0;
	if(sum1>sum2)
		sum=sum1;
	else
		sum=sum2;
	return sum;
}
