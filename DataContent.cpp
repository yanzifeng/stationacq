
#include "DataContent.h"
// add xukun 20131022
// txt translator file->string
#include "StringDataReader.h"
//

bool CDataContent::readTXT(string oriData)
{
	deleteContent();
	oriStr="";
	reebStr="";
	centerStr="";
	istringstream input(oriData);
	string line;
	getline(input,line);
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
	width=w;
	height=0;
	bool getOriPoint=false;
	while (getline(input,line))
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
					data1[i][j]=data[i][j];
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
//			getline(cin,line);
			continue;
		}
		if (num!=w+1)
		{
			return false;
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
	}
	maxHeight=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			if (data[i][j]>maxHeight)
			{
				maxHeight=data[i][j];
			}
		}
	}
	//	ofstream output("D:\\t.txt");
	//	output<<oriPoint.x<<" "<<oriPoint.y<<" "<<endPoint.x<<" "<<endPoint.y<<endl;
	delete []waveLength;
	input.clear();
	return true;
}

IplImage* CDataContent::show3DEMM(int num)
{
	int h,w;
    h=cvFloor(0.7*(endPoint.y-oriPoint.y));
    w=cvFloor(0.7*(endPoint.x-oriPoint.x));
	IplImage* img=cvCreateImage(cvSize(h,w),IPL_DEPTH_8U,1);///本应是cvSize(w,h)，但后面要将图片旋转
	cvSet(img,cvScalarAll(255));
	/////分为num 个 level
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
    int H=300,W=300;
	IplImage *temp=cvCreateImage(cvSize(W,H),img->depth,img->nChannels);
	cvSet(temp,cvScalarAll(255));
	CvRect rect1,rect2;
	rect1=cvRect(0,max(0,img->height-H),min(W,img->width),img->height);
	rect2=cvRect(0,max(0,W-img->height),min(W,img->width),H);
	cvSetImageROI(img,rect1);
	cvSetImageROI(temp,rect2);
	cvCopy(img,temp);
	cvReleaseImage(&img);
	img=temp;
	cvResetImageROI(img);
	return img;
}

void CDataContent::createRG(string reebData)
{
	deleteContent();
	oriStr="";
	reebStr="";
	centerStr="";

//	str=string("test\ntest");
	//istringstream input(reebData);
	string line="",strLeft="";
	size_t pos=reebData.find_first_of('\r');
	line.insert(0,reebData,0,pos);
	strLeft.insert(0,reebData,pos+2,reebData.length()-pos-1);
	MRG=new ReebGraph;
	///the format is level,num,sum,nextSum,center.x,center.y;
	double *numbers;
	int nNum;
	getNumbers(line,numbers,nNum);
	if (nNum!=6)
	{
//		AfxMessageBox(_T("reeb数据有错"));
	}
	int level,num,sum,nextSum;
	CvPoint2D32f center;
	level=(int)(numbers[0]);
	num=(int)(numbers[1]);
	sum=(int)(numbers[2]);
	nextSum=(int)(numbers[3]);
	center.x=numbers[4];
	center.y=numbers[5];
	MRG->level=level;
	MRG->num=num;
	MRG->sum=sum;
	MRG->nextSum=nextSum;
	MRG->center=center;
	if(num>0) MRG->ptr=new ReebGraph[num];
	else MRG->ptr=NULL;
	for (int i=0;i<num;i++)
	{
		createSubRG(strLeft,MRG->ptr+i);
	}
//	printRG(MRG);
}
void CDataContent::createSubRG(string &str,ReebGraph *head)
{
	string line="",strLeft="";
	size_t pos=str.find_first_of('\r');
	line.insert(0,str,0,pos);
	strLeft.insert(0,str,pos+2,str.length()-pos-1);
	str=strLeft;
	double *numbers;
	int nNum;
	getNumbers(line,numbers,nNum);
	if (nNum!=6)
	{
//		AfxMessageBox(_T("reeb数据有错"));
	}
	int level,num,sum,nextSum;
	CvPoint2D32f center;
	level=(int)(numbers[0]);
	num=(int)(numbers[1]);
	sum=(int)(numbers[2]);
	nextSum=(int)(numbers[3]);
	center.x=numbers[4];
	center.y=numbers[5];
	head->level=level;
	head->num=num;
	head->sum=sum;
	head->nextSum=nextSum;
	head->center=center;
	head->ptr=new ReebGraph[num];
	ReebGraph *ptr=head;
	char temp[1000];
	sprintf_s(temp,"%d,%d,%d,%d,%f,%f\r\n",ptr->level,ptr->num,ptr->sum,ptr->nextSum,ptr->center.x,ptr->center.y);

	for (int i=0;i<head->num;i++)
	{
		createSubRG(str,head->ptr+i);
	}
}
void CDataContent::createSubRG(istringstream &input,ReebGraph *head)
{
	string line;
	if (getline(input,line))
	{
		double *numbers;
		int nNum;
		getNumbers(line,numbers,nNum);
		if (nNum!=6)
		{
//			AfxMessageBox(_T("reeb数据有错"));
		}
		int level,num,sum,nextSum;
		CvPoint2D32f center;
		level=(int)(numbers[0]);
		num=(int)(numbers[1]);
		sum=(int)(numbers[2]);
		nextSum=(int)(numbers[3]);
		center.x=numbers[4];
		center.y=numbers[5];
		head->level=level;
		head->num=num;
		head->sum=sum;
		head->nextSum=nextSum;
		head->center=center;
		head->ptr=new ReebGraph[num];
		for (int i=0;i<head->num;i++)
		{
			createSubRG(input,head->ptr+i);
		}
	}

}
bool CDataContent::getData(string path, string &oriData, string &reebData,string &centerData)
{
	CDataContent obj;
	obj.oriStr="";
	obj.reebStr="";
	obj.centerStr="";
	if (obj.readCSV(path))
	{
		obj.changeData();
		obj.createRG();
		obj.printRGStr(obj.MRG);
		oriData=obj.oriStr;
		reebData=obj.reebStr;
		centerData=obj.centerStr;
		obj.deleteContent();
		return true;
	}
	else
	{
		return false;
	}
}

void CDataContent::printRGStr(ReebGraph *ptr)
{
	///the format is level,num,sum,nextSum,center.x,center.y;
	char temp[1000];
	sprintf_s(temp,"%d,%d,%d,%d,%f,%f\r\n",ptr->level,ptr->num,ptr->sum,ptr->nextSum,ptr->center.x,ptr->center.y);
	reebStr+=temp;
	if (ptr->num==0)
	{
		int x,y;
		x=cvFloor(oriPoint.y+5*(ptr->center.x));
		y=cvFloor(oriPoint.x+5*(ptr->center.y));
		sprintf_s(temp,"(%d,%d) ",x,y);
		//temp.Format(_T("(%d,%d)\r\n"),x,y);
		//temp.Format(_T("(%f,%f)\r\n"),ptr->center.x,ptr->center.y);
		centerStr+=temp;
	}

	// 	ofstream output("D:\\output.txt",ofstream::app);
	// 	output<<"level="<<ptr->level<<",num="<<ptr->num<<",sum="<<ptr->sum<<",nextSum="<<ptr->nextSum<<"("<<(ptr->center).x<<","<<(ptr->center).y<<")"<<endl;
	// 	output.close();
	for (int i=0;i<ptr->num;i++)
	{
		printRGStr(ptr->ptr+i);
	}
}

void CDataContent::loadstandparams()
{
    /*
    CStdioFile file;
	bool ret = file.Open(_T("standardparam.ini"),CFile::modeReadWrite);

	if(!ret)
	{
		_factor = 1;
		return;
	}
	int a,b,c;
	CString str;
	file.ReadString(str);
	double s1 = _ttof(str);
	file.ReadString(str);
	double s2 = _ttof(str);

	_factor = (s2/s1)*0.1;

	file.ReadString(str);
	file.ReadString(str);
	_selected = _ttoi(str);

	file.Close();
;
//	str.Format(_T("%f"),_factor);
//	AfxMessageBox(str);
*/
}

bool CDataContent::readCSV(string str)
{
	//add yzf 
	_factor = 1;
	loadstandparams();
	// add xukun 20131024
	// [ 使用istringstream
	deleteContent();
	string stringData = fileDecoderToString(str);
	if(stringData.length() == 0)
		return false;
	istringstream reader(stringData);
	string line;
	while (getline(reader,line))
	{
		double *waveLength;
		int num;
		getNumbers(line,waveLength,num);
		if (num>6)
		{
			break;
		}
	}
	if(stringData.length() == 0)
		return false;
	double *waveLength;
	int num;
	getNumbers(line,waveLength,num);
	oriPoint.x=waveLength[0];
	endPoint.x=waveLength[num-1];
	////
	oriStr=line;
	oriStr+="\n";
	////
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
	getline(reader,line);
	width=w;
	height=0;
	bool getOriPoint=false;
	while (!reader.eof())
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
					data1[i][j]=data[i][j];
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
			getline(reader,line);
			continue;
		}
		if (num!=w+1)
			return false;

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
		////
		oriStr+=line;
		oriStr+="\n";
		////
		getline(reader,line);
	}
	maxHeight=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			
			if(_selected)
			{
				data[i][j] = (data[i][j] / _factor) * 852 * 2;
			}
			if (data[i][j]>maxHeight)
			{
				maxHeight=data[i][j];
				temp.x=j;temp.y=i;
			}
		}
	}
	delete []waveLength;
	reader.clear();
	return true;
	// }
	//
	// add xukun 20131022
	// txt translator file->string
//	return readCSVString(str);
	//
	// mod xukun 20131022
	// txt translator file->string
	/*
	// add xukun 20130926
	// [ for test
// 	ofstream oferr("D:\\xkerr.txt",ios::app);
// 	oferr << "+++++++++++++++++++++++++++++++++++++" << endl;
// 	oferr << str << endl;
	// ]
	deleteContent();
	// add xukun 20130522
	// txt translator
	string tempFile = fileDecoder(str);
	//
	// add xukun 20130926
	// [ for test
//	oferr << tempFile << endl;
	// ]
	locale::global(locale(""));//
	// mod xukun 20130522
	// txt translator
	//ifstream cin(str.c_str());
	ifstream cin(tempFile.c_str());
	//
	locale::global(locale("C"));
	if (!cin)
	{
		// add xukun 20130926
		// [ for test
// 		oferr << "open file error!" << endl;
// 		oferr.close();
		// ]
		return false;
	}
	string line;
	while (!cin.eof())
	{
		getline(cin,line);
		double *waveLength;
		int num;
		getNumbers(line,waveLength,num);
		if (num>6)
		{
			break;
		}
	}
	if (cin.eof())
	{
		cin.close();
		// add xukun 20130926
		// [ for test
// 		oferr << "read data end error!" << endl;
// 		oferr.close();
		// ]
		return false;
	}
	double *waveLength;
	int num;
	getNumbers(line,waveLength,num);
	oriPoint.x=waveLength[0];
	endPoint.x=waveLength[num-1];
	////
	oriStr=line;
	oriStr+="\n";
	////
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
					data1[i][j]=data[i][j];
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
			// add xukun 20130926
			// [ for test
// 			oferr << "num error!" << endl;
// 			oferr.close();
			// ]
			return false;
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
		////
		oriStr+=line;
		oriStr+="\n";
		////
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
	//ofstream output("D:\\t.txt");
	//output<<oriPoint.x<<" "<<oriPoint.y<<" "<<endPoint.x<<" "<<endPoint.y<<endl;
	cin.close();
	delete []waveLength;

	// add xukun 20130522
	// txt translator
	delFile(tempFile);
	//
	// add xukun 20130926
	// [ 等待上面过程结束
	Sleep(5000);
	// ]
	// add xukun 20130926
	// [ for test
// 	oferr << "success!" << endl;
// 	oferr.close();
	// ]
	return true;
	*/
	//
}
/*
bool CDataContent::readCSV(string str,double factor)
{
	// add xukun 20131024
	// [ 使用istringstream
	deleteContent();
	string stringData = fileDecoderToString(str);
	if(stringData.length() == 0)
		return false;
	istringstream reader(stringData);
	string line;
	while (getline(reader,line))
	{
		double *waveLength;
		int num;
		getNumbers(line,waveLength,num);
		if (num>6)
		{
			break;
		}
	}
	if(stringData.length() == 0)
		return false;
	double *waveLength;
	int num;
	getNumbers(line,waveLength,num);
	oriPoint.x=waveLength[0];
	endPoint.x=waveLength[num-1];
	////
	oriStr=line;
	oriStr+="\n";
	////
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
	getline(reader,line);
	width=w;
	height=0;
	bool getOriPoint=false;
	while (!reader.eof())
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
					data1[i][j]=data[i][j];
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
			getline(reader,line);
			continue;
		}
		if (num!=w+1)
			return false;

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
		////
		oriStr+=line;
		oriStr+="\n";
		////
		getline(reader,line);
	}
	maxHeight=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			
			data[i][j] = data[i][j] / factor;

			if (data[i][j]  > maxHeight )
			{
				maxHeight=data[i][j];
				temp.x=j;temp.y=i;
			}
		}
	}
	delete []waveLength;
	reader.clear();
	return true;
	}
*/
//double CDataContent::getPointVal(CvPoint p)
//{
//	if(p.x<oriPoint.x||p.y<oriPoint.y) return -1;
//	if(p.x>endPoint.x||p.y>endPoint.y) return -1;
//	int x,y;
//	x=(p.x-oriPoint.x)/5;
//	y=(p.y-oriPoint.y)/5;
//	return data[y][x];
//}

// add xukun 20131022
// txt translator file->string
bool CDataContent::readCSVString(string str)
{
	deleteContent();
	string stringData = fileDecoderToString(str);
	if(stringData.length() == 0)
		return false;
	CStringDataReader reader(stringData);
	string line;
	while (!reader.endOfData())
	{
		line = reader.getLine();
		double *waveLength;
		int num;
		getNumbers(line,waveLength,num);
		if (num>6)
		{
			break;
		}
	}
	if(stringData.length() == 0)
		return false;
	double *waveLength;
	int num;
	getNumbers(line,waveLength,num);
	oriPoint.x=waveLength[0];
	endPoint.x=waveLength[num-1];
	////
	oriStr=line;
	oriStr+="\n";
	////
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
	line = reader.getLine();
	width=w;
	height=0;
	bool getOriPoint=false;
	while (!reader.endOfData())
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
					data1[i][j]=data[i][j];
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
			line = reader.getLine();
			continue;
		}
		if (num!=w+1)
			return false;

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
		////
		oriStr+=line;
		oriStr+="\n";
		////
		line = reader.getLine();
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
	delete []waveLength;
	return true;
}
//
