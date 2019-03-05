#include <stdlib.h>
#include <stdio.h>
#include "cJSON/cJSON.h"
#include "CJsonHelper.h"

#ifndef STOCKNAME_LEN
#define STOCKNAME_LEN			(32)
#endif

struct ShortSellBaseData
{
	int64_t  nStockGUID;					//股票的GUID;
	int      nIntDayTime;					//时间的数字类型(精确到天);
	int      nStockCode;					//股票的数字号码;
	int      nTotalNum;						//当天总数;
	int      nLeaveNum;						//当天剩余;
	int      nCharge;						//当天股价;
	int      nOffsetNum;					//与上一天相差股数;
	int      nPercent;						//offset_num*1000/(total_num+offset_num);
	char     szStockName[STOCKNAME_LEN];	//股票名称;
	string   stockName;						//股票名称;
};
#ifdef WIN32
#include <Windows.h>
string UTF8ToGBK(string &strUtf8)
{
	string strOutGBK = "";
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	WCHAR *wszGBK = new WCHAR[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char *pszGBK = new char[len + 1];
	memset(pszGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, pszGBK, len, NULL, NULL);
	strOutGBK = pszGBK;
	delete[]pszGBK;
	delete[]wszGBK;
	return strOutGBK;
}
#endif

bool writeResultConfig()
{
	int finished_cities = 1;
	int remaining_cities = 0;

	CJsonHelper jsonHelper;
	jsonHelper.set( "finished_cities", finished_cities );
	jsonHelper.set( "remaining_cities", remaining_cities );


	jsonHelper.addEmptySubArray("cities");

	CJsonHelper* citiesJsonHelper = jsonHelper["cities"];

	for (int i = 0; i < finished_cities; ++i)
	{
		CJsonHelper finishedCityJsonHelper;
		finishedCityJsonHelper.set( "code", "350100" );
		
		string strCityNameUTF8("福州市");
		string strCityName = UTF8ToGBK(strCityNameUTF8);
		finishedCityJsonHelper.set( "city_name", strCityNameUTF8.c_str() );

		finishedCityJsonHelper.set( "total_count", 100 );
		finishedCityJsonHelper.set( "finished_count", 100 );

		citiesJsonHelper->append( &finishedCityJsonHelper );
	}

	//每次都覆盖重写;
	FILE* pFile = fopen("result.json", "wb+");
	if (NULL == pFile)
	{
		return false;
	}

	string strFormatString = jsonHelper.toFormattedString();
	fwrite( strFormatString.c_str(), 1, strFormatString.length(), pFile );
	fclose( pFile );

	return true;
}

int main(int argc, char** argv)
{

	char *s = "{\"list\":[{\"name\":\"xiao hong\",\"age\":10},{\"name\":\"hua hua\",\"age\":11}]}";

	{
		cJSON *root = cJSON_Parse(s);
		if (!root) {
			printf("get root faild !\n");
			return -1;
		}
		cJSON *js_list = cJSON_GetObjectItem(root, "list");
		if (!js_list){
			printf("no list!\n");
			return -1;
		}
		int array_size = cJSON_GetArraySize(js_list);
		printf("array size is %d\n", array_size);
		int i = 0;
		cJSON *item, *js_name, *js_age;
		char *p = NULL;
		for (i = 0; i < array_size; i++) {
			item = cJSON_GetArrayItem(js_list, i);
			if (item) {
				//TODO...
				js_name = cJSON_GetObjectItem(item, "name");
				printf("name is %s\n", js_name->valuestring);
				js_age = cJSON_GetObjectItem(item, "age");
				printf("age is %d\n", js_age->valueint);
			}
		}

		if (root)
			cJSON_Delete(root);
	}

	{
		CJsonHelper jsonHelper;
		if ( !jsonHelper.parse(s) ) {
			printf(" json helper parse failed.\n");
			return -1;
		}

		CJsonHelper* plist = jsonHelper["list"];
		int nlistSize = plist->getArraySize();
		printf("array size is %d\n", nlistSize);
		for (int i = 0; i < nlistSize; ++i)
		{
			CJsonHelper* plistItem = (*plist)[i];
			string strName;
			plistItem->get( "name", strName );
			printf( "name is %s\n", strName.c_str() );
			int nAge;
			plistItem->get("age", nAge);
			printf("age is %d\n", nAge);
		}
	}

	{
		CJsonHelper jsonRootHelper;
		{
			//READ JSON FILE;
			char* szJsonFileName = "20181218.js";
			FILE* fpJson = fopen(szJsonFileName, "r");
			if ( NULL == fpJson )
			{
				printf("fopen failed %s\n", szJsonFileName);
				return -1;
			}
			fseek(fpJson, 0, SEEK_END);
			long lLen = ftell(fpJson);
			fseek(fpJson, 9, SEEK_SET);

			char* pszJsonBuf = (char*)malloc(lLen+1);
			fread( pszJsonBuf, 1, lLen-9, fpJson );
			pszJsonBuf[lLen-9] = '\0';

			bool bParse = jsonRootHelper.parse(pszJsonBuf);
			if (!bParse)
			{
				free(pszJsonBuf);
				pszJsonBuf = NULL;
				fclose(fpJson);
				printf("pase json failed, jsonfile:[%s].\n", szJsonFileName);
				return -1;
			}

			free(pszJsonBuf);
			pszJsonBuf = NULL;
			fclose(fpJson);
		}

		int id = -1;

		int nArraySize = jsonRootHelper.getArraySize();
		for (int i = 0; i < nArraySize; ++i)
		{
			CJsonHelper* pJsonChildHelper = jsonRootHelper[i];

			pJsonChildHelper->get("id", id);
			printf(" %d get id :[%d].\n", i, id);

			CJsonHelper* pContentJsonHelper = (*pJsonChildHelper)["content"];
			for (int j = 0; j < pContentJsonHelper->getArraySize(); ++j)
			{
				CJsonHelper* pContenChildJsonHelper = (*pContentJsonHelper)[j];
				CJsonHelper* pTableJsonHelper = (*pContenChildJsonHelper)["table"];
				CJsonHelper* pTrJsonHelper = (*pTableJsonHelper)["tr"];
				int nTrSize = pTrJsonHelper->getArraySize();
				for (int k = 2; k < nTrSize; ++k)
				{
					CJsonHelper* pTrChildHelper = (*pTrJsonHelper)[k];
					CJsonHelper* pTdJsonHelper = (*pTrChildHelper)["td"];
					int nTdSize = pTdJsonHelper->getArraySize();
					CJsonHelper* pTdChildJsonHelper = (*pTdJsonHelper)[0];

					ShortSellBaseData shortSellBaseData;
					string strValue;

					pTdChildJsonHelper->get(1, strValue);
					shortSellBaseData.nStockCode = atoi(strValue.c_str());
					pTdChildJsonHelper->get(2, shortSellBaseData.stockName);
					pTdChildJsonHelper->get(2, shortSellBaseData.szStockName, STOCKNAME_LEN);
					pTdChildJsonHelper->get(3, strValue);
					shortSellBaseData.nTotalNum = atoi(strValue.c_str());

					string strGbk = UTF8ToGBK(shortSellBaseData.stockName);

					string strCodeName;
				}
			}
		}
	}

	//json的写入文件的例子;
	writeResultConfig();

	system("pause");
	return 0;
}