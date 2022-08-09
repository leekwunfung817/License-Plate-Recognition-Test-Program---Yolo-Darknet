// VideoWnd.cpp : implementation file
//

#include "stdafx.h"

#include "VideoWnd.h"

#include <direct.h>
#include <IO.H>

#include "video_sdk/include/H264Play.h"
#include "darknet/src/image.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoWnd dialog

static int CreateFolder(CString strPath)
{
	//	strPath = _T("G:\Speech\Text\yuyin\Store");//此处可随意定义，但格式必须与所示一致，会依次创建所有的，如果已经创建好了，则不创建
	CString strTmp;
	int i = 0;

	strPath.Trim();
	i = strPath.Find(_T("\\"));//找到根目录

	if (i != 2)
	{
		i = strPath.Find(_T("\\\\"));//找window共享目录
		if (i != 0)
		{
			return 0;
		}
	}


	do
	{
		i = strPath.Find(_T("\\"), i + 1);
		if (i > 0)
		{
			strTmp = strPath.Left(i);
			if (!PathFileExists(strTmp))//文件夹不存在则创建
			{
				CreateDirectory(strTmp, NULL);
			}
		}
		else
		{
			break;
		}
	} while (i>0);

	return 1;

}

CVideoWnd::CVideoWnd(CWnd* pParent /*=NULL*/)
{

	m_nIndex = -1;
	m_iPlayhandle = -1;
	m_nPlaydecHandle = -1;
	m_bRecord = FALSE;
	for ( int i = 0; i < 100; i ++)
	{
		m_strInfoFrame[i] = "ABC";
	}	
	
	m_iPicturePause = FALSE;

	memset(&m_userInfo,0,sizeof(m_userInfo));
	memset(&m_oldMouseRect,0,sizeof(RECT));
	memset(&m_downPoint,0,sizeof(CPoint));
	//m_bTalkIPC = FALSE;

	m_time_sync = 0;
	
	InitializeCriticalSection(&m_CriticalSection);      // 初始化临界区 
	m_capture_dib_buffer = NULL;
	m_capture_dib_len = 0;
	m_capture_dib_buffer_len = 0;

	m_fRectLeft = 0;//识别框与整体框架的比例
	m_fRectTop = 0;
	m_fRectRight = 0;
	m_fRectBottom = 0;
	memset(&m_scale_plate, 0, sizeof(m_scale_plate));
}


CVideoWnd::~CVideoWnd()
{
	Disconnct();

	EnterCriticalSection(&m_CriticalSection); //进入临界区，获得所有权，其他线程就等
	if (m_capture_dib_buffer)
	{
		delete[]m_capture_dib_buffer;
		m_capture_dib_buffer = NULL;
		m_capture_dib_len = 0;
	}
	LeaveCriticalSection(&m_CriticalSection);  //退出临界区

	DeleteCriticalSection(&m_CriticalSection);          // 删除  
}


/////////////////////////////////////////////////////////////////////////////
// CVideoWnd message handlers
int __stdcall RealVideoDataCallBack(long lRealHandle, 
							   long dwDataType, unsigned char *pBuffer,
												long lbufsize, void *pUser)
{
	CVideoWnd *pDataChnl = (CVideoWnd*)pUser;
	//把原始数据输入解码器解码
	H264_PLAY_InputData( pDataChnl->m_nPlaydecHandle ,pBuffer, lbufsize );
	pDataChnl->m_timeReceiveData = COleDateTime::GetCurrentTime();

	return 1;
}

//pFrame->nPacketType：数据类型

//pFrame->pPacketBuffer+8：p帧数据和音频数据

//pFrame->pPacketBuffer+16：I帧数据
FILE* pFile =NULL;


//FILE *pFile2 = fopen("yuan.H264", "wb+");
int __stdcall RealVideoDataCallBack_V2(long lRealHandle,const PACKET_INFO_EX *pFrame, long dwUser)
{
	CVideoWnd *pDataChnl = (CVideoWnd*)dwUser;
	//if (pFrame->nPacketType == VIDEO_I_FRAME)
	{
		BOOL ret=H264_PLAY_InputData( pDataChnl->m_nPlaydecHandle , (unsigned char*)pFrame->pPacketBuffer, pFrame->dwPacketSize);
	}
	
	//if (pFile2)
	//{
	//	fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, pFile2);
	//}

	//if (pFile1)
	//{

		/*else if (pFrame->nPacketType == VIDEO_P_FRAME)
		{
			fwrite(pFrame->pPacketBuffer+8, pFrame->dwPacketSize-8, 1, pFile1);
		}
		else
		{
			fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, pFile1);
		}*/
	//}
# if  0  //标准H264视频播放
 	STDH264_PACKET_INFO stdH264Info;
 	memset(&stdH264Info,0,sizeof(STDH264_PACKET_INFO)); 	
	
    if(pFrame->nPacketType == VIDEO_I_FRAME)
			{
				stdH264Info.pPacketBuffer=pFrame->pPacketBuffer+16;
				stdH264Info.dwPacketSize=pFrame->dwPacketSize-16;
				stdH264Info.dwFrameRate=pFrame->dwFrameRate;
				H264_PLAY_InputStdH264Data(pDataChnl->m_nPlaydecHandle,&stdH264Info);
 				
				if (pFile ==NULL)
				{
					pFile = fopen("c:\\teasta.h264","ab+");
					if (pFile)
					{
						fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
					}
				}else
				{
					if (pFile)
					{
						fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
					}
				}
			}
			else if(pFrame->nPacketType == VIDEO_P_FRAME)
			{
				stdH264Info.pPacketBuffer=pFrame->pPacketBuffer+8;
				stdH264Info.dwPacketSize=pFrame->dwPacketSize-8;
				stdH264Info.dwFrameRate=pFrame->dwFrameRate;
				
				H264_PLAY_InputStdH264Data(pDataChnl->m_nPlaydecHandle,&stdH264Info);
				if (pFile ==NULL)
				{
					pFile = fopen("c:\\teasta.h264","ab+");
					if (pFile)
					{
						fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
					}
				}else
				{
					if (pFile)
					{
						fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
					}
				}
			}
		//myFile.Close();
	
#endif

#if 0
	if (pFrame->nPacketType == AUDIO_PACKET)
	{
		FILE *fp = fopen("stream/AudioTotal.pcm", "ab+");
		if (fp)
		{
			fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
			fclose(fp);
		}
		char filename[256];
		static int index = 0;

		sprintf(filename, "stream/Audio%02d.idx", index++);
		fp = fopen(filename, "ab+");
		if (fp)
		{
			fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
			fclose(fp);
		}
	}
	else
	{
		char filename[256];
		static int index = 0;
		static int iFrame = 0;

		FILE *fp = NULL;
		if (pFrame->nPacketType == VIDEO_I_FRAME)
		{
			sprintf(filename, "stream/stream_%02d.idx", index++);
			fp = fopen(filename, "ab+");
			if (fp)
			{
				fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
				fclose(fp);
			}
			iFrame = 1;
		}
		
		if (iFrame == 1)
		{
			fp = fopen("stream/StreamTotal.h264", "ab+");
			if (fp)
			{
				fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
				fclose(fp);
			}
		}
	}
#endif
	return 1;
}



void CVideoWnd::Disconnct()
{
	//H264_DVR_DelRealDataCallBack(m_iPlayhandle, RealVideoDataCallBack, (long)this);
	RecordStop();
	StopRealPlay();

	//close decoder
	if (m_nPlaydecHandle >= 0)
	{
		memset( &m_oldMouseRect, 0, sizeof(m_oldMouseRect) );
		H264_PLAY_Stop(m_nPlaydecHandle);
		H264_PLAY_CloseStream(m_nPlaydecHandle);
		m_nPlaydecHandle = -1;
		m_lLogin = -1;
		
	}	
	m_userInfo.isConnect = 0;

}
// void CVideoWnd::OnTalkIPC()
// {
// 	int Errn = 0;
// 	if(m_bTalkIPC)
// 	{
// 		BOOL nRet = H264_DVR_StopDevTalk(m_lLogin,m_iChannel,3000);
// 		if (nRet)
// 		{
// 			m_bTalkIPC = FALSE;
// 		}
// 	}
// 	else
// 	{
// 		BOOL nRet = H264_DVR_StartDevTalk(m_lLogin,m_iChannel,3000);
// 		if(!nRet)
// 		{
// 			int Errn = H264_DVR_GetLastError();
// 			if (Errn)
// 			{
// 				CString str;
// 				str.Format("%d",Errn);
// 				AfxMessageBox(str);
// 			}
// 		}
// 		if (nRet)
// 		{
// 			m_bTalkIPC = TRUE;
// 		}
// 	}
// }
void CVideoWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{

#ifdef DISPLAYREGION
	m_downPoint=point;
#endif
	
}
void CALLBACK drawVideoOSDCall(LONG nPort,HDC hDc, void *pUser)
{
	CVideoWnd *pThis = (CVideoWnd*)pUser;
	pThis->drawOSD(nPort,hDc);
}

void CVideoWnd::drawOSD(LONG nPort,HDC hDc)
{
	int i = 0;
	int pos = 0;
	pos = 20;
	int space = 12;
		
	HBRUSH oldbrush;
	oldbrush = (HBRUSH)::GetCurrentObject( hDc, OBJ_BRUSH);
	HBRUSH brush;
	brush = CreateSolidBrush(RGB(255, 0, 0));
//	::SelectObject(hDc, brush);
	CRect videoWndRect;
	GetClientRect(m_userInfo.hWnd, videoWndRect);


		HFONT oldfont;
		oldfont = (HFONT)::GetCurrentObject( hDc, OBJ_FONT );
		

		//改变字体颜色
		::SetBkColor(hDc, RGB(193, 204, 211));
		::SetTextColor( hDc, RGB(187, 30, 70));
//		m_videoPlayer.SetOsdText(INDEX_VIDEO_LEFT, RGB(187, 30, 70), RGB(193, 204, 211), 10, pos += space, 0, chText);

		//更改字体
		LOGFONT lf;
		::GetObject( oldfont, sizeof(lf), &lf );
		wcscpy_s( lf.lfFaceName, 10, _T("Arial" ));
		lf.lfWeight = FW_BOLD;
		HFONT out_ft;		//创建的字体对象
		out_ft = ::CreateFontIndirect( &lf );

		space = CDC::FromHandle(hDc)->GetTextExtent(_T("A")).cy + 5;
		

		::SelectObject( hDc, out_ft );
		for (i = 0; i < 5; i++)
		{
			TextOut( hDc, 10, pos += space, m_strOsd[i], m_strOsd[i].GetLength());
		}

		TextOut( hDc, 10, videoWndRect.Height() - space, m_strAlarmMessage, m_strAlarmMessage.GetLength());//显示动态检测结果

		::SetBkColor(hDc, RGB(193, 204, 211));
		::SetTextColor(hDc, RGB(12, 190, 70));
		TextOut( hDc, 50, 5, m_strOsdTitle, m_strOsdTitle.GetLength());

		HPEN pen;
		HPEN penold;

		COLORREF oldpen_color;

		oldpen_color = SetDCPenColor(hDc, RGB(0,255,2));
		//COLORREF XorColor = GetBkColor(hDc) ^ GetDCPenColor(hDc);
		//SetDCPenColor(hDc, XorColor);
		SelectObject(hDc, GetStockObject(DC_PEN));

		SelectObject(hDc, GetStockObject(NULL_BRUSH));

	

		::Rectangle(hDc, (int)(m_fRectLeft*videoWndRect.Width()), (int)(m_fRectTop*videoWndRect.Height()),
			(int)(m_fRectRight*videoWndRect.Width()), (int)(m_fRectBottom*videoWndRect.Height()));


		::Rectangle(hDc, (int)(m_scale_plate.fLeft *videoWndRect.Width()), (int)(m_scale_plate.fTop*videoWndRect.Height()),
			(int)(m_scale_plate.fRight*videoWndRect.Width()), (int)(m_scale_plate.fBottom*videoWndRect.Height()));

		SetDCPenColor(hDc, oldpen_color);
	//	::Rectangle(hDc, m_rectDetect.left, m_rectDetect.top, m_rectDetect.right, m_rectDetect.bottom+2);

		::SelectObject( hDc, oldfont );
		::SelectObject( hDc, oldbrush);
		::DeleteObject(out_ft);
		::DeleteObject(brush);

	return;


	if ( m_strInfoFrame[nPort] != "" )
	{
		HFONT oldfont;
		oldfont = (HFONT)::GetCurrentObject( hDc, OBJ_FONT );
		

		//改变字体颜色
		::SetTextColor( hDc, RGB(255,0,0) );

		//更改字体
		LOGFONT lf;
		::GetObject( oldfont, sizeof(lf), &lf );
		wcscpy_s( lf.lfFaceName,10, _T("Arial" ));
		lf.lfWeight = FW_BOLD;
		HFONT out_ft;		//创建的字体对象
		out_ft = ::CreateFontIndirect( &lf );

		::SelectObject( hDc, out_ft );

		TextOut( hDc, 10, 10, _T("testtesttest"), strlen("testtesttest") );

		::SelectObject( hDc, oldfont );
		::DeleteObject(out_ft);
	}




}

void __stdcall CVideoWnd::videoInfoFramCallback(LONG nPort, LONG nType, LPCSTR pBuf,LONG nSize, void *pUser)
{
 	CVideoWnd *pThis = (CVideoWnd*)pUser;
 	//收到信息帧, 0x03 代表GPRS信息
 	if ( nType == 0x03 )
 	{
 		pThis->m_strInfoFrame[nPort] = pBuf;
 	}
}


//显示回调
//连续截图时时使用
//pBuf中存储的图像是YV12格式
void __stdcall DisplayCallBackFun(LONG nPort, LPCSTR pBuf, LONG nSize, LONG nWidth, LONG nHeight, LONG nStamp, LONG nType, LONG nUser)
{

	CVideoWnd *pThis = (CVideoWnd *)nUser;

	EnterCriticalSection(&pThis->m_CriticalSection); //进入临界区，获得所有权，其他线程就等
	
	if (nType == T_YV12)
	{

		if (pThis->m_capture_dib_buffer == NULL)
		{
			int dib_length = nWidth * nHeight * 3 + 54;//BMP图像的存储空间
			pThis->m_capture_dib_buffer_len = dib_length;
			pThis->m_capture_dib_buffer = new char[dib_length];
			memset(pThis->m_capture_dib_buffer, 0, dib_length);
		}

		BOOL ret = H264_PLAY_ConvertToBmpFile((PBYTE)pBuf, nSize, nWidth, nHeight, "", pThis->m_capture_dib_buffer);//把YV12模式转换成BMP位图
		//H264_PLAY_ConvertToBmpFile((PBYTE)pBuf, nSize, nWidth, nHeight, "d:\\h264test.bmp", NULL);//直接保存到图片
		if (ret)
		{
			BITMAPFILEHEADER *bmpHeader;
			bmpHeader = (BITMAPFILEHEADER *)pThis->m_capture_dib_buffer;

			//BITMAPINFO *bmpInfo;
			//bmpInfo = (BITMAPINFO *)(pThis->m_capture_dib_buffer + sizeof(BITMAPFILEHEADER));

			//如果宽度不能被4整除, 要重新计算
			//int width_byte = (nWidth * bmpInfo->bmiHeader.biBitCount + 31) / 8;
			//pThis->m_capture_dib_len = width_byte * nHeight;
			//pThis->m_capture_dib_len += sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

			//BMP文件中已经记录了文件长度
			pThis->m_capture_dib_len = bmpHeader->bfSize;

			//CFile file;
			//file.Open(_T("d:\\sss.bmp"), CFile::modeCreate | CFile::modeReadWrite);
			//file.Write(pThis->m_capture_dib_buffer, pThis->m_capture_dib_len);
			//file.Close();
		}
		else
		{
			memset(pThis->m_capture_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
			pThis->m_capture_dib_len = 0;
		}
	}

	LeaveCriticalSection(&pThis->m_CriticalSection);  //退出临界区

}

void __stdcall nProc(LONG nPort, LPCSTR pBuf,LONG nSize,FRAME_INFO * pFrameInfo, LONG nPlayedFrames)
{
	//char StrFileName[256] = {};
	//sprintf(StrFileName, "c:\\Picture1\\%d.bmp", ++i);
	//bool ret = H264_PLAY_ConvertToBmpFile((PBYTE)pBuf, nSize, pFrameInfo->nWidth, pFrameInfo->nHeight, StrFileName);
}

int CVideoWnd::VideoInit(CAMERA_USER_INFO *pUserd)
{

	memcpy(&m_userInfo, pUserd, sizeof(CAMERA_USER_INFO));
	m_iChannel = m_userInfo.nChannel;

	return 1;
}

//注意: 此函数必须在openstream后，play之前设置，否则无效
int CVideoWnd::SetOsdText(DWORD colorText, DWORD colorBack, int pos_x, int pox_y, int transparent, char *pText)
{
	if (m_iPlayhandle <= 0)
	{
		return 0;
	}

	OSD_INFO_TXT osd;
	osd.color = colorText;
	osd.bkColor = colorBack;
	osd.pos_x = pos_x;
	osd.pos_y = pox_y;
	osd.isTransparent = transparent;
	strcpy_s(osd.text, sizeof(osd.text), pText);
	H264_PLAY_SetOsdTex(m_nIndex, &osd);
	return 1;
}

int CVideoWnd::ConnectRealPlay(int nIndex, bool bOsd)
{
	DEV_INFO *pDev = NULL;
	int nChannel = m_iChannel;
	int login = 0;
	//	login = m_rtsp[0].CameraLogin(strIp, strPort, strUsername, strPassword, m_check_cloud_id.GetCheck());
	m_nIndex = nIndex;
	pDev = &m_devInfo;
	if (pDev->lLoginID <= 0)
	{
		login = CameraLogin(CString(m_userInfo.szIp), m_userInfo.nPort, CString(m_userInfo.szUserName), CString(m_userInfo.szPsw), m_userInfo.bCloud);
		if (login <= 0)
		{
			return 0;
		}
	}


	StopRealPlay();

	if ( m_nPlaydecHandle == -1 )
	{
		//open decoder
		BYTE byFileHeadBuf;
		if (H264_PLAY_OpenStream(m_nIndex, &byFileHeadBuf, 1, SOURCE_BUF_MIN * 50))
		{	
			//设置信息帧回调
			H264_PLAY_SetInfoFrameCallBack(m_nIndex, videoInfoFramCallback, this);

			//叠加osd信息
 			if ( bOsd )
			{
				OSD_INFO_TXT osd;
				osd.bkColor = RGB(0,255,0);
				osd.color = RGB(255,0,0);
				osd.pos_x = 10;
				osd.pos_y = 10;
				osd.isTransparent = 0;
				strcpy_s(osd.text, 100, "osd1 info"); 
		//		H264_PLAY_SetOsdTex(m_nIndex, &osd);


				OSD_INFO_TXT osd2;
				osd2.bkColor = RGB(255,0,0);
				osd2.color = RGB(0,255,0);
				osd2.pos_x = 10;
				osd2.pos_y = 40;
				osd2.isTransparent = 0;
				osd2.isBold = 1;
				strcpy_s(osd2.text, 100, "osd2 info"); 
		//		H264_PLAY_SetOsdTex(m_nIndex, &osd2);


				//设置osd叠加回调
				H264_PLAY_RigisterDrawFun(m_nIndex, drawVideoOSDCall, this);
				//OSd信息设置， 单通道
				SDK_OSDInfo Osd;
				Osd.index = 1;
				Osd.nChannel = nChannel;
				Osd.nX = 0;
				Osd.nY = 0;
				strcpy_s(Osd.pOSDStr, 100, "");
			//	long lret = H264_DVR_SetDevConfig(pDev->lLoginID, E_SDK_SET_OSDINFO, 0, (char *)&Osd, sizeof(SDK_OSDInfo));
			}
			else
			{
				//设置osd叠加回调
				H264_PLAY_RigisterDrawFun(m_nIndex, drawVideoOSDCall, this);
				//OSd信息设置， 单通道
				SDK_OSDInfo Osd;
				Osd.index = 1;
				Osd.nChannel = nChannel;
				Osd.nX = 0;
				Osd.nY = 0;
				strcpy_s(Osd.pOSDStr, 100, "");
				long lret = H264_DVR_SetDevConfig(pDev->lLoginID, E_SDK_SET_OSDINFO, 0, (char *)&Osd, sizeof(SDK_OSDInfo));

			}

			H264_PLAY_SetStreamOpenMode(m_nIndex, STREAME_REALTIME);
			//只播放I帧,可降低cpu使用率
			//H264_PLAY_OnlyIFrame(m_nIndex, true);
		//	H264_PLAY_SetDisplayCallBack(m_nIndex, DisplayCallBackFun, (LONG)this);
			H264_PLAY_SetDisplayCallBack(m_nIndex, NULL, NULL);
			//H264_PLAY_SetDecCallBack(m_nIndex, nProc);//设置解码回调, 设置后在回调中自己解码
			if ( H264_PLAY_Play(m_nIndex, this->m_userInfo.hWnd) )
			{
				m_nPlaydecHandle = m_nIndex;
			}
		}	
	}
	H264_DVR_CLIENTINFO playstru;

	playstru.nChannel = nChannel;
	playstru.nStream = 0;//0主码，1子码
	playstru.nMode = 0;
	m_iPlayhandle = H264_DVR_RealPlay(pDev->lLoginID, &playstru);	
	if(m_iPlayhandle <= 0 )
	{
		DWORD dwErr = H264_DVR_GetLastError();
		CString sTemp;
		sTemp.Format(_T("access %s channel%d fail, dwErr = %d"),pDev->szDevName,nChannel, dwErr);
	//	MessageBox(sTemp);
	}
	else
	{
		//set callback to decode receiving data

		//设置原始数据回调, 拿到数据后输入到解码器解码
		H264_DVR_SetRealDataCallBack(m_iPlayhandle, RealVideoDataCallBack, this);
	
		//H264_DVR_MakeKeyFrame(pDev->lLoginID, nChannel, 0);
		//H264_DVR_SetRealDataCallBack_V2(m_iPlayhandle, RealVideoDataCallBack_V2, (long)this);
	}
	m_lLogin = pDev->lLoginID;
	m_iChannel = nChannel;
	m_userInfo.isConnect = 1;


	return m_iPlayhandle;
}


void CVideoWnd::OnDestroy() 
{

	StopRealPlay();

	m_bRecord = FALSE;
}

void CVideoWnd::GetColor(long *nBright, long *nContrast, long *nSaturation, long *nHue)
{
	if(m_iPlayhandle <= 0 )
	{
		return;
	}
	long nRegionNum = 0;
	H264_PLAY_GetColor(m_nPlaydecHandle, nRegionNum, nBright, nContrast, nSaturation, nHue);
}

void CVideoWnd::SetColor(long nBright, long nContrast, long nSaturation, long nHue)
{
	H264_PLAY_SetColor(m_nPlaydecHandle, 0, nBright, nContrast, nSaturation, nHue);
}

BOOL CVideoWnd::SaveRecord()
{
	//获取当前窗口播放句柄
	if ( m_iPlayhandle <= 0 )
	{
		return FALSE;
	}
	
	CString  cFilename;
	
	CTime time = CTime::GetCurrentTime();
	cFilename.Format(_T("%s\\record\\%4d%02d%02d_%02d%02d%02d.h264"), 
		_T("c:"), 
		time.GetYear(), 
		time.GetMonth(), 
		time.GetDay(), 
		time.GetHour(), 
		time.GetMinute(), 
		time.GetSecond());
	

	if (-1 == _access(("c:\\record"), 0))
	{
		CreateDirectory(_T("c:\\record"), NULL);
	}
	if ( m_bRecord )
	{
		if ( H264_PLAY_StopDataRecord(m_nPlaydecHandle) )
		{
			m_bRecord = FALSE;
			//MessageBox(_T("Desktop.StopRecordOk"));
		}
	}
	else
	{
		CString strFileType(_T(".h264"));
		CString strFileName("");
		if (cFilename.Right(5) == _T(".h264"))
		{
			strFileType = _T("h264");
		} 
		else
		{
			//MessageBox("Type Error!");
			return FALSE;
		}

		int iNum = cFilename.ReverseFind('\\');
		if( -1 == iNum)
		{
			return FALSE;
		}
		else
		{
			strFileName = cFilename.Right(cFilename.GetLength()-(iNum +1) );
		}
		
		
		CFileDialog dlg(FALSE, _T(".csv"), strFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("All File(*.h264;)|*.*||"));//"*.dat|*.dat|All Files|*.*|"

		dlg.m_ofn.lpstrInitialDir = cFilename.Left(iNum);

		CString strSaveName;
		if (dlg.DoModal() == IDOK)
		{
			cFilename = dlg.GetPathName();
			int nTemp = 0;
			for(;;)
			{
				int nIndex = cFilename.Find('\\',nTemp);
				if (nIndex == -1) 
				{
					break;
				}
				CString str = cFilename.Left(nIndex);
				nTemp = nIndex+1;
				//_mkdir(str);
				CreateDirectory(str, NULL);
			}

			char chfilename[256] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, cFilename, -1, chfilename, sizeof(chfilename)-1, NULL, NULL);

			
			if (H264_PLAY_StartDataRecord(m_nPlaydecHandle, chfilename, MEDIA_FILE_NONE))
			{
				m_bRecord = TRUE;
			//	MessageBox(_CS("Desktop.StartRecordOk"));
			}
			else
			{
			//	MessageBox(_CS("Desktop.StartRecordFail"));
			}
		}
	}

	return TRUE;
}


void yv12toYUV(unsigned   char *outYuv, unsigned   char *inYv12, int width, int height, int widthStep)
{
	int col, row;
	unsigned int Y, U, V;
	int tmp;
	int idx;
	for (row = 0; row<height; row++)
	{
		idx = row * widthStep;
		int rowptr = row*width;
		for (col = 0; col<width; col++)
		{
			tmp = (row / 2)*(width / 2) + (col / 2);
			Y = (unsigned int)inYv12[row*width + col];
			U = (unsigned int)inYv12[width*height + width*height / 4 + tmp];
			V = (unsigned int)inYv12[width*height + tmp];

			outYuv[idx + col * 3] = Y;
			outYuv[idx + col * 3 + 1] = U;
			outYuv[idx + col * 3 + 2] = V;
		}
	}
}

void Yuv2Rgb(unsigned   char   *src, unsigned   char   *dest, int   cx, int   cy)
{
	/*	int   i,   j;
	unsigned   char *bufY,   *bufU,   *bufV,   *bufRGB;

	bufY   =   src;
	bufU   =   src   +   cx   *   cy;
	bufV   =   src   +   int(cx   *   cy*   5/4);

	unsigned   char   y,   u,   v,   r,   g,   b;

	for(   j   =   0;   j   <   cy;   j++   )
	{
	bufRGB   =   dest   +   cx   *   (cy   -   1   -   j)   *   3   ;
	for(   i   =   0;   i   <   cx;   i++   )
	{
	y   =   *(bufY   +   i   +   j   *   cx);

	u   =   *(bufU   +   (i >>1)   +(   j >>1)*cx/2   )   -   128;
	v   =   *(bufV   +   (i >>1)   +   (j >>1)*cx/2   )   -   128;

	r   =   y   +   1.402   *   v;
	g   =   y   -   0.344   *   u   -   0.714   *   v;
	b   =   y   +   1.717   *   u;


	*(   bufRGB++   )   =   max(   0,   min(   r,   255   )   );
	*(   bufRGB++   )   =   max(   0,   min(   g,   255   )   );
	*(   bufRGB++   )   =   max(   0,   min(   b,   255   )   );
	}
	} */

	int   i, j;
	unsigned   char *bufY, *bufU, *bufV, *bufRGB;

	bufY = src;
	bufU = src + cx   *   cy;
	bufV = src + int(cx   *   cy * 5 / 4);

	unsigned   char   y, u, v, r, g, b;

	for (j = 0; j < cy; j++)
	{
		bufRGB = dest + cx   *   (cy - 1 - j) * 3;
		for (i = 0; i < cx; i++)
		{
			y = *(bufY + i + j   *   cx);

			u = *(bufU + (i >> 1) + (j >> 1)*cx / 2) - 128;
			v = *(bufV + (i >> 1) + (j >> 1)*cx / 2) - 128;

			/*r   =   y   +   1.402   *   v;
			g   =   y   -   0.344   *   u   -   0.714   *   v;
			b   =   y   +   1.717   *   u; */
			r = y + unsigned char(1.375 * v);
			g = y - unsigned char(0.34375 * u) - unsigned char(0.703125  * v);
			b = y + unsigned char(1.734375 * u);


			*(bufRGB++) = max(0, min(r, 255));
			*(bufRGB++) = max(0, min(g, 255));
			*(bufRGB++) = max(0, min(b, 255));
		}
	}
}




int CVideoWnd::CaptureToBuffer()
{

	//从显示回调中抓图
	//int buf_len = 0;
	//if (buf_size > m_capture_dib_len && m_capture_dib_len > 0)
	//{
	//	EnterCriticalSection(&m_CriticalSection); //进入临界区，获得所有权，其他线程就等
	////	yv12toYUV();
	////	Yuv2Rgb(pYUVBuffer, pRGBBuffer + sizeof(BITMAPINFOHEADER), nWidth, nHeight);
	//	memcpy(pBuf, m_capture_dib_buffer, m_capture_dib_len);
	//	buf_len = m_capture_dib_len;
	//	LeaveCriticalSection(&m_CriticalSection);  //退出临界区
	//}
	//return buf_len;


	int res = 0;
	DEV_INFO *pDev = &m_devInfo;
	long lRetLen = 0;
//	if (pDev && pDev->lLoginID > 0 && m_iPlayhandle > 0)
//	if (m_nPlaydecHandle > 0)
	if (m_nPlaydecHandle != -1)
	{
		LONG width = 0;
		LONG height = 0;

		if (m_capture_dib_buffer == NULL && m_capture_dib_buffer_len == 0)
		{
			char c = 0;
			m_capture_dib_buffer = &c;
			lRetLen = H264_PLAY_CatchPicBuf(m_nPlaydecHandle, m_capture_dib_buffer, 0, &width, &height, 0);//抓取BMP图像到内存
			if (width > 0)
			{
				int dib_length = width * height * 3 + 54;//BMP图像的存储空间
				m_capture_dib_buffer = new char[dib_length];

				m_capture_dib_buffer_len = dib_length;
			}
			else
			{
				m_capture_dib_buffer = NULL;
			}
		}

		if (m_capture_dib_buffer && m_capture_dib_buffer_len > 0)
		{

			//H264_PLAY_CatchPic(m_nPlaydecHandle, filename, 0))//直接抓取到文件 ntype:0-bmp, 1-jpg
			lRetLen = H264_PLAY_CatchPicBuf(m_nPlaydecHandle, m_capture_dib_buffer, m_capture_dib_buffer_len, &width, &height, 0);//抓取BMP图像到内存
			if (lRetLen > 0)//返回存储大小
			{
				m_capture_dib_len = lRetLen;
			}
			else
			{
				m_capture_dib_len = 0;
				int Errn = H264_DVR_GetLastError();
				if (Errn)
				{
				}
			}
		}
	//	H264_PLAY_ResetSourceBuffer(m_nPlaydecHandle);

	//	H264_PLAY_ResetBuffer(m_nPlaydecHandle, BUF_VIDEO_RENDER);//BUF_VIDEO_SRC
	}

	return lRetLen;
}

int CVideoWnd::CaptureToFile(CString strFileName, int iQuality)
{
	int res = 0;
	DEV_INFO *pDev = &m_devInfo;

	if (pDev && pDev->lLoginID > 0 && m_iPlayhandle > 0)
	{

		//注意转换成utf8, 方便之后在保存图像
		int size = 0;
		size = WideCharToMultiByte(CP_UTF8, 0, strFileName, -1, 0, 0, NULL, NULL);  //计算转换需要的长度,包括\0
		char *filename = new char[size + 1];	//字串存储

		size = WideCharToMultiByte(CP_UTF8, 0, strFileName, -1, filename, size, NULL, NULL);  //计算转换需要的长度,包括\0
		filename[size] = 0;

#if 1
		char* dib_buffer = filename;
		LONG width = 0;
		LONG height = 0;
		//H264_PLAY_CatchPic(m_nPlaydecHandle, filename, 0))//直接抓取到文件 ntype:0-bmp, 1-jpg
		long lRetLen = H264_PLAY_CatchPicBuf(m_nPlaydecHandle, dib_buffer, 0, &width, &height, 0);//抓取BMP图像到内存

		int dib_length = width * height * 3 + 54;//BMP图像的存储空间
		dib_buffer = new char[dib_length];


		 lRetLen = H264_PLAY_CatchPicBuf(m_nPlaydecHandle, dib_buffer, dib_length, &width, &height, 0);//抓取BMP图像到内存
		if (lRetLen > 0)//返回存储大小
		{
			image im_src = load_image_from_memory(dib_buffer, lRetLen);
			//		ShowPicture(m_pic_source, im_src);
			CreateFolder(strFileName);//检查目录是否存在
			res = save_image_ex(im_src, filename, iQuality);
			free_image(im_src);
		}
		else
		{
			int Errn = H264_DVR_GetLastError();
			if (Errn)
			{

			}
		}
#else
		//从DVR设备抓图, 需要DVR支持此功能, 可以在设备中设置抓图的参数
		int nRetLen;
		unsigned long lRetLen;
		SDK_SystemFunction* pSysFunc = new SDK_SystemFunction;
		H264_DVR_GetDevConfig(pDev->lLoginID, E_SDK_CONFIG_ABILITY_SYSFUNC, 0, (char*)pSysFunc, sizeof(SDK_SystemFunction),
			&lRetLen, 5000);
		if (!pSysFunc->vEncodeFunction[SDK_ENCODE_FUNCTION_TYPE_SNAP_STREAM])
		{
			return 0;
		}
		bool bRet = H264_DVR_CatchPicInBuffer(pDev->lLoginID, 0, dib_buffer, dib_length, &nRetLen);
		if (bRet)
		{
			CFile file;
			file.Open(strFileName, CFile::modeCreate | CFile::modeReadWrite);
			file.Write(dib_buffer, nRetLen);
			file.Close();
		}

		delete pSysFunc;
		pSysFunc = NULL;
#endif


		delete[] dib_buffer;	//释放内存
		dib_buffer = NULL;
		delete[]filename;
		filename = NULL;
	}

	return res;
}

BOOL CVideoWnd::IsRecording()
{
	return m_bRecord;
}

BOOL CVideoWnd::RecordStop()
{

	//获取当前窗口播放句柄
	if (m_iPlayhandle <= 0)
	{
		return FALSE;
	}

	if (m_bRecord)
	{
		if (H264_PLAY_StopDataRecord(m_nPlaydecHandle))
		{
			m_bRecord = FALSE;
			//MessageBox(_T("Desktop.StopRecordOk"));
		}
	}
	return TRUE;
}

int CVideoWnd::RecordToFile(CString strFileName)
{
	//获取当前窗口播放句柄
	if (m_iPlayhandle <= 0)
	{
		return FALSE;
	}

	if (!m_bRecord)
	{
		CreateFolder(strFileName);//检查目录是否存在

		char chfilename[256] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strFileName, -1, chfilename, sizeof(chfilename) - 1, NULL, NULL);

		//MEDIA_FILE_NONE/MEDIA_FILE_AVI可用,MEDIA_FILE_NONE本SDK可播放, AVI也是H264编码,外部播放兼容性好,大小两种差不多
		if (H264_PLAY_StartDataRecord(m_nPlaydecHandle, chfilename, MEDIA_FILE_NONE))
		{
			m_bRecord = TRUE;
			//	MessageBox(_CS("Desktop.StartRecordOk"));
		}
		else
		{
			//	MessageBox(_CS("Desktop.StartRecordFail"));
		}
	}


	return TRUE;
}

void CVideoWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
#ifdef DISPLAYREGION
	memset( &m_oldMouseRect, 0, sizeof(m_oldMouseRect) );
	H264_PLAY_SetDisplayRegion(m_nPlaydecHandle, 0, NULL, NULL, false);
#endif
	
	if ( m_iPlayhandle > 0 )
	{
	}

}

long CVideoWnd::GetLoginHandle()
{
	return m_lLogin;
}

long CVideoWnd::SetDevChnColor(SDK_CONFIG_VIDEOCOLOR* pVideoColor)
{
	
	 long ret=H264_DVR_SetDevConfig(m_lLogin,E_SDK_VIDEOCOLOR,m_iChannel,(char*)pVideoColor,sizeof(SDK_CONFIG_VIDEOCOLOR),5000);
	 return ret;
}

long CVideoWnd::GetDevChnColor(SDK_CONFIG_VIDEOCOLOR* pVideoColor)
{
	//for(int i = 0;i < 100; i++)
	//{
		DWORD dwReturn;
		long ret=H264_DVR_GetDevConfig(m_lLogin,E_SDK_VIDEOCOLOR,m_iChannel,(char*)pVideoColor,sizeof(SDK_CONFIG_VIDEOCOLOR),&dwReturn,3000);
	//}
	return ret;
	//return 1;
}

void CVideoWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
#ifdef DISPLAYREGION

	long nLeft,nTop,nRight,nBottom;
	nLeft=m_downPoint.x;
	nTop=m_downPoint.y;
	nRight=point.x;
	nBottom=point.y;
	long nChannel=m_iChannel;
//*******************************
	//选择范围太小
	if ( (abs(nBottom - nTop) < 15 || abs(nRight - nLeft) < 15) )
	{
		return;
	}
	
	
	RECT mouseRect; //鼠标拉动的矩形框
	// 获取当前视窗位置.
	//..................	
	
	RECT videoWndRect;
	GetClientRect(m_userInfo.hWnd, &videoWndRect);
	
	int width  = videoWndRect.right - videoWndRect.left;
	int height = videoWndRect.bottom - videoWndRect.top; 
	
	//选取坐标
	mouseRect.left = nLeft;
	mouseRect.top = nTop;
	mouseRect.right = nRight;
	mouseRect.bottom = nBottom;

		long nPicWidth = 0;
		long nPicHeight = 0;
		
		if ( mouseRect.left > mouseRect.right )
		{
			mouseRect.left = nRight;
			mouseRect.right = nLeft;
		}
		if ( mouseRect.top > mouseRect.bottom )
		{
			mouseRect.top = nBottom;
			mouseRect.bottom = nTop;
		}
		nPicWidth = 8192;
		nPicHeight = 8192;
		//H264_PLAY_GetPictureSize(m_nPlaydecHandle, &nPicWidth, &nPicHeight);
		
		int oldWidth = m_oldMouseRect.right -m_oldMouseRect.left;
		int oldHeight = m_oldMouseRect.bottom - m_oldMouseRect.top;
		if( oldHeight != 0 && oldWidth != 0 )
		{
			int newWidth = mouseRect.right - mouseRect.left;
			int newHeight = mouseRect.bottom - mouseRect.top;
			
			mouseRect.left = m_oldMouseRect.left + mouseRect.left * oldWidth / width;
			mouseRect.right = mouseRect.left + oldWidth * newWidth / width;
			mouseRect.top = m_oldMouseRect.top + mouseRect.top * oldHeight / height;
			mouseRect.bottom = mouseRect.top + oldHeight * newHeight / height;
		}
		else
		{
			
			mouseRect.left = mouseRect.left * nPicWidth / width;
			mouseRect.right = mouseRect.right * nPicWidth / width;
			
			mouseRect.top = mouseRect.top * nPicHeight / height;
			mouseRect.bottom = mouseRect.bottom * nPicHeight / height;
		}
		
		
		memcpy( &m_oldMouseRect, &mouseRect, sizeof(mouseRect) );
		
		TRACE("left = %d top = %d right = %d bottpm = %d \n", mouseRect.left, mouseRect.top, mouseRect.right, mouseRect.bottom);
		BOOL ret = H264_PLAY_SetDisplayRegion(m_nPlaydecHandle, 0, &mouseRect, NULL, TRUE);

	#endif

}



//检查用户名密码等是否可以登录成功
//bCloud 1-云ip登录
int CVideoWnd::CameraLogin(CString strIP, int nPort, CString strUserName, CString strPassword, int bCloud)
{
	H264_DVR_DEVICEINFO OutDev;
	memset(&OutDev, 0, sizeof(OutDev));

	int nError = H264_DVR_ILLEGAL_PARAM;
	char chIp[20] = { 0 };
	char chUserName[33] = { 0 };
	char chPassword[33] = { 0 };

	if (strIP.IsEmpty()) return nError;


	WideCharToMultiByte(CP_ACP, 0, strIP, -1, chIp, sizeof(chIp)-1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strUserName, -1, chUserName, sizeof(chUserName)-1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strPassword, -1, chPassword, sizeof(chPassword)-1, NULL, NULL);



	//设置尝试连接设备次数和等待时间
	H264_DVR_SetConnectTime(3000, 1);//设置尝试连接1次，等待时间3s
	long lLogin = 0;
	if (bCloud)
	{
		lLogin = H264_DVR_Login_Cloud(chIp, nPort, chUserName,
			chPassword, &OutDev, &nError, NULL);
	}
	else
	{
		lLogin = H264_DVR_Login(chIp, nPort, chUserName,
			chPassword, &OutDev, &nError);
	}
	if (lLogin <= 0)
	{
		CString strErr;
		switch (nError)
		{
		case H264_DVR_PASSWORD_NOT_VALID:
			strErr = _T("Error.PwdErr");
			break;
		case H264_DVR_NOPOWER:
			strErr = _T("Error.NoPower");
			break;
		case H264_DVR_LOGIN_USER_NOEXIST:
			strErr = _T("Error.UserNotExisted");
			break;
		case H264_DVR_USER_LOCKED:
			strErr = _T("Login.Locked");
			break;
		case H264_DVR_USER_IN_BLACKLIST:
			strErr = _T("Error.InBlackList");
			break;
		case H264_DVR_USER_HAS_USED:
			strErr = _T("Error.HasLogined");
			break;
		case H264_DVR_CONNECT_DEVICE_ERROR:
			strErr = _T("Error.NotFound");
			break;
		default:
		{

				   strErr = _T("Log.Error");
		}

			break;
		}
		//AFXMessageBox(strErr);
		return nError;
	}
	m_devInfo.nTotalChannel = OutDev.byChanNum + OutDev.iDigChannel;
	m_devInfo.lLoginID = lLogin;
	m_devInfo.nPort = nPort;
	//strcpy(m_devInfo.szDevName, m_strDevName);
	_snprintf_s(m_devInfo.szUserName, sizeof(m_devInfo.szUserName), "%s", chUserName);
	_snprintf_s(m_devInfo.szPsw, sizeof(m_devInfo.szPsw), "%s", chPassword);
	if (bCloud)
	{
		m_devInfo.szIpaddress[0] = 0;
		_snprintf_s(m_devInfo.szCloudID, sizeof(m_devInfo.szCloudID), "%s", chIp);
	}
	else
	{
		_snprintf_s(m_devInfo.szIpaddress, sizeof(m_devInfo.szIpaddress), "%s", chIp);
		_snprintf_s(m_devInfo.szCloudID, sizeof(m_devInfo.szCloudID), "%s", OutDev.sSerialNumber);

	}

	//if (m_bSerialID)
	//{
	//	m_devInfo.bSerialID = m_bSerialID;
	//	m_devInfo.nSerPort = m_HostPort;
	//	strcpy(m_devInfo.szSerIP, m_strHostIP.GetBuffer(0));
	//	strcpy(m_devInfo.szSerialInfo, m_DevSerialID.GetBuffer(0));
	//}

	m_devInfo.bCloud = 0;
	if (bCloud != 0)
	{
		m_devInfo.bCloud = 1;
	}


	H264_DVR_SetupAlarmChan(lLogin);//注册登录id到报警消息, 产生警报时发到相应的登录设备上


	if (lLogin > 0)
	{
		return lLogin;
	}

	if (nError <= 0) return nError;

	return H264_DVR_SDK_UNKNOWNERROR;
}


long CVideoWnd::DevLogin()
{
	DEV_INFO* pdev = &m_devInfo;
	if (pdev->bSerialID)//如果之前是DDNS获取ip;这里先获取动态ip
	{
		int maxDeviceNum = 100;  //最大支持设备数量100
		DDNS_INFO *pDDNSInfo = new DDNS_INFO[maxDeviceNum];
		SearchMode searchmode;
		int nReNum = 0;  //实际获得的设备数量		
		searchmode.nType = DDNS_SERIAL;
		strcpy_s(searchmode.szSerIP, sizeof(searchmode.szSerIP), pdev->szSerIP);
		searchmode.nSerPort = pdev->nSerPort;
		strcpy_s(searchmode.szSerialInfo, sizeof(searchmode.szSerialInfo), pdev->szSerialInfo);
		CString strTemp;
		BOOL bret = H264_DVR_GetDDNSInfo(searchmode, pDDNSInfo, maxDeviceNum, nReNum);
		if (!bret)
		{
			delete pDDNSInfo;
			pDDNSInfo = NULL;
			return FALSE;
		}
		memcpy(pdev->szIpaddress, pDDNSInfo[0].IP, 15);
		pdev->nPort = pDDNSInfo[0].MediaPort;
	}

	H264_DVR_DEVICEINFO OutDev;
	memset(&OutDev, 0, sizeof(OutDev));
	int nError = 0;
	//设置尝试连接设备次数和等待时间
	H264_DVR_SetConnectTime(3000, 1);//设置尝试连接1次，等待时间3s
	long lLogin = 0;
	if (pdev->bCloud != 0)
	{
		lLogin = H264_DVR_Login_Cloud(pdev->szCloudID, pdev->nPort, pdev->szUserName,
			pdev->szPsw, &OutDev, &nError, NULL);
	}
	else
	{
		lLogin = H264_DVR_Login(pdev->szIpaddress, pdev->nPort, pdev->szUserName,
			pdev->szPsw, &OutDev, &nError);
	}
	if (lLogin <= 0)
	{
		int nErr = H264_DVR_GetLastError();
		if (nErr == H264_DVR_PASSWORD_NOT_VALID)
		{
			//MessageBox(_CS("Error.PwdErr"));
		}
		else
		{
			//MessageBox(_CS("Error.NotFound"));

		}
		return lLogin;
	}


	H264_DVR_SetupAlarmChan(lLogin);

	m_devInfo.lLoginID = lLogin;

	return lLogin;
}





//FILE *pFile2 = fopen("yuan.H264", "wb+");
int __stdcall RealDataCallBack_V2(long lRealHandle, const PACKET_INFO_EX *pFrame, void *pUser)
{
	CVideoWnd *pDataChnl = (CVideoWnd*)pUser;
	//if (pFrame->nPacketType == VIDEO_I_FRAME)
	{
		BOOL ret = H264_PLAY_InputData(pDataChnl->m_nPlaydecHandle, (unsigned char*)pFrame->pPacketBuffer, pFrame->dwPacketSize);
	}

	//if (pFile2)
	//{
	//	fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, pFile2);
	//}

	//if (pFile1)
	//{

	/*else if (pFrame->nPacketType == VIDEO_P_FRAME)
	{
	fwrite(pFrame->pPacketBuffer+8, pFrame->dwPacketSize-8, 1, pFile1);
	}
	else
	{
	fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, pFile1);
	}*/
	//}
# if  0  //标准H264视频播放
	STDH264_PACKET_INFO stdH264Info;
	memset(&stdH264Info, 0, sizeof(STDH264_PACKET_INFO));

	if (pFrame->nPacketType == VIDEO_I_FRAME)
	{
		stdH264Info.pPacketBuffer = pFrame->pPacketBuffer + 16;
		stdH264Info.dwPacketSize = pFrame->dwPacketSize - 16;
		stdH264Info.dwFrameRate = pFrame->dwFrameRate;
		H264_PLAY_InputStdH264Data(pDataChnl->m_nPlaydecHandle, &stdH264Info);

		if (pFile == NULL)
		{
			pFile = fopen("c:\\teasta.h264", "ab+");
			if (pFile)
			{
				fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
			}
		}
		else
		{
			if (pFile)
			{
				fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
			}
		}
	}
	else if (pFrame->nPacketType == VIDEO_P_FRAME)
	{
		stdH264Info.pPacketBuffer = pFrame->pPacketBuffer + 8;
		stdH264Info.dwPacketSize = pFrame->dwPacketSize - 8;
		stdH264Info.dwFrameRate = pFrame->dwFrameRate;

		H264_PLAY_InputStdH264Data(pDataChnl->m_nPlaydecHandle, &stdH264Info);
		if (pFile == NULL)
		{
			pFile = fopen("c:\\teasta.h264", "ab+");
			if (pFile)
			{
				fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
			}
		}
		else
		{
			if (pFile)
			{
				fwrite(stdH264Info.pPacketBuffer, 1, stdH264Info.dwPacketSize, pFile);
			}
		}
	}
	//myFile.Close();

#endif

#if 0
	if (pFrame->nPacketType == AUDIO_PACKET)
	{
		FILE *fp = fopen("stream/AudioTotal.pcm", "ab+");
		if (fp)
		{
			fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
			fclose(fp);
		}
		char filename[256];
		static int index = 0;

		sprintf(filename, "stream/Audio%02d.idx", index++);
		fp = fopen(filename, "ab+");
		if (fp)
		{
			fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
			fclose(fp);
		}
	}
	else
	{
		char filename[256];
		static int index = 0;
		static int iFrame = 0;

		FILE *fp = NULL;
		if (pFrame->nPacketType == VIDEO_I_FRAME)
		{
			sprintf(filename, "stream/stream_%02d.idx", index++);
			fp = fopen(filename, "ab+");
			if (fp)
			{
				fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
				fclose(fp);
			}
			iFrame = 1;
		}

		if (iFrame == 1)
		{
			fp = fopen("stream/StreamTotal.h264", "ab+");
			if (fp)
			{
				fwrite(pFrame->pPacketBuffer, pFrame->dwPacketSize, 1, fp);
				fclose(fp);
			}
		}
	}
#endif
	return 1;
}


int CVideoWnd::IsPicturePause()
{
	return m_iPicturePause;
}


int CVideoWnd::PicturePause(int pause)
{
	if (m_iPlayhandle <= 0)
	{
		m_iPicturePause = 0;
		return m_iPicturePause;
	}

	if (m_iPicturePause == pause)
	{
		return m_iPicturePause;
	}


	if (pause < 0)
	{
		m_iPicturePause = !m_iPicturePause;
	}
	else
	{
		m_iPicturePause = pause;
	}
		
//	H264_PLAY_Pause(m_nPlaydecHandle, m_iPicturePause);

	if (m_iPicturePause)
	{
		H264_PLAY_Stop(m_nPlaydecHandle);
	}
	else
	{
		H264_PLAY_Play(m_nPlaydecHandle, m_userInfo.hWnd);
	}
	CRect rect;

	GetClientRect(m_userInfo.hWnd, &rect);
	InvalidateRect(m_userInfo.hWnd, &rect, TRUE);

	SendMessage(m_userInfo.hWnd, WM_PAINT, 0, 0);

//	Invalidate(m_userInfo.hWnd);
	//	H264_PLAY_Pause(m_nPlaydecHandle, m_iPicturePause);


	return m_iPicturePause;
}



void CVideoWnd::StopRealPlay()
{

	if (m_iPlayhandle != -1)
	{
		//H264_DVR_DelRealDataCallBack(m_iPlayhandle, RealDataCallBack, (long)this);
		H264_DVR_DelRealDataCallBack_V2(m_iPlayhandle, RealDataCallBack_V2, this);
		if (!H264_DVR_StopRealPlay(m_iPlayhandle))
		{
			TRACE("H264_DVR_StopRealPlay fail m_iPlayhandle = %d", m_iPlayhandle);
		}
		m_iPlayhandle = -1;
	}

}

void CVideoWnd::TimeSync()
{
	m_time_sync = 1;
}


void __stdcall CVideoWnd::SDKPlayFileEndCallback(LONG nPort, void *pUser)
{
	HWND hwnd = (HWND)pUser;
	SendMessage(hwnd, WM_VIDEO_PLAY_MESSAGE, nPort, NULL);
}

int CVideoWnd::PlayLocalFile(int nIndex, CString strFile, HWND hwndVideo, HWND hwndMsg)
{
	//	login = m_rtsp[0].CameraLogin(strIp, strPort, strUsername, strPassword, m_check_cloud_id.GetCheck());
	m_nIndex = nIndex;

	if (m_nPlaydecHandle != -1)
	{
		StopLocalFile();
	}

	if (m_nPlaydecHandle == -1)
	{
		//open decoder
//		BYTE byFileHeadBuf;

		char chfilename[256] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strFile, -1, chfilename, sizeof(chfilename) - 1, NULL, NULL);
		if (H264_PLAY_OpenFile(m_nIndex, chfilename))
		{
			H264_PLAY_SetFileEndCallBack(m_nIndex, SDKPlayFileEndCallback, hwndMsg);

			//设置信息帧回调
			H264_PLAY_SetInfoFrameCallBack(m_nIndex, videoInfoFramCallback, this);


			//H264_PLAY_SetStreamOpenMode(m_nIndex, STREAME_REALTIME);
			//只播放I帧,可降低cpu使用率
			//H264_PLAY_OnlyIFrame(m_nIndex, true);
			//	H264_PLAY_SetDisplayCallBack(m_nIndex, DisplayCallBackFun, (LONG)this);
			//H264_PLAY_SetDisplayCallBack(m_nIndex, NULL, NULL);
			//H264_PLAY_SetDecCallBack(m_nIndex, nProc);//设置解码回调, 设置后在回调中自己解码
			if (H264_PLAY_Play(m_nIndex, hwndVideo))
			{
				m_nPlaydecHandle = m_nIndex;
			}
		}
	}
	m_userInfo.isConnect = 1;
	return m_nPlaydecHandle;
}

int CVideoWnd::StopLocalFile()
{
	if (m_nPlaydecHandle != -1)
	{
		H264_PLAY_Stop(m_nPlaydecHandle);
		H264_PLAY_CloseFile(m_nPlaydecHandle);

		m_nPlaydecHandle = -1;
	}

	return 1;
		
}