
// CPosLicenseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CPosLicense.h"
#include "CPosLicenseDlg.h"
#include "afxdialogex.h"


#include "darknet/src/license_detect.h"
#include "base64.h"
#include "afxwin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	static CString GetFileVersion();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_static_version;
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VERSION, m_static_version);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCPosLicenseDlg dialog



CCPosLicenseDlg::CCPosLicenseDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CPOSLICENSE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	m_msgPort = LICENSE_MSG_PORT;
	memset(&m_image_plate, 0, sizeof(m_image_plate));
	memset(&m_image_full, 0, sizeof(m_image_full));
	m_list_total = 0;
	m_list_counter = 0;	
	
	m_iHideWindow = FALSE;
	
	m_hLicenseThread = 0; 
	m_hLicenseEvent = 0;
	m_iLicenseRunType = 0;
	m_funRunning = 0;
	InitializeCriticalSection(&m_CriticalSectionImage);      // ��ʼ���ٽ��� 
	m_image_plate_last = 0;//��¼�ϴ�״̬,����ˢ��
	m_image_full_last = 0;

	m_iAreaSetting = 0;
	m_fLeftVideoAreaLeft = 0;
	m_fLeftVideoAreaTop = 0;
	m_fLeftVideoAreaRight = 0;
	m_fLeftVideoAreaBottom = 0;
	m_fRightVideoAreaLeft = 0;
	m_fRightVideoAreaTop = 0;
	m_fRightVideoAreaRight = 0;
	m_fRightVideoAreaBottom = 0;

	m_iMakeSureNumValue = 1;
	m_iSaveVideoResult = 0;
	m_iSaveTrain = 0;
	m_iSaveImage = 0;

	m_iRecognizeStep = 0;
}

void CCPosLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_edit_filename);
	DDX_Control(pDX, IDC_EDIT_LICENSE, m_edit_license);
	DDX_Control(pDX, IDC_PIC_PLATE, m_pic_plate);
	DDX_Control(pDX, IDC_PIC_SOURCE, m_pic_source);
	DDX_Control(pDX, IDC_EDIT_CONFIDENCE, m_edit_confidence);
	DDX_Control(pDX, IDC_EDIT_MESSAGE, m_edit_message);
	//  DDX_Control(pDX, IDC_CHECK_OUTPUT_PLATE, m_check_output_plate);
	DDX_Control(pDX, IDC_CHECK_OUTPUT_TRAIN, m_check_output_train);
	DDX_Control(pDX, IDC_CHECK_OUTPUT_IMAGE, m_check_output_image);
	DDX_Control(pDX, IDC_LIST_FOLDER, m_list_folder);
	DDX_Control(pDX, IDC_B_DETECT_STOP, m_b_detect_stop);
	DDX_Control(pDX, IDC_B_DETECT_PLATE, m_b_detect_plate);
	DDX_Control(pDX, IDC_EDIT_CAR_TYPE, m_edit_car_type);
	DDX_Control(pDX, IDC_EDIT_VIDEO_LEFT_IP, m_edit_video_left_ip);
	DDX_Control(pDX, IDC_EDIT_VIDEO_LEFT_PASSWORD, m_edit_video_left_password);
	DDX_Control(pDX, IDC_EDIT_VIDEO_LEFT_PORT, m_edit_video_left_port);
	DDX_Control(pDX, IDC_EDIT_VIDEO_RIGHT_IP, m_edit_video_right_ip);
	DDX_Control(pDX, IDC_EDIT_VIDEO_RIGHT_PASSWORD, m_edit_video_right_password);
	DDX_Control(pDX, IDC_EDIT_VIDEO_RIGHT_PORT, m_edit_video_right_port);
	DDX_Control(pDX, IDC_PIC_VIDEO_LEFT, m_pic_video_left);
	DDX_Control(pDX, IDC_PIC_VIDEO_RIGHT, m_pic_video_right);
	DDX_Control(pDX, IDC_B_VIDEO_START, m_b_video_start);
	DDX_Control(pDX, IDC_EDIT_MAKESURE_NUM, m_edit_makesure_num);
	DDX_Control(pDX, IDC_CHECK_HIDE_RUN, m_check_hide_run);
	DDX_Control(pDX, IDC_STATIC_VERSION, m_static_version);
	DDX_Control(pDX, IDC_STATIC_MAIN_TITLE, m_static_main_title);
	DDX_Control(pDX, IDC_CHECK_VIDEO_LEFT_RECT, m_check_video_left_rect);
	DDX_Control(pDX, IDC_CHECK_VIDEO_RIGHT_RECT, m_check_video_right_rect);
	DDX_Control(pDX, IDC_CHECK_OUTPUT_VIDEO_RESULT, m_check_output_video_result);
	DDX_Control(pDX, IDC_CHECK_CAPTURE, m_check_capture);
	DDX_Control(pDX, IDC_EDIT_MAIN_TITLE, m_edit_main_title);
}

BEGIN_MESSAGE_MAP(CCPosLicenseDlg, CDialogEx)
	ON_MESSAGE(WM_LICENSE_THREAD_MESSAGE, &CCPosLicenseDlg::OnLicenseThread)
	ON_MESSAGE(WM_VIDEO_PLAY_MESSAGE, &CCPosLicenseDlg::OnVideoPlayMessage)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_B_DETECT_PLATE, &CCPosLicenseDlg::OnBnClickedBDetectPlate)
	ON_BN_CLICKED(IDC_B_LOAD_FILE, &CCPosLicenseDlg::OnBnClickedBLoadFile)
	ON_WM_COPYDATA()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CCPosLicenseDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCPosLicenseDlg::OnBnClickedCancel)
	ON_WM_NCPAINT()
	ON_BN_CLICKED(IDC_B_LOAD_FOLDER, &CCPosLicenseDlg::OnBnClickedBLoadFolder)
	ON_BN_CLICKED(IDC_B_DETECT_STOP, &CCPosLicenseDlg::OnBnClickedBDetectStop)
	ON_BN_CLICKED(IDC_B_SAVE, &CCPosLicenseDlg::OnBnClickedBSave)
	ON_BN_CLICKED(IDC_B_VIDEO_START, &CCPosLicenseDlg::OnBnClickedBVideoStart)
	ON_STN_CLICKED(IDC_PIC_VIDEO_LEFT, &CCPosLicenseDlg::OnStnClickedPicVideoLeft)
	ON_STN_CLICKED(IDC_PIC_VIDEO_RIGHT, &CCPosLicenseDlg::OnStnClickedPicVideoRight)
	ON_BN_CLICKED(IDC_CHECK_VIDEO_LEFT_RECT, &CCPosLicenseDlg::OnBnClickedCheckVideoLeftRect)
//	ON_BN_CLICKED(IDC_CHECK_VIDEO_RIGHT_RECT2, &CCPosLicenseDlg::OnBnClickedCheckVideoRightRect2)
ON_BN_CLICKED(IDC_CHECK_VIDEO_RIGHT_RECT, &CCPosLicenseDlg::OnBnClickedCheckVideoRightRect)
ON_STN_CLICKED(IDC_STATIC_MAIN_TITLE, &CCPosLicenseDlg::OnStnClickedStaticMainTitle)
END_MESSAGE_MAP()


CString ReadSetup(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault)
{
	TCHAR strConfig[1024];
	CString strPathName;
	memset(strConfig, 0, sizeof(strConfig));
	strPathName = GetAppPath() + _T("setup_license.ini");

	GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, strConfig, sizeof(strConfig) / 2, strPathName);

	return CString(strConfig);

}

BOOL WriteSetup(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpValue)
{

	CString strPathName;

	strPathName = GetAppPath() + _T("setup_license.ini");

	return WritePrivateProfileStringW(lpAppName, lpKeyName, lpValue, strPathName);

}



//urlת�ɴ�ip
//���ת���ɹ���ʾurl��ip��ַ�� ���ʧ�����ж�Ϊ��id������0
int NetVideoUrlTrans(CString strUrl, char *pSave, int len)
{
	int iCloud = 0;

	unsigned char tmp;
	char c;
	unsigned char i, j;

	char strsrc[128] = { 0 };
	unsigned char ip_int[4] = { 0 };

	unsigned char *ipaddr = ip_int;// (unsigned char *)pSave;

	CString strHead;
	CString strRes;

	strHead = strUrl.Left(7);
	if (strHead.CompareNoCase(_T("http://")) == 0 || strHead.CompareNoCase(_T("rtsp://")) == 0)
	{
		strUrl.Delete(0, 7);
	}


	WideCharToMultiByte(CP_ACP, 0, strUrl, -1, strsrc, sizeof(strsrc) - 1, NULL, NULL);

	char *addrstr = strsrc;


	tmp = 0;

	for (i = 0; i < 4; ++i)
	{
		j = 0;
		do
		{
			c = *addrstr;
			++j;
			if (j > 4)//����4�����֣�ip����
			{
				iCloud = 1;
				i = 4; break;//��ֹ
			}
			if (c == '.' || c == 0)
			{
				*ipaddr = tmp;
				++ipaddr;
				tmp = 0;
			}
			else if (c >= '0' && c <= '9')
			{
				tmp = (tmp * 10) + (c - '0');
			}
			else if (i < 3)//�����֣�ip����
			{
				iCloud = 1;
				i = 4; break;//��ֹ
			}
			else
			{
				*ipaddr = tmp;	//���һ��
				break;
			}

			++addrstr;
		} while (c != '.' && c != 0);
	}

	if (iCloud)
	{
		WideCharToMultiByte(CP_ACP, 0, strUrl, -1, pSave, len - 1, NULL, NULL);
	}
	else
	{
		_snprintf(pSave, len - 1, "%d.%d.%d.%d", ip_int[0], ip_int[1], ip_int[2], ip_int[3]);

	}

	return iCloud;

}


int iMontionCnt = 0;
BOOL CCPosLicenseDlg::DealwithAlarm(long lDevcID, char* pBuf, DWORD dwLen)
{
	SDK_AlarmInfo alarmInfo;
	memcpy(&alarmInfo, pBuf, dwLen);
	
	CString str;

	char cIpaddress[64] = { 0 };
	//if (m_devInfo.bCloud == 1)
	//{
	//	strcpy(cIpaddress, m_devInfo.szCloudID);
	//}
	//else
	//{
	//	strcpy(cIpaddress, m_devInfo.szIpaddress);
	//}

	//SDK_EventCodeTypes
	if ((SDK_EVENT_CODE_NET_ALARM == alarmInfo.iEvent
		|| SDK_EVENT_CODE_MANUAL_ALARM == alarmInfo.iEvent
		|| SDK_EVENT_CODE_LOCAL_ALARM == alarmInfo.iEvent) && !alarmInfo.iStatus)
	{

		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.Externalalarm"));
		//AfxMessageBox(str);
	}
	else if (SDK_EVENT_CODE_VIDEO_MOTION == alarmInfo.iEvent) // iStatus 0:������ʼ��1:��������
	{

		if (!alarmInfo.iStatus)
		{
			//str.Format(_T("IP:%s,%s:%d,%s"),
			//CString(m_videoPlayer.m_videoWnd[i].m_userInfo.szIp), _T("Log.Channel"),
			//	alarmInfo.nChannel + 1, _T("AccountMSG.MotionAlarm"));
			//iMontionCnt = 0;
			iMontionCnt++;
			str.Format(_T("Motion Alarm %d"), iMontionCnt);
		}
		else
		{
			//iMontionCnt++;
			//str.Format(_T("Motion Alarm %d"), iMontionCnt);
		}

		for (int i = 0; i < MAXWNDNUM; i++)
		{
			if (lDevcID == m_videoPlayer.m_videoWnd[i].m_lLogin)
			{
				m_videoPlayer.m_videoWnd[i].m_strAlarmMessage = str;
				
			//	if (!alarmInfo.iStatus)
				{
					if (m_iCapture && i == INDEX_VIDEO_LEFT)
					{
						SetTimer(4, 1, 0);
					}
					else if (m_iCapture && i == INDEX_VIDEO_RIGHT)
					{
						SetTimer(5, 1, 0);
					}
				}
			}
		}

	}
	else if (SDK_EVENT_CODE_VIDEO_LOSS == alarmInfo.iEvent && !alarmInfo.iStatus)
	{

		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.LossAlarm"));

	}
	else if (SDK_EVENT_CODE_VIDEO_BLIND == alarmInfo.iEvent && !alarmInfo.iStatus)
	{

		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.BlindAlarm"));

	}
	else if (SDK_EVENT_CODE_STORAGE_FAILURE == alarmInfo.iEvent && !alarmInfo.iStatus)
	{
		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.StorageFailureAlarm"));
	}
	else if (SDK_EVENT_CODE_LOW_SPACE == alarmInfo.iEvent && !alarmInfo.iStatus)
	{
		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.LowSpaceAlarm"));
	}
	else if (SDK_EVENT_CODE_HumanDetectAlarm == alarmInfo.iEvent && !alarmInfo.iStatus)
	{
		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.HumanDetectAlarm"));
	}
	else if (SDK_EVENT_CODE_FaceDetectAlarm == alarmInfo.iEvent && !alarmInfo.iStatus)
	{
		str.Format(_T("IP:%s,%s:%d,%s"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, _T("AccountMSG.FaceDetectAlarm"));
	}
	else
	{
		str.Format(_T("IP:%s,%s:%d,Event:%d,Status:%d"),
			CString(cIpaddress), _T("Log.Channel"),
			alarmInfo.nChannel + 1, alarmInfo.iEvent, alarmInfo.iStatus);
	}

	return TRUE;
}

//message callback function
bool __stdcall VideoMessageCallBack(long lLoginID, unsigned char *pBuf, unsigned long dwBufLen, void *pUser, int nType, void * pDataInfo)
{
	if (nType == ALARM_TYPE)
	{
		CCPosLicenseDlg *pDlg = (CCPosLicenseDlg *)pUser;
		pDlg->DealwithAlarm(lLoginID, (char *)pBuf, dwBufLen);
	}
	else
	{

	}

	return TRUE;
}


// CCPosLicenseDlg message handlers

BOOL CCPosLicenseDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_static_version.SetWindowText(_T("Ver ") + CAboutDlg::GetFileVersion());

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


//	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	m_strDlgMyName = _T("cpos_car_plate_license");

	CString strIp;
	CString strPort;
	CString strPassword;
	CString str;

	strIp = ReadSetup(_T("VIDEO"), _T("CAMERA_LEFT_IP"), _T(""));
	strPort = ReadSetup(_T("VIDEO"), _T("CAMERA_LEFT_PORT"), _T("34567"));
	strPassword = ReadSetup(_T("VIDEO"), _T("CAMERA_LEFT_PASSWORD"), _T(""));
	strPassword = PasswordDecode(strPassword);

	m_edit_video_left_ip.SetWindowText(strIp);
	m_edit_video_left_port.SetWindowText(strPort);
	m_edit_video_left_password.SetWindowText(strPassword);

	m_strLeftVideoUrl = strIp;	//ip������ip��ʾ��id
	m_iLeftVideoPort = _wtoi(strPort);	//��Ƶ���Ӷ˿�
	m_strLeftVideoPassword = strPassword;	//��Ƶ��������


	strIp = ReadSetup(_T("VIDEO"), _T("CAMERA_RIGHT_IP"), _T(""));
	strPort = ReadSetup(_T("VIDEO"), _T("CAMERA_RIGHT_PORT"), _T("34567"));
	strPassword = ReadSetup(_T("VIDEO"), _T("CAMERA_RIGHT_PASSWORD"), _T(""));
	strPassword = PasswordDecode(strPassword);

	m_edit_video_right_ip.SetWindowText(strIp);
	m_edit_video_right_port.SetWindowText(strPort);
	m_edit_video_right_password.SetWindowText(strPassword);

	m_strRightVideoUrl = strIp;	//��Ƶ����
	m_iRightVideoPort = _wtoi(strPort);	//��Ƶ���Ӷ˿�
	m_strRightVideoPassword = strPassword;	//��Ƶ��������
	
	CString strMakeSure;
	strMakeSure = ReadSetup(_T("VIDEO"), _T("MAKESURE_NUM"), _T("3"));
	m_edit_makesure_num.SetWindowText(strMakeSure);
	m_iMakeSureNum = _wtoi(strMakeSure);

	CString strHide;
	strHide = ReadSetup(_T("VIDEO"), _T("WND_HIDE"), _T("1"));
	m_iHideRun = _wtoi(strHide);
	m_check_hide_run.SetCheck(m_iHideRun);

	str = ReadSetup(_T("VIDEO"), _T("CAPTURE"), _T("0"));
	m_iCapture = _wtoi(str);
	m_check_capture.SetCheck(m_iCapture);


	str = ReadSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_LEFT"), _T("0")); m_fLeftVideoAreaLeft = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_TOP"), _T("0")); m_fLeftVideoAreaTop = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_RIGHT"), _T("0")); m_fLeftVideoAreaRight = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_BOTTOM"), _T("0")); m_fLeftVideoAreaBottom = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_LEFT"), _T("0")); m_fRightVideoAreaLeft = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_TOP"), _T("0")); m_fRightVideoAreaTop = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_RIGHT"), _T("0")); m_fRightVideoAreaRight = _wtof(str);
	str = ReadSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_BOTTOM"), _T("0")); m_fRightVideoAreaBottom = _wtof(str);

	str = ReadSetup(_T("VIDEO"), _T("SAVE_RESULT"), _T("0")); m_iSaveVideoResult = _wtof(str);
	m_check_output_video_result.SetCheck(m_iSaveVideoResult);

	int nArgs = 0;

	CString strLid;

	CString strWndName = _T("CPosLicenseRecognize");

	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);	//��ȡ��������, nArgs��������, ��1��Ϊ����·��,
																		//szArglist[0] = ����·��, szArglist[1]Ϊ��1������,�����Կո�����
	if (nArgs >= 2)
	{
		m_msgPort = _wtoi(szArglist[1]);


		m_strParentTitle = szArglist[2];
	
		CString strConfig;
		strConfig.Format(_T("%d"), m_msgPort);
		strWndName = m_strDlgMyName + strConfig;
		m_static_main_title.SetWindowTextW(m_strParentTitle);
		m_iHideWindow = m_iHideRun;
	}

	if (FindWindowW(NULL, strWndName) != NULL)//�ҵ����򴰿�,����������
	{
		EndDialog(0);
	}
	SetWindowTextW(strWndName);
	
	//else
	//{
	//	EndDialog(0);
	//	return FALSE;
	//}

//	m_iHideWindow = TRUE;

	LocalFree(szArglist);// Free memory allocated for CommandLineToArgvW arguments.

	VideoWndinit();

	LicenseThreadStart();

	//���������Լ��˳�
	SetTimer(0, 2000, 0);
	

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CCPosLicenseDlg::VideoWndinit()
{

	if (!m_strLeftVideoUrl.IsEmpty())
	{
		m_videoUser[INDEX_VIDEO_LEFT].bCloud = NetVideoUrlTrans(m_strLeftVideoUrl, m_videoUser[INDEX_VIDEO_LEFT].szIp, sizeof(m_videoUser[0].szIp));
		_snprintf_s(m_videoUser[INDEX_VIDEO_LEFT].szUserName, sizeof(m_videoUser[0].szUserName), "%s", "admin");
		WideCharToMultiByte(CP_ACP, 0, m_strLeftVideoPassword, -1, m_videoUser[INDEX_VIDEO_LEFT].szPsw, sizeof(m_videoUser[0].szPsw) - 1, NULL, NULL);
		m_videoUser[INDEX_VIDEO_LEFT].nPort = m_iLeftVideoPort;
		m_videoUser[INDEX_VIDEO_LEFT].nChannel = 0;
		m_videoUser[INDEX_VIDEO_LEFT].hWnd = m_pic_video_left.m_hWnd;//���ھ��
		m_videoUser[INDEX_VIDEO_LEFT].iAutoReconnect = 1;

		m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;


	}

	if (!m_strRightVideoUrl.IsEmpty())
	{
		m_videoUser[INDEX_VIDEO_RIGHT].bCloud = NetVideoUrlTrans(m_strRightVideoUrl, m_videoUser[INDEX_VIDEO_RIGHT].szIp, sizeof(m_videoUser[0].szIp));
		_snprintf_s(m_videoUser[INDEX_VIDEO_RIGHT].szUserName, sizeof(m_videoUser[0].szUserName), "%s", "admin");
		WideCharToMultiByte(CP_ACP, 0, m_strRightVideoPassword, -1, m_videoUser[INDEX_VIDEO_RIGHT].szPsw, sizeof(m_videoUser[0].szPsw) - 1, NULL, NULL);
		m_videoUser[INDEX_VIDEO_RIGHT].nPort = m_iRightVideoPort;
		m_videoUser[INDEX_VIDEO_RIGHT].nChannel = 0;
		m_videoUser[INDEX_VIDEO_RIGHT].hWnd = m_pic_video_right.m_hWnd;//���ھ��
		m_videoUser[INDEX_VIDEO_RIGHT].iAutoReconnect = 1;

		m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
		m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;

	}

	if (!m_strLeftVideoUrl.IsEmpty() || !m_strRightVideoUrl.IsEmpty())
	{
		//��ʼ����Ƶ��
		m_videoPlayer.VideoInit(VideoMessageCallBack, this, &m_videoUser[0], NET_VIDEO_NUMBER, 1);//����һ��ʵ�ʳ�ʼ��, ֮������ʵ���Ļص�����һ��
	}

	if (!m_strLeftVideoUrl.IsEmpty())
	{
		m_videoPlayer.TimeSync(INDEX_VIDEO_LEFT);
	}

	if (!m_strRightVideoUrl.IsEmpty())
	{
		m_videoPlayer.TimeSync(INDEX_VIDEO_RIGHT);
	}



}

void CCPosLicenseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


int CCPosLicenseDlg::Stretch(HDC hDC,
	int XDest, int YDest, int nDestWidth, int nDestHeight,
	int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, BYTE *lpDib,
	UINT iUsage, DWORD dwRop)
{
	BITMAPINFO *pbmi;


	int palsize;
	palsize = PALETTESIZE(24);  //if m_nBitCount==24 then palsize=0
												  //if it is 8, palsize=256 if is 4,then 16
	pbmi = (BITMAPINFO *)new BYTE[sizeof(BITMAPINFO) + sizeof(RGBQUAD)*palsize];
	memset(pbmi->bmiColors, 0, 4);	//24λ����޵�ɫ������

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = nSrcWidth;
	pbmi->bmiHeader.biHeight = nSrcHeight;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 24;	//24λ
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = 0;
	pbmi->bmiHeader.biXPelsPerMeter = 0;
	pbmi->bmiHeader.biYPelsPerMeter = 0;
	pbmi->bmiHeader.biClrUsed = 0;
	pbmi->bmiHeader.biClrImportant = 0;

	SetStretchBltMode(hDC, COLORONCOLOR);
	int ret = StretchDIBits(hDC, XDest, YDest, nDestWidth, nDestHeight,
		XSrc, YSrc, nSrcWidth, nSrcHeight, lpDib,
		pbmi, iUsage, dwRop);

	delete[] pbmi;
	return ret;
}


BOOL CCPosLicenseDlg::ShowPicture(HWND hwnd, image &im)
{
	if (hwnd == NULL || im.data == NULL || im.w == 0 || im.h == 0)
	{
		return FALSE;
	}

	CRect rect;
	::GetWindowRect(hwnd, rect);

	HDC hDC = ::GetDC(hwnd);

	int dib_width = DIB_BYTE_PER_LINE(im.w , im.c * 8);
	unsigned char* data = (unsigned char*)calloc(dib_width * im.h , sizeof(unsigned char));
	image_to_dib(im, data);

	if (data)
	{
		Stretch(hDC,
			0, 0, rect.Width(), rect.Height(),
			0, 0, im.w, im.h, (BYTE *)data,
			DIB_RGB_COLORS, SRCCOPY);

		::ReleaseDC(m_hWnd, hDC);
		free(data);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCPosLicenseDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	if(!m_iHideWindow)
	{ 



	EnterCriticalSection(&m_CriticalSectionImage); //�����ٽ������������Ȩ�������߳̾͵�
	if (ShowPicture(m_pic_source.m_hWnd, m_image_full))
	{
		m_image_full_last = 1;
	}
	else
	{
		m_image_full_last = 0;

	}
	if (ShowPicture(m_pic_plate.m_hWnd, m_image_plate))
	{
		m_image_plate_last = 1;
	}
	else
	{
		m_image_plate_last = 0;
	}
	LeaveCriticalSection(&m_CriticalSectionImage);  //�˳��ٽ���
	
 }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCPosLicenseDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CCPosLicenseDlg::LicenseRecognize(CString strFilePath, char *pFile, int file_size)
{

	CString strLicense;

	m_edit_license.SetWindowTextW(_T(""));
	COleDateTime timeNow = COleDateTime::GetCurrentTime();// ��ȡ��ǰʱ��
	COleDateTimeSpan spanParkTime = 0;
	int err = 0;
	char car_type = -1;
	m_iSaveTrain = m_check_output_train.GetCheck();
	m_iSaveImage = m_check_output_image.GetCheck();
	int save_video_result = 0;
	save_video_result = m_check_output_video_result.GetCheck();
	


	if (!strFilePath.IsEmpty())
	{
		if (strFilePath.Right(4) == _T(".mp4") || strFilePath.Right(4) == _T("h264"))
		{
			if (m_videoLocal.PlayLocalFile(INDEX_VIDEO_LOCAL, strFilePath, m_pic_video_right.m_hWnd, m_hWnd) >= 0) // 20210414180010_32345678.mp4
			{
				Sleep(10);
				int videoCaptureResult = m_videoLocal.CaptureToBuffer();

				err = 0;

				if (videoCaptureResult && m_videoLocal.m_capture_dib_buffer)
				{

				//	EnterCriticalSection(&pThis->m_CriticalSectionImage); //�����ٽ������������Ȩ�������߳̾͵�
					if (m_image_full.data != 0)  release_image(&m_image_full);
					m_image_full = load_image_from_memory(m_videoLocal.m_capture_dib_buffer, m_videoLocal.m_capture_dib_len);
					//		ShowPicture(m_pic_source, im_src);
				//	LeaveCriticalSection(&pThis->m_CriticalSectionImage);  //�˳��ٽ���
				}
			
				SendMessage(WM_PAINT);
			}
		}
		err = 0;
	//	strLicense = plate_detect(pThis->m_strShowImageName, 0, 0, &pThis->m_image_plate, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fLeftVideoAreaLeft, pThis->m_fLeftVideoAreaTop, pThis->m_fLeftVideoAreaRight, pThis->m_fLeftVideoAreaBottom);
	}


//	strLicense = plate_detect(strFilePath, pFile, file_size, &m_image_plate, &err, save_train, save_image, car_type, m_fLeftVideoAreaLeft, m_fLeftVideoAreaTop, m_fLeftVideoAreaRight, m_fLeftVideoAreaBottom);


	CString strCarType;

	strCarType = get_car_type(car_type - '0');

	//switch (car_type)
	//{
	//case '0':
	//	strCarType = _T("taxi_mark");
	//	break;
	//case '1':
	//	strCarType = _T("taxi");
	//	break;
	//case '2':
	//	strCarType = _T("bus");
	//	break;
	//case '3':
	//	strCarType = _T("privatecar");
	//	break;
	//case '4':
	//	strCarType = _T("vehicle_s");
	//	break;
	//case '5':
	//	strCarType = _T("vehicle_m");
	//	break;
	//case '6':
	//	strCarType = _T("motorcycle");
	//	break;
	//default:
	//	strCarType.Empty();
	//	break;
	//}

	m_edit_car_type.SetWindowTextW(strCarType);


	PostMessage(WM_PAINT);

	spanParkTime = COleDateTime::GetCurrentTime() - timeNow;
	CString str;
	str.Format(_T("%f Seconds"), spanParkTime.GetTotalSeconds());
	m_edit_message.SetWindowTextW(str);
	m_edit_license.SetWindowTextW(strLicense);

	return strLicense;
}

void CCPosLicenseDlg::OnBnClickedBDetectPlate()
{
	CString strFilePath;

	m_edit_filename.GetWindowTextW(strFilePath);
	int err = 0;
	m_iSaveTrain = m_check_output_train.GetCheck();
	m_iSaveImage = m_check_output_image.GetCheck();


	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFilePath);

	if (bWorking )
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory())
		{
			finder.Close();

			BOOL isfind = FALSE;
			CString strFileName;
			CString strFileAll;

			BOOL isShow = FALSE;

			m_list_folder.ResetContent();
			CString strFolder = strFilePath;
			if (strFolder.Right(1) != _T("\\")) strFolder += _T("\\");

			isfind = finder.FindFile(strFolder + _T("*.*"));
			while (isfind)
			{
				isfind = finder.FindNextFile();
				strFileName = finder.GetFileName();
				if (strFileName.Right(4) == _T(".jpg") || strFileName.Right(4) == _T(".mp4"))
				{
					m_list_folder.AddString(strFileName);
				}
				else
				{
					continue;
				}
			}
			m_list_total = m_list_folder.GetCount();

			if (m_list_counter >= m_list_total)
			{
				m_list_counter = -1;
			}
			SetTimer(1, 1, 0);
			m_b_detect_stop.EnableWindow(TRUE);
			m_b_detect_plate.EnableWindow(FALSE);

		}
		else
		{
			m_b_detect_stop.EnableWindow(TRUE);
			m_b_detect_plate.EnableWindow(FALSE);

			m_strShowImageName = strFilePath;

			if (!m_strShowImageName.IsEmpty())
			{
				char source_picture_file[512] = { 0 };

				WideCharToMultiByte(CP_ACP, 0, m_strShowImageName, -1, source_picture_file, sizeof(source_picture_file) - 1, NULL, NULL);
				if (m_image_full.data != 0) release_image(&m_image_full);
					

				m_image_full = load_image(source_picture_file, 0, 0, 0);
			}

			LicenseRun(LICENSE_FUN_RECOGNIZE);
		//	LicenseRecognize(strFilePath);


			m_b_detect_stop.EnableWindow(TRUE);
			m_b_detect_plate.EnableWindow(TRUE);

		}


	}
	







}


void CCPosLicenseDlg::OnBnClickedBLoadFile()
{

	CFileDialog dlgOpen(TRUE, _T("*.*"), _T(""), NULL, _T(""));//"*.dat|*.dat|All Files|*.*|"

	if (dlgOpen.DoModal() == IDOK)
	{
		CString strFilePath = dlgOpen.GetPathName();


		m_edit_filename.SetWindowTextW(strFilePath);
		m_strShowImageName = strFilePath;

		PostMessage(WM_PAINT);
	}


}






void CCPosLicenseDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	LicenseRun(LICENSE_FUN_EXIT);


	plate_mem_free();

	DeleteCriticalSection(&m_CriticalSectionImage);          // ɾ��  


	CDialogEx::OnClose();
	CDialogEx::OnOK();
}


void CCPosLicenseDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK();
	//OnClose();
}


void CCPosLicenseDlg::OnBnClickedCancel()
{
	OnClose();
	// TODO: Add your control notification handler code here
	//CDialogEx::OnCancel();
	//OnClose();
}


void CCPosLicenseDlg::OnTimer(UINT_PTR nIDEvent)
{

	HWND hwnd = 0;
	if (nIDEvent == 0)
	{
		if(m_iHideWindow || !m_strParentTitle.IsEmpty())
		{
			if (!m_strParentTitle.IsEmpty())
			{
				hwnd = ::FindWindow(NULL, m_strParentTitle);
			}

			if (hwnd == NULL)	// �������˳�, ������Լ��˳�
			{
				KillTimer(0);

				DWORD function = LICENSE_FUN_EXIT;
				COPYDATASTRUCT data;
				data.dwData = function;
				RecognizeRun(&data);

			//	MessageBox(_T("timer exit"));
				EndDialog(0);
			}
		}

	}


	if (nIDEvent == 1)
	{
		KillTimer(1);


		if (m_list_counter < m_list_total-1)
		{

			if (m_videoLocal.m_nPlaydecHandle < 0 && m_iLicenseRunType != LICENSE_FUN_RECOGNIZE &&  m_iLicenseRunType != LICENSE_FUN_RUNNING)//δ����
			{

				m_list_counter++;

				CString str;
				m_list_folder.SetCurSel(m_list_counter);
				m_list_folder.GetText(m_list_counter, str);

				CString strFileName;
				m_edit_filename.GetWindowTextW(strFileName);
				if (strFileName.Right(1) != _T("\\")) strFileName += _T("\\");
				m_strShowImageName = strFileName + str;

				SendMessage(WM_PAINT);

				//LicenseRecognize(m_strShowImageName);

				LicenseRun(LICENSE_FUN_RECOGNIZE);

			}



			SetTimer(1, 1, NULL);
			m_b_detect_stop.EnableWindow(TRUE);
			m_b_detect_plate.EnableWindow(FALSE);
		}
		else
		{
			m_b_detect_stop.EnableWindow(FALSE);
			m_b_detect_plate.EnableWindow(TRUE);
			m_list_folder.SetCurSel(-1);
		}



	}

	//ʵʱ��Ƶʶ��
	if (nIDEvent == 2)
	{

		LicenseRun(LICENSE_FUN_RECOGNIZE);

	}

	//������Ƶ
	if (nIDEvent == 3)
	{

		LicenseRun(LICENSE_FUN_RECOGNIZE);

	}

	//ץ��
	if (nIDEvent == 4)
	{
		KillTimer(4);
		OnStnClickedPicVideoLeft();

	}
	if (nIDEvent == 5)
	{
		KillTimer(5);
		OnStnClickedPicVideoRight();

	}

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CCPosLicenseDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{

	RecognizeRun(pCopyDataStruct);

	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}

//ִ���������͵Ŀ���ָ��
//dwFunction - 4BYTE, BYTE0=���ܣ� BYTE1-2=��һ��������BYTE3=�ڶ���������InitComm BYTE1,BYTE2-3��
//dwValue - �������ݣ���Ҫ���ڴ���AddValue , Deduct���վݺ��룬���ڱ���LOG
int CCPosLicenseDlg::RecognizeRun(COPYDATASTRUCT* pCopyDataStruct)
{

	ULONG dwFunction;
	int i = 0;
	DWORD dwValue = 0;

	int iRecThreadRun = 0;//

	dwFunction = (LICENSE_FUNCTION)pCopyDataStruct->dwData;




	CString strUrl;
	char *pFile = NULL;
	LICENSE_RESULT *pResult = NULL;
	if (pCopyDataStruct->cbData != sizeof(LICENSE_RESULT))
	{
		return 0;
	}
	else
	{
		pResult = (LICENSE_RESULT *)pCopyDataStruct->lpData;
		strUrl = CString(pResult->url);
		if (pResult->file_buf && pResult->file_len)
		{
			pFile = new char[pResult->file_len];
			memcpy(pFile, pResult->file_buf, pResult->file_len);
		}
	}
	


	CString str;
	COPYDATASTRUCT copydata;
	DWORD len = 0;	//����ֵ�ĳ���
	LICENSE_RESULT result;
	int res = 0;
	memset(&result, 0, sizeof(result));
	copydata.cbData = sizeof(result);
	copydata.lpData = &result;

	m_funRunning = result.function = dwFunction;
	memcpy(result.url, pResult->url, sizeof(result.url));

	switch (dwFunction)
	{
	case LICENSE_FUN_SETUP:
	{



		break;
	}
	case LICENSE_FUN_RECOGNIZE:
	{
		m_iRecognizeStep = 0;
		m_strShowImageName = strUrl;
		LicenseRun(LICENSE_FUN_RECOGNIZE);
		iRecThreadRun = 1;
		break;
	}
	case LICENSE_FUN_RESET:
	{
		for (int i = 0; i < 10; i++)
		{
			m_strLicenseResult[i].Empty();
		}

		break;
	}
	case LICENSE_FUN_EXIT:
	{
		break;
	}
	default:
		break;
	}


	if (pFile)
	{
		delete[]pFile;
	}

	copydata.dwData = m_msgPort;	//�ô��ں���ָʾģ��, ָ�����ص����������ĸ�ģ��(�ĸ��˴�ͨ����ģ��)


	if (!iRecThreadRun && !m_strParentTitle.IsEmpty())
	{
		HWND hwnd = ::FindWindow(NULL, m_strParentTitle);
		if (hwnd)
		{
			::SendMessageW(hwnd, WM_COPYDATA, NULL, (LPARAM)&copydata);
		}
	}

	return res;
}


void CCPosLicenseDlg::OnNcPaint()
{
	CDialog::OnNcPaint();
	static int i = 2;
	if (i <= 2)
	{
		if (m_iHideWindow)
		{
			ShowWindow(SW_HIDE);
		}
		i--;
	}
	else
	{
		CDialog::OnNcPaint();
	}

}


BOOL CCPosLicenseDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}

	if (pMsg->hwnd == m_pic_video_left.m_hWnd && m_check_video_left_rect.GetCheck() == BST_CHECKED)
	{
		CRect rect;
		m_pic_video_left.GetClientRect(rect);

		if (pMsg->message == WM_LBUTTONDOWN)
		{
			if (m_iAreaSetting == 0)
			{

				CPoint pt;
				GetCursorPos(&pt);
				m_pic_video_left.ScreenToClient(&pt);//����������껻���list��������

				m_fLeftVideoAreaRight = m_fLeftVideoAreaLeft = (float)((float)pt.x / rect.Width());
				m_fLeftVideoAreaBottom = m_fLeftVideoAreaTop = (float)((float)pt.y / rect.Height());

				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;


				m_iAreaSetting = 1;
			}


		}

		if (pMsg->message == WM_MOUSEMOVE
			&& (m_iAreaSetting == 1 || m_iAreaSetting == 2)
			)
		{
			CPoint pt;
			GetCursorPos(&pt);
			m_pic_video_left.ScreenToClient(&pt);//����������껻���list��������


			m_fLeftVideoAreaRight = (float)((float)pt.x / rect.Width());
			m_fLeftVideoAreaBottom = (float)((float)pt.y / rect.Height());

			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;

			m_iAreaSetting = 2;
		}

		if (pMsg->message == WM_LBUTTONDOWN
			&& m_iAreaSetting == 2
			)
		{
			CPoint pt;
			GetCursorPos(&pt);
			m_pic_video_left.ScreenToClient(&pt);//����������껻���list��������

			m_fLeftVideoAreaRight = (float)((float)pt.x / rect.Width());
			m_fLeftVideoAreaBottom = (float)((float)pt.y / rect.Height());

			m_check_video_left_rect.SetCheck(BST_UNCHECKED);

			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;

			m_check_video_left_rect.EnableWindow(TRUE);
			m_check_video_right_rect.EnableWindow(TRUE);

			m_iAreaSetting = 0;


		}
	}
		
	
	if (pMsg->hwnd == m_pic_video_right.m_hWnd && m_check_video_right_rect.GetCheck() == BST_CHECKED)
	{
		CRect rect;
		m_pic_video_right.GetClientRect(rect);

		if (pMsg->message == WM_LBUTTONDOWN)
		{
			if (m_iAreaSetting == 0)
			{

				CPoint pt;
				GetCursorPos(&pt);
				m_pic_video_right.ScreenToClient(&pt);//����������껻���list��������

				m_fRightVideoAreaRight = m_fRightVideoAreaLeft = (float)((float)pt.x / rect.Width());
				m_fRightVideoAreaBottom = m_fRightVideoAreaTop = (float)((float)pt.y / rect.Height());

				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;


				m_iAreaSetting = 1;
			}


		}

		if (pMsg->message == WM_MOUSEMOVE
			&& (m_iAreaSetting == 1 || m_iAreaSetting == 2)
			)
		{
			CPoint pt;
			GetCursorPos(&pt);
			m_pic_video_right.ScreenToClient(&pt);//����������껻���list��������


			m_fRightVideoAreaRight = (float)((float)pt.x / rect.Width());
			m_fRightVideoAreaBottom = (float)((float)pt.y / rect.Height());

			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;

			m_iAreaSetting = 2;
		}

		if (pMsg->message == WM_LBUTTONDOWN
			&& m_iAreaSetting == 2
			)
		{
			CPoint pt;
			GetCursorPos(&pt);
			m_pic_video_right.ScreenToClient(&pt);//����������껻���list��������

			m_fRightVideoAreaRight = (float)((float)pt.x / rect.Width());
			m_fRightVideoAreaBottom = (float)((float)pt.y / rect.Height());

			m_check_video_right_rect.SetCheck(BST_UNCHECKED);

			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;

			m_check_video_left_rect.EnableWindow(TRUE);
			m_check_video_right_rect.EnableWindow(TRUE);

			m_iAreaSetting = 0;


		}
	}
	
	return CDialogEx::PreTranslateMessage(pMsg);
}


int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);//(LPARAM)_T("C:\\Program Files")
	}

	return 0;

}

CString FolderPath(CString strDefaulltFolder, CString strTitle)
{

	CString cursel = _T("");

	BROWSEINFO bi;
	WCHAR name[MAX_PATH] = { 0 };
	ZeroMemory(&bi, sizeof(BROWSEINFO));

	bi.hwndOwner = NULL;
	bi.pszDisplayName = name;
	bi.lpszTitle = strTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;//����ѡ����ļ�����Ϊ�ļ���(ѡ������ʱȷ����ť��ɫ)
	bi.lpfn = BrowseCallbackProc;//�ص����������ڼ���Ĭ���ļ���
	bi.lParam = (LPARAM)(strDefaulltFolder.GetBuffer(strDefaulltFolder.GetLength()));//����Ĭ���ļ��е�·��

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (idl == NULL)
	{
		return _T("");
	}
	SHGetPathFromIDList(idl, cursel.GetBuffer(MAX_PATH));
	cursel.ReleaseBuffer();
	//		if(cursel.Right(1)!="\\")cursel +="\\" ;

	return cursel;

}
void CCPosLicenseDlg::OnBnClickedBLoadFolder()
{
		
	CString strFolderPath;
	CString str;

	m_edit_filename.GetWindowTextW(strFolderPath);
	str = FolderPath(strFolderPath, _T("Select Folder"));
	if (!str.IsEmpty())
	{
		m_edit_filename.SetWindowTextW(str);
		m_list_counter = -1;
	}


}


void CCPosLicenseDlg::OnBnClickedBDetectStop()
{
	KillTimer(1);
	m_b_detect_plate.EnableWindow(TRUE);
}


void CCPosLicenseDlg::OnBnClickedBSave()
{
	CString str;

	CString strPort;

	CString strMakeSure;
	CString strHide;

	m_edit_video_left_ip.GetWindowText(m_strLeftVideoUrl);
	m_edit_video_left_port.GetWindowText(strPort);	
	m_iLeftVideoPort = _wtoi(strPort);	//��Ƶ���Ӷ˿�
	m_edit_video_left_password.GetWindowText(m_strLeftVideoPassword);

	m_edit_makesure_num.GetWindowText(strMakeSure);
	m_iMakeSureNum = _wtoi(strMakeSure);

	strHide.Format(_T("%d"), m_check_hide_run.GetCheck());
	m_iHideRun = _wtoi(strHide);


	WriteSetup(_T("VIDEO"), _T("CAMERA_LEFT_IP"), m_strLeftVideoUrl);
	WriteSetup(_T("VIDEO"), _T("CAMERA_LEFT_PORT"), strPort);
	WriteSetup(_T("VIDEO"), _T("CAMERA_LEFT_PASSWORD"), PasswordEncode(m_strLeftVideoPassword));

	m_edit_video_right_ip.GetWindowText(m_strRightVideoUrl);
	m_edit_video_right_port.GetWindowText(strPort);
	m_iRightVideoPort = _wtoi(strPort);	//��Ƶ���Ӷ˿�
	m_edit_video_right_password.GetWindowText(m_strRightVideoPassword);


	WriteSetup(_T("VIDEO"), _T("CAMERA_RIGHT_IP"), m_strRightVideoUrl);
	WriteSetup(_T("VIDEO"), _T("CAMERA_RIGHT_PORT"), strPort);
	WriteSetup(_T("VIDEO"), _T("CAMERA_RIGHT_PASSWORD"), PasswordEncode(m_strRightVideoPassword));

	WriteSetup(_T("VIDEO"), _T("MAKESURE_NUM"), strMakeSure);
	WriteSetup(_T("VIDEO"), _T("WND_HIDE"), strHide);

	m_iSaveVideoResult = m_check_output_video_result.GetCheck();
	str.Format(_T("%d"), m_iSaveVideoResult);
	WriteSetup(_T("VIDEO"), _T("SAVE_RESULT"), str);

	m_iCapture = m_check_capture.GetCheck();
	str.Format(_T("%d"), m_iCapture);
	WriteSetup(_T("VIDEO"), _T("CAPTURE"), str);

	str.Format(_T("%f"), m_fLeftVideoAreaLeft < m_fLeftVideoAreaRight ? m_fLeftVideoAreaLeft : m_fLeftVideoAreaRight);  WriteSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_LEFT"), str);
	str.Format(_T("%f"), m_fLeftVideoAreaTop < m_fLeftVideoAreaBottom ? m_fLeftVideoAreaTop : m_fLeftVideoAreaBottom);  WriteSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_TOP"), str);
	str.Format(_T("%f"), m_fLeftVideoAreaLeft > m_fLeftVideoAreaRight ? m_fLeftVideoAreaLeft : m_fLeftVideoAreaRight);  WriteSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_RIGHT"), str);
	str.Format(_T("%f"), m_fLeftVideoAreaTop > m_fLeftVideoAreaBottom ? m_fLeftVideoAreaTop : m_fLeftVideoAreaBottom);  WriteSetup(_T("VIDEO"), _T("LEFT_VIDEO_AREA_BOTTOM"), str);

	str.Format(_T("%f"), m_fRightVideoAreaLeft < m_fRightVideoAreaRight ? m_fRightVideoAreaLeft : m_fRightVideoAreaRight);  WriteSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_LEFT"), str);
	str.Format(_T("%f"), m_fRightVideoAreaTop < m_fRightVideoAreaBottom ? m_fRightVideoAreaTop : m_fRightVideoAreaBottom);  WriteSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_TOP"), str);
	str.Format(_T("%f"), m_fRightVideoAreaLeft > m_fRightVideoAreaRight ? m_fRightVideoAreaLeft : m_fRightVideoAreaRight);  WriteSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_RIGHT"), str);
	str.Format(_T("%f"), m_fRightVideoAreaTop > m_fRightVideoAreaBottom ? m_fRightVideoAreaTop : m_fRightVideoAreaBottom);  WriteSetup(_T("VIDEO"), _T("RIGHT_VIDEO_AREA_BOTTOM"), str);

	m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;


//	m_videoPlayer.StopAllVideos();


//	VideoWndinit();
}

void CCPosLicenseDlg::LicenseInsertLeft(CString strLicense, char car_type)
{
	m_strLicenseResult[8] = m_strLicenseResult[6];
	m_strLicenseResult[6] = m_strLicenseResult[4];
	m_strLicenseResult[4] = m_strLicenseResult[2];
	m_strLicenseResult[2] = m_strLicenseResult[0];
	m_strLicenseResult[0] = strLicense;


	m_iCarType[8] = m_iCarType[6];
	m_iCarType[6] = m_iCarType[4];
	m_iCarType[4] = m_iCarType[2];
	m_iCarType[2] = m_iCarType[0];
	m_iCarType[0] = car_type;

	return;

	m_strLicenseResult[4] = m_strLicenseResult[3];
	m_strLicenseResult[3] = m_strLicenseResult[2];
	m_strLicenseResult[2] = m_strLicenseResult[1];
	m_strLicenseResult[1] = m_strLicenseResult[0];
	m_strLicenseResult[0] = strLicense;


	m_iCarType[4] = m_iCarType[3];
	m_iCarType[3] = m_iCarType[2];
	m_iCarType[2] = m_iCarType[1];
	m_iCarType[1] = m_iCarType[0];
	m_iCarType[0] = car_type;



}

void CCPosLicenseDlg::LicenseInsertRight(CString strLicense, char car_type)
{
	m_strLicenseResult[9] = m_strLicenseResult[7];
	m_strLicenseResult[7] = m_strLicenseResult[5];
	m_strLicenseResult[5] = m_strLicenseResult[3];
	m_strLicenseResult[3] = m_strLicenseResult[1];
	m_strLicenseResult[1] = strLicense;

	m_iCarType[9] = m_iCarType[7];
	m_iCarType[7] = m_iCarType[5];
	m_iCarType[5] = m_iCarType[3];
	m_iCarType[3] = m_iCarType[1];
	m_iCarType[1] = car_type;

	return;

	m_strLicenseResult[9] = m_strLicenseResult[8];
	m_strLicenseResult[8] = m_strLicenseResult[7];
	m_strLicenseResult[7] = m_strLicenseResult[6];
	m_strLicenseResult[6] = m_strLicenseResult[5];
	m_strLicenseResult[5] = strLicense;

	m_iCarType[9] = m_iCarType[8];
	m_iCarType[8] = m_iCarType[7];
	m_iCarType[7] = m_iCarType[6];
	m_iCarType[6] = m_iCarType[5];
	m_iCarType[5] = car_type;

}

//���ĳ���ִ��ĸ�������������, �򷵻ش��ִ�, ���򷵻ؿ��ִ�
CString CCPosLicenseDlg::LicenseInsertEnsure(int number)
{
	CString strCmp;
	int counter = 0;
	int i = 0;
	int j = 0;


	//m_strLicenseResult[0] = _T("1");
	//m_strLicenseResult[1] = _T("1");
	//m_strLicenseResult[2] = _T("0");
	//m_strLicenseResult[3] = _T("4");
	//m_strLicenseResult[4] = _T("1");
	//m_strLicenseResult[5] = _T("0");
	//m_strLicenseResult[6] = _T("2");





	CString strLicense[10];//����10�����������ͬ
	int index = 0;

	int cnt_max[10] = { 0 };
	int car_type = 0;
	memset(&cnt_max, 0, sizeof(cnt_max));

	for (i = 0; i < 10; i++)
	{
		strCmp = m_strLicenseResult[i];

		if (strCmp.IsEmpty()) continue;

		for (j = 0; j < 10; j++)//��ѯ������Ƿ��Ѿ�����, ���ڵĲ��ٲ�ѯ
		{
			if (strCmp == strLicense[j])
			{
				break;
			}
		}

		if (j < 10)//�Ѵ���
		{
			continue;
		}

		counter = 0;
		for (j = 0; j < 10; j++)
		{
			if (strCmp == m_strLicenseResult[j])
			{
				counter++;
			}
		}

		strLicense[index] = strCmp;
		cnt_max[index] = counter;

		if (counter >= number)//�Ѿ���������, ���治�ټ���
		{
			break;
		}

		index++;
	}

	int max = 0;
	index = 0;
	for (i = 0; i < 10; i++)
	{
		if (cnt_max[i] > max)
		{
			max = cnt_max[i];
			index = i;
		}
	}


	if (max >= number)
	{
		strCmp = strLicense[index];
	}
	else
	{
		strCmp.Empty();
	}
	return strCmp;

}

void CCPosLicenseDlg::VideoOsdRefresh(CString strTitle)
{
	int i = 0;
	int pos = 0;
	CString str;

	//m_strLicenseResult[0] = _T("AB12345");
	//m_strLicenseResult[1] = _T("DE13424");
	//m_strLicenseResult[2] = _T("DY3321");
	//m_strLicenseResult[3] = _T("YF2342");
	//m_strLicenseResult[4] = _T("KYT324");

	//m_strLicenseResult[5] = _T("AB12345");
	//m_strLicenseResult[6] = _T("DE13424");
	//m_strLicenseResult[7] = _T("DY3321");
	//m_strLicenseResult[8] = _T("YF2342");
	//m_strLicenseResult[9] = _T("KYT324");
			
	m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_strOsdTitle = strTitle;


	for (i = 0; i < 10; i++)
	{
		if (i & 0x01)
		{
			str = m_strLicenseResult[i];
			if (str.GetLength() < 10)
			{
				str.Format(_T("%10s          "), str);
			}
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_strOsd[i/2] = str;
		}
		else
		{
			str = m_strLicenseResult[i];
			if (str.GetLength() < 10)
			{
				str.Format(_T("%10s          "), str);
			}
			m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_strOsd[i/2] = str;
		}
	}




}


LRESULT CCPosLicenseDlg::OnLicenseThread(WPARAM wParam, LPARAM lParam)
{
	CString str;
	CString strLicense;
	static int run_counter = 0;
	int i = 0;
	CString strCarType;

	LICENSE_RESULT license_result;
	if (wParam == sizeof(LICENSE_RESULT))
	{
		memcpy(&license_result, (unsigned char *)lParam, wParam);	//�ӻ����и�������

		if (license_result.function == LICENSE_FUN_READY)
		{
			m_b_video_start.SetWindowTextW(_T("Ready"));
		}
		else if (license_result.function == LICENSE_FUN_RECOGNIZE)
		{

		}
		else if (license_result.function == LICENSE_FUN_COMPLETE)
		{
			run_counter++;

			strLicense = CString(license_result.license);
			strCarType = get_car_type(license_result.car_type - '0');

			m_edit_car_type.SetWindowTextW(strCarType);

		//	m_edit_message.SetWindowTextW(str);
		//	m_edit_license.SetWindowTextW(strLicense);
		}

		strLicense = LicenseInsertEnsure(m_iMakeSureNumValue);

		if (!strLicense.IsEmpty())
		{
			//ʶ��ɹ�ֹͣ����
			KillTimer(3);
			m_videoLocal.StopLocalFile();

			COPYDATASTRUCT copydata;
			DWORD len = 0;	//����ֵ�ĳ���
			LICENSE_RESULT result;
			int res = 0;
			memset(&result, 0, sizeof(result));
			copydata.cbData = sizeof(result);
			copydata.lpData = &result;

			result.function = m_funRunning;
			copydata.dwData = m_msgPort;	//�ô��ں���ָʾģ��, ָ�����ص����������ĸ�ģ��(�ĸ��˴�ͨ����ģ��)
			WideCharToMultiByte(CP_ACP, 0, strLicense, -1, result.license, sizeof(result.license) - 1, NULL, NULL);
			if (!m_strRecognizeImageName.IsEmpty())
			{
				WideCharToMultiByte(CP_ACP, 0, m_strRecognizeImageName, -1, result.url, sizeof(result.url) - 1, NULL, NULL);
			}

			result.car_type = m_iCarType[0];
			if (!m_strParentTitle.IsEmpty())
			{
				HWND hwnd = ::FindWindow(NULL, m_strParentTitle);
				if (hwnd)
				{
					::SendMessageW(hwnd, WM_COPYDATA, NULL, (LPARAM)&copydata);
					for (i = 0; i < 10; i++)
					{
						m_strLicenseResult[i].Empty();
					}

				}
			}



		}
		else if (!strCarType.IsEmpty())
		{
			COPYDATASTRUCT copydata;
			DWORD len = 0;	//����ֵ�ĳ���
			LICENSE_RESULT result;
			int res = 0;
			memset(&result, 0, sizeof(result));
			copydata.cbData = sizeof(result);
			copydata.lpData = &result;

			result.function = m_funRunning;
			copydata.dwData = m_msgPort;	//�ô��ں���ָʾģ��, ָ�����ص����������ĸ�ģ��(�ĸ��˴�ͨ����ģ��)

			if (!m_strRecognizeImageName.IsEmpty())
			{
				WideCharToMultiByte(CP_ACP, 0, m_strRecognizeImageName, -1, result.url, sizeof(result.url) - 1, NULL, NULL);
			}

			result.car_type = m_iCarType[0];
			if (!m_strParentTitle.IsEmpty())
			{
				HWND hwnd = ::FindWindow(NULL, m_strParentTitle);
				if (hwnd)
				{
					::SendMessageW(hwnd, WM_COPYDATA, NULL, (LPARAM)&copydata);
					for (i = 0; i < 10; i++)
					{
						m_strLicenseResult[i].Empty();
					}

				}
			}
		}
		else
		{
			//m_edit_car_type.SetWindowTextW(_T(""));
		}


		if (!m_iHideWindow)
		{

			//ˢ����ʾͼƬ
			CRect rect;
			if (m_image_plate_last == 1 && m_image_plate.data == NULL)
			{
				m_pic_plate.GetWindowRect(rect);
				ScreenToClient(rect);
				RedrawWindow(rect);
			}


			if (m_image_full_last == 1 && m_image_full.data == NULL)
			{
				m_pic_source.GetWindowRect(rect);
				ScreenToClient(rect);
				RedrawWindow(rect);
			}
			
			SendMessage(WM_PAINT);

			
			if (strLicense.GetLength() < 10)
			{
				strLicense.Format(_T("%s"), strLicense);
			}


			m_edit_license.SetWindowTextW(strLicense);

			str.Format(_T("%d "), run_counter);

			m_edit_message.SetWindowTextW(str);
			VideoOsdRefresh(strLicense);

		}

	}

	return 1;
}



DWORD CCPosLicenseDlg::LicenseThreadFunction(LPVOID parm)
{
	CCPosLicenseDlg *pThis = (CCPosLicenseDlg*)parm;
	int i = 0;
	int nRetLength = 0;
	DWORD dwError = 0;
	BOOL videoCaptureResult = FALSE;
	LICENSE_RESULT license_result;
	memset(&license_result, 0, sizeof(license_result));

	CString strLicense;
	int err = 0;
	char car_type = -1;
	OCR_RESULT pos_result;
	memset(&pos_result, 0, sizeof(pos_result));
	plate_detect(_T(""), 0, 0, &pThis->m_image_plate, &pos_result, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fLeftVideoAreaLeft, pThis->m_fLeftVideoAreaTop, pThis->m_fLeftVideoAreaRight, pThis->m_fLeftVideoAreaBottom);//����ʶ���ģ��

	if (pThis->m_hWnd)
	{
		license_result.function = LICENSE_FUN_READY;
		::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
	}
	CString strLastImageName;

	CString strVideoResultFileName;
	CString str;

	while (pThis->m_iLicenseRunType != LICENSE_FUN_EXIT)
	{
		dwError = WaitForSingleObject(pThis->m_hLicenseEvent, INFINITE);	//�ڴ˵ȴ��ص��¼�����, �˴�INFINITE���޵ȴ�, Ҳ��������Ϊ��ʱʱ��
		if (dwError == WAIT_OBJECT_0) //�¼�����
		{
			if (pThis->m_iLicenseRunType == LICENSE_FUN_EXIT)
			{
				break;
			}
			else if (pThis->m_iLicenseRunType == LICENSE_FUN_RECOGNIZE)
			{
				pThis->m_iLicenseRunType = LICENSE_FUN_RUNNING;


				// ��ͷʶ�� /////////////////////////////////////////////////////////////////////////////////////

				memset(&license_result, 0, sizeof(license_result));
				strLicense.Empty();
				car_type = -1;
				err = 0;

				if (!pThis->m_strShowImageName.IsEmpty())
				{
					if (pThis->m_strShowImageName.Right(4) == _T(".mp4") || pThis->m_strShowImageName.Right(4) == _T("h264"))
					{
						pThis->m_iMakeSureNumValue = pThis->m_iMakeSureNum;

						//��һ�ε��ÿ�ʼ������Ƶ
						if (pThis->m_videoLocal.m_nPlaydecHandle < 0 || strLastImageName != pThis->m_strShowImageName)//δ����
						{
							pThis->m_iRecognizeStep++;
							pThis->m_strRecognizeImageName = pThis->m_strShowImageName;
							if (pThis->m_videoLocal.PlayLocalFile(INDEX_VIDEO_LOCAL, pThis->m_strShowImageName, pThis->m_pic_video_right.m_hWnd, pThis->m_hWnd) >= 0) //��ʼ����
							{
								if (pThis->m_iSaveVideoResult)
								{
									strVideoResultFileName = pThis->m_strShowImageName;
									strVideoResultFileName.Replace(_T(".mp4"), _T(".txt"));
									SaveStrToTxt(strVideoResultFileName, 0, _T("-----------------\n"));
								}

								strLastImageName = pThis->m_strShowImageName;
								for (i = 0; i < 10; i++)pThis->m_strLicenseResult[i].Empty();
								for (i = 0; i < 10; i++)pThis->m_iCarType[i] = -1;

								pThis->SetTimer(3, 1, 0);//������ʱ��, ����ʶ��
							}
							else//����ʧ��
							{
								strVideoResultFileName.Empty();
								pThis->KillTimer(3);
							}

						}//��ʱ����������ʼʶ��
						else
						{
							//��Ҫ��ε���, ��ʵ�ֶ�֡ʶ��
							//��ʼ��ͼ
							videoCaptureResult = pThis->m_videoLocal.CaptureToBuffer();
							err = 0;

							if (videoCaptureResult && pThis->m_videoLocal.m_capture_dib_buffer && !pThis->m_iHideWindow)
							{
								EnterCriticalSection(&pThis->m_CriticalSectionImage); //�����ٽ������������Ȩ�������߳̾͵�
								if (pThis->m_image_full.data != 0)  release_image(&pThis->m_image_full);
								pThis->m_image_full = load_image_from_memory(pThis->m_videoLocal.m_capture_dib_buffer, pThis->m_videoLocal.m_capture_dib_len);
								LeaveCriticalSection(&pThis->m_CriticalSectionImage);  //�˳��ٽ���
							}
							//��ͼ�ɹ�, ��ʼʶ��
							if (videoCaptureResult)
							{
								DWORD tick_start = GetTickCount();
								memset(&pos_result, 0, sizeof(pos_result));
								strLicense = plate_detect(_T(""), pThis->m_videoLocal.m_capture_dib_buffer, pThis->m_videoLocal.m_capture_dib_len, &pThis->m_image_plate, &pos_result, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fLeftVideoAreaLeft, pThis->m_fLeftVideoAreaTop, pThis->m_fLeftVideoAreaRight, pThis->m_fLeftVideoAreaBottom);
								DWORD tick_end = GetTickCount();


								//������
								if (pThis->m_iSaveVideoResult && !strVideoResultFileName.IsEmpty())
								{
									CString strVideoResult;

									if (err == 5 && !strLicense.IsEmpty())
									{
										strVideoResult = strLicense;
										strVideoResult += _T(",");
										strVideoResult += get_car_type(car_type - '0');
										
									}
									else if (err == 2)
									{
										strVideoResult += _T("NO PLATE,");
										//������
									}
									else if (err == 3)
									{
										strVideoResult += _T("NO CHARACTER,");
										//����������ĸ
									}

									str.Format(_T(",%d ms"), tick_end - tick_start);//
									strVideoResult += str;
									strVideoResult += _T("\n");
									SaveStrToTxt(strVideoResultFileName, 0, strVideoResult);
								}
							}

						}

						

					}
					else
					{
						for (i = 0; i < 10; i++)pThis->m_strLicenseResult[i].Empty();
						for (i = 0; i < 10; i++)pThis->m_iCarType[i] = -1;

						pThis->m_iMakeSureNumValue = 1;

						pThis->KillTimer(3);
						err = 0;
						memset(&pos_result, 0, sizeof(pos_result));
						pThis->m_iRecognizeStep++;

						pThis->m_strRecognizeImageName = pThis->m_strShowImageName;
						strLicense = plate_detect(pThis->m_strShowImageName, 0, 0, &pThis->m_image_plate, &pos_result, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fLeftVideoAreaLeft, pThis->m_fLeftVideoAreaTop, pThis->m_fLeftVideoAreaRight, pThis->m_fLeftVideoAreaBottom);

						if (err == 5 && !strLicense.IsEmpty())
						{
							//�ɹ�ʶ��
						}
						else
						{
							pThis->m_strShowImageName.Replace(_T(".jpg"), _T("_1.mp4"));
							pThis->SetTimer(3, 1, 0);//������Ƶʶ��
						}

					}
				}
				else
				{
					//Ĭ��ʹ����ͷ
					//ʵʱ��Ƶʶ��
					if (!pThis->m_strLeftVideoUrl.IsEmpty())
					{
						pThis->m_iMakeSureNumValue = pThis->m_iMakeSureNum;

						pThis->KillTimer(3);

						videoCaptureResult = pThis->m_videoPlayer.CaptureToBuffer(INDEX_VIDEO_LEFT);

						//pThis->LicenseRecognize(_T(""), pThis->m_dib_buffer, pThis->m_dib_size);
						err = 0;

						if (videoCaptureResult && pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_capture_dib_buffer && !pThis->m_iHideWindow)
						{

							EnterCriticalSection(&pThis->m_CriticalSectionImage); //�����ٽ������������Ȩ�������߳̾͵�
							if (pThis->m_image_full.data != 0)  release_image(&pThis->m_image_full);
							pThis->m_image_full = load_image_from_memory(pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_capture_dib_buffer, pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_capture_dib_len);
							//		ShowPicture(m_pic_source, im_src);
							LeaveCriticalSection(&pThis->m_CriticalSectionImage);  //�˳��ٽ���
						}
						if (videoCaptureResult)
						{
							memset(&pos_result, 0, sizeof(pos_result));
							strLicense = plate_detect(_T(""), pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_capture_dib_buffer, pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_capture_dib_len, &pThis->m_image_plate, &pos_result, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fLeftVideoAreaLeft, pThis->m_fLeftVideoAreaTop, pThis->m_fLeftVideoAreaRight, pThis->m_fLeftVideoAreaBottom);
						
						
							if (pos_result.x > 0)
							{
								
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fLeft = (float)((float)pos_result.x / pThis->m_image_full.w);
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fTop = (float)((float)pos_result.y / pThis->m_image_full.h);
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fRight = (float)((float)(pos_result.x + pos_result.w) / pThis->m_image_full.w);
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fBottom = (float)((float)(pos_result.y + pos_result.h) / pThis->m_image_full.h);
							}
							else
							{
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fLeft = 0;
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fTop = 0;
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fRight = 0;
								pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_scale_plate.fBottom = 0;
							}
						
						}
						//	strLicense = _T("AB1234"); err = 5;

					}
				}


				//������ͷ/������Ƶʶ����
				if (err == 5 && !strLicense.IsEmpty())
				{
					license_result.function = LICENSE_FUN_COMPLETE;
					WideCharToMultiByte(CP_ACP, 0, strLicense, -1, license_result.license, sizeof(license_result.license) - 1, NULL, NULL);
					license_result.car_type = car_type;
					pThis->LicenseInsertLeft(strLicense, car_type);
					if (pThis->m_hWnd)
					{
						::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
					}


				}
				else if (err == 2 && strLicense.IsEmpty() && car_type >= 0)//δʶ�𵽳���,��ʶ�𵽳���
				{
					license_result.function = LICENSE_FUN_COMPLETE;
					WideCharToMultiByte(CP_ACP, 0, strLicense, -1, license_result.license, sizeof(license_result.license) - 1, NULL, NULL);
					license_result.car_type = car_type;
					pThis->LicenseInsertLeft(strLicense, car_type);
					if (pThis->m_hWnd)
					{
						::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
					}
				}
				else//ʧ�ܣ�����һ֡�ճ���
				{
					license_result.function = LICENSE_FUN_RECOGNIZE;
					pThis->LicenseInsertLeft(strLicense, car_type);
					::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
				}


				// �Ҿ�ͷʶ�� /////////////////////////////////////////////////////////////////////////////////////
				//�Ҿ�ͷֻ����Ƶʶ��
				memset(&license_result, 0, sizeof(license_result));
				strLicense.Empty();

				err = 0;
				if (!pThis->m_strRightVideoUrl.IsEmpty())
				{

					videoCaptureResult = pThis->m_videoPlayer.CaptureToBuffer(INDEX_VIDEO_RIGHT);

					//pThis->LicenseRecognize(_T(""), pThis->m_dib_buffer, pThis->m_dib_size);
					err = 0;

					if (videoCaptureResult && pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_capture_dib_buffer && !pThis->m_iHideWindow)
					{

						EnterCriticalSection(&pThis->m_CriticalSectionImage); //�����ٽ������������Ȩ�������߳̾͵�

						if (pThis->m_image_full.data != 0)  release_image(&pThis->m_image_full);
						pThis->m_image_full = load_image_from_memory(pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_capture_dib_buffer, pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_capture_dib_len);
						//		ShowPicture(m_pic_source, im_src);
						LeaveCriticalSection(&pThis->m_CriticalSectionImage);  //�˳��ٽ���

					}
					if (videoCaptureResult)
					{
						memset(&pos_result, 0, sizeof(pos_result));
						
						strLicense = plate_detect(_T(""), pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_capture_dib_buffer, pThis->m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_capture_dib_len, &pThis->m_image_plate, &pos_result, &err, pThis->m_iSaveTrain, pThis->m_iSaveImage, car_type, pThis->m_fRightVideoAreaLeft, pThis->m_fRightVideoAreaTop, pThis->m_fRightVideoAreaRight, pThis->m_fRightVideoAreaBottom);
						
					
					}


					if (err == 5 && !strLicense.IsEmpty())
					{
						license_result.function = LICENSE_FUN_COMPLETE;
						WideCharToMultiByte(CP_ACP, 0, strLicense, -1, license_result.license, sizeof(license_result.license) - 1, NULL, NULL);
						license_result.car_type = car_type;
						if (pThis->m_hWnd)
						{
							::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
						}
						pThis->LicenseInsertRight(strLicense, car_type);

					}
					else if (err == 2 && strLicense.IsEmpty() && car_type >= 0)//δʶ�𵽳���,��ʶ�𵽳���
					{
						license_result.function = LICENSE_FUN_COMPLETE;
						WideCharToMultiByte(CP_ACP, 0, strLicense, -1, license_result.license, sizeof(license_result.license) - 1, NULL, NULL);
						license_result.car_type = car_type;
						pThis->LicenseInsertRight(strLicense, car_type);
						if (pThis->m_hWnd)
						{
							::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
						}
					}
					else//ʧ�ܣ�����һ֡�ճ���
					{
						license_result.function = LICENSE_FUN_RECOGNIZE;
						::SendMessage(pThis->m_hWnd, WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));
						pThis->LicenseInsertRight(strLicense, car_type);
					}
				}


				//����ʶ������� ׼����һ��ʶ��
				pThis->m_iLicenseRunType = LICENSE_FUN_COMPLETE;
			}



			ResetEvent(pThis->m_hLicenseEvent);

		}
		else
		{
			if (dwError == WAIT_TIMEOUT)//��ʱ
			{

			}
			else if (dwError == WAIT_FAILED)
			{

			}

		}


	}

	CloseHandle(pThis->m_hLicenseThread);
	CloseHandle(pThis->m_hLicenseEvent);
	pThis->m_hLicenseThread = NULL;
	pThis->m_hLicenseEvent = NULL;

	return 0;
}



int CCPosLicenseDlg::LicenseRun(int iType)
{

	if (NULL != m_hLicenseThread)
	{
		//����ʶ��, ������
		if (iType == LICENSE_FUN_RECOGNIZE &&  m_iLicenseRunType == LICENSE_FUN_RUNNING)
		{
			return 0;
		}

		m_iLicenseRunType = iType;

		SetEvent(m_hLicenseEvent);	//�����ź�, ��ֹ�źŵȴ�
		if (m_iLicenseRunType == LICENSE_FUN_EXIT)
		{
			if (m_hLicenseThread)
			{
				if (WAIT_TIMEOUT == WaitForSingleObject(m_hLicenseThread, 1000))//�ȴ��߳��ź�, �߳̽����ź�
				{
					TerminateThread(m_hLicenseThread, 0);	//����ź�ʧ��, ǿ���߳���ֹ(������ǿ����ֹ�߳�)
				}
			}
		}

	}

	return 0;
}
int CCPosLicenseDlg::LicenseThreadStart()
{
	if (m_hLicenseEvent != NULL)
	{
		ResetEvent(m_hLicenseEvent);
	}
	else
	{
		m_hLicenseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	// �����������߳�  
	if (m_hLicenseThread == 0)
	{

		DWORD dwThreadID;
		m_hLicenseThread = CreateThread(NULL, 0, LicenseThreadFunction, this, 0, &dwThreadID);
		return TRUE;
	}

	return FALSE;

}







void CCPosLicenseDlg::OnBnClickedBVideoStart()
{
	//LicenseRun(LICENSE_FUN_RECOGNIZE);
	//return;

	m_iSaveTrain = m_check_output_train.GetCheck();
	m_iSaveImage = m_check_output_image.GetCheck();

	if (KillTimer(2))
	{
		m_b_video_start.SetWindowTextW(_T("Ready"));
		m_strShowImageName.Empty();
		m_edit_filename.SetWindowTextW(m_strShowImageName);

		for (int i = 0; i < 10; i++)
		{
			m_strLicenseResult[i].Empty();
		}
		LICENSE_RESULT license_result;
		memset(&license_result, 0, sizeof(license_result));
		license_result.function = LICENSE_FUN_READY;
		SendMessage(WM_LICENSE_THREAD_MESSAGE, sizeof(LICENSE_RESULT), (LPARAM)(&license_result));

	}
	else
	{
		SetTimer(2, 1, 0);
		m_b_video_start.SetWindowTextW(_T("Running"));
	}

}


void CCPosLicenseDlg::OnStnClickedPicVideoLeft()
{
	//static int pau = 1;
	//m_videoPlayer.PicturePause(INDEX_VIDEO_LEFT, pau);
	//pau = !pau;

	CString strFileName;
	COleDateTime time = COleDateTime::GetCurrentTime();
	strFileName.Format(_T("%sCapture\\%s\\%s\\%s\\%s\\%s"),
		GetAppPath().GetBuffer(),
		m_strLeftVideoUrl.GetBuffer(),
		time.Format(_T("%Y")).GetBuffer(),
		time.Format(_T("%m")).GetBuffer(),
		time.Format(_T("%d")).GetBuffer(),
		time.Format(_T("%Y%m%d%H%M%S.jpg")).GetBuffer()
	);

	if (m_videoPlayer.CaptureToFile(INDEX_VIDEO_LEFT, strFileName) == 0)
	{
		CString str;
		m_edit_message.GetWindowText(str);
		m_edit_message.SetWindowText(str + _T("Save Error."));
	}


}


void CCPosLicenseDlg::OnStnClickedPicVideoRight()
{

//	m_videoPlayer.CaptureToFile(INDEX_VIDEO_RIGHT, _T("d:\\test_right.jpg"));
	CString strFileName;

	COleDateTime time = COleDateTime::GetCurrentTime();


	strFileName.Format(_T("%sCapture\\%s\\%s\\%s\\%s\\%s"),
		GetAppPath().GetBuffer(),
		m_strRightVideoUrl.GetBuffer(),
		time.Format(_T("%Y")).GetBuffer(),
		time.Format(_T("%m")).GetBuffer(),
		time.Format(_T("%d")).GetBuffer(),
		time.Format(_T("%Y%m%d%H%M%S.jpg")).GetBuffer()
	);

	if(m_videoPlayer.CaptureToFile(INDEX_VIDEO_RIGHT, strFileName) == 0)
	{
		CString str;
		m_edit_message.GetWindowText(str);
		m_edit_message.SetWindowText(str + _T("Save Error."));
	}


}

#pragma comment(lib, "Version.lib")
CString CAboutDlg::GetFileVersion()
{
	CString strResult = NULL, strVersion = NULL;
	UINT uSize = NULL;
	LPWSTR pInfo = NULL, pBuff = NULL;
	TCHAR szAppPath[MAX_PATH] = { 0 };

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	::GetModuleFileName(NULL, szAppPath, MAX_PATH);

	uSize = GetFileVersionInfoSizeEx(FILE_VER_GET_LOCALISED, szAppPath, 0);         //���ȫ����Դ�汾��Ϣ�Ĵ�С
	pBuff = new WCHAR[uSize];

	if (GetFileVersionInfoExW(FILE_VER_GET_LOCALISED, szAppPath, NULL, uSize, pBuff))       //���ȫ����Դ�汾��Ϣ
	{
		if (VerQueryValueW(pBuff, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &uSize))        //�����Դ�е����Դ���ҳ
		{
			strVersion.Format(L"\\StringFileInfo\\%04x%04x\\FileVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
			if (!VerQueryValueW(pBuff, strVersion.GetBuffer(), (LPVOID*)&pInfo, &uSize))     //ȡ����Դ�е���Ϣ
			{
				// AfxMessageBox(L"��ȡ��˾����ʧ��! ");
			}
			strResult = pInfo;
			strVersion.ReleaseBuffer();
		}
	}

	delete[]pBuff;

	return CString(strResult);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str = _T("Version ") + GetFileVersion();
	//str += FILEVERSION;
	m_static_version.SetWindowText(str);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCPosLicenseDlg::OnBnClickedCheckVideoLeftRect()
{
	if (m_check_video_left_rect.GetCheck() == BST_UNCHECKED)
	{
		if (m_iAreaSetting == 0)//δѡ������ȡ��
		{
			if (MessageBox(_T("ȡ���R�e�^��?"), _T("�_�J"), MB_YESNO) == IDYES)
			{
				m_fLeftVideoAreaLeft = 0;
				m_fLeftVideoAreaTop = 0;
				m_fLeftVideoAreaRight = 0;
				m_fLeftVideoAreaBottom = 0;

				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectLeft = m_fLeftVideoAreaLeft;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectTop = m_fLeftVideoAreaTop;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectRight = m_fLeftVideoAreaRight;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_LEFT].m_fRectBottom = m_fLeftVideoAreaBottom;

			}
		}

		m_iAreaSetting = 0;

		m_check_video_left_rect.EnableWindow(TRUE);
		m_check_video_right_rect.EnableWindow(TRUE);

	}
	else
	{
		m_check_video_right_rect.EnableWindow(FALSE);
	}
	
}



void CCPosLicenseDlg::OnBnClickedCheckVideoRightRect()
{
	if (m_check_video_right_rect.GetCheck() == BST_UNCHECKED)
	{
		if (m_iAreaSetting == 0)//δѡ������ȡ��
		{
			if (MessageBox(_T("ȡ���R�e�^��?"), _T("�_�J"), MB_YESNO) == IDYES)
			{

				m_fRightVideoAreaLeft = 0;
				m_fRightVideoAreaTop = 0;
				m_fRightVideoAreaRight = 0;
				m_fRightVideoAreaBottom = 0;

				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectLeft = m_fRightVideoAreaLeft;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectTop = m_fRightVideoAreaTop;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectRight = m_fRightVideoAreaRight;
				m_videoPlayer.m_videoWnd[INDEX_VIDEO_RIGHT].m_fRectBottom = m_fRightVideoAreaBottom;


			}
		}

		m_iAreaSetting = 0;

		m_check_video_left_rect.EnableWindow(TRUE);
		m_check_video_right_rect.EnableWindow(TRUE);


	}
	else
	{
		m_check_video_left_rect.EnableWindow(FALSE);

	}

}


//���Ž���
LRESULT CCPosLicenseDlg::OnVideoPlayMessage(WPARAM wParam, LPARAM lParam)
{
	KillTimer(3);
	m_videoLocal.StopLocalFile();

	//���Ž���, δ�ҵ�,������ͼƬ��

	if (m_iRecognizeStep == 1)
	{
		m_strShowImageName.Replace(_T("_1.mp4"), _T(".jpg"));
		SetTimer(3, 1, 0);//����ʶ��
	}

	return 1;
}



void CCPosLicenseDlg::OnStnClickedStaticMainTitle()
{
	CString strConfig;
	m_edit_main_title.GetWindowTextW(strConfig);

	if (strConfig.IsEmpty())return;

	static int g_iRestTitle = 0;
	
	if (g_iRestTitle < 3)
	{
		g_iRestTitle++;
		return;
	}

	if (MessageBox(_T("���O?"), _T("�_�J"), MB_YESNO) == IDYES)
	{

		SetWindowTextW(m_strDlgMyName + strConfig);
	}
	
	g_iRestTitle = 0;
}
