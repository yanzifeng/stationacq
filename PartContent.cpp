
#include "PartContent.h"
#include "content.h"

void CPartContent::GetMinMaxVal(double* valList,int N,int p,double &minVal,double &maxVal,double &meanVal)
{
	double meanX=0;
	for (int i=0;i<N;i++)
	{
		meanX+=valList[i];
	}
	meanX/=N;meanVal=meanX;
	if (N<=p)
	{
		minVal=maxVal=meanX;
		minVal-=0.001;maxVal+=0.001;
		return;
	}
	CvMat *CMat=cvCreateMat(1,p+1,CV_32FC1);
	for (int i=0;i<=p;i++)
	{
		double temp=0;
		for (int j=0;j<N-i;j++)
		{
			temp+=(valList[j]-meanX)*(valList[j+i]-meanX);
		}
		cvSetReal2D(CMat,0,i,temp/N);
	}
	CvMat *TMat=cvCreateMat(p,p,CV_32FC1);
	for (int i=0;i<p;i++)
	{
		for (int j=0;j<p;j++)
		{
			int t=(i+j)%p;
			double temp=cvGetReal2D(CMat,0,t);
			cvSetReal2D(TMat,i,j,temp);
		}
	}
	CvMat *bMat=cvCreateMat(p,1,CV_32FC1);
	for (int i=0;i<p;i++)
	{
		double temp;
		temp=cvGetReal2D(CMat,0,i+1);
		cvSetReal2D(bMat,i,0,temp);
	}
	CvMat *alpha1=cvCreateMat(p,1,CV_32FC1);
	CvMat *tempTMat=cvCloneMat(TMat);
	cvInv(TMat,tempTMat);
	cvGEMM(tempTMat,bMat,1,NULL,0,alpha1);
	cvReleaseMat(&tempTMat);
	double alpha0;
	CvScalar s=cvSum(alpha1);
	alpha0=(1-s.val[0])*meanX;
	double theta2;
	theta2=cvGetReal2D(CMat,0,0);

	for (int i=0;i<alpha1->height;i++)
	{
		double temp1,temp2;
		temp1=cvGetReal2D(alpha1,i,0);
		temp2=cvGetReal2D(CMat,0,i+1);
		theta2-=temp1*temp2;
	}
	double nowX;
	nowX=alpha0;
	for (int i=0;i<alpha1->height;i++)
	{
		double temp1,temp2;
		temp1=cvGetReal2D(alpha1,i,0);
		temp2=valList[N-1-i-1];
		nowX+=temp1*temp2;
	}
	minVal=nowX-3*sqrt(theta2);
	maxVal=nowX+3*sqrt(theta2);
	cvReleaseMat(&alpha1);
	cvReleaseMat(&bMat);
	cvReleaseMat(&TMat);
	cvReleaseMat(&CMat);
}
double* CPartContent::partialMatch(CContent &obj1, CContent &obj2, int *order,int &matchNum,int &costNum)
{
	matchNum=0;
	const int cenMaxNum=100;
	double costTemp[cenMaxNum];
	const int sNum=6;
	CvScalar sList[6];
	sList[0]=CV_RGB(255,0,0);sList[1]=CV_RGB(0,255,0);sList[2]=CV_RGB(0,0,255);
	sList[3]=CV_RGB(255,255,0);sList[4]=CV_RGB(255,0,255);sList[5]=CV_RGB(0,255,255);
	typedef vector<PNode>::iterator Iterator;
	double sum1=0,sum2=0;
	for (Iterator iter=obj1.nodeVector.begin();iter!=obj1.nodeVector.end();iter++)
	{
		PNode node=*iter;
		sum1+=node.sum;
	}
	for (Iterator iter=obj2.nodeVector.begin();iter!=obj2.nodeVector.end();iter++)
	{
		PNode node=*iter;
		sum2+=node.sum;
	}
	int num=0;
	costTemp[0]=0;
	int iter1Pos=0;
	//总信息(仅考虑中心位置信息)，分信息：强度对比信息/形状信息/倾斜信息/强度之合占总之比
	for (Iterator iter1=obj1.nodeVector.begin();iter1!=obj1.nodeVector.end();iter1++)
	{
		order[iter1Pos]=-1;
		PNode node1=*iter1;
		costTemp[num+1]=0;
		costTemp[num+2]=0;
		costTemp[num+3]=0;
		costTemp[num+4]=0;
		int index=0;
		for (Iterator iter2=obj2.nodeVector.begin();iter2!=obj2.nodeVector.end();iter2++)
		{
			index++;
			PNode node2=*iter2;
			double detaX,detaY;
			detaX=fabs(node1.center.x-node2.center.x);
			detaY=fabs(node1.center.y-node2.center.y);
			if (detaX<=2&&detaY<=2)
			{
				order[iter1Pos]=index;
				double scale=40;
				double d=sqrt(detaX*detaX+detaY*detaY)/scale;
				costTemp[0]+=exp(-d)*(node1.sum+node2.sum);
#if 1
				double t1,t2;
				t1=node1.sum/sum1;t2=node2.sum/sum2;
				double ratio;ratio=min(t1,t2)/max(t1,t2);
				if(ratio>0.7) ratio=1;
				else ratio =0.8;
				costTemp[num+1]=exp(-d)*ratio;
				double w,h;
				w=node1.minBBox.size.width;h=node1.minBBox.size.height;
				t1=min(w,h)/max(w,h);
				w=node2.minBBox.size.width;h=node2.minBBox.size.height;
				t2=min(w,h)/max(w,h);
				costTemp[num+2]=exp(-d)*min(t1,t2)/max(t1,t2);
				t1=fabs(node1.minBBox.angle);t2=fabs(node2.minBBox.angle);
				if(t1>t2){double temp;temp=t1;t1=t2;t2=temp;}
				if(fabs(t2)<pow(10.0,-2)||(fabs(t1)<pow(10.0,-2)&&fabs(t2-90)<pow(10.0,-2))) {t1=1;t2=1;}
				costTemp[num+3]=exp(-d)*min(t1,t2)/max(t1,t2);

				costTemp[num+4]=(node1.sum+node2.sum)/(sum1+sum2);
#else
				double t1,t2;
				t1=node1.sum/sum1;t2=node2.sum/sum2;
				costTemp[num+1]=exp(-d)*min(t1,t2)/max(t1,t2);
				double w,h;
				w=node1.minBBox.size.width;h=node1.minBBox.size.height;
				t1=min(w,h)/max(w,h);
				w=node2.minBBox.size.width;h=node2.minBBox.size.height;
				t2=min(w,h)/max(w,h);
				costTemp[num+2]=exp(-d)*min(t1,t2)/max(t1,t2);
				t1=fabs(node1.minBBox.angle);t2=fabs(node2.minBBox.angle);
				if(t1>t2){double temp;temp=t1;t1=t2;t2=temp;}
				if(fabs(t2)<pow(10.0,-2)) {t1=1;t2=1;}
				costTemp[num+3]=exp(-d)*min(t1,t2)/max(t1,t2);
#endif
				matchNum++;
				break;
			}
		}
		num+=4;
		iter1Pos++;
	}
	costNum=num+1;
	double* cost=new double[num+1];///
	cost[0]=costTemp[0];
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<num/4;j++)
		{
			cost[(num/4)*i+j+1]=costTemp[4*j+i+1];
		}
	}
	cost[0]=cost[0]/(sum1+sum2);
	// 	cost[0]/=sum1;
	// 	cost[1]/=sum2;
	// 	cost[2]/=(sum1+sum2);
// 	ofstream output;output.open("E:\\lucyCost.txt");
// 	for (int i=0;i<=num;i++)
// 	{
// 		output<<costTemp[i]<<" ";
// 	}
// 	output<<endl;
// 	for (int i=0;i<=num;i++)
// 	{
// 		output<<cost[i]<<" ";
// 	}
// 	output<<endl;
	return cost;
}
double CPartContent::completeMatch(CContent &obj1,CContent &obj2, double &partSim,int *order,int &matchNum,double* pCost,int &costNum)
{
	double cost;
	cost=compareRG(obj1.MRG,obj2.MRG);
	double *costTemp=CPartContent::partialMatch(obj1,obj2,order,matchNum,costNum);
	for (int i=0;i<costNum;i++)
	{
		pCost[i]=costTemp[i];
	}
	delete costTemp;
	if(pCost[0]>0.9)
	{
		///to correct cost
		bool flag=true;//true表示cost值应该高，false表示cost应该低
		///如果cost过高，则各峰的匹配值应该会有低的，用各峰的匹配值来修正
		///如果cost过低，则各峰的匹配值应该都较高，用pCost[0]修正
		int obj1CSize=obj1.nodeVector.size();
		double threValue=0.8;
		double avgCost=0;
		double sum=0;
		for (int i=0;i<obj1.nodeVector.size();i++)
		{
			int index;index=order[i];
			if(index<0) continue;
			if(pCost[i+1]<threValue) flag=false;
			if(pCost[i+1+obj1CSize]<threValue) flag=false;
			avgCost+=pCost[i+1];
			avgCost+=pCost[i+1+obj1CSize];
			avgCost+=pCost[i+1+2*obj1CSize];
			sum+=3;
		}
		avgCost/=sum;
		if(flag&&cost<0.9) cost=pCost[0];
		else if(!flag) cost=avgCost;
	}

	//draw partial match
	double finalPartialCost=0;

	for (int i=0;i<obj1.nodeVector.size();i++)
	{
		int index;index=order[i];
		if(index<0) continue;
		int nodeSize=obj1.nodeVector.size();
		finalPartialCost+=pCost[nodeSize*3+1+i]*(pCost[nodeSize*0+1+i]+pCost[nodeSize*1+1+i]+pCost[nodeSize*2+1+i]);
		//str.sprintf("(%d,%d): %.3f; ",i+1,index,pCost[i]);
		//allLocSim+=str;
		// 		str+=QString("(");str+=QString::number(i+1);str+=QString(",");
		// 		str+=QString::number(index);str+=QString("): ");
		// 		str+=QString::number(pCost[i]);
	}
	//ui.LocAllSimLabel->setText(allLocSim);
	finalPartialCost/=3;
	partSim=finalPartialCost;
	return  cost;
}
double* CPartContent::partialMatch(CContent &obj1,CContent &obj2,IplImage *img1,IplImage *img2,int &matchNum)
{
	const int cenMaxNum=100;
	double costTemp[cenMaxNum];
	const int sNum=6;
	CvScalar sList[6];
	sList[0]=CV_RGB(255,0,0);sList[1]=CV_RGB(0,255,0);sList[2]=CV_RGB(0,0,255);
	sList[3]=CV_RGB(255,255,0);sList[4]=CV_RGB(255,0,255);sList[5]=CV_RGB(0,255,255);
	typedef vector<PNode>::iterator Iterator;
	double sum1=0,sum2=0;
	for (Iterator iter=obj1.nodeVector.begin();iter!=obj1.nodeVector.end();iter++)
	{
		PNode node=*iter;
		sum1+=node.sum;
	}
	for (Iterator iter=obj2.nodeVector.begin();iter!=obj2.nodeVector.end();iter++)
	{
		PNode node=*iter;
		sum2+=node.sum;
	}
	int num=0;
	costTemp[0]=0;
	for (Iterator iter1=obj1.nodeVector.begin();iter1!=obj1.nodeVector.end();iter1++)
	{
		PNode node1=*iter1;
		costTemp[num+1]=0;
		for (Iterator iter2=obj2.nodeVector.begin();iter2!=obj2.nodeVector.end();iter2++)
		{
			PNode node2=*iter2;
			double detaX,detaY;
			detaX=fabs(node1.center.x-node2.center.x);
			detaY=fabs(node1.center.y-node2.center.y);
			if (detaX<=2&&detaY<=2)
			{

				double scale=40;
				double d=sqrt(detaX*detaX+detaY*detaY)/scale;
				costTemp[0]+=exp(-d)*(node1.sum+node2.sum);
				double t1,t2;
				t1=node1.sum/sum1;t2=node2.sum/sum2;
				costTemp[num+1]=exp(-d)*min(t1,t2)/max(t1,t2);
				CvPoint p1,p2;
				int h,w;///img for truth
				int imgH,imgW;//img for show
				int dHeight,dWidth;//data
				dWidth=obj1.endPoint.y-obj1.oriPoint.y;
				dHeight=obj1.endPoint.x-obj1.oriPoint.x;
				h=SCALE*dHeight;w=SCALE*dWidth;
				imgW=img1->width;
				imgH=img1->height;
				////
				CvScalar s;
				s=sList[(num%sNum)];
				p1.x=cvFloor(node1.center.y*5*(w-1)/(dWidth-1));
				p1.y=cvFloor(imgH-1-node1.center.x*5*(h-1)/(dHeight-1));
				char text[10];
				itoa(num+1,text,10);
				cvText(img1,text,p1.x,p1.y);
				//	cvCircle(img1,p1,5,s,5);
				dWidth=obj2.endPoint.y-obj2.oriPoint.y;
				dHeight=obj2.endPoint.x-obj2.oriPoint.x;
				h=SCALE*dHeight;w=SCALE*dWidth;
				imgW=img2->width;
				imgH=img2->height;
				p2.x=cvFloor(node2.center.y*5*(w-1)/(dWidth-1));
				p2.y=cvFloor(imgH-1-node2.center.x*5*(h-1)/(dHeight-1));
				cvText(img2,text,p2.x,p2.y);
				//	cvCircle(img2,p2,5,s,5);
				////
				num++;
				break;
			}
		}
	}
	matchNum=num;
	double* cost=new double[num+1];///
	for (int i=0;i<=num;i++)
	{
		cost[i]=costTemp[i];
	}
	cost[0]=cost[0]/(sum1+sum2);
	// 	cost[0]/=sum1;
	// 	cost[1]/=sum2;
	// 	cost[2]/=(sum1+sum2);
	return cost;
}
double* CPartContent::partialMatch(vector<PNode> &nodes1,vector<PNode> &nodes2)
{
	double* cost=new double[3];///the value for data1, for data2, for both
	for (int i=0;i<3;i++)
	{
		cost[i]=0;
	}
	if (nodes1.size()<=0||nodes2.size()<=0)
	{
		return cost;
	}
	typedef vector<PNode>::iterator Iterator;
	double sum1=0,sum2=0;
	for (Iterator iter=nodes1.begin();iter!=nodes1.end();iter++)
	{
		PNode node=*iter;
		sum1+=node.sum;
	}
	for (Iterator iter=nodes2.begin();iter!=nodes2.end();iter++)
	{
		PNode node=*iter;
		sum2+=node.sum;
	}
	int num=0;
	for (Iterator iter1=nodes1.begin();iter1!=nodes1.end();iter1++)
	{
		PNode node1=*iter1;
		for (Iterator iter2=nodes2.begin();iter2!=nodes2.end();iter2++)
		{
			PNode node2=*iter2;
			double detaX,detaY;
			detaX=fabs(node1.center.x-node2.center.x);
			detaY=fabs(node1.center.y-node2.center.y);
			if (detaX<=3&&detaY<=3)
			{

				double scale=40;
				double d=sqrt(detaX*detaX+detaY*detaY)/scale;
				cost[0]+=exp(-d)*node1.sum;
				cost[1]+=exp(-d)*node2.sum;
				cost[2]+=exp(-d)*(node1.sum+node2.sum);
				////
				break;

			}
		}
	}
	cost[0]/=sum1;
	cost[1]/=sum2;
	cost[2]/=(sum1+sum2);
	return cost;
}
