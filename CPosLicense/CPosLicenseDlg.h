
// CPosLicenseDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "darknet/src/image.h"
#include "VideoManager.h"

#define WM_LICENSE_THREAD_MESSAGE WM_USER+100		//�Զ�����Ϣ
//#define WM_VIDEO_PLAY_MESSAGE WM_USER+120	//����Ƶ���Ѷ���


#define NET_VIDEO_NUMBER 2	//��Ƶ����

#define INDEX_VIDEO_LEFT 0
#define INDEX_VIDEO_RIGHT 1
#define INDEX_VIDEO_LOCAL 2 //ͨ��2���ڱ�����Ƶ����


//���ͱ��
#define CAR_TYPE_INDEX_PRIVATE 0
#define CAR_TYPE_INDEX_TRUCK 1
#define CAR_TYPE_INDEX_MOTORCYCLE 2
#define CAR_TYPE_INDEX_HEAVY_TRUCK 3
#define CAR_TYPE_INDEX_TAXI 4

#define LICENSE_MSG_PORT 0x100	//0x100�������ڰ˴�ͨ

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
	DWORD function;//��������, ����/ʶ��
	char url[128];//·��, �������ļ�·����, ������ַ, ��Ƶ��ַ
	unsigned char *file_buf;//����Ѿ������ļ�, ֱ�Ӹ���ͼƬ�ļ���ַ
	int file_len;//ͼƬ�ļ�����
	char car_type;//ʶ�������
	char license[16];//ʶ����ɺ�ĳ���
	char confidence[16];//��Ӧʶ�����ϵ�ÿ���ַ��Ŀɿ���(0-100��ʾ�ٷֱ�)
}LICENSE_RESULT;


// CCPosLicenseDlg dialog
class CCPosLicenseDlg : public CDialogEx
{
// Construction
public:
	CCPosLicenseDlg(CWnd* pParent = NULL);	// standard constructor

	int m_msgPort;//�˿ں�, ����������ʶ�����ĸ�����ͨѶ
	CString	m_strParentTitle;
	CString	m_strDlgMyName;
	CString	m_strShowImageName;
	CString	m_strShowImageNameLast;//�ϴβ��ŵ��ļ�, �����ļ���δ������һ������ʱ, ����ֹͣ����ʶ��
	CString	m_strRecognizeImageName;//���ر���ʶ����ļ���
	
	CRITICAL_SECTION m_CriticalSectionImage; // �����ٽ���, ����


	int RecognizeRun(COPYDATASTRUCT* pCopyDataStruct);
	CString LicenseRecognize(CString strFilePath, char *pFile=0, int file_size=0);

	DWORD m_funRunning;

	int m_list_total;
	int m_list_counter;
	int m_iHideWindow;

	CString m_strLeftVideoUrl;	//ip������ip��ʾ��id
	CString m_strRightVideoUrl;	//��Ƶ����
	int m_iLeftVideoPort;	//��Ƶ���Ӷ˿�
	int m_iRightVideoPort;	//��Ƶ���Ӷ˿�
	CString m_strLeftVideoPassword;	//��Ƶ��������
	CString m_strRightVideoPassword;	//��Ƶ��������
	int m_iMakeSureNum;	//��ͬ����ȷ��
	int m_iMakeSureNumValue;//����ʶ���ȷ��ֵ,ͼƬֻ��Ҫһ��ȷ��

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

	CVideoWnd m_videoLocal;//���ڲ��ű�����Ƶ

	CString m_strLicenseResult[10];//10�����,ǰ5��Ϊ������ͷ,��5��Ϊ������ͷ
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
	int m_image_plate_last;//��¼�ϴ�״̬,����ˢ��
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
