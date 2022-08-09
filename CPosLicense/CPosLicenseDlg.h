
// CPosLicenseDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "darknet/src/image.h"
#include "VideoManager.h"

#define WM_LICENSE_THREAD_MESSAGE WM_USER+100		//自定义消息
//#define WM_VIDEO_PLAY_MESSAGE WM_USER+120	//在视频中已定义


#define NET_VIDEO_NUMBER 2	//视频个数

#define INDEX_VIDEO_LEFT 0
#define INDEX_VIDEO_RIGHT 1
#define INDEX_VIDEO_LOCAL 2 //通道2用于本地视频播放


//车型编号
#define CAR_TYPE_INDEX_PRIVATE 0
#define CAR_TYPE_INDEX_TRUCK 1
#define CAR_TYPE_INDEX_MOTORCYCLE 2
#define CAR_TYPE_INDEX_HEAVY_TRUCK 3
#define CAR_TYPE_INDEX_TAXI 4

#define LICENSE_MSG_PORT 0x100	//0x100以下用于八达通

typedef enum enumLicense_Fucntion
{
	LICENSE_FUN_SETUP,
	LICENSE_FUN_RECOGNIZE,
	LICENSE_FUN_EXIT,
	LICENSE_FUN_RUNNING,
	LICENSE_FUN_READY,
	LICENSE_FUN_COMPLETE,
	LICENSE_FUN_RESET,
}LICENSE_FUNCTION;

typedef struct tagLicense_RESULT
{
	DWORD function;//函数功能, 设置/识别
	char url[128];//路径, 可以是文件路径名, 拍照网址, 视频网址
	unsigned char *file_buf;//如果已经读出文件, 直接给出图片文件地址
	int file_len;//图片文件长度
	char car_type;//识别出来型
	char license[16];//识别完成后的车牌
	char confidence[16];//对应识别车牌上的每个字符的可靠度(0-100表示百分比)
}LICENSE_RESULT;


// CCPosLicenseDlg dialog
class CCPosLicenseDlg : public CDialogEx
{
// Construction
public:
	CCPosLicenseDlg(CWnd* pParent = NULL);	// standard constructor

	int m_msgPort;//端口号, 用于主程序识别是哪个程序通讯
	CString	m_strParentTitle;
	CString	m_strDlgMyName;
	CString	m_strShowImageName;
	CString	m_strShowImageNameLast;//上次播放的文件, 当本文件还未播完下一个又来时, 立即停止本次识别
	CString	m_strRecognizeImageName;//返回本次识别的文件名
	
	CRITICAL_SECTION m_CriticalSectionImage; // 定义临界区, 互斥


	int RecognizeRun(COPYDATASTRUCT* pCopyDataStruct);
	CString LicenseRecognize(CString strFilePath, char *pFile=0, int file_size=0);

	DWORD m_funRunning;

	int m_list_total;
	int m_list_counter;
	int m_iHideWindow;

	CString m_strLeftVideoUrl;	//ip，不是ip表示云id
	CString m_strRightVideoUrl;	//视频链接
	int m_iLeftVideoPort;	//视频链接端口
	int m_iRightVideoPort;	//视频链接端口
	CString m_strLeftVideoPassword;	//视频链接密码
	CString m_strRightVideoPassword;	//视频链接密码
	int m_iMakeSureNum;	//相同车牌确认
	int m_iMakeSureNumValue;//本次识别的确认值,图片只需要一次确认

	int m_iSaveVideoResult;
	int m_iCapture;
	int m_iSaveTrain;
	int m_iSaveImage;

	int m_iRecognizeStep;

	int m_iHideRun;	//

	float m_fLeftVideoAreaLeft;
	float m_fLeftVideoAreaTop;
	float m_fLeftVideoAreaRight;
	float m_fLeftVideoAreaBottom;
	float m_fRightVideoAreaLeft;
	float m_fRightVideoAreaTop;
	float m_fRightVideoAreaRight;
	float m_fRightVideoAreaBottom;

	int m_iAreaSetting;

	CVideoManager m_videoPlayer;
	CAMERA_USER_INFO m_videoUser[NET_VIDEO_NUMBER];
	BOOL DealwithAlarm(long lDevcID, char* pBuf, DWORD dwLen);

	CVideoWnd m_videoLocal;//用于播放本地视频

	CString m_strLicenseResult[10];//10个结果,前5个为左摄像头,后5个为后摄像头
	char m_iCarType[10];

	void VideoWndinit();
	void VideoOsdRefresh(CString strTitle);
	void LicenseInsertLeft(CString strLicense, char car_type);
	void LicenseInsertRight(CString strLicense, char car_type);
	CString LicenseInsertEnsure(int number);


	int m_iLicenseRunType;
	int LicenseThreadStart();
	static DWORD WINAPI LicenseThreadFunction(LPVOID pParam);
	HANDLE m_hLicenseThread;
	HANDLE m_hLicenseEvent;
	int LicenseRun(int iType);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CPOSLICENSE_DIALOG };
#endif

	protected:


	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	int Stretch(HDC hDC,
		int XDest, int YDest, int nDestWidth, int nDestHeight,
		int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, BYTE *lpDib,
		UINT iUsage, DWORD dwRop);
	BOOL ShowPicture(HWND hwnd, image &im);

	image m_image_plate;
	image m_image_full;
	int m_image_plate_last;//记录上次状态,用于刷新
	int m_image_full_last;



// Implementation
protected:
	HICON m_hIcon;
		
	LRESULT OnLicenseThread(WPARAM wParam, LPARAM lParam);
	LRESULT OnVideoPlayMessage(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBDetectPlate();
	afx_msg void OnBnClickedBLoadFile();
	CEdit m_edit_filename;
	CEdit m_edit_license;
	CStatic m_pic_plate;
	CStatic m_pic_source;
	CEdit m_edit_confidence;
	CEdit m_edit_message;
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNcPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	CButton m_check_output_plate;
	CButton m_check_output_train;
	CButton m_check_output_image;
	CListBox m_list_folder;
	afx_msg void OnBnClickedBLoadFolder();
	afx_msg void OnBnClickedBDetectStop();
	CButton m_b_detect_stop;
	CButton m_b_detect_plate;
	CEdit m_edit_car_type;
	afx_msg void OnBnClickedBSave();
	CEdit m_edit_video_left_ip;
	CEdit m_edit_video_left_password;
	CEdit m_edit_video_left_port;
	CEdit m_edit_video_right_ip;
	CEdit m_edit_video_right_password;
	CEdit m_edit_video_right_port;
	CStatic m_pic_video_left;
	CStatic m_pic_video_right;
	afx_msg void OnBnClickedBVideoStart();
	CButton m_b_video_start;
	CEdit m_edit_makesure_num;
	afx_msg void OnStnClickedPicVideoLeft();
	afx_msg void OnStnClickedPicVideoRight();
	CButton m_check_hide_run;
	CStatic m_static_version;
	CStatic m_static_main_title;
	CButton m_check_video_left_rect;
	afx_msg void OnBnClickedCheckVideoLeftRect();
//	afx_msg void OnBnClickedCheckVideoRightRect2();
	afx_msg void OnBnClickedCheckVideoRightRect();
	CButton m_check_video_right_rect;
	CButton m_check_output_video_result;
	CButton m_check_capture;
	CEdit m_edit_main_title;
	afx_msg void OnStnClickedStaticMainTitle();
};
