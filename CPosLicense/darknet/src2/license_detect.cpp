
// CPosLicense.cpp : Defines the class behaviors for the application.
//
#include "windows.h"

#include "stdafx.h"
#include <afxwin.h>         // MFC core and standard components

#include "license_detect.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "darknet.h"
#include "network.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "demo.h"
#include "option_list.h"
#include <locale.h>


#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int(*__compar_fn_t)(const void*, const void*);
#ifdef __USE_GNU
typedef __compar_fn_t comparison_fn_t;
#endif
#endif

#include "http_stream.h"

#pragma comment(lib, "darknet/pthreads/lib/x86/pthreadVC2.lib")

network g_net_car_type = { 0 };
network g_net_plate = { 0 };
network g_net_letter = { 0 };
char **g_letter_names = NULL;	//ָ����ĸ���ַ�
int g_letter_names_num = 0;	//�ַ�����
char **g_car_type_names = NULL;	//ָ����ĸ���ַ�
int g_car_type_names_num = 0;	//�ַ�����

//���ĳ�����ĸ��
#define LETTER_MAX_LEN 16

typedef struct tagOcrResult
{
	char text;
	int x;
	int y;
	int w;
	int h;
}OCR_RESULT;




CString GetAppPath()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	strPath.ReleaseBuffer();
	int iPos;
	iPos = strPath.ReverseFind(_T('\\'));
	strPath = strPath.Left(iPos + 1);
	return strPath;
}


BOOL SaveStrToTxt(CString strFileName, int isEmpty, CString strTxt)
{


	CStdioFile fileTxt;
	if (isEmpty == 1)
	{
		if (fileTxt.Open(strFileName, CFile::modeCreate | CFile::modeReadWrite) == FALSE)
			return FALSE;
	}
	else if (isEmpty == 0)
	{
		if (fileTxt.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite) == FALSE)
			return FALSE;
	}
	else
		return FALSE;
	fileTxt.SeekToEnd();


	CString szChar = L"";
	szChar = setlocale(LC_CTYPE, ("chs"));
	/*CStdioFile *pFile = new CStdioFile(L"c:\\chinese.txt",CFile::modeCreate | CFile::modeWrite);
	CString szStr = L"�л����񹲺͹�";

	pFile->WriteString(szStr.GetBuffer());
	pFile->Close();
	delete pFile;
	*/
	fileTxt.WriteString(strTxt.GetBuffer());
	fileTxt.Close();
	return TRUE;

}

int CreateFolder(CString strPath)
{
	//	strPath = _T("G:\Speech\Text\yuyin\Store");//�˴������ⶨ�壬����ʽ��������ʾһ�£������δ������еģ�����Ѿ��������ˣ��򲻴���
	CString strTmp;
	int i = 0;

	strPath.Trim();
	i = strPath.Find(_T("\\"));//�ҵ���Ŀ¼

	if (i != 2)
	{
		i = strPath.Find(_T("\\\\"));//��window����Ŀ¼
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
			if (!PathFileExists(strTmp))//�ļ��в������򴴽�
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


int load_net_file(network *net, char *datacfg, char *cfgfile, char *weightfile, char *app_path)
{

	int res = 0;
	*net = parse_network_cfg_custom(cfgfile, 1, 1); // set batch=1	//���������ļ�
	if (weightfile)
	{
		load_weights(net, weightfile);	//����Ȩ���ļ�
	}

	fuse_conv_batchnorm(*net);
	calculate_binary_weights(*net);

	//if (net->layers[net->n - 1].classes != names_size) //�����ļ�����
	//{
	//	return 0;
	//}

	return 1;
}


//char detect_type0-����λ��, 1-��ĸ, 2-����

// OCR_RESULT *pResult , == NULL ��ʾ�����ҵ�������ͼƬ, ��NULL��ʾ������ĸλ��
//source_name ԭʼͼƬ����, ���ڱ�����ʱ�ļ�, �ձ�ʾ������
image plate_detect_cpos(char detect_type, network *net, image source_image, float thresh,
	float hier_thresh,int letter_box, char *app_path, char **names, OCR_RESULT *pResult, char *source_name, int is_save_train, int is_save_image)
{


	image im_plate;
	image im_ocr_save;

	memset(&im_plate, 0, sizeof(im_plate));
	memset(&im_ocr_save, 0, sizeof(im_ocr_save));

	int res = 0;


	int j;
	float nms = .45;    // 0.4F

	do
	{

		image sized;

		if (letter_box)
		{
			sized = letterbox_image(source_image, net->w, net->h);	//���ͼƬ��ģ��Ĵ�С(���пհ�)
		}
		else
		{
			sized = resize_image(source_image, net->w, net->h);	//���������С��ģ���С(�����), ����Ч���õ�
		}
		layer net_layers = net->layers[net->n - 1];
		

		float *X = sized.data;

		//time= what_time_is_it_now();
		double time = get_time_point();


		network_predict(*net, X);


		//��ȡ�����
		int nboxes = 0;
		detection *dets = get_network_boxes(net, source_image.w, source_image.h, thresh, hier_thresh, 0, 1, &nboxes, letter_box);
		if (nms) do_nms_sort(dets, nboxes, net_layers.classes, nms);

		const float thresh = 0.005; // function get_network_boxes() has already filtred dets by actual threshold

		int obj_index = 0;
		int i, j;
		int class_id = -1;

		CString strOutputConent;
		CString strOutputFilePath;
		CString strOutputFileName;
		CString strOutputImageName;
		CString strSourceFileName;
		CString strSourcePathName;
		CString str;
		char save_pic_name[512] = { 0 };

		strSourcePathName = CString(source_name);
		int iPos;
		iPos = strSourcePathName.ReverseFind(_T('\\'));
		if (iPos < 0)  iPos = strSourcePathName.ReverseFind('/');
		if (iPos >= 0)
		{
			strOutputFilePath = strSourcePathName.Left(iPos + 1);

			strOutputFilePath += _T("train\\");

			if (is_save_image || is_save_train)
			{
				CreateFolder(strOutputFilePath);
			}

			strSourceFileName = strSourcePathName.Right(strSourcePathName.GetLength() - iPos - 1);
		}


		strOutputImageName = strOutputFileName = strOutputFilePath + strSourceFileName;


		for (i = 0; i < nboxes; ++i)
		{
			strOutputImageName = strOutputFilePath + strSourceFileName;
			class_id = -1;
			float prob = 0;
			for (j = 0; j < net_layers.classes; ++j)
			{
				if (dets[i].prob[j] > thresh && dets[i].prob[j] > prob)//
				{
					prob = dets[i].prob[j];
					class_id = j;
				}
			}

			if (class_id >= 0 && prob > 0.5)//�ɿ��Գ���0.7
			{
				int x = 0;
				int y = 0;
				int width = 0;
				int height = 0;
				width = dets[i].bbox.w * source_image.w;	//����Ŀ�
				height = source_image.h* dets[i].bbox.h;	//����ĸ�

				x = (source_image.w * dets[i].bbox.x) - width / 2;	//��������Ͻ�X
				y = (source_image.h * dets[i].bbox.y) - height / 2;	//��������Ͻ�Y
				
				if (is_save_image)
				{
					if (pResult == NULL && detect_type == 0)
					{
						str.Format(_T("_plate_%d"), class_id);
						strOutputImageName.Replace(_T(".jpg"), str);
					}
					else if (pResult && detect_type == 1)//��ĸλ��
					{
						str.Format(_T("_ocr_%d"), class_id);
						strOutputImageName.Replace(_T(".jpg"), str);
					}
					else if (pResult && detect_type == 2)//����
					{
						str.Format(_T("_car_type_%d"), class_id);
						strOutputImageName.Replace(_T(".jpg"), str);
					}
					WideCharToMultiByte(CP_ACP, 0, strOutputImageName, -1, save_pic_name, sizeof(save_pic_name) - 1, NULL, NULL);

					image crop;
					crop = crop_image(source_image, x, y, width, height);
					//snprintf(save_pic_name, sizeof(save_pic_name), "block_%d", class_id);
					save_image(crop, save_pic_name);
					free_image(crop);
				}


				str.Format(_T("%d %2.4f %2.4f %2.4f %2.4f\n"), class_id, dets[i].bbox.x, dets[i].bbox.y, dets[i].bbox.w, dets[i].bbox.h);
				strOutputConent += str;

				//����λ��
				if (pResult == NULL && detect_type == 0)
				{
					strOutputFileName = strOutputFilePath + _T("plate_train_") + strSourceFileName;
					strOutputFileName.Replace(_T(".jpg"), _T(".txt"));

					if(class_id == 1)
					{
						im_plate = crop_image(source_image, x, y, width, height);
					//	break;
					}
				}
				else if (pResult && obj_index < LETTER_MAX_LEN && detect_type == 1)//��ĸλ��
				{
					strOutputFileName = strOutputFilePath + _T("ocr_train_") + strSourceFileName;
					strOutputFileName.Replace(_T(".jpg"), _T(".txt"));

					//image crop;
					//crop = crop_image(source_image, x, y, width, height);
					//snprintf(spic_name, sizeof(spic_name), "block_%d", class_id);
					//save_image(crop, spic_name);
					//free_image(crop);

					pResult[obj_index].text = *names[class_id];

					x += width / 2;	//���ĵ�, �ɱ�֤Yֵ��Ϊ0, ����֮�����
					y += height / 2;
					
					pResult[obj_index].x = x;
					pResult[obj_index].y = y;
					pResult[obj_index].w = width;
					pResult[obj_index].h = height;
					obj_index++;

				}
				else if (pResult && obj_index < LETTER_MAX_LEN && detect_type == 2)
				{
					strOutputFileName = strOutputFilePath + _T("car_type_train_") + strSourceFileName;
					strOutputFileName.Replace(_T(".jpg"), _T(".txt"));

					//image crop;
					//crop = crop_image(source_image, x, y, width, height);
					//snprintf(spic_name, sizeof(spic_name), "block_%d", class_id);
					//save_image(crop, spic_name);
					//free_image(crop);


					pResult[obj_index].text = '0' + class_id;//���ַ�'0',����洢����

					x += width / 2;	//���ĵ�, �ɱ�֤Yֵ��Ϊ0, ����֮�����
					y += height / 2;

					pResult[obj_index].x = x;
					pResult[obj_index].y = y;
					pResult[obj_index].w = width;
					pResult[obj_index].h = height;
					obj_index++;

				}



			}
			
		}

		if (is_save_train)
		{
			SaveStrToTxt(strOutputFileName, TRUE, strOutputConent);

		}


		//�Ѽ����������
//		draw_detections_v3(source_image, dets, nboxes, thresh, names, alphabet, net_layers.classes, ext_output);
//		save_image(source_image, "predictions");

		free_detections(dets, nboxes);
		free_image(sized);

	} while (0);


	return im_plate;
}

//order == 0 , x��С��������
//order == 1, y ��С��������

void result_sort(OCR_RESULT *pResult, int order)
{

	OCR_RESULT tmp_result;
	int i = 0;
	int j = 0;
	int len = 0;

	for (i = 0; i < LETTER_MAX_LEN; i++)
	{
		if (pResult[i].text == 0)break;
	}
	len = i;

	int value_front = 0;
	int value_this = 0;

	//��Y��������
	for (i = 0; i<len; i++)
	{
		for (j = len - 1; j>i; j--)
		{
			if (order == 0)
			{
				value_front = pResult[j - 1].x;
				value_this = pResult[j].x;
			}
			else
			{
				value_front = pResult[j - 1].y;
				value_this = pResult[j].y;
			}

			if (value_front > value_this)
			{
				tmp_result.text = pResult[j - 1].text;
				tmp_result.x = pResult[j - 1].x;
				tmp_result.y = pResult[j - 1].y;
				tmp_result.w = pResult[j - 1].w;
				tmp_result.h = pResult[j - 1].h;

				pResult[j - 1].text = pResult[j].text;
				pResult[j - 1].x = pResult[j].x;
				pResult[j - 1].y = pResult[j].y;
				pResult[j - 1].w = pResult[j].w;
				pResult[j - 1].h = pResult[j].h;

				pResult[j].text = tmp_result.text;
				pResult[j].x = tmp_result.x;
				pResult[j].y = tmp_result.y;
				pResult[j].w = tmp_result.w;
				pResult[j].h = tmp_result.h;
			}
		}
	}
}


//��������
//���ؼ�⵽��ͼƬ��б��
float plate_licent_sort(OCR_RESULT *pResult)
{
	OCR_RESULT tmp_result;
	int i = 0;
	int j = 0;
	int iMax = 0;
	int len = 0;
	OCR_RESULT res_x[LETTER_MAX_LEN];
	OCR_RESULT res_y[LETTER_MAX_LEN];
	float slop_tmp[LETTER_MAX_LEN];

	memset(&res_x, 0, sizeof(res_x));
	memset(&res_y, 0, sizeof(res_y));
	memset(&slop_tmp, 0, sizeof(slop_tmp));

	for (i = 0; i < LETTER_MAX_LEN; i++)
	{
		if(pResult[i].text == 0)break;
	}
	len = i;

	if (len == 0) return 0;

	float slop = 0.0;
	float fTmp = 0.0;

	int line_num = 0;//
	int width_avg = 0;
	int height_avg = 0;
	int iTmp = 0; 

	for (i = 0; i < len; i++)
	{
		width_avg += pResult[i].w;
		height_avg += pResult[i].h;
	}
	width_avg /= len;
	height_avg /= len;


	result_sort(pResult, 0);//X������

	for (i = 0; i < len-1; i++)
	{
		iTmp = abs(pResult[i + 1].y - pResult[i].y);

		if (iTmp > height_avg / 2)	//�����������Yֵ�仯�ܴ�, ����Ϊ������
		{
			line_num = 1;//����
			break;
		}
	}

	//ֻ��һ��
	if(line_num == 0)
	{
		//������б��
		slop = 0.0;
		fTmp = 0.0;
		for (i = 0; i < len-1; i++)
		{
			if (pResult[i + 1].text == 0 || pResult[i + 1].y == 0)break;

			fTmp = pResult[i].y;
			fTmp = fTmp / pResult[i + 1].y;
			slop += fTmp;
		}

		slop /= i;

		return slop;
	}


	//������, ��Ҫ������

	//��Y��������
	result_sort(pResult, 1);//

	signed int x_dir = 0;//��¼X�Ǳ���Ǳ�С
	for (i = 0; i < len - 1; i++)
	{
		//if (pResult[i + 1].x > pResult[i].x && abs(pResult[i + 1].y - pResult[i].y) < height_avg / 2)
		//{
		//	if (x_dir < 0)	//��������С, ��ʾ�ڶ���
		//	{
		//		break;
		//	}
		//	x_dir++;//��������
		//}
		//else
		//{
		//	if (x_dir > 0)	//��������С, ��ʾ�ڶ���
		//	{
		//		break;
		//	}
		//	x_dir-- ;//��С������
		//}

		if (abs(pResult[i + 1].y - pResult[i].y) > height_avg / 2)	//Yֵ���߶ȵ�һ������, ����Ϊ������һ��
		{
			break;
		}

	}
	

	//��������
	int k = 0;
	for (j = 0; j < len; j++)
	{
		if (j <= i)//���Ƶ�һ��
		{
			res_x[j].text = pResult[j].text;
			res_x[j].x = pResult[j].x;
			res_x[j].y = pResult[j].y;
			res_x[j].w = pResult[j].w;
			res_x[j].h = pResult[j].h;
		}
		else//���Ƶڶ���
		{
			res_y[k].text = pResult[j].text;
			res_y[k].x = pResult[j].x;
			res_y[k].y = pResult[j].y;
			res_y[k].w = pResult[j].w;
			res_y[k].h = pResult[j].h;
			k++;
		}
	}

	result_sort(res_x, 0);//��һ������
	result_sort(res_y, 0);//�ڶ�������

	k = 0;
	for (j = 0; j < len; j++)
	{
		if (j <= i)
		{
			pResult[j].text = res_x[j].text;
			pResult[j].x = res_x[j].x;
			pResult[j].y = res_x[j].y;
			pResult[j].w = res_x[j].w;
			pResult[j].h = res_x[j].h;
		}
		else
		{
			pResult[j].text = res_y[k].text;
			pResult[j].x = res_y[k].x;
			pResult[j].y = res_y[k].y;
			pResult[j].w = res_y[k].w;
			pResult[j].h = res_y[k].h;
			k++;
		}
	}

	//������б��
	slop = 0.0;
	fTmp = 0.0;
	for (i = 0; i < len - 1; i++)
	{
		if (res_y[i + 1].text == 0 || res_y[i + 1].y == 0 )break;
		fTmp = res_y[i].y;
		fTmp = fTmp / res_y[i + 1].y;
		slop += fTmp;
	}

	slop /= i;
	return slop;

}



int plate_detect_test(char *datacfg, char *cfgfile, char *weightfile, char *filename, image im, float thresh,
	float hier_thresh, int dont_show, int ext_output, int save_labels, char *outfile, int letter_box, char *app_path)
{

	int res = 0;
	list *options = read_data_cfg(datacfg);	//��ȡʶ�����, �����ļ�����Ŀ¼
	char *name_list = option_find_str(options, "names", "data/names.list"); //�ڲ����б����Ƿ��ҵ�Ŀ¼
	int names_size = 0;

	char lable_file[512] = { 0 };
	snprintf(lable_file, sizeof(lable_file), "%s%s", app_path, name_list);	//ת���ɳ�������Ŀ¼�ľ���·��
	name_list = lable_file;

	char **names = get_labels_custom(name_list, &names_size); //get_labels(name_list);	//��ȡ���б�ǩ����

	image **alphabet = load_alphabet();	//�������б�ǩͼƬ, ����ʶ����ɺ���ͼƬ�ڱ�����
	network net = parse_network_cfg_custom(cfgfile, 1, 1); // set batch=1	//���������ļ�
	if (weightfile)
	{
		load_weights(&net, weightfile);	//����Ȩ���ļ�
	}


	fuse_conv_batchnorm(net);

	calculate_binary_weights(net);


	if (net.layers[net.n - 1].classes != names_size) //�����ļ�����
	{
		printf(" Error: in the file %s number of names %d that isn't equal to classes=%d in the file %s \n",
			name_list, names_size, net.layers[net.n - 1].classes, cfgfile);
		if (net.layers[net.n - 1].classes > names_size)
		{
			//getchar();
		}
	}

	srand(2222222);
	char buff[256];
	char *input = buff;
	char *json_buf = NULL;
	int json_image_id = 0;
	FILE* json_file = NULL;
	if (outfile)
	{
		json_file = fopen(outfile, "wb");
		char *tmp = "[\n";
		fwrite(tmp, sizeof(char), strlen(tmp), json_file);
	}
	int j;
	float nms = .45;    // 0.4F

	do
	{
		if (filename)
		{
			strncpy(input, filename, 256);
			if (strlen(input) > 0)
			{
				if (input[strlen(input) - 1] == 0x0d) input[strlen(input) - 1] = 0;
			}
		}
		else
		{
			break;
		}
		//image im;
		//image sized = load_image_resize(input, net.w, net.h, net.c, &im);
		//image im = load_image(input, 0, 0, net.c);
		image sized;
		if (letter_box) sized = letterbox_image(im, net.w, net.h);
		else sized = resize_image(im, net.w, net.h);
		layer net_layers = net.layers[net.n - 1];

		//box *boxes = calloc(l.w*l.h*l.n, sizeof(box));
		//float **probs = calloc(l.w*l.h*l.n, sizeof(float*));
		//for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = (float*)calloc(l.classes, sizeof(float));

		float *X = sized.data;

		//time= what_time_is_it_now();
		double time = get_time_point();


		network_predict(net, X);


		//network_predict_image(&net, im); letterbox = 1;
		printf("%s: Predicted in %lf milli-seconds.\n", input, ((double)get_time_point() - time) / 1000);
		//printf("%s: Predicted in %f seconds.\n", input, (what_time_is_it_now()-time));

		//��ȡ�����
		int nboxes = 0;
		detection *dets = get_network_boxes(&net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes, letter_box);
		if (nms) do_nms_sort(dets, nboxes, net_layers.classes, nms);

		//�Ѽ����������
		draw_detections_v3(im, dets, nboxes, thresh, names, alphabet, net_layers.classes, ext_output);

		save_image(im, "predictions");
		if (!dont_show)
		{
			show_image(im, "predictions");
		}

		if (outfile)
		{
			if (json_buf)
			{
				char *tmp = ", \n";
				fwrite(tmp, sizeof(char), strlen(tmp), json_file);
			}
			++json_image_id;
			json_buf = detection_to_json(dets, nboxes, net_layers.classes, names, json_image_id, input);

			fwrite(json_buf, sizeof(char), strlen(json_buf), json_file);
			free(json_buf);
		}

		// pseudo labeling concept - fast.ai
		if (save_labels)
		{
			char labelpath[4096];
			replace_image_to_label(input, labelpath);

			FILE* fw = fopen(labelpath, "wb");
			int i;
			for (i = 0; i < nboxes; ++i)
			{
				char buff[1024];
				int class_id = -1;
				float prob = 0;
				for (j = 0; j < net_layers.classes; ++j)
				{
					if (dets[i].prob[j] > thresh && dets[i].prob[j] > prob)
					{
						prob = dets[i].prob[j];
						class_id = j;
					}
				}
				if (class_id >= 0)
				{
					sprintf(buff, "%d %2.4f %2.4f %2.4f %2.4f\n", class_id, dets[i].bbox.x, dets[i].bbox.y, dets[i].bbox.w, dets[i].bbox.h);
					fwrite(buff, sizeof(char), strlen(buff), fw);
				}
			}
			fclose(fw);
		}

		free_detections(dets, nboxes);
		free_image(sized);

		if (!dont_show)
		{
			wait_until_press_key_cv();
			destroy_all_windows_cv();
		}


	} while (0);

	if (outfile)
	{
		char *tmp = "\n]";
		fwrite(tmp, sizeof(char), strlen(tmp), json_file);
		fclose(json_file);
	}

	// free memory
	free_ptrs((void**)names, net.layers[net.n - 1].classes);
	free_list_contents_kvp(options);
	free_list(options);

	int i;
	const int nsize = 8;
	for (j = 0; j < nsize; ++j)
	{
		for (i = 32; i < 127; ++i)
		{
			free_image(alphabet[j][i]);
		}
		free(alphabet[j]);
	}
	free(alphabet);

	free_network(net);

	return res;
}

//CString strFileName ���ļ���ȡҪʶ���ͼƬ
//char *pFile, int file_size ����ļ���Ϊ�գ���ʹ���ڴ��е�ͼƬdib
//image *pImagePlate,ʶ��������ͼƬ�����ڷ��ؽ����ʾ
//int *err ���ش�����
//int save_train �Ƿ񱣴�������txt(����ѵ��)
//int save_image �Ƿ񱣴���ͼƬ
//char &car_type ���ڷ���ʶ������ĳ���


// isDebug 1��ʾ������ʱ�ļ�
CString plate_detect(CString strFileName, char *pFile, int file_size, image *pImagePlate, int *err, int save_train, int save_image, char &car_type)
{
	
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CString strLicense;
	CString strAppPath = GetAppPath();

	CString strPlateCfgFile = strAppPath + _T("cpos_lpr\\lpr_model.cfg");
	CString strPlateWeightFile = strAppPath + _T("cpos_lpr\\lpr_model.weights");

	CString strOcrCfgFile = strAppPath + _T("cpos_ocr\\ocr_model.cfg");
	CString strOcrWeightFile = strAppPath + _T("cpos_ocr\\ocr_model.weights");
	CString strOcrLetterFile = strAppPath + _T("cpos_ocr\\ocr_classes.names");	//OCRʶ���ַ��б��ļ�

	CString strCarTypeCfgFile = strAppPath + _T("cpos_ctr\\ctr_model.cfg");
	CString strCarTypeWeightFile = strAppPath + _T("cpos_ctr\\ctr_model.weights");
	CString strCarTypeLableFile = strAppPath + _T("cpos_ctr\\ctr_classes.names");	//OCRʶ���ַ��б��ļ�






	int i = 0;


	char app_path[512] = { 0 };
	char source_picture_file[512] = { 0 };
	char save_picture_plate_file[512] = { 0 };//����

	char plate_cfg_file[512] = { 0 };
	char plate_weight_file[512] = { 0 };
//	char plate_coco_data[512] = { 0 };

	char ocr_cfg_file[512] = { 0 };
	char ocr_weight_file[512] = { 0 };
//	char ocr_coco_data[512] = { 0 };
	char ocr_letter_lable_file[512] = { 0 };

	char car_type_cfg_file[512] = { 0 };
	char car_type_weight_file[512] = { 0 };
	char car_type_lable_file[512] = { 0 };

	WideCharToMultiByte(CP_ACP, 0, strAppPath, -1, app_path, sizeof(app_path) - 1, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strFileName, -1, source_picture_file, sizeof(source_picture_file) - 1, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strPlateCfgFile, -1, plate_cfg_file, sizeof(plate_cfg_file) - 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strPlateWeightFile, -1, plate_weight_file, sizeof(plate_weight_file) - 1, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strOcrCfgFile, -1, ocr_cfg_file, sizeof(ocr_cfg_file) - 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strOcrWeightFile, -1, ocr_weight_file, sizeof(ocr_weight_file) - 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strOcrLetterFile, -1, ocr_letter_lable_file, sizeof(ocr_letter_lable_file) - 1, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strCarTypeCfgFile, -1, car_type_cfg_file, sizeof(car_type_cfg_file) - 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strCarTypeWeightFile, -1, car_type_weight_file, sizeof(car_type_weight_file) - 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strCarTypeLableFile, -1, car_type_lable_file, sizeof(car_type_lable_file) - 1, NULL, NULL);


		float thresh = 0.24; //��ֵ
		int ext_output = 0; //�Ƿ����������

		OCR_RESULT letter_result[LETTER_MAX_LEN];//���16����ĸ

		memset(&letter_result, 0, sizeof(letter_result));

		image im_plate;
		image im_ocr;
		image im_src;
		memset(&im_plate, 0, sizeof(im_plate));
		memset(&im_ocr, 0, sizeof(im_ocr));
		memset(&im_src, 0, sizeof(im_src));

		do
		{

			if (g_net_plate.n == 0)
			{
				load_net_file(&g_net_plate, NULL, plate_cfg_file, plate_weight_file, app_path);
			}

			if (g_net_letter.n == 0)
			{
				load_net_file(&g_net_letter, NULL, ocr_cfg_file, ocr_weight_file, app_path);
			}
			if (g_net_car_type.n == 0)
			{
				load_net_file(&g_net_car_type, NULL, car_type_cfg_file, car_type_weight_file, app_path);
			}
				

			if(g_letter_names == NULL)
			{
	//			list *options = read_data_cfg(ocr_coco_data);	//��ȡʶ�����, �����ļ�����Ŀ¼
	//			char *name_list = option_find_str(options, "names", "data/names.list"); //�ڲ����б����Ƿ��ҵ�Ŀ¼
				g_letter_names = get_labels_custom(ocr_letter_lable_file, &g_letter_names_num); //get_labels(name_list);	//��ȡ���б�ǩ����
			}

			if(g_car_type_names == NULL)
			{
	//			list *options = read_data_cfg(ocr_coco_data);	//��ȡʶ�����, �����ļ�����Ŀ¼
	//			char *name_list = option_find_str(options, "names", "data/names.list"); //�ڲ����б����Ƿ��ҵ�Ŀ¼
				g_car_type_names = get_labels_custom(car_type_lable_file, &g_car_type_names_num); //get_labels(name_list);	//��ȡ���б�ǩ����
			}


			//image **alphabet = load_alphabet();	//�������б�ǩͼƬ, ����ʶ����ɺ���ͼƬ������

			//����ԭʼͼƬ
			if (!strFileName.IsEmpty())
			{
				im_src = load_image(source_picture_file, 0, 0, g_net_plate.c); //��������ĳߴ粻ͬ, ���Զ����쵽�����ߴ�
			}
			else if (pFile && file_size > 0)
			{
				im_src = load_image_from_memory(pFile, file_size);
			}

			if (im_src.data == 0 || (im_src.w == 10 && im_src.h == 10) )
			{
				*err = 1;
				break;
			}
			



			//ʶ����
			plate_detect_cpos(2, &g_net_car_type, im_src, thresh, 0.5, 0, app_path, g_car_type_names, letter_result, source_picture_file, save_train, save_image);

			for (i = 0; i < LETTER_MAX_LEN; i++)
			{
				if (letter_result[i].text != 0)//��鵽
				{
					car_type = letter_result[i].text;
				};
			}
			

			//ʶ�����Ƶ�λ��
			im_plate = plate_detect_cpos(0, &g_net_plate, im_src, thresh, 0.5, 0, app_path, 0, 0, source_picture_file, save_train, save_image);
		
			if (pImagePlate)
			{
				free_image(*pImagePlate);
				*pImagePlate = copy_image(im_plate);
			}

			if (!im_plate.data) *err = 2;

//			if (im_plate.data) save_image(im_plate, "plate");	//����ͼƬ,������

			memset(&letter_result, 0, sizeof(letter_result));

			//ʶ�������ϵ��ַ�
			im_ocr = plate_detect_cpos(1, &g_net_letter, im_plate, thresh, 0.5, 0, app_path, g_letter_names, letter_result, source_picture_file, save_train, save_image);
			if (im_ocr.data) free_image(im_ocr);
			else
				*err = 3;
			float slop = 0;
			slop = plate_licent_sort(letter_result);
			*err = 5;
			//����, ����м��ļ�
			//plate_detect_test(ocr_coco_data, ocr_cfg_file, ocr_weight_file, source_picture_file, im_plate, thresh, 0.5, 1, ext_output, 0, "coco_out.txt", 0, app_path);


			// free memory
//			free_ptrs((void**)names, g_net_letter.layers[g_net_letter.n - 1].classes);
//			free_list_contents_kvp(options);
//			free_list(options);

		} while (0);


		free_image(im_plate);
		free_image(im_src);

		for (i = 0; i < LETTER_MAX_LEN; i++)
		{
			if (letter_result[i].text == 0)break;
			strLicense += letter_result[i].text;
		}


//		free_network(g_net_plate);
//		free_network(g_net_letter);
//		g_net_plate.n = 0;
//		g_net_letter.n = 0;

		return strLicense;
}
CString get_car_type(int index)
{
	CString strCarType;
	if (index >= 0 && index < g_car_type_names_num && g_car_type_names)
	{
		strCarType = CString(g_car_type_names[index]);
	}

	return strCarType;
}

void plate_mem_free()
{
	free_network(g_net_plate);
	free_network(g_net_letter);
	g_net_plate.n = 0;
	g_net_letter.n = 0;

	free_ptrs((void**)g_letter_names, g_letter_names_num);
	g_letter_names_num = 0;
	g_letter_names = NULL;

	free_ptrs((void**)g_car_type_names, g_car_type_names_num);
	g_car_type_names_num = 0;
	g_car_type_names = NULL;

}

