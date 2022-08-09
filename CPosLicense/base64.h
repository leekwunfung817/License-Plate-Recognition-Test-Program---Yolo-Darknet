
#pragma once

#include <string>

#include <sstream>	//字符格式化输入输出<< >> 
#include <iomanip>	//输出流格式控制
#include <iostream>     //标准输入输出,打印到控制台,相当于printf,  std::cout, std::endl

// base64

struct Base64Date6
{
	unsigned int d4:6;
	unsigned int d3:6;
	unsigned int d2:6;
	unsigned int d1:6;
};

//base64对应字符
static char ConvertToBase64(char uc)
{
	if(uc < 26)return 'A'+ uc;
	else if(uc < 52)return 'a'+(uc -26);
	else if(uc < 62)return '0'+(uc -52);
	else if(uc == 62)return '+';
	return '/';
}


char Base64CharCheck(unsigned char uc)
{

	if(uc >= 'A' && uc <= 'z')
	{
		return 1;
	}
	else if(uc >= '0' && uc <= '9')
	{
		return 1;
	}
	else if(uc == '/' || uc == '+' || uc == '=')
	{
		return 1;
	}

	return 0;
}



static void string2base64(std::string& str_src, std::string& str_encode)
{

	struct  Base64Date6 *ddd = NULL;
	int i = 0;

	char *tmp = NULL;
	char cc = '\0';
	int len = str_src.length();

	char *buf = new char[len + 2];
	memset(buf, 0, len + 2);

	strcpy(buf, str_src.c_str());
	str_encode.clear();

	for (i = 1; i <= len / 3; i++)
	{
		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6*)tmp;

		str_encode += ConvertToBase64((unsigned int)ddd->d1);
		str_encode += ConvertToBase64((unsigned int)ddd->d2);
		str_encode += ConvertToBase64((unsigned int)ddd->d3);
		str_encode += ConvertToBase64((unsigned int)ddd->d4);

		//dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		//dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		//dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
		//dbuf[(i-1)*4+3]=ConvertToBase64((unsigned int)ddd->d4);
	}

	if (len % 3 == 1)
	{

		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6*)tmp;
		str_encode += ConvertToBase64((unsigned int)ddd->d1);
		str_encode += ConvertToBase64((unsigned int)ddd->d2);
		str_encode += '=';
		str_encode += '=';

	}

	if (len % 3 == 2)
	{
		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6*)tmp;
		str_encode += ConvertToBase64((unsigned int)ddd->d1);
		str_encode += ConvertToBase64((unsigned int)ddd->d2);
		str_encode += ConvertToBase64((unsigned int)ddd->d3);
		str_encode += '=';
	}
	delete[]buf;
}

static void base64_decode(std::string &str_src, std::string &str_decode)
{
//根据base64表，以字符找到对应的十进制数据  
    int table[]={0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,62,0,0,0,
    		 63,52,53,54,55,56,57,58,
    		 59,60,61,0,0,0,0,0,0,0,0,
    		 1,2,3,4,5,6,7,8,9,10,11,12,
    		 13,14,15,16,17,18,19,20,21,
    		 22,23,24,25,0,0,0,0,0,0,26,
    		 27,28,29,30,31,32,33,34,35,
    		 36,37,38,39,40,41,42,43,44,
    		 45,46,47,48,49,50,51
    	       };  
    long len;  
    long str_len;  
	char res[4] = {0, 0, 0, 0};  
    int i,j;  
  
	str_decode.clear();

//计算解码后的字符串长度  
    len = str_src.length();
//判断编码后的字符串后是否有=
	if(str_src.find("==") != std::string::npos)
	{
		str_len=len/4*3-2; 
	}
    else if(str_src.find("=") != std::string::npos)
    {
		str_len=len/4*3-1;  
	}
    else  
	{
        str_len=len/4*3;  
	}
 
	char ch = 0;
  
//以4个字符为一位进行解码 
	char cnt = 0;
	 
    for(i=0; i<len; i++)  
    { 
		if(!Base64CharCheck(str_src.at(i)))
		{
			continue;
		}
		res[cnt] = str_src.at(i);
		cnt++;
		if(cnt == 4)
		{
			res[0]=((unsigned char)table[res[0]])<<2 | (((unsigned char)table[res[1]])>>4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
			res[1]=(((unsigned char)table[res[1]])<<4) | (((unsigned char)table[res[2]])>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
			res[2]=(((unsigned char)table[res[2]])<<6) | ((unsigned char)table[res[3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
			res[3] = 0;
			 str_decode.append(res);
			cnt = 0;
		}

        //res[0]=((unsigned char)table[str_src.at(i)])<<2 | (((unsigned char)table[str_src.at(i+1)])>>4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        //res[1]=(((unsigned char)table[str_src.at(i+1)])<<4) | (((unsigned char)table[str_src.at(i+2)])>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        //res[2]=(((unsigned char)table[str_src.at(i+2)])<<6) | ((unsigned char)table[str_src.at(i+3)]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
	}  
   
} 

// base64的实现

void  EncodeBase64_test(char*dbuf, char*buf128, int len)
{

	struct  Base64Date6 *ddd =NULL;
	int i =0;
	
	char *tmp =NULL;
	char cc ='\0';

	char *buf = new char[len + 2];	//+2 因为字符长度是以3为倍数
	memset(buf, 0, len + 2);

	strcpy(buf, buf128);

	for(i =1;i <=len/3; i++)
	{
		tmp = buf +(i-1)*3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3]=ConvertToBase64((unsigned int)ddd->d4);
	}

	if(len %3==1)
	{

		tmp =buf +(i-1)*3;
		cc =tmp[2];
		tmp[2]=tmp[0];
		tmp[0]=cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]='=';
		dbuf[(i-1)*4+3]='=';
	}

	if(len%3==2)
	{
		tmp =buf+(i-1)*3;
		cc =tmp[2];
		tmp[2]=tmp[0];
		tmp[0]=cc;
		ddd =(struct Base64Date6*)tmp;
		dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3]='=';
	}

	delete []buf;

	return;
}


//输入密码原文，返回加密后的密码
CString PasswordEncode(CString strPassword)
{
	CString strEncode;
	std::string str_src;
	std::string str_encode;
	
	str_src = CT2A(strPassword.GetString());
	string2base64(str_src, str_encode);
	string2base64(str_encode, str_src);
	string2base64(str_src, str_encode);



	strEncode = CString(str_encode.c_str());

	return strEncode;
}

//输入密码密文，返回解密后的密码
CString PasswordDecode(CString strPassword)
{
	CString strDecode;
	std::string str_src;
	std::string str_decode;
	str_src = CT2A(strPassword.GetString());
	base64_decode(str_src, str_decode);
	base64_decode(str_decode, str_src);
	base64_decode(str_src, str_decode);

	strDecode = CString(str_decode.c_str());
	return strDecode;
}


