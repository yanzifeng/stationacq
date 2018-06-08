
#include "content.h"
#include <vector>

void CContent::createRG()
{
	int maxLevel=8;
	ReebGraph *head=nCreateRG(maxLevel);
	int sum=computeNextSum(head,0);
// 	head->sum=sum;
// 	head->nextSum=sum;
	if (head->num>1)
	{
		int pos=0;
		int maxSum=0;
		for (int i=0;i<head->num;i++)
		{
			ReebGraph *temp=head->ptr+i;
			if (temp->nextSum>maxSum)
			{
				maxSum=temp->nextSum;
				pos=i;
			}
		}
		ReebGraph *temp=new ReebGraph;
		for (int i=0;i<head->num;i++)
		{
			if (i==pos)
			{
				temp->num=(head->ptr+i)->num;
				temp->sum=(head->ptr+i)->sum;
				temp->center=(head->ptr+i)->center;
				temp->minBBox=(head->ptr+i)->minBBox;
				temp->nextSum=(head->ptr+i)->nextSum;
				temp->ptr=(head->ptr+i)->ptr;
				temp->level=(head->ptr+i)->level;
			}
			else
				releaseLink(head->ptr+i);
		}
		delete [](head->ptr);
		head->ptr=temp;
		head->num=1;
		head->sum=temp->sum;
		head->nextSum=head->sum;
	}

 //	ofstream output("E:\\output.txt",ofstream::app);
 //	output<<"-------------output----------"<<endl;
// 	output<<"sum="<<sum<<endl;
// 	output<<a[0]<<" "<<b[0]<<" "<<a[1]<<" "<<b[1]<<endl;
	MRG=new ReebGraph;
	initNode(MRG);
	MRG->num=head->num;
	MRG->ptr=new ReebGraph[MRG->num];
	(MRG->ptr)->sum=0;
	createTree(head->ptr,MRG->ptr);
/*	output<<"after createTree"<<endl;*/

	printRG(MRG);
//	output.close();

	releaseLink(head);
	delete head;
/*	output<<"after releaselink"<<endl;*/
	MRG->sum=(MRG->ptr)->sum+(MRG->ptr)->nextSum;
	MRG->nextSum=MRG->sum;
	MRG->center=(MRG->ptr)->center;
	MRG->level=(MRG->ptr)->level;

	setCenters(MRG);
/*	output<<"after setCenters"<<endl;*/
	sortCenters();
// 	output<<"after sortCenters"<<endl;
// 	output.close();

//	printRG(MRG);
//	cvSaveImage("D:\\showImg.jpg",showImg);
//	cvNamedWindow("show");
//	cvShowImage("show",showImg);
//	cvWaitKey(0);
//	printRG(head);
//	printf("*********************\n");
}
void CContent::createTree(ReebGraph* lHead,ReebGraph* rHead)
{
	CvPoint2D32f p=lHead->center;
/*	double deta1=1000,deta2=1000;
	if (a[0]!=0||b[0]!=0)
	{
		deta1=fabs(p.y-a[0]*p.x-b[0]);
	}
	if (a[1]!=0||b[1]!=0)
	{
		deta2=fabs(p.y-a[1]*p.x-b[1]);
	}
	if (deta1<5||deta2<5)
	{
		rHead->sum=0;
		return;
	}*/
	if (lHead->num==0)
	{
		rHead->num=0;
		rHead->ptr=0;
		rHead->sum+=lHead->sum;
		rHead->nextSum=0;
		rHead->center=lHead->center;
		rHead->level=0;
		rHead->minBBox=lHead->minBBox;
	}
	else if (lHead->num==1)
	{
		createTree(lHead->ptr,rHead);
		if (rHead->sum!=0)
		{
			rHead->sum+=lHead->sum;
		}
		///如果该峰超量程，则用顶端的minBBox
		int x,y;x=cvFloor(rHead->center.x);y=cvFloor(rHead->center.y);
		double value=data[y][x];
		if (value<9000)
		{
			rHead->minBBox=lHead->minBBox;///
		}
	}
	else if(lHead->num>1)
	{
		rHead->sum+=lHead->sum;
		rHead->center=lHead->center;
		rHead->nextSum=0;
		rHead->num=0;
		rHead->ptr=new ReebGraph[lHead->num];
		rHead->level=0;
		rHead->minBBox=lHead->minBBox;
		for (int i=0;i<lHead->num;i++)
		{
			int k=rHead->num;
			(rHead->ptr+k)->sum=0;
			createTree(lHead->ptr+i,rHead->ptr+k);
			if ((rHead->ptr+k)->sum>0)
			{
				rHead->num+=1;
				rHead->nextSum+=(rHead->ptr+k)->sum+(rHead->ptr+k)->nextSum;
				if (rHead->level<((rHead->ptr+k)->level+1))
				{
					rHead->level=((rHead->ptr+k)->level+1);
				}
			}
		}
		if (rHead->num==1)
		{
			ReebGraph *ptr=rHead->ptr;
			rHead->center=ptr->center;
			rHead->nextSum=ptr->nextSum;
			rHead->num=ptr->num;
			rHead->ptr=ptr->ptr;
			rHead->sum+=ptr->sum;
			rHead->level=ptr->level;
			//rHead->minBBox=ptr->minBBox;///
			int x,y;x=cvFloor(rHead->center.x);y=cvFloor(rHead->center.y);
			double value=data[y][x];
			///如果该峰超量程，则用顶端的minBBox
			if (value>9000)
			{
				rHead->minBBox=lHead->minBBox;///
			}
			delete []ptr;
		}
	}
}
double CContent::compareRG(ReebGraph* RG1,ReebGraph* RG2)
{
	RGSum1=RG1->sum;
	RGSum2=RG2->sum;
	RG1=RG1->ptr;RG2=RG2->ptr;
	if (RG1->level>RG2->level)
	{
		ReebGraph *temp;
		temp=RG1;RG1=RG2;RG2=temp;
	}
	bool flag=true;
	{
		CvPoint2D32f cen1,cen2;
		cen1=RG1->center;
		cen2=RG2->center;
		double d1,d2;
		d1=fabs(cen1.x-cen2.x);
		d2=fabs(cen1.y-cen2.y);
		if (d1<=2&&d2<=2)
		{
			flag=false;
		}
	}
	CvPoint2D32f oCenter;
	////
	if (flag)
	{
		CvPoint2D32f pCenter=RG2->center;///////
		while (RG1->level<RG2->level)
		{
			pCenter=RG2->center;///////
			CvPoint2D32f p1=RG1->center;
			CvPoint2D32f p2;
			double d=100000;
			int pos=-1;
			for (int j=0;j<RG2->num;j++)
			{
				p2=(RG2->ptr+j)->center;
				double d1=0;
				d1=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
				d1=sqrt(d1);
				if (d>d1)
				{
					d=d1;pos=j;
				}
			}
			RG2=(RG2->ptr+pos);
		}
		oCenter=RG2->center;
		/////修订
		{
			double d1,d2;
			CvPoint2D32f p1=RG1->center;
			CvPoint2D32f p2=RG2->center;
			d1=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
			d1=sqrt(d1);
			p2=pCenter;
			d2=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
			d2=sqrt(d2);
			if (d1>d2)
			{
				RG2->center=pCenter;
			}
		}
	}
	double sumCost=0;
	sumCost=nCompareRG(RG1,RG2);
	if(flag)
		RG2->center=oCenter;
	return sumCost;
}
double CContent::nCompareRG(ReebGraph* leftHead,ReebGraph* rightHead)
{
// 	ofstream output("D:\\temp.txt",ofstream::app);
// 	output<<"*****************"<<endl;
// 	static int f=1;
// 	if (f==1)
// 	{
// 		output<<RGSum1<<" "<<RGSum2<<endl;
// 		f++;
// 	}
	double cost=0;
	if (leftHead->num==0||rightHead->num==0)
	{
		double a,b;
		a=leftHead->sum;
		b=rightHead->sum;
	/*	if (leftHead->num==0)
		{
			a=leftHead->sum;
		}
		else
		{
			a=leftHead->nextSum+leftHead->sum;
		}
		if (rightHead->num==0)
		{
			b=rightHead->sum;
		}
		else
		{
			b=rightHead->nextSum+rightHead->sum;
		}*/
		CvPoint2D32f p1=leftHead->center;
		CvPoint2D32f p2=rightHead->center;
		double d1,d2;
		d1=sqrt(p1.x*p1.x+p1.y*p1.y);
		d2=sqrt(p2.x*p2.x+p2.y*p2.y);
		if (d1>d2)
		{
			double t;
			t=d1;d1=d2;d2=t;
		}
	//	output<<a<<" "<<b<<" "<<(d1/d2)<<endl;
#if 0
		cost=(d1/d2)*((a+b)/(RGSum1+RGSum2));
#else
		double d=0;
		double detaX,detaY;
		double scale=5;
		detaX=fabs(p1.x-p2.x);
		detaY=fabs(p1.y-p2.y);
		if (detaX<=2&&detaY<=2)
		{
			scale=40;
		}
		d=sqrt(detaX*detaX+detaY*detaY)/scale;
		cost=exp(-d)*((a+b)/(RGSum1+RGSum2));
#endif
		return cost;
	}
	else
	{
		CvPoint2D32f p1=leftHead->center;
		CvPoint2D32f p2=rightHead->center;
		double d1,d2;
		d1=sqrt(p1.x*p1.x+p1.y*p1.y);
		d2=sqrt(p2.x*p2.x+p2.y*p2.y);
		if (d1>d2)
		{
			double t;
			t=d1;d1=d2;d2=t;
		}
		double a,b;
		a=leftHead->sum;
		b=rightHead->sum;
	//	output<<a<<" "<<b<<" "<<(d1/d2)<<endl;
#if 0
		cost=(d1/d2)*((a+b)/(RGSum1+RGSum2));
#else
		double d=0;
		double detaX,detaY;
		double scale=20;
		detaX=fabs(p1.x-p2.x);
		detaY=fabs(p1.y-p2.y);
		if (detaX<=2&&detaY<=2)
		{
			scale=40;
		}
		d=sqrt(detaX*detaX+detaY*detaY)/scale;
	//	d=sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y))/10;
	//	d=sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y))/5;
		cost=exp(-d)*((a+b)/(RGSum1+RGSum2));
#endif
		
	}
	bool *flag=new bool[rightHead->num];
	for (int i=0;i<rightHead->num;i++)
	{
		flag[i]=false;
	}
	CvMat *costMat=cvCreateMat(leftHead->num,rightHead->num,CV_32FC1);
	for (int i=0;i<leftHead->num;i++)
	{
		CvPoint2D32f p1=(leftHead->ptr+i)->center;
		CvPoint2D32f p2;
		for (int j=0;j<rightHead->num;j++)
		{
			if (flag[j])
			{
				continue;
			}
			p2=(rightHead->ptr+j)->center;
			double d1=0;
			d1=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
			d1=sqrt(d1);
			cvSetReal2D(costMat,i,j,d1);
		}
	}
	CAssignment ass;
	CvMat *match=ass.Assignment(costMat,false);
	for (int j=0;j<match->width;j++)
	{
		int i=cvGetReal2D(match,0,j)-1;
		if (i<0)
		{
			continue;
		}
		cost+=nCompareRG(leftHead->ptr+i,rightHead->ptr+j);
	}
	delete []flag;
	return cost;
}
int CContent::computeNextSum(ReebGraph *head,int pSum)
{
#if 1
	head->sum=1;//////
	if (head->num==0)
	{
		head->nextSum=0;
		return head->sum;
	}
	int sum=0;
	for (int i=0;i<head->num;i++)
	{
		int sumTemp=computeNextSum(head->ptr+i,pSum+head->sum);
		if (head->num>=2&&((sumTemp+pSum)<=6))
		{
			releaseLink(head->ptr+i);
			for (int j=i;j<head->num-1;j++)
			{
				head->ptr[j].center=head->ptr[j+1].center;
				head->ptr[j].level=head->ptr[j+1].level;
				head->ptr[j].nextSum=head->ptr[j+1].nextSum;
				head->ptr[j].num=head->ptr[j+1].num;
				head->ptr[j].ptr=head->ptr[j+1].ptr;
				head->ptr[j].sum=head->ptr[j+1].sum;
			}
			head->num=head->num-1;
			i--;
			continue;
		}
		sum+=sumTemp;
	}
	head->nextSum=sum;
	sum+=head->sum;
	return sum;
#else
	head->sum=1;//////
	if (head->num==0)
	{
		head->nextSum=0;
		return head->sum;
	}
	int sum=0;
	for (int i=0;i<head->num;i++)
	{
		sum+=computeNextSum(head->ptr+i);
	}
	head->nextSum=sum;
	sum+=head->sum;
	return sum;
#endif
}
ReebGraph* CContent::nCreateRG(int level)
{
	ReebGraph *RGHead=new ReebGraph;
	int K=pow(2.0,level);
	flag=new bool*[height];
	RG=new int**[height];
	for (int i=0;i<height;i++)
	{
		flag[i]=new bool[width];
		RG[i]=new int*[width];
	}
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			RG[i][j]=new int[K];
			for (int k=0;k<K;k++)
			{
				RG[i][j][k]=-1;
			}
		}
	}
	int threshold=30;
	RGHead->sum=0;
	RGHead->num=0;
	for(int k=0;k<K;k++)
	{
		double sHeight;
		if (k==0)
			sHeight=pow(10.0,-4);
		else
			sHeight=1.0*k*maxHeight/K;
		////debug
//		sHeight=12*maxHeight/K;//977
		///
		setFlag(sHeight);///check the areas with the value bigger than sHeight
// 		{
// 			ofstream output;output.open("D:\\flag.txt");
// 			for (int i=0;i<height-1;i++)
// 			{
// 				for (int j=0;j<width-1;j++)
// 				{
// 					if (flag[i][j])
// 					{
// 						output<<"1";
// 					}
// 					else
// 						output<<"0";
// 				}
// 				output<<endl;
// 			}
// 			output.close();
// 		}
		int fNum=0;
		for (int i=0;i<height-1;i++)
		{
			for (int j=0;j<width-1;j++)
			{
				if (flag[i][j])
				{
					fNum++;
				}
			}
		}
//		printf("fNum=%d\n",fNum);
		vector<CvPoint> vecP;
		vector<CvPoint2D32f> vecC;
		vector<int> vecO;
		vector<int>vecS;
		vector<CvBox2D> vecB;
		int order=0;
		for (int i=0;i<height-1;i++)
		{
			for (int j=0;j<width-1;j++)
			{
				int sum;
				if (flag[i][j])
				{
					CvPoint2D32f center;
					CvBox2D minBBox;
					setRG(k,cvPoint(j,i),order,sum,center,minBBox);
					if (sum>threshold)
					{
						vecP.push_back(cvPoint(j,i));
						vecC.push_back(center);
						vecO.push_back(order);
						vecS.push_back(sum);
						vecB.push_back(minBBox);
						order++;
					}
		//			printf("sum=%d",sum);
				}
			}
		}
	//	printf("\norder=%d\n",order);
		int correct[100];
		for (int i=0;i<order;i++)
		{
			CvPoint pt;int sum;CvPoint2D32f center;CvBox2D minBBox;
			pt = vecP[i];sum = vecS[i]; center = vecC[i];minBBox = vecB[i];
			ReebGraph* ptrRG=RGHead;
			for (int l=0;l<k;l++)
			{
				int m=RG[pt.y][pt.x][l];
				ptrRG=(ptrRG->ptr+m);
			//	printf("m=%d\n",m);
			}
			if (ptrRG->num==0)
			{
				ptrRG->ptr=new ReebGraph[order-i];
			}
			int index=ptrRG->num;
			((ptrRG->ptr)[index]).num=0;
			((ptrRG->ptr)[index]).sum=sum;
			((ptrRG->ptr)[index]).center=center;
			((ptrRG->ptr)[index]).level=k;
			((ptrRG->ptr)[index]).minBBox=minBBox;
			correct[i]=index;
			ptrRG->num+=1;
		//	printf("%d,%d,sum=%d\n",ptrRG->num,i,((ptrRG->ptr)[index]).sum);
		}
		for (int i=0;i<height-1;i++)
		{
			for (int j=0;j<width-1;j++)
			{
				int orNum=RG[i][j][k];////order num;
				if (orNum>=0&&orNum<order)
				{
					RG[i][j][k]=correct[orNum];
				}else
				{
					RG[i][j][k]=-1;
				}
			}
		}
//		printf("****k=%d****\n",k);
//		cvWaitKey(0);
	}
//	printRG(RGHead);
	if (flag!=0)
	{
		for (int i=0;i<height;i++)
		{
			delete[](flag[i]);
		}
		delete []flag;
	}
	flag=0;
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
	RG=0;
	return RGHead;
}
void CContent::setFlag(double sHeight)
{
#if 1
	for (int i=0;i<height-1;i++)
	{
		for (int j=0;j<width-1;j++)
		{
			if(data[i][j]<sHeight)
				flag[i][j]=false;
			else
				flag[i][j]=true;
		}
	}
#else
	for (int i=0;i<height-1;i++)
	{
		for (int j=0;j<width-1;j++)
		{
			bool bigger=true;
			int detaX[4]={0,1,1,0};
			int detaY[4]={0,0,1,1};
			for (int k=0;k<4;k++)
			{
				int kx,ky;
				kx=j+detaX[k];
				ky=i+detaY[k];
				if (data[ky][kx]<sHeight)
				{
					bigger=false;
				}
			}
			flag[i][j]=bigger;
		}
	}
#endif
}
void CContent::setRG(int sLevel,CvPoint p,int order,int &sum,CvPoint2D32f &center,CvBox2D &minBBox)
{
	if (!flag[p.y][p.x])
	{
		sum=0;
		return;
	}
	sum=0;center=cvPoint2D32f(0,0);minBBox.center=cvPoint2D32f(0,0);minBBox.angle=0;minBBox.size=cvSize2D32f(0,0);
	vector<CvPoint> temp;
	temp.push_back(p);
	CvPoint nowP;
	CvPoint2D32f sumP=cvPoint2D32f(0.0,0.0);
	//for the minimum bounding box
	//CvPoint *points=(CvPoint*)malloc((8*height*width)*sizeof(CvPoint)) ;
	CvPoint *points=new CvPoint[8*height*width];

	//if(!points)return;
    uint32_t pNum=0;
	while (!temp.empty())
	{
		nowP=temp.back();
		temp.pop_back();
		sumP.x+=nowP.x;
		sumP.y+=nowP.y;
		flag[nowP.y][nowP.x]=false;
		RG[nowP.y][nowP.x][sLevel]=order;
		sum++;
		int detaX[8]={-1,0,1,-1,1,-1,0,1};
		int detaY[8]={-1,-1,-1,0,0,1,1,1};
		for (int k=0;k<8;k++)
		{
			int kx,ky;
			kx=nowP.x+detaX[k];
			ky=nowP.y+detaY[k];
			if (kx<0||ky<0||kx>=width-1||ky>=height-1||!flag[ky][kx])
			{
				continue;
			}
			temp.push_back(cvPoint(kx,ky));
			points[pNum++]=cvPoint(kx,ky);
		}

	}
	if(pNum>5)
	{
		CvMat pointMat=cvMat(1,pNum,CV_32SC2,points);
		minBBox=cvMinAreaRect2( &pointMat, 0);
		double h,w;
		h=minBBox.size.height;w=minBBox.size.width;
	}
	//free(points);
	delete []points;
	center.x=sumP.x/sum;
	center.y=sumP.y/sum;
}
IplImage* CContent::getSpecificHeight(double sHeight)
{
	int h=640,w=h;////w====h
	IplImage* img=cvCreateImage(cvSize(h,w),IPL_DEPTH_8U,1);///本应是cvSize(w,h)，但后面要将图片旋转
	cvSet(img,cvScalarAll(255));
	IplImage* temp=cvCreateImage(cvSize(h,w),IPL_DEPTH_8U,1);
	cvSet(temp,cvScalarAll(255));
	for (int i=0;i<height-1;i++)
	{
		for (int j=0;j<width-1;j++)
		{
			CvPoint2D64d* points=new CvPoint2D64d[4];
			int num1=0;
			double a,b;
			int ax[4]={0,0,0,1};
			int ay[4]={0,0,1,0};
			int bx[4]={1,0,1,1};
			int by[4]={0,1,1,1};
			int flag[4];
			for (int k=0;k<4;k++)
			{
				int ayk=i+ay[k];
				int axk=j+ax[k];
				int byk=i+by[k];
				int bxk=j+bx[k];
				a=data[ayk][axk];b=data[byk][bxk];
				if (fabs(a-b)<pow(10.0,-4))
				{
					continue;
				}
				if(((a<b)&&(sHeight>=a)&&(sHeight<=b))||((a>b)&&(sHeight>=b)&&(sHeight<=a)))
				{
					if (axk!=bxk)
					{
						points[num1].x=(sHeight-a)/(b-a)+axk;
						points[num1].y=ayk;
					}
					else
					{
						points[num1].x=axk;
						points[num1].y=(sHeight-a)/(b-a)+ayk;
					}
					flag[num1]=k;
					num1++;
				}
			}
			for (int k1=0;k1<num1-1;k1++)
			{
				for (int k2=k1+1;k2<num1;k2++)
				{
					CvPoint p1,p2;
					p1.x=cvFloor(points[k1].y*h/(height-1));
					p1.y=cvFloor(w-points[k1].x*w/(width-1));
					p2.x=cvFloor(points[k2].y*h/(height-1));
					p2.y=cvFloor(w-points[k2].x*w/(width-1));
					cvLine(img,p1,p2,cvScalarAll(0));
				}
			}
			delete []points;
		}
	}//for
	for (int i=0;i<height-1;i++)
	{
		for (int j=0;j<width-1;j++)
		{
		/*	if (data[i][j]>=sHeight)
			{
				int detaX[4]={-1,0,0,-1};
				int detaY[4]={-1,-1,0,0};
				for (int k=0;k<4;k++)
				{
					int kx,ky;
					kx=j+detaX[k];
					ky=i+detaY[k];
					if (kx>=0&&ky>=0)
					{
						CvPoint p1,p2;
						p1.x=ky*h/(height-1);
						p1.y=w-kx*w/(width-1);
						p2.x=(ky+1)*h/(height-1);
						p2.y=w-(kx+1)*w/(width-1);
						cvRectangle(temp,p1,p2,cvScalarAll(0));
					}
				}
			}*/
			bool bigger=true;
			int detaX[4]={0,1,1,0};
			int detaY[4]={0,0,1,1};
			for (int k=0;k<4;k++)
			{
				int kx,ky;
				kx=j+detaX[k];
				ky=i+detaY[k];
				if (data[ky][kx]<sHeight)
				{
					bigger=false;
				}
			}
			if (bigger)
			{
				CvPoint p1,p2;
				p1.x=i*h/(height-1);
				p1.y=w-j*w/(width-1);
				p2.x=(i+1)*h/(height-1);
				p2.y=w-(j+1)*w/(width-1);
				cvRectangle(temp,p1,p2,cvScalarAll(0));
			}
		/*	double minValue,maxValue;
			minValue=data[i][j];maxValue=data[i][j];
			int detaX[3]={1,1,0};
			int detaY[3]={0,1,1};
			for (int k=0;k<3;k++)
			{
				int kx,ky;
				kx=j+detaX[k];
				ky=i+detaY[k];
				if (minValue>data[ky][kx])
				{
					minValue=data[ky][kx];
				}
				if (maxValue<data[ky][kx])
				{
					maxValue=data[ky][kx];
				}
			}
			if (sHeight<maxValue)
			{
				CvPoint p1,p2;
				p1.x=i*h/(height-1);
				p1.y=w-j*w/(width-1);
				p2.x=(i+1)*h/(height-1);
				p2.y=w-(j+1)*w/(width-1);
				cvRectangle(temp,p1,p2,cvScalarAll(0));
			}*/
		}
	}
//	cvNamedWindow("hahah");
//	cvShowImage("hahah",temp);
	cvReleaseImage(&temp);
	return img;
}
IplImage* CContent::draw3DEMM()
{
//	int h=640,w=640;
	int h,w;
	h=cvFloor(SCALE*(endPoint.y-oriPoint.y));
	w=cvFloor(SCALE*(endPoint.x-oriPoint.x));
	IplImage* img=cvCreateImage(cvSize(h,w),IPL_DEPTH_8U,1);///本应是cvSize(w,h)，但后面要将图片旋转
	cvSet(img,cvScalarAll(255));
	const int num=50;/////分为num 个 level
	for (int i=0;i<height-1;i++)
	{
		for (int j=0;j<width-1;j++)
		{
			CvPoint3D64d* points=new CvPoint3D64d[4*num];
			int num1=0;
			double a,b,c,d;
			int ax[4]={0,0,0,1};
			int ay[4]={0,0,1,0};
			int bx[4]={1,0,1,1};
			int by[4]={0,1,1,1};
			for (int k=0;k<4;k++)
			{
				int ayk=i+ay[k];
				int axk=j+ax[k];
				int byk=i+by[k];
				int bxk=j+bx[k];
				a=data[ayk][axk];b=data[byk][bxk];
				c=a;d=b;
				if ((a==0&&b==0)||(a>=maxHeight&&b>=maxHeight)||fabs(c-d)<0.2)
				{
					continue;
				}
				if (a>b)
				{
					double temp;
					temp=a;a=b;b=temp;
				}
#if 0
				int t=a*num/maxHeight;
				while (t*maxHeight/num<=b)
				{
					if (t!=0&&t*maxHeight/num>=a)
					{
						double temp=t*maxHeight/num;
						if ((temp-c)/(d-c)<0||(temp-c)/(d-c)>1)
						{
							cout<<"error"<<endl;
						}
						if (axk!=bxk)
						{
							points[num1].x=(temp-c)/(d-c)+axk;
							points[num1].y=ayk;
						}
						else
						{
							points[num1].x=axk;
							points[num1].y=(temp-c)/(d-c)+ayk;
						}
						points[num1].z=t;
						num1++;
						if (t>num||num1>4*num)
						{
							cout<<t<<endl;
							cout<<"error"<<endl;
						}
					}
					t++;
				}///while
#else
				int t1=0,t2=0;
				t1=cvCeil(a*num/maxHeight);
				t2=cvFloor(b*num/maxHeight);
				t2=min(t2,num-1);
				for (int t=t1;t<=t2;t++)
				{
					double temp=t*maxHeight/num;
					if (axk!=bxk)
					{
						points[num1].x=(temp-c)/(d-c)+axk;
						points[num1].y=ayk;
					}
					else
					{
						points[num1].x=axk;
						points[num1].y=(temp-c)/(d-c)+ayk;
					}
					points[num1].z=t;
					num1++;
				}
#endif
			}
			for (int k1=0;k1<num1-1;k1++)
			{
				for (int k2=k1+1;k2<num1;k2++)
				{
					if (points[k2].z==points[k1].z)
					{
						CvPoint p1,p2;
						p1.x=cvFloor(points[k1].y*(h-1)/(height-1));
						p1.y=cvFloor(w-1-points[k1].x*(w-1)/(width-1));
						p2.x=cvFloor(points[k2].y*(h-1)/(height-1));
						p2.y=cvFloor(w-1-points[k2].x*(w-1)/(width-1));
						cvLine(img,p1,p2,cvScalarAll(0));
					}
				}
			}
			delete []points;
		}
	}//for
//	cvSaveImage("D:\\test\\temp.jpg",img);
	////统一尺寸
 	{
		/*cvNamedWindow("lucy");
		cvShowImage("lucy",img);
		cvWaitKey(0);*/
 		int H=570,W=630;
 		IplImage *temp=cvCreateImage(cvSize(W,H),img->depth,img->nChannels);
 		cvSet(temp,cvScalarAll(255));
 		CvRect rect1,rect2;
		rect1=cvRect(0,max(0,img->height-H),min(W,img->width),min(H,img->height));
		rect2=cvRect(0,max(0,H-img->height),min(W,img->width),min(H,img->height));
 //		rect1=cvRect(0,max(0,img->height-H),min(W,img->width),img->height);
 //		rect2=cvRect(0,max(0,W-img->height),min(W,img->width),H);
 		cvSetImageROI(img,rect1);
 		cvSetImageROI(temp,rect2);
 		cvCopy(img,temp);
 		cvReleaseImage(&img);
 		img=temp;
 		cvResetImageROI(img);
 	}
	if(oriImage) 
	{
		cvReleaseImage(&oriImage);oriImage=NULL;
	}
	oriImage=cvCloneImage(img);
// 	cvNamedWindow("show");
// 	cvShowImage("show",oriImage);
// 	cvWaitKey(0);
	return img;
}
void CContent::changeData()
{
    IplImage *img = draw3DEMM();///保留原图
    cvReleaseImage(&img);
	double a1=0,b1=0,a2=0,b2=0;

	a1=1;b1=(230-oriPoint.y)/5-(230-oriPoint.x)/5;
	a2=2;b2=(435-oriPoint.y)/5-(220-oriPoint.x)/5;
	double maxN=9999.9;
	for (int i=0;i<height;i++)
	{
		//int j=cvFloor((i-b1)/a1);
		int j=cvFloor((i-b1)/a1)-2;
		if (j<0)
		{
			j=0;
		}
		else if (j>=width)
		{
			break;
		}
		int pos=j;
		for (int k=pos;k<width;k++)
		{
			data[i][k]=0;
		}
	}
	for (int i=0;i<height;i++)
	{
		int j=cvFloor((i-b2)/a2)+2;
		if (j<0)
		{
			continue;
		}
		else if (j>=width)
		{
			j=width-1;
		}
		int pos=j;
		for (int k=0;k<=pos;k++)
		{
			data[i][k]=0;
		}
	}
	// 	{
	// 		ofstream output;output.open("D:\\data.txt");
	// 		for (int i=0;i<height;i++)
	// 		{
	// 			for (int j=0;j<width;j++)
	// 			{
	// 				output<<data[i][j]<<" ";
	// 			}
	// 			output<<endl;
	// 		}
	// 		output.close();
	// 	}
	maxHeight=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<i-5;j++)
		{
			if (j>=width)
			{
				break;
			}
			if (data[i][j]!=maxN&&data[i][j]>maxHeight)
			{
				maxHeight=data[i][j];
			}
		}
	}
	//	ofstream output("D:\\t.txt");
	//	output<<maxHeight<<endl;
	a[0]=a1;a[1]=a2;b[0]=b1;b[1]=b2;
}
void CContent::getNumbers(const string line,double* (&numbers),int &num)
{
	double temp[1000];
	char number[20];
	int n=0,k=0;
	for (int i=0;i<line.length();i++)
	{
		char ch=line[i];
		if(isalpha(ch))
		{
			num=0;
			return;
		}
		if (ch!=9&&ch!=',')
		{
			number[n]=ch;
			n++;
			if (i==line.length()-1)
			{
				number[n]='\0';
				temp[k]=strtod(number,NULL);
				k++;
			}
		}
		else if(n>0)
		{
			number[n]='\0';
			temp[k]=strtod(number,NULL);
			k++;
			n=0;
		}
	}
	num=k;
	numbers=new double[num];
	for (int i=0;i<num;i++)
	{
		numbers[i]=temp[i];
	}
}
bool CContent::readCSV(string str)
{
	deleteContent();
	// add xukun 20130522
	// txt translator
	string tempFile = fileDecoder(str);
	//
	locale::global(locale(""));//
	// mod xukun 20130522
	// txt translator
	//ifstream cin(str.c_str());
	ifstream cin(tempFile.c_str());
	//
	locale::global(locale("C"));
	if (!cin)
	{
//		AfxMessageBox(_T("不能读取数据"));
		return false;
	}
	string test("Data Points");
	string line;
	while (!cin.eof())
	{
		getline(cin,line);
		double *waveLength;
		int num;
		getNumbers(line,waveLength,num);
		if (num>10)
		{
			break;
		}
	}
	if (cin.eof())
	{
		cin.close();
		return false;
	}
/*	while (!cin.eof())
	{
		getline(cin,line);
		string temp;
		temp=line.substr(0,11);
		if (test==temp)
		{
			break;
		}
	}
	if (!cin.eof())
	{
		getline(cin,line);
	}
	else
	{
		cin.clear();
		cin.close();
		locale::global(locale(""));//
		cin.open(str.c_str());
		locale::global(locale("C"));
		while (!cin.eof())
		{
			getline(cin,line);
			double *waveLength;
			int num;
			getNumbers(line,waveLength,num);
			if (num>30)
			{
				break;
			}
		}
	}*/
	double *waveLength;
	int num;
	getNumbers(line,waveLength,num);
	oriPoint.x=waveLength[0];
	endPoint.x=waveLength[num-1];
	int w,h;
	w=num;
	h=3*num;
	{
		data=new double*[h];
		hData=h;/////
		for (int i=0;i<h;i++)
		{
			data[i]=new double[w];
		}
	}
	getline(cin,line);
	width=w;
	height=0;
	bool getOriPoint=false;
	while (!cin.eof())
	{
		if (height>=h)
		{
			double** data1;
			data1=new double*[2*h];
			hData=2*h;/////
			for (int i=0;i<2*h;i++)
			{
				data1[i]=new double[w];
			}
			for (int i=0;i<height;i++)
			{
				for (int j=0;j<width;j++)
				{
					data1[i][j]=fabs(data[i][j]);
				}
			}
			for (int i=0;i<h;i++)
			{
				delete [](data[i]);
			}
			delete []data;
			data=data1;
		}
		double *numbers;
		getNumbers(line,numbers,num);
		if (num<=0)
		{
			getline(cin,line);
			continue;
		}
		if (num!=w+1)
		{
			cout<<height<<endl;
			cout<<num<<" "<<w<<endl;
			cout<<"error happen"<<endl;
		}
		if (!getOriPoint)
		{
			oriPoint.y=numbers[0];
			getOriPoint=true;
		}
		endPoint.y=numbers[0];
		for (int i=1;i<num;i++)
		{
			if (numbers[0]<waveLength[i-1])
			{
				data[height][i-1]=0;
			}
			else
				data[height][i-1]=numbers[i];
		}
		height++;
		delete []numbers;
		getline(cin,line);
	}
	maxHeight=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			if (data[i][j]>maxHeight)
			{
				maxHeight=data[i][j];
				temp.x=j;temp.y=i;
			}
		}
	}
//	ofstream output("D:\\t.txt");
//	output<<oriPoint.x<<" "<<oriPoint.y<<" "<<endPoint.x<<" "<<endPoint.y<<endl;
	cin.close();
	delete []waveLength;

	// add xukun 20130522
	// txt translator
	delFile(tempFile);
	//

	return true;
}
void CContent::printRG(ReebGraph *ptr)
{
// 	ofstream output("E:\\output.txt",ofstream::app);
// 	output<<"level="<<ptr->level<<",num="<<ptr->num<<",sum="<<ptr->sum<<",nextSum="<<ptr->nextSum<<"("<<(ptr->center).x<<","<<(ptr->center).y<<")"<<endl;
// 	output<<"CvBox2D.center=("<<ptr->minBBox.center.x<<","<<ptr->minBBox.center.y<<") ";
// 	output<<"CvBox2D.size(height,width)=("<<ptr->minBBox.size.height<<","<<ptr->minBBox.size.width<<") ";
// 	output<<"CvBox2D.angle="<<ptr->minBBox.angle<<endl;
// 	output.close();
	if (ptr->num==0)
	{
		CvPoint2D32f center;
		center.y=ptr->center.x;
		center.x=ptr->center.y;
		if (showImg)
		{
			int w,h;
			w=showImg->width;h=showImg->height;
			CvPoint p;
		//	p.x=cvFloor(ptr->center.y*(h-1)/(height-1));
		//	p.y=cvFloor(w-1-ptr->center.x*(w-1)/(width-1));
			p.x=cvFloor(ptr->center.y*(w-1)/(height-1));
			p.y=cvFloor(h-1-ptr->center.x*(h-1)/(width-1));
			cvCircle(showImg,p,3,CV_RGB(0,0,255),3);
			center.y=ptr->center.x;
			center.x=ptr->center.y;
		//	center.x=ptr->center.y*(w-1)/(height-1);
		//	center.y=ptr->center.x*(h-1)/(width-1);
			int x,y;
			x=cvFloor(ptr->center.x);y=cvFloor(ptr->center.y);
			if (data[y][x]>centerHeight)
			{
				centerHeight=data[y][x];
			}
		}
// 		centerVector.push_back(center);
// 		double intensity;
// 		intensity=data[ptr->center.y][ptr->center.x];
// 		PNode node={ptr->sum,ptr->center,intensity};
// 		nodeVector.push_back(node);
	}
	else
	{

		if (showImg)
		{
			int w,h;
			w=showImg->width;h=showImg->height;
			CvPoint p;
			p.x=cvFloor(ptr->center.y*(w-1)/(height-1));
			p.y=cvFloor(h-1-ptr->center.x*(h-1)/(width-1));
			cvCircle(showImg,p,3,CV_RGB(0,255,0),3);
		}
	}
	for (int i=0;i<ptr->num;i++)
	{
		printRG(ptr->ptr+i);
	}
}
void CContent::setCenters(ReebGraph *ptr)
{
//	ofstream output("D:\\center.txt",ofstream::app);
	if (ptr->num==0)
	{
		CvPoint2D32f center;
		center.x=ptr->center.y;
		center.y=ptr->center.x;
		centerVector.push_back(center);
		double intensity;
		intensity=data[cvFloor(ptr->center.y)][cvFloor(ptr->center.x)];
		PNode node={ptr->sum,ptr->center,intensity,ptr->minBBox};
		nodeVector.push_back(node);
		int x,y;
		x=cvFloor(ptr->center.x);y=cvFloor(ptr->center.y);
		if (data[y][x]>centerHeight)
		{
			centerHeight=data[y][x];
		}
	//	output<<ptr->sum<<"\t("<<ptr->center.x<<","<<ptr->center.y<<")"<<endl;
	}
	for (int i=0;i<ptr->num;i++)
	{
		setCenters(ptr->ptr+i);
	}
//	output.close();
}
void CContent::compareCenter(vector<CvPoint2D32f> &list1,vector<CvPoint2D32f> &list2)
{
	vector<CvPoint2D32f> tList1,tList2;
	typedef vector<CvPoint2D32f>::iterator Iterator;
	for (Iterator iter1=list1.begin();iter1!=list1.end();iter1++)
	{
		CvPoint2D32f p1=*iter1;
		bool flag=true; 
		for (Iterator iter2=list2.begin();iter2!=list2.end();iter2++)
		{
			CvPoint2D32f p2=*iter2;
			double detaX,detaY;
			detaX=fabs(p1.x-p2.x);
			detaY=fabs(p1.y-p2.y);
			if (detaX<=10&&detaY<=10)
			{
				flag=false;
				break;
			}
		}
		if (flag)
		{
			tList1.push_back(p1);
		}
	}
	for (Iterator iter1=list2.begin();iter1!=list2.end();iter1++)
	{
		CvPoint2D32f p1=*iter1;
		bool flag=true; 
		for (Iterator iter2=list1.begin();iter2!=list1.end();iter2++)
		{
			CvPoint2D32f p2=*iter2;
			double detaX,detaY;
			detaX=fabs(p1.x-p2.x);
			detaY=fabs(p1.y-p2.y);
			if (detaX<=10&&detaY<=10)
			{
				flag=false;
				break;
			}
		}
		if (flag)
		{
			tList2.push_back(p1);
		}
	}
	list1.clear();
	list2.clear();
	if (tList1.size()>0)
	{
		list1.insert(list1.begin(),tList1.begin(),tList1.end());
	}
	if (tList2.size()>0)
	{
		list2.insert(list2.begin(),tList2.begin(),tList2.end());
	}
}

void cvText(IplImage* img, const char* text, int x, int y)
{
	CvFont font;

	double hscale = 1.0;
	double vscale = 1.0;
	int linewidth = 2;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,hscale,vscale,0,linewidth);

	CvScalar textColor =CV_RGB(255,0,0);
	CvPoint textPos =cvPoint(x, y);

	cvPutText(img, text, textPos, &font,textColor);
}
IplImage* CContent::GetPeakImg(IplImage* src)
{
	IplImage *img=cvCloneImage(src);
	int h,w;///img for truth
	int imgH,imgW;//img for show
	int dHeight,dWidth;//data
	dWidth=endPoint.y-oriPoint.y;
	dHeight=endPoint.x-oriPoint.x;
	h=SCALE*dHeight;w=SCALE*dWidth;
	dWidth=dWidth/5+1;dHeight=dHeight/5+1;
	imgW=src->width;
	imgH=src->height;
	int num=0;
	typedef vector<CvPoint2D32f>::iterator Iterator;
	for (Iterator iter=centerVector.begin();iter!=centerVector.end();iter++)
	{
		CvPoint2D32f cenPoint=*iter;
		CvPoint dCenP;
		dCenP.x=cvFloor(cenPoint.x*(w-1)/(dWidth-1));
		dCenP.y=cvFloor(imgH-1-cenPoint.y*(h-1)/(dHeight-1));
		num++;
		char text[10];
		itoa(num,text,10);
		cvText(img,text,dCenP.x,dCenP.y);
	}
	return img;

}

double CContent::getPointVal(CvPoint p)
{
	if(p.x<oriPoint.x||p.y<oriPoint.y) 
		return -1;
	if(p.x>endPoint.x||p.y>endPoint.y) 
		return -1;
	int x,y;
	x=(p.x-oriPoint.x)/5;
	y=(p.y-oriPoint.y)/5;
	return data[y][x];
}

void CContent::sortCenters()
{
	if(centerVector.size()<=0) return;
	if(nodeVector.size()<=0) return;
	if (nodeVector.size()!=centerVector.size()) return;
	size_t cenSum=centerVector.size();
	for (size_t i=0;i<cenSum;i++)
	{
		for (size_t j=0;j<cenSum-i-1;j++)
		{
			CvPoint2D32f point1=centerVector[j];
			CvPoint2D32f point2=centerVector[j+1];
			if ((point1.x+point1.y)>(point2.x+point2.y))
			{
				CvPoint2D32f temp=centerVector[j];
				centerVector[j]=centerVector[j+1];
				centerVector[j+1]=temp;
				int sum;
				CvPoint2D32f center;
				double intensity;
				CvBox2D minBBox;
				sum=nodeVector[j].sum;
				nodeVector[j].sum=nodeVector[j+1].sum;
				nodeVector[j+1].sum=sum;
				center=nodeVector[j].center;
				nodeVector[j].center=nodeVector[j+1].center;
				nodeVector[j+1].center=center;
				intensity=nodeVector[j].intensity;
				nodeVector[j].intensity=nodeVector[j+1].intensity;
				nodeVector[j+1].intensity=intensity;
				minBBox=nodeVector[j].minBBox;
				nodeVector[j].minBBox=nodeVector[j+1].minBBox;
				nodeVector[j+1].minBBox=minBBox;
			}
		}
	}
}
double CContent::RGSum1=0;
double CContent::RGSum2=0;
int CContent::N=0;

// add xukun 20130522
// txt translator
string CContent::fileDecoder(string fname)
{
	ifstream infile(fname.c_str());
	if(!infile.is_open())
		return string("");
	// mod xukun 20131021
	// 让名字不在出现问题
//	string fnameout = fname.insert(fname.length(), "temp", 4);
	string fnameout = fname.insert(fname.length()-3, "t", 1);
	fnameout = fnameout.insert(fnameout.length(), "temp", 4);
	//
	ofstream outfile(fnameout.c_str());
	char *content = new char[101];
	while(!infile.eof())
	{
		char ch;
		infile >> ch;
		ch ^= TRANS_XOR_CHAR;
		outfile << ch;
	}

	infile.close(); 
	outfile.close();
	delete content;

	return fnameout;
}

void CContent::delFile(string fname)
{
	string cmd1;
	cmd1 = string("/c del \"");
//	CString cmd =CString(cmd1.c_str())+GetCString(fname)+CString("\"");
//	ShellExecute(NULL, _T("open"), _T("cmd.exe"), cmd, NULL, SW_HIDE);
}
//

// add xukun 20131022
// txt translator file->string
string CContent::fileDecoderToString(string fname)
{
	// 判断文件类型
	bool isTxtFile = false;
    if(fname.rfind(".dat") == -1 && fname.rfind(".DAT") == -1)
		isTxtFile = true;
	string fileData("");
	ifstream infile(fname.c_str());
	if(!infile.is_open())
		return string("");
	if(isTxtFile)
	{
		char content[101];
		while(!infile.eof())
		{
			infile.read(content, 100);
			content[100] = '\0';
			fileData += string(content);
		}
	}
	else
	{
		char ch;
		while(!infile.eof())
		{
			infile >> ch;
			ch ^= TRANS_XOR_CHAR;
			fileData += ch;
		}
	}
	return fileData;
}
