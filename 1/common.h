#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int GetHistPic(cv::Mat Pic,cv::Mat &histPic,int hist_w,int hist_h);
int RemoveNoiseByRegion(cv::Mat &pic,int threshold,int mode);