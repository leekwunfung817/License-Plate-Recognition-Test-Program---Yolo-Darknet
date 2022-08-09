
#ifndef _LICENSE_DETECT_
#define _LICENSE_DETECT_

#include "im2col.h"


#ifdef __cplusplus
extern "C" {
#endif
	//最大的车牌字母数
#define LETTER_MAX_LEN 16

	typedef struct tagOcrResult
	{
		char text;
		int x;
		int y;
		int w;
		int h;
	}OCR_RESULT;


CString GetAppPath();
int CreateFolder(CString strPath);

BOOL SaveStrToTxt(CString strFileName, int isEmpty, CString strTxt);

CString plate_detect(CString strFileName, char *pFile, int file_size, image *pImagePlate, OCR_RESULT *pResult, int *err, int save_train, int save_image, char &car_type, float area_left, float area_top, float area_right, float area_bottom);
void plate_mem_free();
CString get_car_type(int index);

#ifdef __cplusplus
}
#endif

#endif