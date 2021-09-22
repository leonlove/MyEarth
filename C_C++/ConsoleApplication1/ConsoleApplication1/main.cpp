//#include <iostream>
//#include "curl.h"
//using namespace std;
//
//size_t ncWriteFile(void* buffer, size_t size, size_t nmemb, void* lpVoid)
//{
//	FILE* stream = (FILE *)lpVoid;
//	if (nullptr == stream || nullptr == buffer)
//	{
//		return -1;
//	}
//	size_t nWrite = fwrite(buffer, size, nmemb, stream);
//	return nWrite;
//}
//
//void SendGet()
//{
//	FILE *fp;
//	fp = fopen("1.txt", "w");
//
//	CURL *curl;
//	CURLcode res;
//
//	struct curl_slist *headers = NULL;
//	//headers = curl_slist_append(headers, "Content-Type:application/json");
//	std::string url = "http://www.baidu.com";
//	curl = curl_easy_init();    // 初始化
//	if (curl)
//	{
//		res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());         //请求地址
//		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);      //请求头
//		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ncWriteFile);   //写入数据的回调
//		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);        //文件指针
//		res = curl_easy_perform(curl);   // 执行
//
//		if (res != 0)
//		{
//			curl_slist_free_all(headers);
//			curl_easy_cleanup(curl);
//		}
//	}
//	fclose(fp);
//}
//
//void SendPost()
//{
//	CURL *curl;
//	CURLcode res;
//
//	struct curl_slist *headers = NULL;
//	headers = curl_slist_append(headers, "Content-Type:application/json");
//	std::string url = "http://www.baidu.com";
//	curl = curl_easy_init();    // 初始化
//	if (curl)
//	{
//		res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());         //请求地址
//		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);      //请求头
//		res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);         //请求参数
//		res = curl_easy_setopt(curl, CURLOPT_POST, 1);                  //设置非0表示本次操作为post
//		res = curl_easy_perform(curl);   // 执行
//		if (res != 0)
//		{
//			curl_slist_free_all(headers);
//			curl_easy_cleanup(curl);
//		}
//	}
//}
//
//int main()
//{
//	SendPost();
//	return 0;
//}

#include <iostream>
#include <string>

#include <curl.h>
//#include <types.h>
#include <easy.h>

#include <stdlib.h>
#include <string.h>
using namespace std;

struct MemoryStruct
{
	char *memory;
	size_t size;
	MemoryStruct()
	{
		memory = (char *)malloc(1);
		size = 0;
	}
	~MemoryStruct()
	{
		free(memory);
		memory = NULL;
		size = 0;
	}
};

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory)
	{
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
}

int main()
{
	CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != res)
	{
		cout << "curl init failed" << endl;
		return 1;
	}

	CURL *pCurl;
	pCurl = curl_easy_init();

	if (NULL == pCurl)
	{
		cout << "Init CURL failed..." << endl;
		return -1;
	}

	string url = "http://{IP}:{PORT}/search";
	string filename = "result.json";

	curl_slist *pList = NULL;
	pList = curl_slist_append(pList, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
	pList = curl_slist_append(pList, "Accept:application/json, text/javascript, */*; q=0.01");
	pList = curl_slist_append(pList, "Accept-Language:zh-CN,zh;q=0.8");
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList);

	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str()); //提交表单的URL地址

	curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);  //启用时会将头文件的信息作为数据流输
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);//允许重定向
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

	//将返回结果通过回调函数写到自定义的对象中
	MemoryStruct oDataChunk;
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &oDataChunk);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L); //启用时会汇报所有的信息
												  //post表单参数
	string strJsonData;
	strJsonData = "queryWord=CONTENT:码农&";
	strJsonData += "startTime=1507605327&";
	strJsonData += "endTime=1508210127&";
	strJsonData += "maxCount=500&";
	strJsonData += "contextLength=200";
	//libcur的相关POST配置项
	curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strJsonData.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, strJsonData.size());


	res = curl_easy_perform(pCurl);

	long res_code = 0;
	res = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &res_code);

	if ((res == CURLE_OK) && (res_code == 200 || res_code == 201))
	{
		FILE* fTmpMem = (FILE*)oDataChunk.memory;
		if (!fTmpMem) {

		}
		FILE *fp = fopen(filename.c_str(), "wb");
		if (!fp)
		{
			cout << "open file failed";
			return -1;
		}

		fwrite(fTmpMem, 1, oDataChunk.size, fp);
		fclose(fp);
		return true;
	}

	curl_slist_free_all(pList);
	curl_easy_cleanup(pCurl);
	curl_global_cleanup();

	return 0;
}