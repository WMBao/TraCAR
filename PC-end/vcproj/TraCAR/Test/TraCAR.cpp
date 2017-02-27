#include "stdafx.h"
#include <iostream>
#include "the_car.h"
#include "cv.h"
#include "highgui.h"

using namespace std;

int main()
{
	system("color f1");
	the_car ourCar;
	cout << "***********" << endl;
	char tmp = 0;
	cvNamedWindow("test");
	while (tmp != 13)
	{
		tmp = cvWaitKey(1);
		cout << tmp << endl;
		switch (tmp)
		{
			case 'W':ourCar.move_f(); break;
			case 'S':ourCar.move_b(); break;
			case 'A':ourCar.move_l(); break;
			case 'D':ourCar.move_r(); break;
			case 'Q':ourCar.move_s(); break;
		}
	}
	return 0;
}
