#ifndef _CVideoManager_H_
#define _CVideoManager_H_

#include"afxmt.h"//同步操作时需要包含的头文件

#include "video_sdk/include/NetSdk.h"


#define WM_CAMERA_THREAD_SEARCH_MESSAGE WM_USER+101		//自定义消息

#define		MAXWNDNUM		16		//the maximum number of play window

#include "VideoWnd.h"



class CVideoManager
{
public:

	CVideoManager();
	~CVideoManager();
	
	CWnd *m_pMainWnd;//窗口

	SDK_CONFIG_NET_COMMON_V2 m_Device[100];//搜索到的局域网内的设备
	int m_nDevNum;//搜索到的设备个数
	int m_camera_number;

	int SearchDeviceThread();
	static DWORD WINAPI SearchDeviceFunction(LPVOID pParam);
	HANDLE m_hSearchDeviceThread;
	

	////long m_iPlayhandle;	//play handle	
//	long m_nPlaydecHandle;	//decode handle
	static void __stdcall videoInfoFramCallback(LONG nPort, LONG nType, LPCSTR pBuf, LONG nSize, LONG nUser);
	int m_nIndex;//设置通道号, 每个实例一个唯一通道号

	int VideoInit(fMessCallBack cbAlarmcallback, void *pWnd, CAMERA_USER_INFO *pUser, int num, int bAutoStart);
	void VideoRelease();
	void ReConnect(LONG lLoginID, char *pchDVRIP, LONG nDVRPort);

	void VideoStop(int index);
	int IsRunning(int index);//判断线程句柄来判断是否已完成结束

	void StopAllVideos();

	CVideoWnd m_videoWnd[MAXWNDNUM];

	int VideoConnect(int index);

	int ReConnectThread(int index);
	static DWORD WINAPI ReConnectFunction(LPVOID lpParam);
	HANDLE m_hReConnectThread;

	int CaptureToFile(int index, CString strFileName, int iQuality = 80);
	int RecordToFile(int index, CString strFileName);
	BOOL RecordStop(int index);
	BOOL IsRecording(int index);

	int CaptureToBuffer(int index);
	int SetOsdText(int index, DWORD colorText, DWORD colorBack, int pos_x, int pox_y, int transparent, char *pText);

	int PicturePause(int index, int pause);
	int IsPicturePause(int index);
	void TimeSync(int index);

	BOOL m_exit_rtsp;

	SYSTEMTIME m_timeSet; 






};



#endif





