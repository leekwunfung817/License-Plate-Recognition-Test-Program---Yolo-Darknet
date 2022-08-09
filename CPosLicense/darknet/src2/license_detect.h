
#ifndef _LICENSE_DETECT_
#define _LICENSE_DETECT_

#include "im2col.h"


#ifdef __cplusplus
extern "C" {
#endif

CString GetAppPath();

CString plate_detect(CString strFileName, char *pFile, int file_size, image *pImagePlate, int *err, int save_train, int save_image, char &car_type);
void plate_mem_free();
CString get_car_type(int index);

#ifdef __cplusplus
}
#endif

#endif