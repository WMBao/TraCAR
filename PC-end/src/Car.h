#ifndef CAR

#include "Port.h"

class Car
{
	ComPort port;
	char state;//s,f,b,r,l...x,y,1(oppo.l),2(oppo.r)
	unsigned char tmpSend;
public:
	Car():port(ComPort("COM7"))
	{
		state = 'S';
		move_s();
	}
	void restart()
	{
		state = 'S';
		tmpSend = 'S';
		port.Send(tmpSend);
	}
	void move_f()
	{
		cout << "Forward...\n";
		if (state == 'F')
			return;
		tmpSend = 'F';
		port.Send(tmpSend);
		state = 'F';
	}
	void move_b()
	{
		cout << "Back...\n";
		if (state == 'B')
			return;
		tmpSend = 'B';
		port.Send(tmpSend);
		state = 'B';
	}
	void move_r()
	{
		cout << "Right...\n";
		if (state == 'R')
			return;
		tmpSend = 'R';
		port.Send(tmpSend);
		state = 'R';
	}
	void move_l()
	{
		cout << "Left...\n";
		if (state == 'L')
			return;
		tmpSend = 'L';
		port.Send(tmpSend);
		state = 'L';
	}
	void move_lb()
	{
		cout << "Leftbehind...\n";
		if (state == 'Z')
			return;
		tmpSend = 'Z';
		port.Send(tmpSend);
		state = 'Z';
	}
	void move_rb()
	{
		cout << "Rightbehind...\n";
		if (state == 'X')
			return;
		tmpSend = 'X';
		port.Send(tmpSend);
		state = 'X';
	}
	void move_s()
	{
		cout << "Stop...\n";
		if (state == 'S')
			return;
		tmpSend = 'S';
		port.Send(tmpSend);
		state = 'S';
	}
};

#endif

/*
void move_lb()
{
cout << "leftbehind...\n";
if (state == 'Z')
return;
tmpSend[0] = 'Z';
tmpSend[1] = 'Z';
tmpSend[2] = 'Z';
port.Send(tmpSend);
state = 'Z';
}
void move_rb()
{
cout << "rightbehind...\n";
if (state == 'X')
return;
tmpSend[0] = 'X';
tmpSend[1] = 'X';
tmpSend[2] = 'X';
port.Send(tmpSend);
state = 'X';
}
*/