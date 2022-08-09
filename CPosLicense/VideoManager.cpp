
#include "stdafx.h"
#include "VideoManager.h"

#define ONTIMER_UINT UINT



#include "video_sdk/include/H264Play.h"
#include "video_sdk/include/NetSdk.h"



#ifdef _M_IX86 

#ifdef _DEBUG
#else
#endif

#pragma comment (lib, "video_sdk/x86/NetSdk.lib") 
#pragma comment (lib, "video_sdk/x86/H264Play.lib") 


#else

#pragma comment (lib, "video_sdk/x64/NetSdk.lib") 
#pragma comment (lib, "video_sdk/x64/H264Play.lib") 

#endif


#include <iostream>
#include <fstream>
#include <memory>
#include <Windows.h>





CVideoManager::CVideoManager()
{


	memset(&m_Device, 0, sizeof(m_Device));
	m_nDevNum = 0;//搜索到的设备个数
	m_hSearchDeviceThread = 0;

	m_hReConnectThread = 0;


	m_exit_rtsp = 0;
	
	memset(&m_timeSet, 0, sizeof(m_timeSet));

	m_camera_number = 0;
	
};

CVideoManager::~CVideoManager()
{
	VideoRelease();
};


static void SaveBitmap(unsigned char *data, int width, int height, int bpp, char *pName)
{
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*bpp / 8;

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // 反转图片
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	if (pName == 0) return;

	// 打开文件
	std::ofstream fout(pName, std::ofstream::out | std::ofstream::binary);
	if (!fout)
	{
		return;
	}
	// 使用结束后关闭
	std::shared_ptr<std::ofstream> foutCloser(&fout, [](std::ofstream *f) { f->close(); });

	fout.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BITMAPFILEHEADER));
	fout.write(reinterpret_cast<const char*>(&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER));
	fout.write(reinterpret_cast<const char*>(data), width * height * bpp / 8);
}






int CVideoManager::IsPicturePause(int index)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].IsPicturePause();

	return res;

}


int CVideoManager::PicturePause(int index, int pause)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].PicturePause(pause);

	return res;
}



DWORD CVideoManager::SearchDeviceFunction(LPVOID parm)
{
	CVideoManager *pThis = (CVideoManager*)parm;

	int nRetLength = 0;

	memset(&pThis->m_Device, 0, sizeof(pThis->m_Device));
	pThis->m_nDevNum = 0;//搜索到的设备个数

	bool bRet = H264_DVR_SearchDevice((char *)&pThis->m_Device, sizeof(pThis->m_Device), &nRetLength, 5000);

	if (bRet && nRetLength > 0)
	{
		pThis->m_nDevNum = nRetLength / sizeof(SDK_CONFIG_NET_COMMON_V2);

		SendMessage(pThis->m_pMainWnd->m_hWnd, WM_CAMERA_THREAD_SEARCH_MESSAGE, pThis->m_nDevNum, (LPARAM)(pThis->m_Device));

	}
	CloseHandle(pThis->m_hSearchDeviceThread);
	pThis->m_hSearchDeviceThread = NULL;

	return 0;
}

int CVideoManager::SearchDeviceThread()
{

	// 创建工作者线程  
	if (m_hSearchDeviceThread == 0)
	{
		m_nDevNum = 0;
		DWORD dwThreadID;
		m_hSearchDeviceThread = CreateThread(NULL, 0, SearchDeviceFunction, this, 0, &dwThreadID);
		return TRUE;
	}

	return FALSE;

}




//device disconnect callback
void __stdcall DisConnectBackCallFunc(LONG lLoginID, char *pchDVRIP,
	LONG nDVRPort, DWORD dwUser)
{
	CString str;
	str.Format(_T("%s断线了\n"), pchDVRIP);

	CVideoManager* pThis = (CVideoManager*)dwUser;
	if (pThis == NULL)
	{
		ASSERT(FALSE);
		return;
	}	
	
	//注销断线的连接, 之后重连线程会判断是否重连
	for (int i = 0; i < MAXWNDNUM; i++)
	{
		if (lLoginID == pThis->m_videoWnd[i].GetLoginHandle())
		{
			pThis->m_videoWnd[i].Disconnct();
			H264_DVR_Logout(pThis->m_videoWnd[i].m_devInfo.lLoginID);
			pThis->m_videoWnd[i].m_devInfo.lLoginID = -1;
		}
	}

	//if (m_bActiveConnect)
	//{
	//	StopRigister();
	//	OnCheckActiveConnect();
	//}

}


//初始化, 注册两个回调函数和参数
//fDisConnect cbDisConnect 掉线时的回调处理
//fMessCallBack cbAlarmcallback 发生警报时处理,例如检测到运动
int CVideoManager::VideoInit(fMessCallBack cbAlarmcallback, void *pWnd, CAMERA_USER_INFO *pUser, int num, int bAutoStart)
{
	int i = 0;

	if (num > MAXWNDNUM) return FALSE;
	for (i = 0; i < num; i++)
	{
		m_videoWnd[i].VideoInit(pUser++);
		if (bAutoStart)m_videoWnd[i].m_devInfo.lLoginID = -1;
	}
	m_camera_number = num;

	int res = 0;
	//initialize

	m_pMainWnd = (CWnd *)pWnd;//窗口


	BOOL bResult = H264_DVR_Init((fDisConnect)DisConnectBackCallFunc, this);

	//he messages received in SDK from DVR which need to upload， such as alarm information，diary information，may do through callback function
	H264_DVR_SetDVRMessCallBack(cbAlarmcallback, pWnd);

	H264_DVR_SetConnectTime(1000, 3);
	if (bAutoStart)
	{
		ReConnectThread(0);
	}

	return res;
}

void CVideoManager::VideoRelease()
{
	H264_DVR_Cleanup();
}


void CVideoManager::StopAllVideos()
{
	int i = 0;
	for (i = 0; i < m_camera_number; i++)
	{
		memset(&m_videoWnd[i].m_userInfo, 0, sizeof(CAMERA_USER_INFO));
		m_videoWnd[i].Disconnct();
		H264_DVR_Logout(m_videoWnd[i].m_devInfo.lLoginID);
		m_videoWnd[i].m_devInfo.lLoginID = -1;
	}

}


void CVideoManager::VideoStop(int index)
{
	int res = 0;
	if (index >= m_camera_number) return ;

	m_videoWnd[index].StopRealPlay();

	return ;


}

int CVideoManager::IsRunning(int index)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	if (m_videoWnd[index].m_devInfo.lLoginID > 0 && m_videoWnd[index].GetHandle() > 0)
	{
		res = TRUE;
	}
	return res;
}

int CVideoManager::CaptureToFile(int index, CString strFileName, int iQuality)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].CaptureToFile(strFileName, iQuality);

	return res;
}


int CVideoManager::RecordToFile(int index, CString strFileName)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].RecordToFile(strFileName);

	return res;
}

BOOL CVideoManager::RecordStop(int index)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].RecordStop();

	return res;
}

BOOL CVideoManager::IsRecording(int index)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].IsRecording();

	return res;
}

int CVideoManager::CaptureToBuffer(int index)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].CaptureToBuffer();
	return res;
}

void CVideoManager::ReConnect(LONG lLoginID, char *pchDVRIP, LONG nDVRPort)
{
	//clear all play window of this device

	//do reconnect
	//m_devDLg.ReConnect(lLoginID, pchDVRIP, nDVRPort);

	//开始重连线程
	ReConnectThread(0);


}

int CVideoManager::VideoConnect(int index)
{
	int res = 0;	
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].ConnectRealPlay(index);

	return res;

}

int CVideoManager::SetOsdText(int index, DWORD colorText, DWORD colorBack, int pos_x, int pox_y, int transparent, char *pText)
{
	int res = 0;
	if (index >= m_camera_number) return 0;

	res = m_videoWnd[index].SetOsdText(colorText, colorBack, pos_x, pox_y, transparent, pText);

	return res;
}


DWORD WINAPI CVideoManager::ReConnectFunction(LPVOID lpParam)
{

	CVideoManager *pThis = 0;
	int result = 0;
	pThis = (CVideoManager *)lpParam;
	int i = 0;

	COleDateTime timeNow;
	pThis->m_exit_rtsp = 0;

	while (1)
	{
		
		for (i = 0; i < pThis->m_camera_number; i++)
		{
			if (strlen(pThis->m_videoWnd[i].m_userInfo.szIp) > 0 )
			{
				if (pThis->m_videoWnd[i].m_devInfo.lLoginID == -1 || pThis->m_videoWnd[i].GetHandle() <= 0 || pThis->m_videoWnd[i].m_nPlaydecHandle == -1)
				{
					if (pThis->m_videoWnd[i].m_userInfo.iAutoReconnect)//自动重试
					{
						result = pThis->m_videoWnd[i].ConnectRealPlay(i, TRUE);
					}

				}
				else//运行正常
				{



					if (pThis->m_videoWnd[i].m_time_sync)
					{

						timeNow = COleDateTime::GetCurrentTime();
						//设置设备的系统时间
						SDK_SYSTEM_TIME pSysTime;
						pSysTime.year = timeNow.GetYear();
						pSysTime.month = timeNow.GetMonth();
						pSysTime.day = timeNow.GetDay();
						pSysTime.wday = timeNow.GetDayOfWeek();
						pSysTime.hour = timeNow.GetHour();
						pSysTime.minute = timeNow.GetMinute();
						pSysTime.second = timeNow.GetSecond();

						int nret = H264_DVR_SetSystemDateTime(pThis->m_videoWnd[i].m_devInfo.lLoginID, &pSysTime);
						if (nret < 0)
						{
							//MessageBox("设置失败！");
						}
						else
						{
							//MessageBox("设置成功！");
						}
						pThis->m_videoWnd[i].m_time_sync = 0;
					}

					
					COleDateTimeSpan spanParkTime = 0;
					spanParkTime = COleDateTime::GetCurrentTime() - pThis->m_videoWnd[i].m_timeReceiveData;
					if (spanParkTime.GetTotalSeconds() > 10)//如果数据中断10秒,但是没有收到断线回调, 主动退出后重新连接
					{
						pThis->m_videoWnd[i].Disconnct();
						H264_DVR_Logout(pThis->m_videoWnd[i].m_devInfo.lLoginID);
						pThis->m_videoWnd[i].m_devInfo.lLoginID = -1;
					}


				}
			}
		
			Sleep(1000);
		}

		if (pThis->m_exit_rtsp)
		{
			break;
		}

		Sleep(1000);
	}

	CloseHandle(pThis->m_hReConnectThread);
	pThis->m_hReConnectThread = NULL;
	pThis->m_exit_rtsp = 0;

	return 1;
}



int CVideoManager::ReConnectThread(int index)
{
	// 创建工作者线程  
	if (m_hReConnectThread == 0)
	{
		DWORD dwThreadID;
		m_hReConnectThread = CreateThread(NULL, 0, ReConnectFunction, this, 0, &dwThreadID);
	}

	return 1;

}

void CVideoManager::TimeSync(int index)
{
	int res = 0;
	if (index >= m_camera_number) return;

	m_videoWnd[index].TimeSync();

	return;

}




