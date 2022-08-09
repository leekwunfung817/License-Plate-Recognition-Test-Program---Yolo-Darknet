#if !defined(AFX_VIDEOWND_H__98BE2781_143A_4D3D_8443_ECDB0E1F2968__INCLUDED_)
#define AFX_VIDEOWND_H__98BE2781_143A_4D3D_8443_ECDB0E1F2968__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoWnd.h : header file
//
#include "video_sdk/include/NetSdk.h"

#define WM_VIDEO_PLAY_MESSAGE WM_USER+120	

#define DIB_BUFFER_SIZE 1920*1080*3+54

//device information
typedef struct _DEV_INFO
{
	int	nListNum;		   //position in the list

	long lLoginID;			//login handle
	long lID;				//device ID
	char szDevName[60];		//device name
	char szIpaddress[15];	//device IP
	char szUserName[8];		//user name
	char szPsw[8];			//pass word
	int nPort;				//port number
	int nTotalChannel;		//total channel

	SDK_CONFIG_NET_COMMON_V2 NetComm;                  // net config
	H264_DVR_DEVICEINFO  NetDeviceInfo;

	BOOL bSerialID;//be SerialNumber login
	char szSerIP[DDNS_MAX_DDNS_IPSIZE];//server ip
	int nSerPort;           //server port
	char szSerialInfo[DDNS_MAX_SERIALINFO_SIZE];  //SerialNumber
	int devctype;
	BOOL bCloud;
	char szCloudID[64];	//云序列号
}DEV_INFO; 


//本地保存的摄像头资料, 不用每次搜索IP
typedef struct _CAMERA_USER_INFO
{
	char szIp[33];	//device IP
	char szUserName[33];		//user name
	char szPsw[33];			//pass word
	int nPort;				//port number
	int nChannel;		//连接的通道 channel
	int bCloud;//1表示ip地址为云id
	HWND hWnd;//窗口句柄
	int isConnect;//是否在线
	int iAutoReconnect;//是否自动重连
}CAMERA_USER_INFO;

//比例矩形,保存四个点与原矩形的长宽比例坐标
typedef struct _RECT_SCALE
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
}RECT_SCALE;



/////////////////////////////////////////////////////////////////////////////
// CVideoWnd dialog
#define  DISPLAYREGION

class CVideoWnd
{
// Construction
public:
	CVideoWnd(CWnd* pParent = NULL);   // standard constructor
	~CVideoWnd();

	DEV_INFO m_devInfo;
	CAMERA_USER_INFO m_userInfo;
	int CameraLogin(CString strIP, int strPort, CString strUserName, CString strPassword, int bCloud);
	int VideoInit(CAMERA_USER_INFO *pUserd);
	long DevLogin();
	int CaptureToFile(CString strFileName, int iQuality = 80);
	int CaptureToBuffer();

	int RecordToFile(CString strFileName);
	BOOL RecordStop();
	BOOL IsRecording();

	int SetOsdText(DWORD colorText, DWORD colorBack, int pos_x, int pox_y, int transparent, char *pText);

	BOOL m_iPicturePause;//暂停显示, 但视频流继续
	
	CRITICAL_SECTION m_CriticalSection; // 定义临界区, 互斥
	char *m_capture_dib_buffer;//抓图缓冲
	int m_capture_dib_buffer_len;//缓冲内存大小
	int m_capture_dib_len;//抓到的图像实际大小

	int IsPicturePause();

	int PicturePause(int pause);

	void StopRealPlay();
	void Disconnct();

	COleDateTime m_timeReceiveData;//上次收到解码数据的时间

	char m_time_sync;
	void TimeSync();
	
	static void __stdcall SDKPlayFileEndCallback(LONG nPort, void *pUser);
	int PlayLocalFile(int nIndex, CString strFile, HWND hwndVideo, HWND hwndMsg);
	int StopLocalFile();

	protected:

protected:


	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnDestroy();
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);


private:
	int m_nIndex;	//index
	long m_iPlayhandle;	//play handle
	BOOL m_bRecord;	//is recording or not
public:
	long m_nPlaydecHandle;	//decode handle
	long m_lLogin; //login handle
	int m_iChannel; //play channel
	void SetWndIndex(int nIndex)
	{
		m_nIndex = nIndex;
	}
	int ConnectRealPlay(int nIndex, bool bOsd = false);

	void GetColor(long *nBright, long *nContrast, long *nSaturation, long *nHue);
	void SetColor(long nBright, long nContrast, long nSaturation, long nHue);

	long GetHandle()
	{
		return m_iPlayhandle;
	}
	BOOL SaveRecord();

	long GetLoginHandle();

     //afx_msg void OnTalkIPC();
	void drawOSD(LONG nPort,HDC hDc);

	long SetDevChnColor(SDK_CONFIG_VIDEOCOLOR* pVideoColor);
	long GetDevChnColor(SDK_CONFIG_VIDEOCOLOR* pVideoColor);

	static void __stdcall videoInfoFramCallback(LONG nPort, LONG nType, LPCSTR pBuf,LONG nSize, void *pUser);
	CString m_strInfoFrame[100];

	CString m_strOsd[10];
	CString m_strOsdTitle;
	CString m_strAlarmMessage;

	float m_fRectLeft;//识别框与整体框架的比例
	float m_fRectTop;
	float m_fRectRight;
	float m_fRectBottom;

	RECT_SCALE m_scale_plate;

	//CRect m_rectDetect;

public:
	RECT m_oldMouseRect;//局部放大，记入区域
	CPoint m_downPoint;
	//BOOL m_bTalkIPC;
};

#endif // !defined(AFX_VIDEOWND_H__98BE2781_143A_4D3D_8443_ECDB0E1F2968__INCLUDED_)
