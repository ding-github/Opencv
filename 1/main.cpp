#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "common.h"

#define MIN_CONTOUR_RATIO 2
#define MAX_CONTOUR_RATIO 5
#define MIN_CONTOUR_AREA 150
#define MIN_CONTOUR_HEIGHT 30
#define MIN_CONTOUR_LENGTH 80

//根据尺寸判断
int JudgeByRectSize(cv::Rect rect)
{
	if((rect.area()>MIN_CONTOUR_AREA)&&(rect.height>MIN_CONTOUR_HEIGHT)&&(rect.width>MIN_CONTOUR_LENGTH)
		&&(rect.width/rect.height>=MIN_CONTOUR_RATIO)&&(rect.width/rect.height<=MAX_CONTOUR_RATIO))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//根据颜色比例-二值图像
int JudgeByColorPercent(cv::Mat &img,int threshold)
{
	int cnt=0;
	for(int i=0;i<img.rows;i++)
	{
		for(int j=0;j<img.cols;j++)
		{
			if(img.at<uchar>(i,j) == 255)
			{
				cnt++;
			}
		}
	}
	std::cout<<"cnt: "<<cnt<<std::endl;
	std::cout<<"total: "<<img.rows*img.cols<<std::endl;
	if((cnt*100)/(img.rows*img.cols)>threshold)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

#define RED			0
#define GREEN		1
#define BLUE		2
#define WHITE		3
//从hsv提前颜色
int GetColorFromHsv(cv::Mat ROI,int color,cv::Mat &result)
{
	const cv::Scalar hsvBlueLow(100,43,46);
	const cv::Scalar hsvBlueHeight(124,255,255);
	const cv::Scalar hsvWhiteLow(0,0,221);
	const cv::Scalar hsvWhiteHeight(180,30,255);
	if(color == BLUE)
	{
		cv::inRange(ROI,hsvBlueLow,hsvBlueHeight,result);
		cv::threshold(result,result,1,255,cv::THRESH_BINARY);
	}
	else if(color == WHITE)
	{
		cv::inRange(ROI,hsvWhiteLow,hsvWhiteHeight,result);
		cv::threshold(result,result,1,255,cv::THRESH_BINARY);
	}
	return 0;
}

int CalcuAimPointNum(cv::Mat img,int mode,int aimPoint,cv::Mat &result)
{
	if(!img.data)
	{
		std::cout<<"img error !"<<std::endl;
		return -1;
	}
	if((mode!=0)&&(mode!=1))
	{
		std::cout<<"mode error !"<<std::endl;
		return -1;
	}
	if((aimPoint!=0)&&(aimPoint!=255))
	{
		std::cout<<"aimPoint error !"<<std::endl;
		return -1;
	}
	int cnt = 0;
	//统计行
	if(mode == 0)
	{
		for(int i=0;i<img.rows;i++)
		{
			cnt = 0;
			for(int j=0;j<img.cols;j++)
			{
				if(img.at<uchar>(i,j)==aimPoint)
				{
					cnt++;
				}
			}
			result.at<uchar>(i,0) = cnt;
		}
	}
	else if(mode == 1)
	{
		for(int i=0;i<img.cols;i++)
		{
			cnt = 0;
			for(int j=0;j<img.rows;j++)
			{
				if(img.at<uchar>(j,i)==aimPoint)
				{
					cnt++;
				}
			}
			result.at<uchar>(i,0) = cnt;
		}
	}
	return 0;
}

int main()
{
	cv::Mat img = cv::imread("E:\\图像处理\\project\\车牌识别\\素材\\车1.jpg");
	if(!img.data)
	{
		std::cout<<"open pic error !"<<std::endl;
		return -1;
	}
	//cv::imshow("img",img);
	//1.直接转灰度图
	cv::Mat imgGray;
	cv::cvtColor(img,imgGray,CV_BGR2GRAY);
	//cv::imshow("imgGray",imgGray);

	//开操作
	cv::Mat imgOpen = imgGray.clone();
	cv::Mat elementOpen = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(10,10));
	morphologyEx(imgGray,imgOpen,cv::MORPH_OPEN,elementOpen);
	//cv::imshow("imgOpen",imgOpen);

	//减运算  去除亮背景
	cv::Mat imgSub;
	imgSub = imgGray-imgOpen;
	//cv::imshow("imgSub",imgSub);

	//滤波
	cv::Mat imgBlur;
	cv::GaussianBlur(imgSub,imgBlur,cv::Size(5,5),0,0);
	//cv::imshow("imgBlur",imgBlur);

	//增强边缘
	cv::Mat imgAdd;
	imgAdd = imgSub-imgBlur+imgSub;
	//cv::imshow("imgAdd",imgAdd);

	//提取边缘
	cv::Mat gradX;
	cv::Mat imgSobel;
	cv::Sobel(imgAdd,gradX,CV_16S,1,0,3,1,1,cv::BORDER_DEFAULT);
	cv::convertScaleAbs(gradX,imgSobel);
	//cv::imshow("imgSobel",imgSobel);

	//阈值化
	cv::Mat imgThreshold;
	cv::threshold(imgSobel,imgThreshold,254,255,cv::THRESH_BINARY);
	//cv::imshow("imgThreshold",imgThreshold);

	//闭操作
	cv::Mat imgClose = imgThreshold.clone();
	cv::Mat elementClose = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(11,11));
	morphologyEx(imgThreshold,imgClose,cv::MORPH_CLOSE,elementClose);
	//cv::imshow("imgClose",imgClose);

	//降噪
	cv::Mat imgRemoveNoise = imgClose.clone();
	RemoveNoiseByRegion(imgRemoveNoise,100,1);
	//cv::imshow("imgRemoveNoise",imgRemoveNoise);
	//寻找边界
	cv::Mat imgContour = imgRemoveNoise.clone();
	cv::vector<cv::vector<cv::Point>> contours; //双重向量
	cv::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgContour,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
	//绘制边界
	//cv::Mat imgTest = cv::Mat(img.size(),CV_8UC1,cv::Scalar(0));
	//for(int i=0;i<contours.size();i++)
	//{
	//	for(int j=0;j<contours[i].size();j++)
	//	{
	//		cv::Point p = cv::Point(contours[i][j].x,contours[i][j].y);
	//		imgTest.at<uchar>(p) = 255;
	//	}
	//}
	//cv::imshow("imgTest",imgTest);
	//提取区域
	//cv::imshow("imgContour",imgContour);

	cv::Rect Rect;
	cv::Mat ROI;
	cv::Mat ROIHSV;
	cv::vector<cv::Rect> RectList;
	int flag = 0;
	for(int i=0;i<contours.size();i++)
	{
		Rect = cv::boundingRect(contours[i]); //外接正矩形
		
		if(JudgeByRectSize(Rect))
		{
			RectList.push_back(Rect);
		}
	}
	std::cout<<"rectlist size:"<<RectList.size()<<std::endl;

	//根据hsv颜色范围判断
	if(RectList.size()>1)
	{
		int i;
		for(i=0;i<RectList.size();i++)
		{
			ROI = img(cv::Rect(RectList[i].x,RectList[i].y,RectList[i].width,RectList[i].height));
			cv::cvtColor(ROI,ROIHSV,CV_BGR2HSV);
			cv::Mat aa;
			GetColorFromHsv(ROIHSV,BLUE,aa);
			if(JudgeByColorPercent(aa,30))
			{
				std::cout<<"Rect "<<i<<" is ok "<<std::endl;
				flag = 1;
				break;
			}
		}
		if(flag == 1)
		{
			Rect = RectList[i];
		}
		else
		{
			std::cout<<"can not find rect "<<std::endl;
			cv::waitKey(0);
			return 0;
		}
	}
	else
	{
		Rect = RectList[0];
	}
	ROI = img(cv::Rect(Rect.x,Rect.y,Rect.width,Rect.height));
	cv::imwrite("E:\\图像处理\\素材\\rect.bmp",ROI);
	cv::rectangle(img,Rect,cv::Scalar(0,0,255),2,8);
	cv::imshow("img",img);
	cv::waitKey(0);
	return 0;
}
