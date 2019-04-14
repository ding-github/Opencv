#include "common.h"

int GetHistPic(cv::Mat Pic,cv::Mat &histPic,int hist_w,int hist_h)
{
	int type = Pic.type();
	cv::Mat tempPic;
	if((hist_w<=0)||(hist_h<=0))
	{
		std::cout<<"param error"<<std::endl;
	}
	if(type == CV_8UC1)
	{
		tempPic = Pic.clone();
	}
	else
	{
		cv::cvtColor(Pic,tempPic,CV_BGR2GRAY);
	}

	const int channels = 0;
	const int histSize = 256;
	float pranges[2] = {0,255};
	const float* ranges[1] = {pranges};
	cv::MatND hist;

	cv::calcHist(&tempPic,1,&channels,cv::Mat(),hist,1,&histSize,ranges);
	int nHistSize = 255;
	int bin_w = cvRound((double)hist_w/nHistSize);
	//cv::Mat histImg(hist_w,hist_h,CV_8UC3,cv::Scalar(255,255,255));
	normalize(hist,hist,0,histPic.rows,cv::NORM_MINMAX,-1,cv::Mat());
	for(int i=0;i<nHistSize;i++)
	{
		line(histPic,cv::Point(bin_w*(i),hist_h),cv::Point(bin_w*(i),cvRound(hist_h-hist.at<float>(i))),cv::Scalar(255,0,255),2,8,0);
	}
	return 0;
}

//¡¨Õ®”ÚΩµ‘Î
int RemoveNoiseByRegion(cv::Mat &pic,int threshold,int mode)
{
	int row = pic.rows;
	int col = pic.cols;
	int color = 1;
	int Noise;
	int NotNoise;

	if(mode == 0)
	{
		Noise = 0;
		NotNoise = 255;
	}
	else if(mode == 1)
	{
		Noise = 255;
		NotNoise = 0;
	}
	else
	{
		std::cout<<"mode error"<<std::endl;
		return -1;
	}

	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			if((pic.at<uchar>(i,j)==Noise)&&(color<255))
			{
				cv::floodFill(pic,cvPoint(j,i),cv::Scalar(color));
				color++;
			}
			
		}
	}

	int colorcnt[256] = {0};
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			if((pic.at<uchar>(i,j)>0)&&(pic.at<uchar>(i,j)<255))
			{
				colorcnt[pic.at<uchar>(i,j)]++;
			}
		}
	}
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			if((pic.at<uchar>(i,j)>0)&&(pic.at<uchar>(i,j)<255))
			{
				if(colorcnt[pic.at<uchar>(i,j)]<=threshold)
				{
					pic.at<uchar>(i,j) = NotNoise;
				}
				else
				{
					pic.at<uchar>(i,j) = Noise;
				}
			}
			
		}
	}
	return 0;
}