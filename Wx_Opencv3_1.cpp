// WinXin_OpenCv3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <fstream>//注意：一旦包含了这个文件，你不再需要（为了使用cout/cin）包含iostream.h，因为fstream.h已经自动包含了它。
#include <math.h>
#include <Windows.h>
#include <stdlib.h>


using namespace		cv;
using namespace 	std;

Mat src_Image;
Mat dst_Image;
Mat Character;
static int i = 0;

//获取手机图片
//Get cell phone picture
void get_sourceimg();
//获取棋子坐标
//Get chess coordinates
Point Get_Pos(Mat& Src_Image, Mat& Tem_Image);
//获取下一个点的坐标
//Get the coordinates of the next point
Point Get_NextPos(Mat& Src_Image);
int Get_distance(Point& first_Point, Point& next_Point);
void jump(int&g_distance);

int _tmain(int argc, _TCHAR* argv[])
{
	int num = 1;
	while (num)
	{
		//get_sourceimg();
		namedWindow("Src", CV_WINDOW_NORMAL);
		src_Image = imread("autojump.png");	// Load an image from file
		dst_Image = src_Image.clone();
		Character = imread("object.png");
		if (src_Image.empty() || Character.empty())
		{
			cout << "could not open or find the image" << endl;
			cin.get();
			return -1;
		}
		Point Character_p = Get_Pos(src_Image, Character);
		cout << "Character_Pos: " << Character_p << endl;
		Point next_p = Get_NextPos(src_Image);
		cout << "Next_Pos: " << next_p << endl;
		int g_distance = Get_distance(Character_p, next_p);
		cout << "Distance: " << g_distance << endl;
		//jump(g_distance);
		num--;
		Sleep(1000);
	}

	waitKey(0);
	return 0;
}

//-----------获取手机图片----------
//-----------Get cell phone picture------------
void get_sourceimg()
{
	system("adb shell screencap -p /sdcard/autojump.png");
	system("adb pull /sdcard/autojump.png");
}

//------------获取当前位置---------------
//------------Get the current location-----------
Point Get_Pos(Mat& Src_Image, Mat& Tem_Image)
{
	matchTemplate(Src_Image, Tem_Image, dst_Image, CV_TM_SQDIFF);
	double minVal, maxVal;
	Point minLoc, maxLoc, MatchLoc;
	//寻找最佳匹配位置  
	//Find the best match
	minMaxLoc(dst_Image, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	MatchLoc = minLoc;
	//标记  mark
	//rectangle(Src_Image, Rect(MatchLoc, Size(Character.cols, Character.rows)), Scalar(255, 255, 0), 1, 8, 0);
	//putText(Src_Image, "object", Point(MatchLoc.x + Character.cols*0.5, MatchLoc.y + Character.rows), 2, 2, Scalar(0, 0, 255));
	return Point(MatchLoc.x + Character.cols*0.5, MatchLoc.y + Character.rows);
}


//------------------获取下一点位置------------------
//-----------------Get the next location--------------
Point Get_NextPos(Mat& Src_Image)
{
	Point point1;
	Point point2;
	Point Middle_P;
	int nY_Min = Src_Image.rows;
	int nX_Min = Src_Image.cols;

	int nY_Max = 0;
	int nX_Max = 0;
	int nIdY = 0;

	cvtColor(Src_Image, Src_Image, COLOR_BGR2GRAY);
	blur(Src_Image, Src_Image, Size(5, 5));
	dilate(Src_Image, Src_Image, 0);
	Canny(Src_Image, Src_Image, 2.5, 8.4);
	//imshow("Src", src_Image);

	vector<vector<Point>> contours;//二维向量容器, 获取轮廓 
	//vector<Vec4i> hierarchy;  //Error!!: vector iterator + offset out of range
	findContours(Src_Image, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point());//找到关键的角点

	//vector<vector<Point> > ::const_iterator it = contours.begin();
	std::vector<std::vector<Point>>::const_iterator it = contours.begin();
	while (it + 1 != contours.end())
	{
		if (it->size() < 200)
		{
			it = contours.erase(it);
			//cout << "it: " << it->size() << endl;
		}
		else
		{
			it++;
		}
	}

	for (int i = 0; i < contours.size(); i++)
	{
		//	cout << i<<"  :   " << contours[i].size() << endl << endl;
		if (contours[i].size() > 200)
		{
			for (int j = 0; j < contours[i].size(); j++)
			{			//contours[i]代表的是第i个轮廓，contours[i].size()代表的是第i个轮廓上所有的像素点数  
						// "contours[i]"Represents the i-th outline, "contours[i].size()"Represents the i-th outline of all the pixels
				if (contours[i][j].y < nY_Min)
				{
					nY_Min = contours[i][j].y;   //找到最低的y值, Find the lowest y value
					point1 = contours[i][j];
					nIdY = i;					//记录该轮廓contours[nIdY]
				}
			}
		}
	}

	for (int j = 0; j < contours[nIdY].size(); j++)//在该轮廓继续找到x最大值nX_Max;
	{
		if (contours[nIdY][j].x > nX_Max && contours[nIdY][j].x < Src_Image.cols)
		{
			nX_Max = contours[nIdY][j].x;
			point2 = contours[nIdY][j];
		}
	}


	int minY = Src_Image.cols;
	//nX_Max 匹配多个Y值, 在多个Y值中查找最小的那个
	for (int j = 0; j < contours[nIdY].size(); j++)
	{
		if (contours[nIdY][j].x == nX_Max && contours[nIdY][j].y < minY)
		{
			minY = contours[nIdY][j].y;
			point2 = contours[nIdY][j];
		}
	}

	Middle_P = Point(point1.x, point2.y);
	circle(Src_Image, Middle_P, 20, Scalar(255, 0, 0));
	imshow("Src", Src_Image);
	return Point(point1.x, point2.y);
}

//--------------获取距离----------------------
//--------------Get the distance--------------------
int Get_distance(Point& first_Point, Point& next_Point)
{
	int A = first_Point.x - next_Point.y;
	int B = first_Point.y - next_Point.y;
	return int(pow(pow(A, 2) + pow(B, 2), 0.5));//根号下 x^2 + Y^2;
}

//-------------------跳-----------------------
//-------------------jump---------------------
void jump(int &g_distance)
{
	char AA[50];
	double distance_ = g_distance * 1.35;

	int rand_x = int(320 + rand() % 50);
	int rand_y = int(410 + rand() % 50);
	sprintf_s(AA, "adb shell input swipe %d %d %d %d %d", rand_x, rand_y, rand_x, rand_y, (int)distance_);
	cout << AA << endl;
	cout << distance_ << endl;
	system(AA);
}
