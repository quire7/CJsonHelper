/********************************************************************
created:	2014/10/01
filename: 	CJsonHelper.h
file base:	CJsonHelper
purpose:	cJSON library c++ encapsulation;
author:		fanxiangdong;
mail:		fanxiangdong7@126.com;
qq:			435337751;
*********************************************************************/
#ifndef __CJSON_HELPER_H__
#define __CJSON_HELPER_H__

#include <string>
#include <map>
using std::string;
using std::map;

// Define integer types with known size: int64_t, uint64_t.
// If this doesn't work then insert compiler-specific definitions here:
#if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1600)
	// Compilers supporting C99 or C++0x have stdint.h defining these integer types
	#include <stdint.h>
	#include <inttypes.h>
	#define __STDC_FORMAT_MACROS
#elif defined(_MSC_VER)
	typedef   signed __int64  int64_t;
	typedef unsigned __int64 uint64_t;
#else
	typedef long long int64_t;
	typedef unsigned long long uint64_t;
#endif



struct cJSON;
class CJsonHelper
{
private:
	typedef map<string, CJsonHelper*>			CJsonHelperObjRefMap;
	typedef CJsonHelperObjRefMap::iterator		CJsonHelperObjRefMapIter;
	typedef map<unsigned int, CJsonHelper*>		CJsonHelperArrayRefMap;
	typedef CJsonHelperArrayRefMap::iterator	CJsonHelperArrayRefMapIter;
private:
	bool			m_bRoot;
	cJSON*			m_pJsonData;
	char*			m_pszErrorMsg;

	CJsonHelperObjRefMap	m_CJsonHelperObjRefMap;
	CJsonHelperArrayRefMap	m_CJsonHelperArrayRefMap;

public:
	CJsonHelper();
	CJsonHelper( const CJsonHelper& refOther );
	~CJsonHelper();


	CJsonHelper& operator=( const CJsonHelper& refOther );
	bool operator == ( const CJsonHelper& refOther ) const;

	bool parse( const char* szJson );
	void clear();
	bool isEmpty() const;
	bool isArray() const;
	bool isObject() const;

	const char* getError() const { return m_pszErrorMsg; };

	string toString() const;
	string toFormattedString() const;

public:// method of ordinary json;
	bool addEmptySubObject( const char* szKey );
	bool addEmptySubArray( const char* szKey );

	CJsonHelper* operator[]( const char* szKey );
	string operator()( const char* szKey ) const;

	//get method of json object;
	bool get( const char* szKey, CJsonHelper** pCJsonHelperAddr );
	bool get( const char* szKey, string& strValue ) const;
	bool get( const char* szKey, char* szValue, int nBufLen ) const;
	bool get( const char* szKey, bool& bValue ) const;
	bool get( const char* szKey, int& nValue ) const;
	bool get( const char* szKey, float& fValue ) const;
	bool get( const char* szKey, double& dValue ) const;
	bool get( const char* szKey, int64_t& llValue ) const;
	bool get( const char* szKey, uint64_t& ullValue ) const;

	//set method of json object;
	bool set( const char* szKey, const CJsonHelper* pCJsonHelper );
	bool set( const char* szKey, const char* szValue );
	bool set( const char* szKey, bool bValue );
	bool set( const char* szKey, int nValue );
	bool set( const char* szKey, float fValue );
	bool set( const char* szKey, double dValue );
	bool set( const char* szKey, int64_t llValue );
	bool set( const char* szKey, uint64_t ullValue );

	//delete method of json object;
	bool del( const char* szKey );

public:// method of json array;
	int getArraySize() const;
	CJsonHelper* operator[]( int uiWhich );
	string operator()( int uiWhich ) const;

	//get method of json array;
	bool get( int uiWhich, CJsonHelper** pCJsonHelperAddr );
	bool get( int uiWhich, string& strValue ) const;
	bool get( int uiWhich, char* szValue, int nBufLen ) const;
	bool get( int uiWhich, bool& bValue ) const;
	bool get( int uiWhich, int& nValue ) const;
	bool get( int uiWhich, float& fValue ) const;
	bool get( int uiWhich, double& dValue ) const;
	bool get( int uiWhich, int64_t& llValue ) const;
	bool get( int uiWhich, uint64_t& ullValue ) const;

	//set method of json array(set method must have uiWhich item);
	bool set( int uiWhich, const CJsonHelper* pCJsonHelper );
	bool set( int uiWhich, const char* szValue );
	bool set( int uiWhich, bool bValue );
	bool set( int uiWhich, int nValue );
	bool set( int uiWhich, float fValue );
	bool set( int uiWhich, double dValue );
	bool set( int uiWhich, int64_t llValue );
	bool set( int uiWhich, uint64_t ullValue );

	//delete method of json array;
	bool del( int uiWhich );

	//append to json array end;
	bool append( const CJsonHelper* pCJsonHelper );
	bool append( const char* szValue );
	bool append( bool bValue );
	bool append( int nValue );
	bool append( float fValue );
	bool append( double dValue );
	bool append( int64_t llValue );
	bool append( uint64_t ullValue );

private:
	void allocErrorMsg();
	bool isVaild( const char* szStr ) const;

	//object method;
	bool setCommon( const char* szKey, cJSON* pItem );
	bool setCommonNumber( const char* szKey, double num );

	bool get( cJSON* pItem, bool& bValue ) const;
	bool get( cJSON* pItem, int& nValue ) const;
	bool get( cJSON* pItem, float& fValue ) const;
	bool get( cJSON* pItem, double& dValue ) const;
	bool get( cJSON* pItem, int64_t& llValue ) const;
	bool get( cJSON* pItem, uint64_t& ullValue ) const;

	//array method;
	bool isArrayItem( int uiWhich ) const;
	bool setCommon( int uiWhich, cJSON* pItem );
	bool setCommonNumber( int uiWhich, double num );

	//append to json array;
	bool appendCommon( cJSON* pItem );
	bool appendCommonNumber( double num );

	explicit CJsonHelper(cJSON* pJsonData);
};


#endif