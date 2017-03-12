#pragma once
#include <Windows.h>
#include <iostream>
using namespace std;

class ComPort
{
public:
	HANDLE hCom;
	DWORD dwError;
	BOOL bComOpened;
	OVERLAPPED wrOverlapped;
	ComPort(LPCSTR portName)
	{
		hCom = NULL;
		dwError = NULL;
		bComOpened = NULL;
		//打开串口
		hCom = CreateFile(portName, //串口号
			GENERIC_READ | GENERIC_WRITE, //充许读和写
			0, //独占方式
			NULL,
			OPEN_EXISTING, //打开而不是创建
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //重叠方式：异步I/O结构
			NULL
			);
		if (hCom == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError(); //取得错误代码
			cout << portName << " Opened Fail. Error code: " << dwError << endl;
		}
		else
		{
			bComOpened = TRUE;
			cout << portName << " Opened Successful." << endl;
		}
		//设置缓冲区大小
		if (!SetupComm(hCom, 2048, 2048))
		{ 
			//输入缓冲区和输出缓冲区的大小都是2048字节
			cout << "SetupComm Fail! Close Comm!" << endl;
			CloseHandle(hCom);
		}
		else
		{
			cout << "SetupComm OK!" << endl;
		}
		COMMTIMEOUTS TimeOuts;
		memset(&TimeOuts, 0, sizeof(TimeOuts));
		//设定读超时
		TimeOuts.ReadIntervalTimeout = 1000;
		TimeOuts.ReadTotalTimeoutMultiplier = 500;
		TimeOuts.ReadTotalTimeoutConstant = 5000;
		//设定写超时
		TimeOuts.WriteTotalTimeoutMultiplier = 500;
		TimeOuts.WriteTotalTimeoutConstant = 2000;
		//设定超时
		SetCommTimeouts(hCom, &TimeOuts);
		//设置串口参数
		DCB dcb;
		if (!GetCommState(hCom, &dcb))
		{
			cout << "GetCommState Fail! Close Comm!" << endl;
			CloseHandle(hCom);
		}
		else
		{
			cout << "GetCommState OK!" << endl;
		}
		dcb.DCBlength = sizeof(dcb);
		//设定波特率为9600，无奇偶校验位，字节长度为8位，1个停止位
		if (!BuildCommDCB("9600,n,8,1", &dcb))
		{
			cout << "BuileCOmmDCB Fail,Comm Close!" << endl;
			CloseHandle(hCom);
		}
		if (SetCommState(hCom, &dcb))
		{
			cout << "SetCommState OK!" << endl;
		}
		//初始化重叠结构
		ZeroMemory(&wrOverlapped, sizeof(wrOverlapped));
		if (wrOverlapped.hEvent != NULL)
		{
			ResetEvent(wrOverlapped.hEvent);
			wrOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR); 
		txIndex = 0;
		rxIndex = 0;
	}

	virtual ~ComPort(void)
	{
		if (bComOpened)
			CloseHandle(hCom);
	}

	unsigned char rxIndex;
	unsigned char lpInBuffer[1024];

	int Sync(void)
	{
		int i = 1000;
		while (rxIndex != txIndex && txIndex != 0 && (--i))
		{
			int size = Read();
			for (int i = 0; i<size; i++)
			if (((lpInBuffer[i] & 0x80) == 0) && (lpInBuffer[i] != 0))
				rxIndex = lpInBuffer[i];
		}
		if (i<1)
			return -1;
		return rxIndex;
	}

	int Read(void)
	{	
		//异步读串口
		DWORD dwBytesRead = 1024;
		COMSTAT ComStat;
		DWORD dwErrorFlags;
		OVERLAPPED m_osRead;
		memset(&m_osRead, 0, sizeof(OVERLAPPED));
		m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		ClearCommError(hCom, &dwErrorFlags, &ComStat);//读之前清除错误
		dwBytesRead = min(dwBytesRead, (DWORD)ComStat.cbInQue);
		if (!dwBytesRead)
			return FALSE;
		BOOL bReadStatus;
		bReadStatus = ReadFile(hCom, lpInBuffer,
			dwBytesRead, &dwBytesRead, &m_osRead);
		if (!bReadStatus) 
		{
			if (GetLastError() == ERROR_IO_PENDING) //GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
			{
				WaitForSingleObject(m_osRead.hEvent, 2000);	//使用WaitForSingleObject函数等待,直到读操作完成或延时已达到2秒钟；当串口读操作进行完毕后，m_osRead的hEvent事件会变为有信号
				PurgeComm(hCom, PURGE_TXABORT |
					PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return dwBytesRead;
			}
			return 0;
		}
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);//清空串口的输入输出缓冲区
		return dwBytesRead;
	}

	unsigned char txIndex;

	int Send(unsigned char command)
	{
		//command |= 0x80;
		++txIndex;
		txIndex &= ~0x80;
		DWORD dwError;
		DWORD dwWantSend = 1;
		DWORD dwRealSend = 0;
		char* pReadBuf = NULL;
		unsigned char* pSendBuffer = &command;
		if (ClearCommError(hCom, &dwError, NULL))
		{
			PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);
			cout<<"PurgeComm OK!"<<endl;
		}
		if (!WriteFile(hCom, pSendBuffer, dwWantSend, &dwRealSend, &wrOverlapped))
		{
			dwError = GetLastError();
			if (dwError == ERROR_IO_PENDING)
			{
				while (!GetOverlappedResult(hCom, &wrOverlapped, &dwRealSend, FALSE))
				{
					if (GetLastError() == ERROR_IO_INCOMPLETE)
					{
						cout<<"Sending..."<<endl;
						continue;
					}
					else
					{
						cout << "Send Fail. Error code: " << dwError << endl;
						ClearCommError(hCom, &dwError, NULL);
						break;
					}
				}
			}
		}
		return dwRealSend;
	}

	bool Close(void)
	{
		CloseHandle(hCom);
	}
};




