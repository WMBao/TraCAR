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
		//�򿪴���
		hCom = CreateFile(portName, //���ں�
			GENERIC_READ | GENERIC_WRITE, //�������д
			0, //��ռ��ʽ
			NULL,
			OPEN_EXISTING, //�򿪶����Ǵ���
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //�ص���ʽ���첽I/O�ṹ
			NULL
			);
		if (hCom == INVALID_HANDLE_VALUE)
		{
			dwError = GetLastError(); //ȡ�ô������
			cout << portName << " Opened Fail. Error code: " << dwError << endl;
		}
		else
		{
			bComOpened = TRUE;
			cout << portName << " Opened Successful." << endl;
		}
		//���û�������С
		if (!SetupComm(hCom, 2048, 2048))
		{ 
			//���뻺����������������Ĵ�С����2048�ֽ�
			cout << "SetupComm Fail! Close Comm!" << endl;
			CloseHandle(hCom);
		}
		else
		{
			cout << "SetupComm OK!" << endl;
		}
		COMMTIMEOUTS TimeOuts;
		memset(&TimeOuts, 0, sizeof(TimeOuts));
		//�趨����ʱ
		TimeOuts.ReadIntervalTimeout = 1000;
		TimeOuts.ReadTotalTimeoutMultiplier = 500;
		TimeOuts.ReadTotalTimeoutConstant = 5000;
		//�趨д��ʱ
		TimeOuts.WriteTotalTimeoutMultiplier = 500;
		TimeOuts.WriteTotalTimeoutConstant = 2000;
		//�趨��ʱ
		SetCommTimeouts(hCom, &TimeOuts);
		//���ô��ڲ���
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
		//�趨������Ϊ9600������żУ��λ���ֽڳ���Ϊ8λ��1��ֹͣλ
		if (!BuildCommDCB("9600,n,8,1", &dcb))
		{
			cout << "BuileCOmmDCB Fail,Comm Close!" << endl;
			CloseHandle(hCom);
		}
		if (SetCommState(hCom, &dcb))
		{
			cout << "SetCommState OK!" << endl;
		}
		//��ʼ���ص��ṹ
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
		//�첽������
		DWORD dwBytesRead = 1024;
		COMSTAT ComStat;
		DWORD dwErrorFlags;
		OVERLAPPED m_osRead;
		memset(&m_osRead, 0, sizeof(OVERLAPPED));
		m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		ClearCommError(hCom, &dwErrorFlags, &ComStat);//��֮ǰ�������
		dwBytesRead = min(dwBytesRead, (DWORD)ComStat.cbInQue);
		if (!dwBytesRead)
			return FALSE;
		BOOL bReadStatus;
		bReadStatus = ReadFile(hCom, lpInBuffer,
			dwBytesRead, &dwBytesRead, &m_osRead);
		if (!bReadStatus) 
		{
			if (GetLastError() == ERROR_IO_PENDING) //GetLastError()��������ERROR_IO_PENDING,�����������ڽ��ж�����
			{
				WaitForSingleObject(m_osRead.hEvent, 2000);	//ʹ��WaitForSingleObject�����ȴ�,ֱ����������ɻ���ʱ�Ѵﵽ2���ӣ������ڶ�����������Ϻ�m_osRead��hEvent�¼����Ϊ���ź�
				PurgeComm(hCom, PURGE_TXABORT |
					PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
				return dwBytesRead;
			}
			return 0;
		}
		PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);//��մ��ڵ��������������
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




