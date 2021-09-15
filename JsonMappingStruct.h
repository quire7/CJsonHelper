/********************************************************************
created:	2015/12/01
filename: 	JsonMappingStruct.h
file base:	JsonMappingStruct
purpose:	json data mapping c++ struct;
author:		fanxiangdong;
mail:		fanxiangdong7@126.com;
qq:			435337751;
*********************************************************************/
#ifndef __ND_SHARE_BASE_JSON_MAPPING_STRUCT_H__
#define __ND_SHARE_BASE_JSON_MAPPING_STRUCT_H__

#include <type_traits>

#if defined(_MSC_VER) && defined(_NOEXCEPT) && _MSC_VER < 1900
#ifndef constexpr
#define constexpr
#endif
#endif

#include "CJsonHelper.h"

//// linux c++11 used;
//template< class T >
//using decay_t = typename std::decay<T>::type;
//
//template< bool B, class T = void >
//using enable_if_t = typename std::enable_if<B, T>::type;


template<typename Class, typename T>
struct Property {
	constexpr Property(T Class::*aMember, const char* aName) : member{ aMember }, name{ aName } {}

	using Type = T;

	T Class::*member;
	const char* name;
};

template<std::size_t iteration, typename T>
bool doSetData(T&& object, const CJsonHelper* pCJsonData) {
	// get the property;
	//constexpr auto property = std::get<iteration>(std::decay_t<T>::properties); //this must save;
	auto property = std::get<iteration>(std::decay_t<T>::properties);

	// get the type of the property;
	using Type = typename decltype(property)::Type;

	// set the value to the member;
	//object.*(property.member) = asAny<Type>(data[property.name]);
	return pCJsonData->get(property.name, object.*(property.member));
}

template<std::size_t iteration, typename T, std::enable_if_t<(iteration > 0)>* = nullptr >
bool setData(T&& object, const CJsonHelper* pCJsonData) {
	if ( !doSetData<iteration>(object, pCJsonData) ) {
		return false;
	}
	// next iteration;
	return setData<iteration - 1>(object, pCJsonData);
}

template<std::size_t iteration, typename T, std::enable_if_t<(iteration == 0)>* = nullptr >
bool setData(T&& object, const CJsonHelper* pCJsonData) {
	return doSetData<iteration>(object, pCJsonData);
}

template<typename T>
T fromJson(const CJsonHelper* pCJsonData) {
	T object;

	setData<std::tuple_size<decltype(T::properties)>::value - 1>(object, pCJsonData);

	return object;
}

template<typename T>
T* fromJsonToPointer(const CJsonHelper* pCJsonData) {
	T* pObject = new T;

	if ( !setData<std::tuple_size<decltype(T::properties)>::value - 1>(*pObject, pCJsonData) ) {
		delete pObject;
		pObject = nullptr;
	}
	
	return pObject;
}

#endif


////LINUX MUST GCC 5 OR LARGE,WINDOWS MUST MSVC19.11;
//
//#include "CJsonHelper/JsonMappingStruct.h"
//
//struct User {
//public:
//	std::string username;
//	std::string info;
//
//	constexpr static auto properties = std::make_tuple(
//		Property<User, std::string>(&User::username, "username"),
//		Property<User, std::string>(&User::info, "info")
//		);
//};
//
//{
//	CJsonHelper jsonHelper;
//	jsonHelper.set("username", "blur.apk");
//	jsonHelper.set("info", "data");
//
//	User u = fromJson<User>(&jsonHelper);
//	printf("username:%s.\n", u.username.c_str());
//	printf("username:%s.\n", u.info.c_str());
//}
