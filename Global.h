#pragma once

// Standard include file
// Contents:
// Includes C/C++ libraries, STL libraries, IOStream and String libraries
// Includes the most important boost headers
// Defines the import + export, override and exception handling macros
// Defines the vstd library
// Includes the logger

// This file shouldn't be changed, except if there is a important header file missing which is shared among several projects.

/*
 * Global.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <cstdio>
#include <stdio.h>
#ifdef _WIN32
#include <tchar.h>
#else
#include "tchar_amigaos4.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
//#include <unordered_map>
#include <utility>
#include <vector>

//The only available version is 3, as of Boost 1.50
#include <boost/version.hpp>

#define BOOST_FILESYSTEM_VERSION 3
#if ( BOOST_VERSION>105000 )
#define BOOST_THREAD_VERSION 3
#endif
#define BOOST_THREAD_DONT_PROVIDE_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE 1
//#define BOOST_SYSTEM_NO_DEPRECATED 1

#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

#ifdef ANDROID
#include <android/log.h>
#endif

// Integral data types
typedef boost::uint64_t ui64; //unsigned int 64 bits (8 bytes)
typedef boost::uint32_t ui32;  //unsigned int 32 bits (4 bytes)
typedef boost::uint16_t ui16; //unsigned int 16 bits (2 bytes)
typedef boost::uint8_t ui8; //unsigned int 8 bits (1 byte)
typedef boost::int64_t si64; //signed int 64 bits (8 bytes)
typedef boost::int32_t si32; //signed int 32 bits (4 bytes)
typedef boost::int16_t si16; //signed int 16 bits (2 bytes)
typedef boost::int8_t si8; //signed int 8 bits (1 byte)

#if defined _M_X64 && defined _WIN32 //Win64 -> cannot load 32-bit DLLs for video handling
	#define DISABLE_VIDEO
#endif

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ )
#endif

// Import + Export macro declarations
#ifdef _WIN32
#ifdef __GNUC__
#define DLL_EXPORT __attribute__((dllexport))
#else
#define DLL_EXPORT __declspec(dllexport)
#endif
#else
#if defined(__GNUC__) && GCC_VERSION >= 400
#define DLL_EXPORT	__attribute__ ((visibility("default")))
#else
#define DLL_EXPORT
#endif
#endif

#ifdef _WIN32
#ifdef __GNUC__
#define DLL_IMPORT __attribute__((dllimport))
#else
#define DLL_IMPORT __declspec(dllimport)
#endif
#else
#if defined(__GNUC__) && GCC_VERSION >= 400
#define DLL_IMPORT	__attribute__ ((visibility("default")))
#else
#define DLL_IMPORT
#endif
#endif

#ifdef VCMI_DLL
#define DLL_LINKAGE DLL_EXPORT
#else
#define DLL_LINKAGE DLL_IMPORT
#endif

//defining available c++11 features

//initialization lists - only gcc-4.4 or later
#if defined(__clang__) || (defined(__GNUC__) && (GCC_VERSION >= 404))
#define CPP11_USE_INITIALIZERS_LIST
#endif

//nullptr -  only msvc and gcc-4.6 or later, othervice define it  as NULL
#if !defined(_MSC_VER) && !(defined(__GNUC__) && (GCC_VERSION >= 406))
#define nullptr NULL
#endif

//override keyword - only msvc and gcc-4.7 or later.
#if !defined(_MSC_VER) && !(defined(__GNUC__) && (GCC_VERSION >= 407))
#define override
#endif

//workaround to support existing code
#define OVERRIDE override

//a normal std::map with a const operator[] for sanity
template<typename KeyT, typename ValT>
class bmap : public std::map<KeyT, ValT>
{
public:
	const ValT & operator[](KeyT key) const
	{
		return this->find(key)->second;
	}
	ValT & operator[](KeyT key)
	{
		return static_cast<std::map<KeyT, ValT> &>(*this)[key];
	}
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<std::map<KeyT, ValT> &>(*this);
	}
};

namespace vstd
{
	//returns true if container c contains item i
	template <typename Container, typename Item>
	bool contains(const Container & c, const Item &i)
	{
		return std::find(boost::begin(c), boost::end(c),i) != boost::end(c);
	}

	//returns true if container c contains item i
	template <typename Container, typename Pred>
	bool contains_if(const Container & c, Pred p)
	{
		return std::find_if(boost::begin(c), boost::end(c), p) != boost::end(c);
	}

	//returns true if map c contains item i
	template <typename V, typename Item, typename Item2>
	bool contains(const std::map<Item,V> & c, const Item2 &i)
	{
		return c.find(i)!=c.end();
	}

	//returns true if bmap c contains item i
	template <typename V, typename Item, typename Item2>
	bool contains(const bmap<Item,V> & c, const Item2 &i)
	{
		return c.find(i)!=c.end();
	}

	//returns true if unordered set c contains item i
	template <typename Item>
	bool contains(const boost::unordered_set<Item> & c, const Item &i)
	{
		return c.find(i)!=c.end();
	}

	//returns position of first element in vector c equal to s, if there is no such element, -1 is returned
	template <typename Container, typename T2>
	int find_pos(const Container & c, const T2 &s)
	{
		size_t i=0;
		for (auto iter = boost::begin(c); iter != boost::end(c); iter++, i++)
			if(*iter == s)
				return i;
		return -1;
	}

	//Func(T1,T2) must say if these elements matches
	template <typename T1, typename T2, typename Func>
	int find_pos(const std::vector<T1> & c, const T2 &s, const Func &f)
	{
		for(size_t i=0; i < c.size(); ++i)
			if(f(c[i],s))
				return i;
		return -1;
	}

	//returns iterator to the given element if present in container, end() if not
	template <typename Container, typename Item>
	typename Container::iterator find(Container & c, const Item &i)
	{
		return std::find(c.begin(),c.end(),i);
	}

	//returns const iterator to the given element if present in container, end() if not
	template <typename Container, typename Item>
	typename Container::const_iterator find(const Container & c, const Item &i)
	{
		return std::find(c.begin(),c.end(),i);
	}

	//removes element i from container c, returns false if c does not contain i
	template <typename Container, typename Item>
	typename Container::size_type operator-=(Container &c, const Item &i)
	{
		typename Container::iterator itr = find(c,i);
		if(itr == c.end())
			return false;
		c.erase(itr);
		return true;
	}

	//assigns greater of (a, b) to a and returns maximum of (a, b)
	template <typename t1, typename t2>
	t1 &amax(t1 &a, const t2 &b)
	{
		if(a >= b)
			return a;
		else
		{
			a = b;
			return a;
		}
	}

	//assigns smaller of (a, b) to a and returns minimum of (a, b)
	template <typename t1, typename t2>
	t1 &amin(t1 &a, const t2 &b)
	{
		if(a <= b)
			return a;
		else
		{
			a = b;
			return a;
		}
	}

	//makes a to fit the range <b, c>
	template <typename t1, typename t2, typename t3>
	t1 &abetween(t1 &a, const t2 &b, const t3 &c)
	{
		amax(a,b);
		amin(a,c);
		return a;
	}

	//checks if a is between b and c
	template <typename t1, typename t2, typename t3>
	bool isbetween(const t1 &value, const t2 &min, const t3 &max)
	{
		return value > min && value < max;
	}

	//checks if a is within b and c
	template <typename t1, typename t2, typename t3>
	bool iswithin(const t1 &value, const t2 &min, const t3 &max)
	{
		return value >= min && value <= max;
	}

	template <typename t1, typename t2>
	struct assigner
	{
	public:
		t1 &op1;
		t2 op2;
		assigner(t1 &a1, const t2 & a2)
			:op1(a1), op2(a2)
		{}
		void operator()()
		{
			op1 = op2;
		}
	};

	// Assigns value a2 to a1. The point of time of the real operation can be controlled
	// with the () operator.
	template <typename t1, typename t2>
	assigner<t1,t2> assigno(t1 &a1, const t2 &a2)
	{
		return assigner<t1,t2>(a1,a2);
	}

	//deleted pointer and sets it to NULL
	template <typename T>
	void clear_pointer(T* &ptr)
	{
		delete ptr;
		ptr = NULL;
	}

	template<typename T>
	std::unique_ptr<T> make_unique()
	{
		return std::unique_ptr<T>(new T());
	}
	template<typename T, typename Arg1>
	std::unique_ptr<T> make_unique(Arg1 &&arg1)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1)));
	}
	template<typename T, typename Arg1, typename Arg2>
	std::unique_ptr<T> make_unique(Arg1 &&arg1, Arg2 &&arg2)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)));
	}

	template <typename Container>
	typename Container::const_reference circularAt(const Container &r, size_t index)
	{
		assert(r.size());
		index %= r.size();
		auto itr = boost::begin(r);
		std::advance(itr, index);
		return *itr;
	}

	template<typename Range, typename Predicate>
	void erase_if(Range &vec, Predicate pred)
	{
		vec.erase(boost::remove_if(vec, pred),vec.end());
	}

	template<typename InputRange, typename OutputIterator, typename Predicate>
	OutputIterator copy_if(const InputRange &input, OutputIterator result, Predicate pred)
	{
		return std::copy_if(boost::const_begin(input), boost::end(input), result, pred);
	}

	template <typename Container>
	std::insert_iterator<Container> set_inserter(Container &c)
	{
		return std::inserter(c, c.end());
	}

	//Retuns iterator to the element for which the value of ValueFunction is minimal
	template<class ForwardRange, class ValueFunction>
	auto minElementByFun(const ForwardRange& rng, ValueFunction vf) -> decltype(boost::begin(rng))
	{
		typedef decltype(*boost::begin(rng)) ElemType;
		return boost::min_element(rng, [&] (ElemType lhs, ElemType rhs) -> bool
		{
			return vf(lhs) < vf(rhs);
		});
	}
}

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using vstd::make_unique;

using vstd::operator-=;

namespace range = boost::range;

// can be used for counting arrays
template<typename T, size_t N> char (&_ArrayCountObj(const T (&)[N]))[N];
#define ARRAY_COUNT(arr)    (sizeof(_ArrayCountObj(arr)))


#define THROW_FORMAT(message, formatting_elems)  throw std::runtime_error(boost::str(boost::format(message) % formatting_elems))

//XXX pls dont - 'debug macros' are usually more trouble than it's worth
#define HANDLE_EXCEPTION  \
	catch (const std::exception& e) {	\
	tlog1 << e.what() << std::endl;		\
	throw;								\
}									\
	catch (const std::exception * e)	\
{									\
	tlog1 << e->what()<< std::endl;	\
	throw;							\
}									\
	catch (const std::string& e) {		\
	tlog1 << e << std::endl;		\
	throw;							\
}

#define HANDLE_EXCEPTIONC(COMMAND)  \
	catch (const std::exception& e) {	\
	COMMAND;						\
	tlog1 << e.what() << std::endl;	\
	throw;							\
}									\
	catch (const std::string &e)	\
{									\
	COMMAND;						\
	tlog1 << e << std::endl;	\
	throw;							\
}


#include "lib/CLogger.h"
