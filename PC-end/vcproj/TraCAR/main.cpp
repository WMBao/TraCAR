#include "stdafx.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <highgui.h>
#include <cv.h>
#include <cmath>
#include "Car.h"
#include "Image.h"

using namespace cv;
using namespace std;


//车
Car car;
//原始图像
IplImage *colorfulImg = NULL;
//跟踪轨迹的二值化前景图
IplImage *lineImg = NULL;
//中间图像
IplImage *tmp,*tmp2;
//允许的最多角点数
int mycount = 100;
CvPoint2D32f originPoints[4];
CvPoint2D32f newPoints[4];
CvPoint carPos_head;
CvPoint carPos_tail;
CvPoint2D32f corners[100];
bool visited[100];
int lastPoint = -1;
CvMat *transMat = cvCreateMat(3, 3, CV_32FC1);
CvCapture *pCapture;
int RGB = 70;
int times = 15;
int step = 1;
// 判断当前状态的全局变量
// step == 1, 选择透视变换的左上角
// step == 2, 选择透视变换的右上角
// step == 3, 选择透视变换的左下角
// step == 4, 选择透视变换的右下角
// step == 5, 创建二值化图像
// step == 6, 寻找角点，通过像素点数组corners记录
// step == 7，动态跟踪小车，判断方向
// step == 8, 退出


void cvThin(IplImage* src, IplImage* dst, int iterations = 1);
void mouseEvent(int mouseevent, int x, int y, int flags, void* param);
void exchange(IplImage *img);
void findBlack();
void getTrack();
double calculateAngle(CvPoint2D32f roadPoint);
void destory();
void instruction(double Angel);
//void hough(IplImage *img);
double getDistance(CvPoint2D32f x, CvPoint2D32f y);
bool carPosition(IplImage* src, CvPoint& carPos_head, CvPoint& carPos_tail);

int main()
{
	bool flag = false;
	bool exit = true;
	//防止摄像头初始化失败
	do {
		pCapture = cvCaptureFromCAM(1);
	} while (!pCapture);
	//创建窗口并移动到指定位置
	cvNamedWindow("colorfulImg", 1);
	cvNamedWindow("lineImg", 1);
	cvMoveWindow("colorfulImg", 0, 0);
	cvMoveWindow("lineImg", 690, 0);
	
	while (exit)
	{
		if (!(colorfulImg = cvQueryFrame(pCapture)))
			continue;
		if (step == 5)
		{
			exchange(colorfulImg);
			lineImg = cvCreateImage(cvSize(colorfulImg->width, colorfulImg->height),IPL_DEPTH_8U, 1);
			flag = true;
			findBlack();
		}
		else if (step == 6)
		{
			exchange(colorfulImg);
			tmp = cvCreateImage(cvGetSize(colorfulImg), IPL_DEPTH_8U, 1);
			tmp2 = cvCreateImage(cvGetSize(colorfulImg), IPL_DEPTH_8U, 1);
			cvGoodFeaturesToTrack(lineImg, tmp, tmp2, corners, &mycount, 0.05, 70);
			for (int i = 0; i < mycount; ++i)
			{
				visited[i] = false;
				cvRectangle(colorfulImg, cvPoint((int)corners[i].x - 5, (int)corners[i].y - 5), cvPoint((int)corners[i].x + 5, (int)corners[i].y + 5), CV_RGB(0, 255, 0), 1, CV_AA, 0);
				/*cout << "**********************************" << endl;
				cout << "**********************************" << endl;
				cout << "**********************************" << endl;
				cout << i << endl;
				cout << i << endl;
				cout << i << endl;
				cout << i << endl;
				cout << "**********************************" << endl;
				cout << "**********************************" << endl;
				cout << "**********************************" << endl;*/
			}
		}
		else if (step == 7)
		{
			exchange(colorfulImg);
			carPosition(colorfulImg, carPos_head, carPos_tail);
			getTrack();
		}
		//将图像显示在对应窗口
		cvShowImage("colorfulImg", colorfulImg);
		cvShowImage("lineImg", lineImg);
		//step 1—4
		cvSetMouseCallback("colorfulImg", mouseEvent);
		cvCreateTrackbar("Step", "colorfulImg", &step, 8, NULL);
		cvCreateTrackbar("RGB", "colorfulImg", &RGB, 255, NULL);
		cvCreateTrackbar("times", "colorfulImg", &times, 300, NULL);

		cvWaitKey(1);

		if (step == 5 && flag)
		{
			step++;//自动进入第六步
			cvWaitKey(1000);
			flag = false;
		}
		else if (step == 8)
		{
			cout << "Successful!";
			car.move_s();
			//destory();
			return 0;
		}
	}
}

void mouseEvent(int mouseevent, int x, int y, int flags, void *param)
{
	if (step == 1 && mouseevent == CV_EVENT_LBUTTONDOWN)
	{
		originPoints[0] = cvPoint2D32f(x, y);
		++step;
	}
	else if (step == 2 && mouseevent == CV_EVENT_LBUTTONDOWN)
	{
		originPoints[1] = cvPoint2D32f(x, y);
		++step;
	}
	else if (step == 3 && mouseevent == CV_EVENT_LBUTTONDOWN)
	{
		originPoints[2] = cvPoint2D32f(x, y);
		++step;
	}
	else if (step == 4 && mouseevent == CV_EVENT_LBUTTONDOWN)
	{
		originPoints[3] = cvPoint2D32f(x, y);
		++step;
	}
}

void exchange(IplImage *img)
{
	if (step == 5)
	{
		newPoints[0] = cvPoint2D32f(0, 0);
		newPoints[1] = cvPoint2D32f(img->width, 0);
		newPoints[2] = cvPoint2D32f(0, img->height);
		newPoints[3] = cvPoint2D32f(img->width, img->height);
		cvGetPerspectiveTransform(originPoints, newPoints, transMat);
	}
	cvWarpPerspective(img, img, transMat);
}

void findBlack()
{
	RgbImage  imgA(colorfulImg);
	BwImage   imgB(lineImg);
	for (int i = 0; i<colorfulImg->height; i++)
	{
		for (int j = 0; j<colorfulImg->width; j++)
		{
			if (imgA[i][j].b<RGB && imgA[i][j].g<RGB && imgA[i][j].r<RGB)
			{
				imgB[i][j] = 255;
			}
			else
			{
				imgB[i][j] = 0;
			}
		}
	}
	cvThin(lineImg, lineImg, times);
	//hough(lineImg);
}

//经典细化算法
void cvThin(IplImage* src, IplImage* dst, int iterations)
{
	cvCopyImage(src, dst);
	BwImage dstdat(dst);
	IplImage* t_image = cvCloneImage(src);
	BwImage t_dat(t_image);
	for (int n = 0; n < iterations; n++)
	for (int s = 0; s <= 1; s++) {
		cvCopyImage(dst, t_image);
		for (int i = 0; i < src->height; i++)
		for (int j = 0; j < src->width; j++)
		if (t_dat[i][j]) {
			int a = 0, b = 0;
			int d[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 },
			{ 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };
			int p[8];
			p[0] = (i == 0) ? 0 : t_dat[i - 1][j];
			for (int k = 1; k <= 8; k++) {
				if (i + d[k % 8][0] < 0 || i + d[k % 8][0] >= src->height ||
					j + d[k % 8][1] < 0 || j + d[k % 8][1] >= src->width)
					p[k % 8] = 0;
				else p[k % 8] = t_dat[i + d[k % 8][0]][j + d[k % 8][1]];
				if (p[k % 8]) {
					b++;
					if (!p[k - 1]) a++;
				}
			}
			if (b >= 2 && b <= 6 && a == 1)
			{
				if (!s && !(p[2] && p[4] && (p[0] || p[6])))
					dstdat[i][j] = 0;
				else if (s && !(p[0] && p[6] && (p[2] || p[4])))
					dstdat[i][j] = 0;
			}
		}
	}
	cvReleaseImage(&t_image);
}

bool carPosition(IplImage* src, CvPoint& carPos_head, CvPoint& carPos_tail)
{
	IplImage* img_pocessed = cvCloneImage(src);
	CvSize img_size = cvGetSize(img_pocessed);
	IplImage* img_hsv = cvCreateImage(img_size, IPL_DEPTH_8U, 3);
	cvCvtColor(img_pocessed, img_hsv, CV_RGB2HSV);
	IplImage* img_h = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_s = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_v = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_b = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_g = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_r = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_red = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	IplImage* img_blue = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	cvSplit(img_pocessed, img_b, img_g, img_r, 0);
	cvSplit(img_hsv, img_h, img_s, img_v, 0);
	cvThreshold(img_r, img_r, 90, 255, CV_THRESH_BINARY);//90需调试
	cvThreshold(img_g, img_g, 90, 255, CV_THRESH_BINARY);//90需调试
	cvThreshold(img_b, img_b, 100, 255, CV_THRESH_BINARY);
	cvThreshold(img_s, img_s, 90, 255, CV_THRESH_BINARY);//90需调试
	cvThreshold(img_h, img_h, 150, 255, CV_THRESH_BINARY_INV);
	cvThreshold(img_v, img_v, 40, 255, CV_THRESH_BINARY);
	//饱和度和红度二值化后的图像按位与
	cvAnd(img_r, img_s, img_red);
	//腐蚀，用于消除噪音和独点
	cvErode(img_red, img_red, 0, 2);
	//使用cvMoments求红色区域重心
	CvMoments moments_red, moments_blue;
	cvMoments(img_red, &moments_red, 1);
	//cvShowImage("red", img_red);
	double m00, m10, m01;
	m00 = cvGetSpatialMoment(&moments_red, 0, 0);
	m10 = cvGetSpatialMoment(&moments_red, 1, 0);
	m01 = cvGetSpatialMoment(&moments_red, 0, 1);
	//cvRound--将double四舍五入转换成int
	int red_x = cvRound(m10 / m00);
	int red_y = cvRound(m01 / m00);
	//找不到红点说明找不到车
	if (red_x>img_size.width || red_x<0 || red_y>img_size.height || red_y<0)
	{
		cout << "Fail to find the car!" << endl;
		return false;
	}
	//以红点为中心建立一个白矩形，将蓝点的搜索范围限制于此
	int length = 150;//length < 240
	int x = (red_x - length>0) ? red_x - length : 0;
	int y = (red_y - length>0) ? red_y - length : 0;
	int width = (red_x + length < img_size.width) ? 2 * length : img_size.width - x;
	int height = (red_y + length < img_size.height) ? 2 * length : img_size.height - y;
	IplImage* mask = cvCreateImage(img_size, IPL_DEPTH_8U, 1);
	cvSetZero(mask);
	cvSetImageROI(mask, cvRect(x, y, width, height));
	cvSet(mask, cvScalar(255, 0, 0, 0));
	cvResetImageROI(mask);
	cvAnd(img_s, mask, mask);
	cvAnd(img_b, mask, img_blue);
	cvErode(img_blue, img_blue, 0, 2);
	//找到蓝色区域重心
	cvMoments(img_blue, &moments_blue, 1);
	m00 = cvGetSpatialMoment(&moments_blue, 0, 0);
	m10 = cvGetSpatialMoment(&moments_blue, 1, 0);
	m01 = cvGetSpatialMoment(&moments_blue, 0, 1);
	cvResetImageROI(img_blue);
	//cvShowImage("blue", img_blue);
	int blue_x = 0;
	int blue_y = 0;
	blue_x = cvRound(m10 / m00);
	blue_y = cvRound(m01 / m00);
	if (blue_x>img_size.width || blue_x<0 || blue_y>img_size.height || blue_y<0)
	{
		cout << "Fail to find the car!" << endl;
		return false;
	}
	carPos_head = cvPoint(red_x, red_y);
	carPos_tail = cvPoint(blue_x, blue_y);
	cvCircle(src, carPos_head, 10, CV_RGB(173, 255, 47), 1);
	cvCircle(src, carPos_tail, 10, CV_RGB(192, 14, 235), 1);
	cvReleaseImage(&img_pocessed);
	cvReleaseImage(&img_hsv);
	cvReleaseImage(&img_b);
	cvReleaseImage(&img_g);
	cvReleaseImage(&img_s);
	cvReleaseImage(&img_h);
	cvReleaseImage(&img_v);
	cvReleaseImage(&img_red);
	cvReleaseImage(&img_blue);
	cvReleaseImage(&mask);
	return true;
}

void getTrack()
{
	CvPoint2D32f centre = cvPoint2D32f((carPos_head.x + carPos_tail.x) / 2, (carPos_head.y + carPos_tail.y) / 2);
	//初始化，找到最近的角点
	if (lastPoint == -1)
	{
		for (int i = 0; i < mycount; ++i)
		{
			if (lastPoint == -1)
			{
				lastPoint = i;
			}
			else if (getDistance(centre, corners[i]) < getDistance(centre, corners[lastPoint]))
			{
				lastPoint = i;
			}
		}
		visited[lastPoint] = true;//?
		return;
	}

	double Angle;
	int minDis = -1;

	for (int i = 0; i < mycount; ++i)
	{
		if (!visited[i] && i != lastPoint)
		{
			if (minDis == -1)
			{
				minDis = i;
			}
			else if (getDistance(corners[i], corners[lastPoint]) < getDistance(corners[minDis], corners[lastPoint]))
			{
				minDis = i;
			}
		}
	}
	if (minDis == -1)
	{
		++step;
	}
	else
	{
		Angle = calculateAngle(corners[minDis]);
		CvPoint2D32f head, tail;
		head.x = carPos_head.x;
		head.y = carPos_head.y;
		tail.x = carPos_tail.x;
		tail.y = carPos_tail.y;
		if (getDistance(corners[lastPoint], head) > getDistance(corners[lastPoint], tail))
		{
			visited[lastPoint] = true;
			cout << lastPoint << endl;

		}
		lastPoint = minDis;
		instruction(Angle);
	}
}

double getDistance(CvPoint2D32f x, CvPoint2D32f y)
{
	return sqrt(pow(x.x - y.x, 2) + pow(x.y - y.y, 2));
}

double calculateAngle(CvPoint2D32f roadPoint)
{
	CvPoint carVector = cvPoint(carPos_head.x - carPos_tail.x, carPos_head.y - carPos_tail.y),
		roadVector = cvPoint((int)roadPoint.x - carPos_head.x, (int)roadPoint.y - carPos_head.y);
	// 小车向量以及下一个路径点与车头的向量
	double carLength = sqrt(pow(carVector.x, 2) + pow(carVector.y, 2)),
		roadLength = sqrt(pow(roadVector.x, 2) + pow(roadVector.y, 2)),
		sinAngle = (carVector.x * roadVector.y - carVector.y * roadVector.x) / (carLength * roadLength);
	// 向量叉乘
	return asin(sinAngle) * 180 / 3.1415926;
}

void instruction(double Angle)
{
	if (Angle<=15 && Angle>=-15)
	{
		car.move_f();
	}
	else if (Angle > 15 && Angle < 70)
	{
		car.move_r();
		cvWaitKey(20);
	}
	else if (Angle >= 70)
	{
		car.move_lb();
		cvWaitKey(100);
	}
	else if (Angle < -15 && Angle > -70)
	{
		car.move_l();
		cvWaitKey(20);
	}
	else if (Angle <= -70)
	{
		car.move_rb();
		cvWaitKey(100);
	}
}

void destory()
{
	//停止小车
	car.move_s();
	//销毁窗口
	cvDestroyWindow("colorfulImg");
	cvDestroyWindow("lineImg");
	//释放图像和矩阵
	cvReleaseImage(&colorfulImg);
	cvReleaseImage(&lineImg);
	cvReleaseMat(&transMat);
	cvReleaseCapture(&pCapture);
}

