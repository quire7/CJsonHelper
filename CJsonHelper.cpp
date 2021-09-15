#include "CJsonHelper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cJSON/cJSON.h"

#define CJsonHelper_ErrorMsg_Len	(256)

#ifndef SNPRINTF
#if defined(_MSC_VER) && (_MSC_VER >= 1200 )
#define SNPRINTF		_snprintf
#elif defined(__GNUC__) && (__GNUC__ >= 3 )
#define SNPRINTF		snprintf
#else
#define SNPRINTF		snprintf
#endif
#endif // !SNPRINTF


CJsonHelper::CJsonHelper():
m_bRoot(true),
m_pJsonData(NULL),
m_pszErrorMsg(NULL)
{
}

CJsonHelper::CJsonHelper( const CJsonHelper& refOther ) :
m_bRoot(true),
m_pJsonData(NULL),
m_pszErrorMsg(NULL)
{
	char* szOtherJsonData = cJSON_PrintUnformatted( refOther.m_pJsonData );
	parse( szOtherJsonData );
	cJSON_free( szOtherJsonData );
}

CJsonHelper::CJsonHelper( cJSON* pJsonData ):
m_bRoot(false),
m_pJsonData(pJsonData),
m_pszErrorMsg(NULL)
{

}

CJsonHelper::~CJsonHelper()
{
	clear();
	if ( NULL != m_pszErrorMsg ) 
	{
		free(m_pszErrorMsg);
		m_pszErrorMsg = NULL;
	}
}

CJsonHelper& CJsonHelper::operator=(const CJsonHelper& refOther)
{
	if ( this != &refOther )
	{
		char* szOtherJsonData = cJSON_PrintUnformatted(refOther.m_pJsonData);
		parse( szOtherJsonData );
		cJSON_free( szOtherJsonData );
	}
	return *this;
}
bool CJsonHelper::operator==(const CJsonHelper& refOther) const
{
	if ( this == &refOther )				return true;
	if ( isEmpty() && refOther.isEmpty() )	return true;
	if ( isEmpty() || refOther.isEmpty() )	return false;

	return cJSON_Compare(m_pJsonData, refOther.m_pJsonData, true );
}

bool CJsonHelper::parse(const char* szJson)
{
	clear();
	m_pJsonData = cJSON_Parse( szJson );
	if ( NULL == m_pJsonData ) 
	{
		allocErrorMsg();
		SNPRINTF( m_pszErrorMsg, CJsonHelper_ErrorMsg_Len - 1, " prase json string error at:[%s].", cJSON_GetErrorPtr() );
		return false;
	}
	return true;
}

void CJsonHelper::clear()
{
	if ( m_bRoot && (NULL != m_pJsonData) )
	{
		cJSON_Delete( m_pJsonData );
		m_pJsonData = NULL;
	}
	for (CJsonHelperObjRefMapIter	iter = m_CJsonHelperObjRefMap.begin();
									iter != m_CJsonHelperObjRefMap.end();
									++iter )
	{
		if ( NULL != iter->second )
		{
			delete iter->second;
			iter->second = NULL;
		}
	}

	for (CJsonHelperArrayRefMapIter iter = m_CJsonHelperArrayRefMap.begin();
									iter != m_CJsonHelperArrayRefMap.end();
									++iter)
	{
		if (NULL != iter->second)
		{
			delete iter->second;
			iter->second = NULL;
		}
	}

	m_CJsonHelperObjRefMap.clear();
	m_CJsonHelperArrayRefMap.clear();
}

bool CJsonHelper::isEmpty() const
{
	return ( NULL == m_pJsonData );
}

bool CJsonHelper::isArray() const
{
	return cJSON_IsArray( m_pJsonData );
}

bool CJsonHelper::isObject() const
{
	return cJSON_IsObject( m_pJsonData );
}

string CJsonHelper::toString() const
{
	if ( isEmpty() )	return string("");
	
	char* szJsonString = cJSON_PrintUnformatted( m_pJsonData );
	string strJsonData( szJsonString );
	cJSON_free(szJsonString);
	return strJsonData;
}

string CJsonHelper::toFormattedString() const
{
	if (isEmpty())		return string("");

	char* szJsonString = cJSON_Print(m_pJsonData);
	string strJsonData(szJsonString);
	cJSON_free(szJsonString);
	return strJsonData;
}

////////////////////////////////////JSON object method start////////////////////////////////////////////////

bool CJsonHelper::addEmptySubObject(const char* szKey)
{
	if ( !isVaild(szKey) )	return false;

	if (NULL == m_pJsonData)
		m_pJsonData = cJSON_CreateObject();
	
	if (!isObject())
	{
		allocErrorMsg();
		SNPRINTF(m_pszErrorMsg, CJsonHelper_ErrorMsg_Len - 1, " CJsonHelper::addEmptySubObject m_pJsonData not a json object.");
		return false;
	}

	cJSON* pJsonItem = cJSON_AddObjectToObject( m_pJsonData, szKey ) ;
	if ( NULL == pJsonItem )
	{
		allocErrorMsg();
		SNPRINTF(m_pszErrorMsg, CJsonHelper_ErrorMsg_Len - 1, " CJsonHelper::addEmptySubObject cJSON_AddObjectToObject failed.");
		return false;
	}
	return true;
}

bool CJsonHelper::addEmptySubArray(const char* szKey)
{
	if ( !isVaild(szKey) )	return false;

	if (NULL == m_pJsonData)
		m_pJsonData = cJSON_CreateObject();
	
	if (!isObject())
	{
		allocErrorMsg();
		SNPRINTF(m_pszErrorMsg, CJsonHelper_ErrorMsg_Len - 1, " CJsonHelper::addEmptySubArray m_pJsonData not a json object.");
		return false;
	}
	cJSON* pJsonItem = cJSON_AddArrayToObject( m_pJsonData, szKey );
	if ( NULL == pJsonItem )
	{
		allocErrorMsg();
		SNPRINTF(m_pszErrorMsg, CJsonHelper_ErrorMsg_Len - 1, " CJsonHelper::addEmptySubArray cJSON_AddArrayToObject failed.");
		return false;
	}
	return true;
}

CJsonHelper* CJsonHelper::operator[](const char* szKey)
{
	if ( !isVaild(szKey) )	return NULL;

	CJsonHelperObjRefMapIter iter = m_CJsonHelperObjRefMap.find( szKey );
	if ( iter != m_CJsonHelperObjRefMap.end() )
		return (iter->second);
	
	if (isEmpty())
		return NULL; //addEmptySubObject( szKey );
	
	if ( !isObject() )
		return NULL;

	CJsonHelper* pJsonHelper = NULL;
	cJSON *pJsonData = cJSON_GetObjectItem( m_pJsonData, szKey );
	if ( pJsonData ) {
		pJsonHelper = new CJsonHelper(pJsonData);
		m_CJsonHelperObjRefMap.insert(std::make_pair(szKey, pJsonHelper));
	}

	return pJsonHelper;
}

string CJsonHelper::operator()(const char* szKey) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return string("");

	cJSON* pJsonItem = cJSON_GetObjectItem( m_pJsonData, szKey );
	if ( NULL == pJsonItem ) return string("");

	if ( cJSON_IsString(pJsonItem) )			return pJsonItem->valuestring;
	else if ( cJSON_IsFalse(pJsonItem) )		return string("false");
	else if ( cJSON_IsTrue(pJsonItem) )			return string("true");
	else if ( cJSON_IsNumber(pJsonItem) )
	{
		char szBuf[64] = {0};
		SNPRINTF( szBuf, sizeof(szBuf)-1, "%g", pJsonItem->valuedouble );
		return szBuf;
	}

	return string("");
}


bool CJsonHelper::get(const char* szKey, CJsonHelper** pCJsonHelperAddr)
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return ( NULL != ( *pCJsonHelperAddr = (*this)[szKey] ) );
}

bool CJsonHelper::get(const char* szKey, string& strValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	strValue = (*this)(szKey);

	return (!strValue.empty());
}

bool CJsonHelper::get(const char* szKey, char* szValue, int nBufLen) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;
	if ( !isVaild(szValue) || nBufLen <= 0 )		return false;

	string strValue = (*this)(szKey);

	int nstrValueLen = strValue.length();
	if ( nstrValueLen >= nBufLen )
		return false;
	
	memcpy( szValue, strValue.c_str(), nstrValueLen );
	szValue[nstrValueLen] = '\0';

	return true;
}

bool CJsonHelper::get(const char* szKey, bool& bValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), bValue );
}

bool CJsonHelper::get(const char* szKey, int& nValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), nValue );
}

bool CJsonHelper::get(const char* szKey, float& fValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), fValue );
}

bool CJsonHelper::get(const char* szKey, double& dValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), dValue );
}

bool CJsonHelper::get(const char* szKey, int64_t& llValue) const
{
	if ( ( !isVaild(szKey) ) || ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), llValue );
}

bool CJsonHelper::get(const char* szKey, uint64_t& ullValue) const
{
	if ( ( !isVaild(szKey) ) ||  ( !isObject() ) )	return false;

	return get( cJSON_GetObjectItem(m_pJsonData, szKey), ullValue );
}

bool CJsonHelper::set( const char* szKey, const CJsonHelper* pCJsonHelper )
{
	if ( ( !isVaild( szKey ) ) || ( NULL == pCJsonHelper ) || pCJsonHelper->isEmpty() )	return false;

	char* szAddJsonData = cJSON_PrintUnformatted( pCJsonHelper->m_pJsonData );
	cJSON* pJsonItem	= cJSON_Parse(szAddJsonData);
	cJSON_free(szAddJsonData);
	
	if ( !setCommon( szKey, pJsonItem ) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}

	return true;
}

bool CJsonHelper::set(const char* szKey, const char* szValue)
{
	if ( ( !isVaild(szKey) ) || ( !isVaild(szValue) ) ) 	return false;

	cJSON* pJsonItem = cJSON_CreateString(szValue);
	if ( !setCommon( szKey, pJsonItem ) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::set(const char* szKey, bool bValue)
{
	if ( !isVaild(szKey) ) 	return false;

	cJSON* pJsonItem = cJSON_CreateBool( bValue );
	if (!setCommon(szKey, pJsonItem))
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::set(const char* szKey, int nValue)
{
	return setCommonNumber( szKey, nValue );
}

bool CJsonHelper::set(const char* szKey, float fValue)
{
	return setCommonNumber( szKey, fValue );
}

bool CJsonHelper::set(const char* szKey, double dValue)
{
	return setCommonNumber( szKey, dValue );
}

bool CJsonHelper::set(const char* szKey, int64_t llValue)
{
	if ( !isVaild(szKey) ) 	return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%lld", llValue);

	return set( szKey, szBuf );
}

bool CJsonHelper::set(const char* szKey, uint64_t ullValue)
{
	if ( !isVaild(szKey) ) 	return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%llu", ullValue);

	return set( szKey, szBuf );
}

bool CJsonHelper::del(const char* szKey)
{
	if ( !isVaild(szKey) ) 	return false;
	if ( !isObject() )		return false;

	cJSON_DeleteItemFromObject( m_pJsonData, szKey );

	CJsonHelperObjRefMapIter iterFind = m_CJsonHelperObjRefMap.find( szKey );
	if ( iterFind != m_CJsonHelperObjRefMap.end() )
	{
		if ( NULL != iterFind->second )
		{
			delete iterFind->second;
			iterFind->second = NULL;
		}
		m_CJsonHelperObjRefMap.erase( iterFind );
	}

	return true;
}

///////////////////////////////////////JSON array method start//////////////////////////////////////////////////

int CJsonHelper::getArraySize() const
{
	if ( !isArray() )	return 0;

	return ( cJSON_GetArraySize(m_pJsonData) );
}


CJsonHelper* CJsonHelper::operator[]( int uiWhich )
{
	CJsonHelperArrayRefMapIter iterFind = m_CJsonHelperArrayRefMap.find(uiWhich);
	if (iterFind != m_CJsonHelperArrayRefMap.end())
		return iterFind->second;

	if (isEmpty())
		return NULL; //m_pJsonData = cJSON_CreateArray();

	if ( !isArrayItem(uiWhich) )
		return NULL;
	
	CJsonHelper* pJsonHelper = new CJsonHelper( cJSON_GetArrayItem(m_pJsonData, uiWhich) );
	m_CJsonHelperArrayRefMap.insert( std::make_pair( uiWhich, pJsonHelper ) );

	return pJsonHelper;
}

string CJsonHelper::operator()( int uiWhich ) const
{
	if ( !isArrayItem(uiWhich) )	return string("");

	cJSON* pJsonItem = cJSON_GetArrayItem( m_pJsonData, uiWhich );
	if ( NULL == pJsonItem ) return string("");

	if ( cJSON_IsString(pJsonItem) )			return pJsonItem->valuestring;
	else if ( cJSON_IsFalse(pJsonItem) )		return string("false");
	else if ( cJSON_IsTrue(pJsonItem) )			return string("true");
	else if ( cJSON_IsNumber(pJsonItem) )
	{
		char szBuf[64] = { 0 };
		SNPRINTF(szBuf, sizeof(szBuf)-1, "%g", pJsonItem->valuedouble);
		return szBuf;
	}

	return string("");
}

bool CJsonHelper::get(int uiWhich, CJsonHelper** pCJsonHelperAddr)
{
	if ( !isArrayItem(uiWhich) )	return false;

	*pCJsonHelperAddr = (*this)[uiWhich];

	return true;
}

bool CJsonHelper::get(int uiWhich, string& strValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	strValue = (*this)(uiWhich);

	return (!strValue.empty());
}

bool CJsonHelper::get(int uiWhich, char* szValue, int nBufLen) const
{
	if ( !isArrayItem(uiWhich) )	return false;
	if ( !isVaild(szValue) || nBufLen <= 0 ) return false;

	string strValue = (*this)(uiWhich);

	int nstrValueLen = strValue.length();
	if ( nstrValueLen >= nBufLen )
		return false;

	memcpy( szValue, strValue.c_str(), nstrValueLen );
	szValue[nstrValueLen] = '\0';

	return true;
}

bool CJsonHelper::get(int uiWhich, bool& bValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), bValue );
}

bool CJsonHelper::get(int uiWhich, int& nValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), nValue );
}

bool CJsonHelper::get(int uiWhich, float& fValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), fValue );
}

bool CJsonHelper::get(int uiWhich, double& dValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), dValue );
}

bool CJsonHelper::get(int uiWhich, int64_t& llValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), llValue );
}

bool CJsonHelper::get(int uiWhich, uint64_t& ullValue) const
{
	if ( !isArrayItem(uiWhich) )	return false;

	return get( cJSON_GetArrayItem(m_pJsonData, uiWhich), ullValue );
}

bool CJsonHelper::set(int uiWhich, const CJsonHelper* pCJsonHelper)
{
	if ( !isArrayItem(uiWhich) )	return false;

	char* szAddJsonData = cJSON_PrintUnformatted(pCJsonHelper->m_pJsonData);
	cJSON* pJsonItem	= cJSON_Parse(szAddJsonData);
	cJSON_free( szAddJsonData );

	if ( !setCommon(uiWhich, pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::set(int uiWhich, const char* szValue)
{
	if ( !isArrayItem(uiWhich) )	return false;
	if ( !isVaild(szValue) )		return false;

	cJSON* pJsonItem = cJSON_CreateString(szValue);
	if ( !setCommon(uiWhich, pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::set(int uiWhich, bool bValue)
{
	if ( !isArrayItem(uiWhich) )	return false;

	cJSON* pJsonItem = cJSON_CreateBool(bValue);
	if ( !setCommon(uiWhich, pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::set(int uiWhich, int nValue)
{
	return setCommonNumber( uiWhich, nValue );
}

bool CJsonHelper::set(int uiWhich, float fValue)
{
	return setCommonNumber( uiWhich, fValue );
}

bool CJsonHelper::set(int uiWhich, double dValue)
{
	return setCommonNumber( uiWhich, dValue );
}

bool CJsonHelper::set(int uiWhich, int64_t llValue)
{
	if ( !isArrayItem(uiWhich) )	return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%lld", llValue);

	return set( uiWhich, szBuf );
}

bool CJsonHelper::set(int uiWhich, uint64_t ullValue)
{
	if ( !isArrayItem(uiWhich) )	return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%llu", ullValue);

	return set(uiWhich, szBuf);
}

bool CJsonHelper::del(int uiWhich)
{
	if ( !isArrayItem(uiWhich) ) 	return false;

	cJSON_DeleteItemFromArray(m_pJsonData, uiWhich);

	CJsonHelperArrayRefMapIter iterFind = m_CJsonHelperArrayRefMap.find(uiWhich);
	if (iterFind != m_CJsonHelperArrayRefMap.end())
	{
		if (NULL != iterFind->second)
		{
			delete iterFind->second;
			iterFind->second = NULL;
		}
		m_CJsonHelperArrayRefMap.erase(iterFind);
	}

	return true;
}

bool CJsonHelper::append(const CJsonHelper* pCJsonHelper)
{
	if ( !isArray() ) return false;
	
	char* szAddJsonData = cJSON_PrintUnformatted(pCJsonHelper->m_pJsonData);
	cJSON* pJsonItem	= cJSON_Parse(szAddJsonData);
	cJSON_free( szAddJsonData );

	if ( !appendCommon(pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::append(const char* szValue)
{
	if (!isArray()) return false;

	cJSON* pJsonItem = cJSON_CreateString(szValue);
	if ( !appendCommon(pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::append(bool bValue)
{
	if (!isArray()) return false;

	cJSON* pJsonItem = cJSON_CreateBool(bValue);
	if ( !appendCommon(pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}

bool CJsonHelper::append(int nValue)
{
	return appendCommonNumber( nValue );
}

bool CJsonHelper::append(float fValue)
{
	return appendCommonNumber( fValue );
}

bool CJsonHelper::append(double dValue)
{
	return appendCommonNumber( dValue );
}

bool CJsonHelper::append(int64_t llValue)
{
	if ( !isArray() ) return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%lld", llValue);

	return append( szBuf );
}

bool CJsonHelper::append(uint64_t ullValue)
{
	if ( !isArray() ) return false;

	char szBuf[64] = { 0 };
	SNPRINTF(szBuf, sizeof(szBuf)-1, "%llu", ullValue);

	return append(szBuf);
}

///////////////////////////////////////////////////////////////////////////
void CJsonHelper::allocErrorMsg()
{
	if (NULL == m_pszErrorMsg) 
	{
		m_pszErrorMsg = (char*)malloc(CJsonHelper_ErrorMsg_Len);
	}
	memset( m_pszErrorMsg, 0, CJsonHelper_ErrorMsg_Len );
}

bool CJsonHelper::isVaild(const char* szStr) const
{
	return ( ( NULL != szStr ) && ( '\0' != szStr[0] ) );
}

bool CJsonHelper::setCommon(const char* szKey, cJSON* pItem)
{
	if ( !isVaild(szKey) || NULL == pItem )	return false;

	if ( NULL == m_pJsonData )
		m_pJsonData = cJSON_CreateObject();

	if ( !isObject() )	return false;

	cJSON* pFindJson = cJSON_GetObjectItem( m_pJsonData, szKey );
	if (pFindJson)
	{
		cJSON_ReplaceItemInObject( m_pJsonData, szKey, pItem );
	}
	else
	{
		cJSON_AddItemToObject( m_pJsonData, szKey, pItem );
	}

	pFindJson = cJSON_GetObjectItem( m_pJsonData, szKey );
	if ( NULL == pFindJson )	return false;
	
	CJsonHelperObjRefMapIter iterFind = m_CJsonHelperObjRefMap.find( szKey );
	if ( iterFind != m_CJsonHelperObjRefMap.end() )
	{
		if ( NULL != iterFind->second )
		{
			iterFind->second->m_pJsonData = pFindJson;
		}
	}

	return true;
}

bool CJsonHelper::setCommonNumber(const char* szKey, double num)
{
	if ( !isVaild(szKey) ) 	return false;

	cJSON* pJsonItem = cJSON_CreateNumber( num );
	if ( !setCommon( szKey, pJsonItem ) )
	{
		cJSON_Delete( pJsonItem );
		return false;
	}
	return true;
}

bool CJsonHelper::get(cJSON* pItem, bool& bValue) const
{
	if ( NULL == pItem )		return false;
	if ( !cJSON_IsBool(pItem) )	return false;

	bValue = cJSON_IsTrue(pItem);
	return true;
}

bool CJsonHelper::get(cJSON* pItem, int& nValue) const
{
	if ( NULL == pItem )			return false;
	if ( !cJSON_IsNumber(pItem) )	return false;

	nValue = pItem->valueint;
	return true;
}

bool CJsonHelper::get(cJSON* pItem, float& fValue) const
{
	if ( NULL == pItem )			return false;
	if ( !cJSON_IsNumber(pItem) )	return false;

	fValue = (float)pItem->valuedouble;
	return true;
}

bool CJsonHelper::get(cJSON* pItem, double& dValue) const
{
	if ( NULL == pItem )			return false;
	if ( !cJSON_IsNumber(pItem) )	return false;

	dValue = pItem->valuedouble;
	return true;
}

bool CJsonHelper::get(cJSON* pItem, int64_t& llValue) const
{
	if ( NULL == pItem )	return false;

	char* szValue = cJSON_GetStringValue( pItem );
	if ( NULL == szValue )	return false;

	sscanf(szValue, "%lld", &llValue);
	return true;
}

bool CJsonHelper::get(cJSON* pItem, uint64_t& ullValue) const
{
	if ( NULL == pItem )	return false;

	char* szValue = cJSON_GetStringValue( pItem );
	if ( NULL == szValue )	return false;

	sscanf(szValue, "%llu", &ullValue);
	return true;
}

bool CJsonHelper::isArrayItem(int uiWhich) const
{
	return ( ( uiWhich >= 0 ) && ( getArraySize() > uiWhich ) );
}

bool CJsonHelper::setCommon(int uiWhich, cJSON* pItem)
{
	if ( NULL == pItem )	return false;
	
	cJSON_ReplaceItemInArray( m_pJsonData, uiWhich, pItem );
	cJSON* pJsonFind = cJSON_GetArrayItem( m_pJsonData, uiWhich );
	if ( NULL == pJsonFind ) return false;

	CJsonHelperArrayRefMapIter iterFind = m_CJsonHelperArrayRefMap.find( uiWhich );
	if ( iterFind != m_CJsonHelperArrayRefMap.end() )
	{
		iterFind->second->m_pJsonData = pJsonFind;
	}

	return true;
}

bool CJsonHelper::setCommonNumber(int uiWhich, double num)
{
	if ( !isArrayItem(uiWhich) ) 	return false;

	cJSON* pJsonItem = cJSON_CreateNumber(num);
	if ( !setCommon(uiWhich, pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}


bool CJsonHelper::appendCommon(cJSON* pItem)
{
	if ( (NULL == pItem) || (!isArray()) )	return false;

	int iArraySizeBeforeAdd = cJSON_GetArraySize( m_pJsonData );
	cJSON_AddItemToArray( m_pJsonData, pItem );
	return ( iArraySizeBeforeAdd < cJSON_GetArraySize(m_pJsonData) );
}

bool CJsonHelper::appendCommonNumber(double num)
{
	if ( !isArray() )	return false;

	cJSON* pJsonItem = cJSON_CreateNumber(num);
	if ( !appendCommon(pJsonItem) )
	{
		cJSON_Delete(pJsonItem);
		return false;
	}
	return true;
}
