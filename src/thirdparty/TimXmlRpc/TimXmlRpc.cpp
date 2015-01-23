/*
XmlRpc C++ client for Windows
-----------------------------

Created by Dr Tim Cooper,  tco@smartsgroup.com,   March 2009.

This lets you talk to web sites using XmlRpc from C++ on Windows.  It differs
from similar libraries as follows:
	- works on Windows
	- supports HTTPS (SSL)
	- uses wininet to manage HTTP/HTTPS, so it's really very minimal
	- much faster than XmlRpc++ which suffers from STL performance problems.

This project consists of 2 files:   "TimXmlRpc.h"  &&  "TimXmlRpc.cpp".

Parts of this project have been taken from Chris Morley's "XmlRpc++" project,
in particular the API.  Chris's contribution is acknowledged by marking his
work with the token:  "/*ChrisMorley/".  Thanks, Chris!

*/
#include "stdafx.h"

#undef UNICODE
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iterator>
#include "TimXmlRpc.h"




static const char VALUE_TAG[]			= "<value>";
static const char VALUE_ETAG[]			= "</value>";

static const char BOOLEAN_TAG[]			= "<boolean>";
static const char BOOLEAN_ETAG[]		= "</boolean>";
static const char DOUBLE_TAG[]			= "<double>";
static const char DOUBLE_ETAG[]			= "</double>";
static const char INT_TAG[]				= "<int>";
static const char INT_ETAG[]			= "</int>";
static const char I4_TAG[]				= "<i4>";
static const char I4_ETAG[]				= "</i4>";
static const char STRING_TAG[]			= "<string>";
static const char STRING_ETAG[]			= "</string>";
static const char DATETIME_TAG[]		= "<dateTime.iso8601>";
static const char DATETIME_ETAG[]		= "</dateTime.iso8601>";
static const char BASE64_TAG[]			= "<base64>";
static const char BASE64_ETAG[]			= "</base64>";

static const char ARRAY_TAG[]		 = "<array>";
static const char DATA_TAG[]			= "<data>";
static const char DATA_ETAG[]		 = "</data>";
static const char ARRAY_ETAG[]		= "</array>";

static const char STRUCT_TAG[]		= "<struct>";
static const char MEMBER_TAG[]		= "<member>";
static const char NAME_TAG[]			= "<name>";
static const char NAME_ETAG[]		 = "</name>";
static const char MEMBER_ETAG[]	 = "</member>";
static const char STRUCT_ETAG[]	 = "</struct>";

std::string XmlRpcValue::_doubleFormat;





//---------------------------------- Misc: -------------------------------

static bool strieq(const char* a, const char* b)
{
	return _stricmp(a, b) == 0;
}


static bool strbegins(const char* bigstr, const char* smallstr, bool casesensitive)
{
    const char* smalls = smallstr;
    const char* bigs = bigstr;
    while (*smalls) {
        if (casesensitive) {
            if (*bigs != *smalls)
                return false;
            else bigs++, smalls++;
        }
        else {
            if (toupper(*bigs) != toupper(*smalls))
                return false;
            else bigs++, smalls++;
        }
    }
    return true;
}


//---------------------------------- ValueArray: -------------------------------

void XmlRpcValue::ValueArray::resize(int n)
{
	if (n >= _allocated) {
		// Optimise growing of the array:
		int power2 = n - 1;
		power2 |= power2 >> 1;
		power2 |= power2 >> 2;
		power2 |= power2 >> 4;
		power2 |= power2 >> 8;
		power2 |= power2 >> 16;
		power2++;

		// Set the size:
		A = (XmlRpcValue*)realloc(A, power2 * sizeof(XmlRpcValue));
		memset(A + _allocated, 0, (power2 - _allocated) * sizeof(XmlRpcValue));
		_allocated = power2;
	}
	_size = n;
}


bool XmlRpcValue::ValueArray::operator==(ValueArray &other)
{
	if (_size != other._size)
		return false;
	for (int i=0; i < _size; i++) {
		if (A[i] != other.A[i])
			return false;
	}
	return true;
}


XmlRpcValue::ValueArray::~ValueArray()
{
	for (int i=0; i < _size; i++)
		A[i].invalidate();
	free(A);
}




//---------------------------------- base64.h: -------------------------------
	/* <Chris Morley> */
static
int _base64Chars[]= {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
						 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
							 '0','1','2','3','4','5','6','7','8','9',
							 '+','/' };


#define _0000_0011 0x03
#define _1111_1100 0xFC
#define _1111_0000 0xF0
#define _0011_0000 0x30
#define _0011_1100 0x3C
#define _0000_1111 0x0F
#define _1100_0000 0xC0
#define _0011_1111 0x3F

#define _EQUAL_CHAR	 (-1)
#define _UNKNOWN_CHAR (-2)

#define _IOS_FAILBIT	 std::ios_base::failbit
#define _IOS_EOFBIT		std::ios_base::eofbit
#define _IOS_BADBIT		std::ios_base::badbit
#define _IOS_GOODBIT	 std::ios_base::goodbit

// TEMPLATE CLASS base64_put
class base64
{
public:

	typedef unsigned char byte_t;
	typedef char						char_type;
	typedef std::char_traits<char>					 traits_type; 

	// base64 requires max line length <= 72 characters
	// you can fill end of line
	// it may be crlf, crlfsp, noline or other class like it


	struct crlf
	{
		template<class _OI>
			_OI operator()(_OI _To) const{
			*_To = std::char_traits<char>::to_char_type('\r'); ++_To;
			*_To = std::char_traits<char>::to_char_type('\n'); ++_To;

			return (_To);
		}
	};


	struct crlfsp
	{
		template<class _OI>
			_OI operator()(_OI _To) const{
			*_To = std::char_traits<char>::to_char_type('\r'); ++_To;
			*_To = std::char_traits<char>::to_char_type('\n'); ++_To;
			*_To = std::char_traits<char>::to_char_type(' '); ++_To;

			return (_To);
		}
	};

	struct noline
	{
		template<class _OI>
			_OI operator()(_OI _To) const{
			return (_To);
		}
	};

	struct three2four
	{
		void zero()
		{
			_data[0] = 0;
			_data[1] = 0;
			_data[2] = 0;
		}

		byte_t get_0()	const
		{
			return _data[0];
		}
		byte_t get_1()	const
		{
			return _data[1];
		}
		byte_t get_2()	const
		{
			return _data[2];
		}

		void set_0(byte_t _ch)
		{
			_data[0] = _ch;
		}

		void set_1(byte_t _ch)
		{
			_data[1] = _ch;
		}

		void set_2(byte_t _ch)
		{
			_data[2] = _ch;
		}

		// 0000 0000	1111 1111	2222 2222
		// xxxx xxxx	xxxx xxxx	xxxx xxxx
		// 0000 0011	1111 2222	2233 3333

		int b64_0()	const	{return (_data[0] & _1111_1100) >> 2;}
		int b64_1()	const	{return ((_data[0] & _0000_0011) << 4) + ((_data[1] & _1111_0000)>>4);}
		int b64_2()	const	{return ((_data[1] & _0000_1111) << 2) + ((_data[2] & _1100_0000)>>6);}
		int b64_3()	const	{return (_data[2] & _0011_1111);}

		void b64_0(int _ch)	{_data[0] = ((_ch & _0011_1111) << 2) | (_0000_0011 & _data[0]);}

		void b64_1(int _ch)	{
			_data[0] = ((_ch & _0011_0000) >> 4) | (_1111_1100 & _data[0]);
			_data[1] = ((_ch & _0000_1111) << 4) | (_0000_1111 & _data[1]);	}

		void b64_2(int _ch)	{
			_data[1] = ((_ch & _0011_1100) >> 2) | (_1111_0000 & _data[1]);
			_data[2] = ((_ch & _0000_0011) << 6) | (_0011_1111 & _data[2]);	}

		void b64_3(int _ch){
			_data[2] = (_ch & _0011_1111) | (_1100_0000 & _data[2]);}

	private:
		byte_t _data[3];

	};




	template<class _II, class _OI, class _State, class _Endline>
		_II put(_II _First, _II _Last, _OI _To, _State& _St, _Endline _Endl)	const
	{
		three2four _3to4;
		int line_octets = 0;

		while(_First != _Last)
		{
			_3to4.zero();

			// 
			_3to4.set_0(*_First);
			_First++;

			if(_First == _Last)
			{
				*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_0()]); ++_To;
				*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_1()]); ++_To;
				*_To = std::char_traits<char>::to_char_type('='); ++_To;
				*_To = std::char_traits<char>::to_char_type('='); ++_To;
				goto __end;
			}

			_3to4.set_1(*_First);
			_First++;

			if(_First == _Last)
			{
				*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_0()]); ++_To;
				*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_1()]); ++_To;
				*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_2()]); ++_To;
				*_To = std::char_traits<char>::to_char_type('='); ++_To;
				goto __end;
			}

			_3to4.set_2(*_First);
			_First++;

			*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_0()]); ++_To;
			*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_1()]); ++_To;
			*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_2()]); ++_To;
			*_To = std::char_traits<char>::to_char_type(_base64Chars[_3to4.b64_3()]); ++_To;

			if(line_octets == 17) // base64 
			{
				//_To = _Endl(_To);
				*_To = '\n'; ++_To;
				line_octets = 0;
			}
			else
				++line_octets;
		}

		__end: ;

		return (_First);

	}


	template<class _II, class _OI, class _State>
		_II get(_II _First, _II _Last, _OI _To, _State& _St) const
	{
		three2four _3to4;
		int _Char;

		while(_First != _Last)
		{

			// Take octet
			_3to4.zero();

			// -- 0 --
			// Search next valid char... 
			while((_Char =	_getCharType(*_First)) < 0 && _Char == _UNKNOWN_CHAR)
			{
				if(++_First == _Last)
				{
					_St |= _IOS_FAILBIT|_IOS_EOFBIT; return _First; // unexpected EOF
				}
			}

			if(_Char == _EQUAL_CHAR){
				// Error! First character in octet can't be '='
				_St |= _IOS_FAILBIT; 
				return _First; 
			}
			else
				_3to4.b64_0(_Char);


			// -- 1 --
			// Search next valid char... 
			while(++_First != _Last)
				if((_Char = _getCharType(*_First)) != _UNKNOWN_CHAR)
					break;

			if(_First == _Last)	{
				_St |= _IOS_FAILBIT|_IOS_EOFBIT; // unexpected EOF 
				return _First;
			}

			if(_Char == _EQUAL_CHAR){
				// Error! Second character in octet can't be '='
				_St |= _IOS_FAILBIT; 
				return _First; 
			}
			else
				_3to4.b64_1(_Char);


			// -- 2 --
			// Search next valid char... 
			while(++_First != _Last)
				if((_Char = _getCharType(*_First)) != _UNKNOWN_CHAR)
					break;

			if(_First == _Last)	{
				// Error! Unexpected EOF. Must be '=' or base64 character
				_St |= _IOS_FAILBIT|_IOS_EOFBIT; 
				return _First; 
			}

			if(_Char == _EQUAL_CHAR){
				// OK!
				_3to4.b64_2(0); 
				_3to4.b64_3(0); 

				// chek for EOF
				if(++_First == _Last)
				{
					// Error! Unexpected EOF. Must be '='. Ignore it.
					//_St |= _IOS_BADBIT|_IOS_EOFBIT;
					_St |= _IOS_EOFBIT;
				}
				else 
					if(_getCharType(*_First) != _EQUAL_CHAR)
					{
						// Error! Must be '='. Ignore it.
						//_St |= _IOS_BADBIT;
					}
				else
					++_First; // Skip '='

				// write 1 byte to output
				*_To = (byte_t) _3to4.get_0();
				return _First;
			}
			else
				_3to4.b64_2(_Char);


			// -- 3 --
			// Search next valid char... 
			while(++_First != _Last)
				if((_Char = _getCharType(*_First)) != _UNKNOWN_CHAR)
					break;

			if(_First == _Last)	{
				// Unexpected EOF. It's error. But ignore it.
				//_St |= _IOS_FAILBIT|_IOS_EOFBIT; 
					_St |= _IOS_EOFBIT; 
				
				return _First; 
			}

			if(_Char == _EQUAL_CHAR)
			{
				// OK!
				_3to4.b64_3(0); 

				// write to output 2 bytes
				*_To = (byte_t) _3to4.get_0();
				*_To = (byte_t) _3to4.get_1();

				++_First; // set position to next character

				return _First;
			}
			else
				_3to4.b64_3(_Char);


			// write to output 3 bytes
			*_To = (byte_t) _3to4.get_0();
			*_To = (byte_t) _3to4.get_1();
			*_To = (byte_t) _3to4.get_2();

			++_First;
			

		} // while(_First != _Last)

		return (_First);
	}

protected:
	
	int _getCharType(int _Ch) const
	{
		if(_base64Chars[62] == _Ch)
			return 62;

		if(_base64Chars[63] == _Ch)
			return 63;

		if((_base64Chars[0] <= _Ch) && (_base64Chars[25] >= _Ch))
			return _Ch - _base64Chars[0];

		if((_base64Chars[26] <= _Ch) && (_base64Chars[51] >= _Ch))
			return _Ch - _base64Chars[26] + 26;

		if((_base64Chars[52] <= _Ch) && (_base64Chars[61] >= _Ch))
			return _Ch - _base64Chars[52] + 52;

		if(_Ch == std::char_traits<char>::to_int_type('='))
			return _EQUAL_CHAR;

		return _UNKNOWN_CHAR;
	}


};



/*-----------------------------------------------------------------------*/

void XmlRpcValue::invalidate()
{
	switch (_type) {
		case TypeString:	free(u.asString); break;
		case TypeDateTime:	delete u.asTime;	 break;
		case TypeBase64:	delete u.asBinary; break;
		case TypeArray:		delete u.asArray;	break;
		case TypeStruct:	delete u.asStruct; break;
		default:			break;
	}
	_type = TypeInvalid;
	u.asBinary = 0;
}


// Type checking
void XmlRpcValue::assertTypeOrInvalid(Type t)
{
	if (_type == TypeInvalid || _type == TypeNil) {
		_type = t;
		switch (_type) {		// Ensure there is a valid value for the type
			case TypeString:	u.asString = _strdup(""); break;
			case TypeDateTime:	u.asTime = new struct tm();		 break;
			case TypeBase64:	u.asBinary = new BinaryData();	break;
			case TypeArray:		u.asArray = new ValueArray();	 break;
			case TypeStruct:	u.asStruct = new ValueStruct(); break;
			default:			u.asBinary = 0; break;
		}
	}
	else if (_type != t)
		throw XmlRpcException("type error");
}


void XmlRpcValue::assertArray(int size) const
{
	if (_type != TypeArray)
		throw XmlRpcException("type error: expected an array");
	else if (int(u.asArray->size()) < size)
		throw XmlRpcException("range error: array index too large");
}


void XmlRpcValue::assertArray(int size)
{
	if (_type == TypeInvalid) {
		_type = TypeArray;
		u.asArray = new ValueArray(size);
	}
	else if (_type == TypeArray) {
		if (int(u.asArray->size()) < size) {
			u.asArray->resize(size);
		}
	}
	else
		throw XmlRpcException("type error: expected an array");
}


void XmlRpcValue::assertStruct()
{
	if (_type == TypeInvalid) {
		_type = TypeStruct;
		u.asStruct = new ValueStruct();
	} else if (_type != TypeStruct)
		throw XmlRpcException("type error: expected a struct");
}


XmlRpcValue::ValueArray::ValueArray(ValueArray &other)
{
	A = NULL; 
	_size = _allocated = 0;
	resize(other._size);
    for (int i=0 ; i < _size; i++) {
        A[i] = other.A[i];
    }
}


/* This copy constructor does a deep copy.  It's equivalent to:  "XmlRpcValue(XmlRpcValue&orig);".
Tim> In my applications, I manage to avoid using copy constructors, && if you're interested
in performance, you should also consider whether you can avoid copy constructors, e.g. by
passing values by reference.
Thanks to Eric Schneider for identifying a fault with a previous version of this copy constructor.
*/
XmlRpcValue& XmlRpcValue::operator=(XmlRpcValue const& rhs)
{
	if (this != &rhs) {
		invalidate();
		_type = rhs._type;
		switch (_type) {
			case TypeBoolean:	u.asBool = rhs.u.asBool; break;
			case TypeInt:		u.asInt = rhs.u.asInt; break;
			case TypeDouble:	u.asDouble = rhs.u.asDouble; break;
			case TypeDateTime:	u.asTime = new struct tm(*rhs.u.asTime); break;
			case TypeString:	u.asString = _strdup(rhs.u.asString); break;
			case TypeBase64:	u.asBinary = new BinaryData(*rhs.u.asBinary); break;
			case TypeArray:		u.asArray = new ValueArray(*rhs.u.asArray); break;
			case TypeStruct:	u.asStruct = new ValueStruct(*rhs.u.asStruct); break;
			default:			u.asBinary = 0; break;
		}
	}
	return *this;
}
	/* </Chris Morley> */




// Map something like:    "T2-D&amp;T1"  to  "T2-D&T1"
// Returns a 'strdup()' version of the input string.

static char* xmlDecode(const char* s, const char* end)
{
	char* dest = (char*)malloc(end - s + 1);
	char* d = dest;
	while (s < end) {
		if (*s != '&')
			*d++ = *s++;
		else if (strbegins(s, "&amp;", true))
			*d++ = '&', s += 5;
		else if (strbegins(s, "&quot;", true))
			*d++ = '\"', s += 6;
		else if (strbegins(s, "&apos;", true)/*! standard*/ || strbegins(s, "&#039;", true))
			*d++ = '\'', s += 6;
		else if (strbegins(s, "&lt;", true))
			*d++ = '<', s += 4;
		else if (strbegins(s, "&gt;", true))
			*d++ = '>', s += 4;
		else if (strbegins(s, "&#", true)) {
			s += 2;
			*d++ = atoi(s);
			while (*s >= '0' && *s <= '9')
				s++;
			if (*s == ';')
				s++;
		}
		else
			*d++ = *s++;	// assert(false);
	}
	*d = '\0';
	return dest;
}


// Replace raw text with xml-encoded entities.

static std::string xmlEncode(const char* s)
{
	std::ostringstream ostr;

	while (*s) {
		if (*s == '&')
			ostr << "&amp;";
		else if (*s == '<')
			ostr << "&lt;";
		else if (*s == '>')
			ostr << "&gt;";
		else if (*s == '"')
			ostr << "&quot;";
		else if (*s == '\'')
			ostr << "&apos;";// Would David prefer:  "&#039;" ?
		else if (*s < ' ' || *s >= 127)
			ostr << "&#" << int((unsigned char)*s) << ';';
		else ostr << *s;
		s++;
	}
	return ostr.str();
}


	/* <Chris Morley> */
// Predicate for tm equality
static bool tmEq(struct tm const& t1, struct tm const& t2) 
{
	return t1.tm_sec == t2.tm_sec && t1.tm_min == t2.tm_min &&
					t1.tm_hour == t2.tm_hour && t1.tm_mday == t1.tm_mday &&
					t1.tm_mon == t2.tm_mon && t1.tm_year == t2.tm_year;
}


bool XmlRpcValue::operator==(XmlRpcValue const& other) const
{
	if (_type != other._type)
		return false;

	switch (_type) {
		case TypeBoolean:	return ( !u.asBool && !other.u.asBool) ||
															( u.asBool && other.u.asBool);
		case TypeInt:		return u.asInt == other.u.asInt;
		case TypeDouble:	return u.asDouble == other.u.asDouble;
		case TypeDateTime:	return tmEq(*u.asTime, *other.u.asTime);
		case TypeString:	return *u.asString == *other.u.asString;
		case TypeBase64:	return *u.asBinary == *other.u.asBinary;
		case TypeArray:		return *u.asArray == *other.u.asArray;

		// The map<>::operator== requires the definition of value< for kcc
		case TypeStruct:	 //return *u.asStruct == *other.u.asStruct;
			{
				if (u.asStruct->size() != other.u.asStruct->size())
					return false;
				
				ValueStruct::const_iterator it1=u.asStruct->begin();
				ValueStruct::const_iterator it2=other.u.asStruct->begin();
				while (it1 != u.asStruct->end()) {
					const XmlRpcValue& v1 = it1->second;
					const XmlRpcValue& v2 = it2->second;
					if ( ! (v1 == v2))
						return false;
					it1++;
					it2++;
				}
				return true;
			}
		default: break;
	}
	return true;		// Both invalid values ...
}


// Works for strings, binary data, arrays, && structs.
int XmlRpcValue::size() const
{
	switch (_type) {
		case TypeString: return int(strlen(u.asString));
		case TypeBase64: return int(u.asBinary->size());
		case TypeArray:	return int(u.asArray->size());
		case TypeStruct: return int(u.asStruct->size());
		default: break;
	}

	throw XmlRpcException("type error");
}


// Checks for existence of struct member
bool XmlRpcValue::hasMember(const std::string& name) const
{
	return _type == TypeStruct && u.asStruct->find(name) != u.asStruct->end();
}
	/* </Chris Morley> */


static void SkipWhiteSpace(const char* &s)
{
	while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
		s++;
}


static char* GobbleTag(const char* &s, char dest[128])
{
	*dest = '\0';
	SkipWhiteSpace(s);
	if (*s != '<')
		return dest;
	const char* t = strchr(s, '>');
	if (t == NULL)
		return dest;
	t++;
	int n = int(t - s);
	if (n >= 128)
		n = 127;
	memcpy(dest, s, n);
	dest[n] = '\0';
	s = t;
	return dest;
}


static void GobbleExpectedTag(const char* &s, const char* etag)
{
	char tag[128];
	GobbleTag(s, tag);
	if (! strieq(tag, etag))
		throw XmlRpcException(std::string("Expecting tag: ") + etag);
}


// Set the value from xml. The chars at *offset into valueXml 
// should be the start of a <value> tag. Destroys any existing value.
void XmlRpcValue::fromXml(const char* &s)
{
	char tag[128];

	invalidate();

	// Gobble the <value> tag:
	GobbleTag(s, tag);
	if (strieq(tag, "<value>"))
		;	// good
	else if (strieq(tag, "<value/>")) {
		// Jeff Rasmussen claims that <value/> is valid XmlRpc. I think he's correct.
		_type = TypeString;
		u.asString = _strdup("");
		return;
	}
	else
		throw XmlRpcException(std::string("Expecting tag: <value>"));

	// Gobble the type tag:
	GobbleTag(s, tag);
	if (*tag == '\0') {
		// If no type is indicated, the type is string.
		stringFromXml(s);
	}
	else if (strieq(tag, BOOLEAN_TAG)) {
		boolFromXml(s);
		GobbleExpectedTag(s, BOOLEAN_ETAG);
	}
	else if (strieq(tag, I4_TAG)) {
		intFromXml(s);
		GobbleExpectedTag(s, I4_ETAG);
	}
	else if (strieq(tag, INT_TAG)) {
		intFromXml(s);
		GobbleExpectedTag(s, INT_ETAG);
	}
	else if (strieq(tag, DOUBLE_TAG)) {
		doubleFromXml(s);
		GobbleExpectedTag(s, DOUBLE_ETAG);
	}
	else if (strieq(tag, STRING_TAG)) {
		stringFromXml(s);
		GobbleExpectedTag(s, STRING_ETAG);
	}
	else if (strieq(tag, DATETIME_TAG)) {
		timeFromXml(s);
		GobbleExpectedTag(s, DATETIME_ETAG);
	}
	else if (strieq(tag, BASE64_TAG)) {
		binaryFromXml(s);
		GobbleExpectedTag(s, BASE64_ETAG);
	}
	else if (strieq(tag, ARRAY_TAG)) {
		arrayFromXml(s);
		GobbleExpectedTag(s, ARRAY_ETAG);
	}
	else if (strieq(tag, STRUCT_TAG)) {
		structFromXml(s);
		GobbleExpectedTag(s, STRUCT_ETAG);
	}
	else if (strieq(tag, "<string/>")) {
		_type = TypeString;
		u.asString = _strdup("");
	}
	else if (strieq(tag, "<nil/>")) {
		_type = TypeNil;
	}
	else if (strieq(tag, "<struct/>")) {
		_type = TypeStruct;
		u.asStruct = new ValueStruct;
	}
	else if (strieq(tag, VALUE_ETAG)) {	
		// "If no type is indicated, the type is string."
		_type = TypeString;
		u.asString = _strdup("");
		return;	// don't gobble VALUE_ETAG because we already did
	}
	else {
		throw XmlRpcException(std::string("Unknown type tag: ") + tag);
	}

	GobbleExpectedTag(s, VALUE_ETAG);
}


// Encode the Value in xml
void XmlRpcValue::toXml(std::ostringstream &ostr) const
{
	switch (_type) {
		case TypeBoolean:		return boolToXml(ostr);
		case TypeInt:			return intToXml(ostr);
		case TypeDouble:		return doubleToXml(ostr);
		case TypeString:		return stringToXml(ostr);
		case TypeDateTime:		return timeToXml(ostr);
		case TypeBase64:		return binaryToXml(ostr);
		case TypeArray:			return arrayToXml(ostr);
		case TypeStruct:		return structToXml(ostr);
		case TypeNil:			return nilToXml(ostr);
		case TypeInvalid:		throw XmlRpcException("Undefined XmlRpc value");
		default: break;
	}
}


void XmlRpcValue::boolFromXml(const char* &s)
{
	if (*s == '0' && s[1] == '<') {
		_type = TypeBoolean;
		u.asBool = false;
		s++;
	}
	else if (*s == '1' && s[1] == '<') {
		_type = TypeBoolean;
		u.asBool = true;
		s++;
	}
	else throw XmlRpcException("bad BOOL");
}


void XmlRpcValue::boolToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << BOOLEAN_TAG << (u.asBool ? "1" : "0") << BOOLEAN_ETAG << VALUE_ETAG;
}


void XmlRpcValue::intFromXml(const char* &s)
{
	char* valueEnd;
	long ivalue = strtol(s, &valueEnd, 10);
	if (valueEnd == s)
		throw XmlRpcException("Bad double");
	_type = TypeInt;
	u.asInt = int(ivalue);
	s = valueEnd;
}


void XmlRpcValue::intToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << I4_TAG << u.asInt << I4_ETAG << VALUE_ETAG;
}


void XmlRpcValue::doubleFromXml(const char* &s)
{
	char* valueEnd;
	double dvalue = strtod(s, &valueEnd);
	if (valueEnd == s)
		throw XmlRpcException("Bad double");
	_type = TypeDouble;
	u.asDouble = dvalue;
	s = valueEnd;
}


void XmlRpcValue::doubleToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << DOUBLE_TAG << std::fixed << u.asDouble << DOUBLE_ETAG << VALUE_ETAG;
	// This will use the default ostream::precision() to display the double.  To display
	// values with greater accuracy, call e.g.  'ostr.precision(12)' at the top level.
}


void XmlRpcValue::stringFromXml(const char* &s)
{
	const char* valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw XmlRpcException("Bad string");
	_type = TypeString;
	u.asString = xmlDecode(s, valueEnd);
	s = valueEnd;
}


void XmlRpcValue::stringToXml(std::ostringstream &ostr) const
{
	if (u.asString == NULL || *u.asString == '\0')
		ostr << VALUE_TAG << "<string/>" << VALUE_ETAG;
	else ostr << VALUE_TAG << xmlEncode(u.asString) << VALUE_ETAG;
	// The 'STRING_TAG' is optional.
}


void XmlRpcValue::nilToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << "<nil/>" << VALUE_ETAG;
}


void XmlRpcValue::timeFromXml(const char* &s)
{
	const char* valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw XmlRpcException("Bad time value");

	struct tm t;
	if (sscanf_s(s, "%4d%2d%2dT%2d:%2d:%2d", &t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec) != 6)
		throw XmlRpcException("Bad time value");

	t.tm_isdst = -1;
	t.tm_mon -= 1;
	_type = TypeDateTime;
	u.asTime = new struct tm(t);
	s = valueEnd;
}


void XmlRpcValue::timeToXml(std::ostringstream &ostr) const
{
	struct tm* t = u.asTime;
	char buf[30];
	_snprintf_s(buf, sizeof(buf), sizeof(buf)-1, 
				"%4d%02d%02dT%02d:%02d:%02d", 
				t->tm_year,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);

	ostr << VALUE_TAG << DATETIME_TAG << buf << DATETIME_ETAG << VALUE_ETAG;
}


void XmlRpcValue::binaryFromXml(const char* &s)
{
	const char* valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw XmlRpcException("Bad base64");

	_type = TypeBase64;
	u.asBinary = new BinaryData();
	// check whether base64 encodings can contain chars xml encodes...

	// convert from base64 to binary
	int iostatus = 0;
	base64 decoder;
	std::back_insert_iterator<BinaryData> ins = std::back_inserter(*(u.asBinary));
	decoder.get(s, valueEnd, ins, iostatus);

	s = valueEnd;
}


void XmlRpcValue::binaryToXml(std::ostringstream &ostr) const
{
	// convert to base64
	std::vector<char> base64data;
	int iostatus = 0;
	base64 encoder;
	std::back_insert_iterator<std::vector<char> > ins = std::back_inserter(base64data);
	encoder.put(u.asBinary->begin(), u.asBinary->end(), ins, iostatus, base64::crlf());

	// Wrap with xml
	ostr << VALUE_TAG << BASE64_TAG;
	ostr.write(&base64data[0], base64data.size());
	ostr << BASE64_ETAG << VALUE_ETAG;
}


void XmlRpcValue::arrayFromXml(const char* &s)
{
	char tag[128];

	_type = TypeArray;
	u.asArray = new ValueArray;

	GobbleTag(s, tag);
	if (strieq(tag, "<data/>"))
		return;
	if (! strieq(tag, DATA_TAG))
		throw XmlRpcException("Expecting tag:  <data>");

	do {
		SkipWhiteSpace(s);
		if (strbegins(s, DATA_ETAG, true))
			break;
		int n = u.asArray->size();
		u.asArray->resize(n+1);
		u.asArray->at(n).fromXml(s);
	} while (true);

	// Skip the trailing </data>
	GobbleExpectedTag(s, DATA_ETAG);
}


void XmlRpcValue::arrayToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << ARRAY_TAG << DATA_TAG;

	int limit = int(u.asArray->size());
	for (int i=0; i < limit; ++i)
		 u.asArray->at(i).toXml(ostr);

	ostr << DATA_ETAG << ARRAY_ETAG << VALUE_ETAG;
}


void XmlRpcValue::structFromXml(const char* &s)
{
	_type = TypeStruct;
	u.asStruct = new ValueStruct;

	SkipWhiteSpace(s);
	while (strbegins(s, MEMBER_TAG, true)) {
		s += strlen(MEMBER_TAG);

		// name
		GobbleExpectedTag(s, NAME_TAG);
		const char* nameEnd = strchr(s, '<');
		if (nameEnd == NULL)
			throw XmlRpcException("Bad 'name' tag in struct");
		char* name = xmlDecode(s, nameEnd);
		s = nameEnd;
		GobbleExpectedTag(s, NAME_ETAG);

		// value
		XmlRpcValue val;
		val.fromXml(s);
		if ( ! val.valid()) {
			invalidate();
			return;
		}
		const std::pair<const std::string, XmlRpcValue> p(name, val);
		u.asStruct->insert(p);
		free(name);

		GobbleExpectedTag(s, MEMBER_ETAG);
		SkipWhiteSpace(s);
	}
}


void XmlRpcValue::structToXml(std::ostringstream &ostr) const
{
	ostr << VALUE_TAG << STRUCT_TAG;

	ValueStruct::const_iterator it;
	for (it=u.asStruct->begin(); it!=u.asStruct->end(); ++it) {
		ostr << MEMBER_TAG << NAME_TAG << xmlEncode(it->first.c_str()) << NAME_ETAG;
		it->second.toXml(ostr);
		ostr << MEMBER_ETAG;
	}

	ostr << STRUCT_ETAG << VALUE_ETAG << '\n';
}


bool XmlRpcValue::parseMethodResponse(const char* s)
{
	char xmlVersion[128];
	char tag[128];

	GobbleTag(s, xmlVersion);
	if (! strbegins(xmlVersion, "<?xml version", false)) {
		if (strstr(s, "<html") != NULL || strstr(s, "<HTML") != NULL) {
			throw XmlRpcException("It looks like you've configured the wrong URL. That URL is sending us HTML text, "
						"whereas we need RPC data.");
    } else if (strieq(xmlVersion, "<methodResponse>")) {
    } else throw XmlRpcException(std::string(s));
	}
	GobbleTag(s, tag);
	if (! strieq(tag, "<methodResponse>")) {
		if (strstr(s, "<title>Bad request!</title>"))
			throw XmlRpcException(std::string("Talking HTTP to a HTTPS server?"));
		else if (strstr(s, "Object not found"))
			throw XmlRpcException(std::string("Wrong URL (\"Object not found\")"));
		else throw XmlRpcException(std::string(s));
	}
	SkipWhiteSpace(s);
	if (strbegins(s, "<fault>", true)) {
		GobbleExpectedTag(s, "<fault>");
		fromXml(s);
		GobbleExpectedTag(s, "</fault>");
		return false;
	}
	else {
		GobbleExpectedTag(s, "<params>");
		GobbleExpectedTag(s, "<param>");
		fromXml(s);
		// There's false real need to parse the bits at the end, is there?
		//GobbleExpectedTag(s, "</param>");
		//GobbleExpectedTag(s, "</params>");
	}
	//GobbleExpectedTag(s, "</methodResponse>");
	return true;
}


void XmlRpcValue::buildCall(const char* method, std::ostringstream &ostr) const
{
	ostr << "<?xml version=\"1.0\"?>\r\n";
	ostr << "<methodCall><methodName>" << method << "</methodName>\r\n<params>";
	if (getType() == XmlRpcValue::TypeArray)	{
		for (int i=0; i < size(); ++i) {
			ostr << "<param>";
			(*this)[i].toXml(ostr);
			ostr << "</param>";
		}
	}
	else if (getType() == XmlRpcValue::TypeInvalid) {
	}
	else {
		ostr << "<param>";
		toXml(ostr);
		ostr << "</param>\n";
	}
	ostr << "</params></methodCall>\r\n";
}




/*-------------------------- class XmlRpcClient: ----------------------*/

class XmlRpcImplementation {
	XmlRpcClient::protocol_enum protocol;
	bool ignoreCertificateAuthority;
	HINTERNET hInternet;
	HINTERNET hConnect;
	std::string object;
	int port;

	void hadError(const char* function);
	bool connect(const char* server);

public:
	XmlRpcCallback Callback;
	void *context;
	int totalBytes;
	std::string errmsg;
	int HttpErrcode;
	bool isFault;

	XmlRpcImplementation(const char* server, int port, const char* object, XmlRpcClient::protocol_enum protocol);
	XmlRpcImplementation(const char* URI);
	bool execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result);
	void setCallback(XmlRpcCallback Callback, void* context)
			{ this->Callback = Callback; this->context = context; }
	void setIgnoreCertificateAuthority(bool value) { ignoreCertificateAuthority = value; }
	~XmlRpcImplementation();
};


XmlRpcClient::XmlRpcClient(const char* server, int port, const char* object, protocol_enum protocol)
{
	secret = new XmlRpcImplementation(server, port, object, protocol);
}


XmlRpcClient::XmlRpcClient(const char* URI)
{
	secret = new XmlRpcImplementation(URI);
}


bool XmlRpcClient::execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result)
{
	return secret->execute(method, params, result);
}


std::string XmlRpcClient::getError()
{
	if (secret->errmsg.size() > 1254)
		secret->errmsg.resize(1254);
	return secret->errmsg;
}


void XmlRpcClient::setError(std::string msg)
{
	secret->errmsg = msg;
}


int XmlRpcClient::getHttpErrorCode()
{
	return secret->HttpErrcode;
}


void XmlRpcClient::setCallback(XmlRpcCallback Callback, void* context)
{
	secret->setCallback(Callback, context);
}


void XmlRpcClient::setIgnoreCertificateAuthority(bool value)
{
	secret->setIgnoreCertificateAuthority(value);
}


bool XmlRpcClient::isFault() const
{
	return secret->isFault;
}


void XmlRpcClient::close()
{
	delete secret;
	secret = NULL;
}


XmlRpcImplementation::XmlRpcImplementation(const char* server, int _port, const char* _object, 
												XmlRpcClient::protocol_enum _protocol)
{
	port = _port;
	object = _object;
	HttpErrcode = -1;
	if (_protocol == XmlRpcClient::XMLRPC_AUTO)
		protocol =	(port == 80) ? XmlRpcClient::XMLRPC_HTTP : 
					(port == 443) ? XmlRpcClient::XMLRPC_HTTPS : XmlRpcClient::XMLRPC_HTTP;
	else protocol = _protocol;
	ignoreCertificateAuthority = false;
	hConnect = NULL;
	connect(server);
}


XmlRpcImplementation::XmlRpcImplementation(const char* URI)
{
	port = 0;
	ignoreCertificateAuthority = false;
	if (strbegins(URI, "https://", false)) {
		protocol = XmlRpcClient::XMLRPC_HTTPS;
		URI += 8;
		port = 443;
	}
	else if (strbegins(URI, "http://", false)) {
		protocol = XmlRpcClient::XMLRPC_HTTP;
		URI += 7;
		port = 80;
	}
	else {
		errmsg = "The URI must begin with \"https://\" or \"http://\".";
		return;
	}
	const char* t = URI;
	while (*t != ':' && *t != '\0' && *t != '/')
		t++;
	std::string server(URI, t - URI);
	if (*t == ':') {
		t++;
		port = atoi(t);
		while (*t >= '0' && *t <= '9')
			t++;
	}
	object = t;		// should start with '/'.
	connect(server.c_str());
}


bool XmlRpcImplementation::connect(const char* server)
{
	Callback = NULL;
	context = NULL;
	totalBytes = 0;
	hInternet = InternetOpen("XmlRpc", 0, NULL, NULL, 0);
	if (hInternet == NULL) {
		hadError("InternetOpen");
		return false;
	}
	hConnect = InternetConnect(hInternet, server, port, 
					NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)this);
	if (hConnect == NULL) {
		hadError("InternetConnect");
		return false;
	}
	DWORD dwTimeout=999000;		// 999 seconds
	InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeout, sizeof(DWORD)); 
	InternetSetOption(hConnect, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeout, sizeof(DWORD));
	return true;
}


// Converts a GetLastError() code into a human-readable string.
void XmlRpcImplementation::hadError(const char* function)
{   
	errmsg = function;
	errmsg += " : ";

	int LastError = GetLastError();
	if (LastError == ERROR_INTERNET_TIMEOUT)
		errmsg += "Internet timeout";
	else if (LastError == ERROR_INTERNET_INVALID_CA)
		errmsg += "Invalid certificate authority";
	else if (LastError == ERROR_INTERNET_SECURITY_CHANNEL_ERROR)
		errmsg += "Talking HTTPS to an HTTP server?";
	else if (LastError == ERROR_INTERNET_CANNOT_CONNECT)
		errmsg += "Failed to connect";
	else if (LastError == ERROR_INTERNET_NAME_NOT_RESOLVED)
		errmsg += "Name not resolved";
	else if (LastError == ERROR_INTERNET_INVALID_URL)
		errmsg += "Invalid URL";
	else if (LastError == ERROR_INTERNET_NAME_NOT_RESOLVED)
		errmsg += "Domain name not resolved";
	else if (LastError == ERROR_INTERNET_CONNECTION_RESET)
		errmsg += "Connection reset";
	else if (LastError == ERROR_INTERNET_NOT_INITIALIZED)
		errmsg += "Internet not initialised";
	else if (LastError == ERROR_INTERNET_CONNECTION_ABORTED)
		errmsg += "Connection aborted";
	else if (LastError == ERROR_INTERNET_SEC_CERT_REV_FAILED)
		errmsg += "Unable to check whether security certificate was revoked or not. Is your system clock time correct?";
	else {
		static char* buf;
    	FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				LastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(char*)&buf,
				0,
				NULL
		);
		if (buf == NULL) {
			char tmp[512];
			sprintf_s(tmp, "Error %d", LastError);
			errmsg += tmp;
		}
		else {
			errmsg += buf;
		    LocalFree(buf);
		}
	}
}


static void CALLBACK myInternetCallback(HINTERNET hInternet,
				DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation,
				DWORD dwStatusInformationLength)
{
	XmlRpcImplementation *connection = (XmlRpcImplementation*)dwContext;
	if (connection && connection->Callback) {
		static char buf[512];
		char* status;
		switch (dwInternetStatus) {
			case INTERNET_STATUS_RECEIVING_RESPONSE:	if (connection->totalBytes == 0)
															status = "Waiting for response"; 
														else status = "Receiving response";
														break;
			case INTERNET_STATUS_RESPONSE_RECEIVED:		status = "Response received"; break;
			case INTERNET_STATUS_HANDLE_CLOSING:		status = "Handle closing"; break;
			case INTERNET_STATUS_REQUEST_SENT:			status = "Data sent"; break;
			case INTERNET_STATUS_SENDING_REQUEST:		status = "Sending data"; break;
			default:									status = buf; 
														sprintf_s(buf, "Status %d", dwInternetStatus);
														break;
		}
		connection->Callback(connection->context, status);
	}
}


bool XmlRpcImplementation::execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result)
{
  result.clear();
	errmsg = "";

	if (hConnect == NULL) {
		errmsg = "No connection";
		return false;
	}

	// Build the request as an XML file:
	std::ostringstream ostr;
	try {
		params.buildCall(method, ostr);
	} catch (XmlRpcException e) {
		errmsg = e.getMessage();
		return false;
	}

	// Create the HttpOpenRequest object:
	if (Callback)
		Callback(context, "Sending data");
	const char* acceptTypes[2] = { "text/*", NULL };
	int flags = INTERNET_FLAG_DONT_CACHE;
	flags |= INTERNET_FLAG_KEEP_CONNECTION;
	if (protocol != XmlRpcClient::XMLRPC_HTTP)
		flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
	HINTERNET hHttpFile = HttpOpenRequest(
					  hConnect,
					  "POST",
					  object.c_str(),
					  HTTP_VERSION,
					  NULL,
					  acceptTypes,
					  flags, 
					  (DWORD_PTR)this);
	if (hHttpFile == NULL) {
		hadError("HttpOpenRequest");
		return false;
	}
	InternetSetStatusCallback(hHttpFile, myInternetCallback);

	if (ignoreCertificateAuthority) {
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);

		InternetQueryOption(hHttpFile, INTERNET_OPTION_SECURITY_FLAGS,
					(LPVOID)&dwFlags, &dwBuffLen);
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		InternetSetOption(hHttpFile, INTERNET_OPTION_SECURITY_FLAGS,
							&dwFlags, sizeof (dwFlags) );
	}

	// Add the 'Content-Type' && 'Content-length' headers
	char header[255];		// Thanks, Anthony Chan.
	sprintf_s(header, "Content-Type: text/xml\r\nContent-length: %d", ostr.str().size());
	HttpAddRequestHeaders(hHttpFile, header, (DWORD)strlen(header), HTTP_ADDREQ_FLAG_ADD);

	// Send the request:
	if (! HttpSendRequest(hHttpFile, NULL, 0, (LPVOID)ostr.str().c_str(), (DWORD)ostr.str().size())) {
		hadError("HttpSendRequest");
		return false;
	}
	if (Callback)
		Callback(context, "Data sent...");

	// Read the response:
	char* buf = NULL;
	int len = 0;
	do {
		DWORD bytesAvailable;
		if (!InternetQueryDataAvailable(hHttpFile, &bytesAvailable, 0, (DWORD_PTR)this)) {
			hadError("InternetQueryDataAvailable");
			break;
		}
		if (bytesAvailable == 0)
			break;		// This is the EOF condition.

		buf = (char*)realloc(buf, len+bytesAvailable+1);

		// Read the data from the HINTERNET handle.
		DWORD bytesRead;
		if (!InternetReadFile(hHttpFile,
							 (LPVOID)(buf + len),
							 bytesAvailable,
							 &bytesRead))
		{
			hadError("InternetReadFile");
			break;
		}

		len += bytesRead;
		buf[len] = '\0';
		totalBytes = len;

	} while (true);

	// Roel Vanhout's insertion:  Did we get a HTTP_STATUS_OK response?  If not, why not?
	// XMLRPC spec says always return 200 for a valid answer. So if it's anything other than
 	// 200, it's an error (i.e., no support for redirects etc.).
	DWORD status_code;
 	DWORD buf_size, dummy;
  	buf_size = sizeof(DWORD);
 	if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &buf_size, 0)) {
 		errmsg = "Could not query HTTP result status";
 		free(buf);
 		return false;
 	}
  	if (status_code != 200) {
 		buf_size = 0;
 		HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_TEXT, &dummy, &buf_size, 0);
 		buf_size++;	// For the '\0'
 		char* status_text = (char*)::malloc(buf_size);
 		if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_TEXT, status_text, &buf_size, 0))
 			errmsg = "Could not query HTTP result status";
		else {
			char buf[512];
			sprintf_s(buf, "Low level (HTTP) error: %d %s", HttpErrcode, status_text);
			errmsg = buf;
		}
	    InternetCloseHandle(hHttpFile);
 		HttpErrcode = status_code;
 		free(buf);
 		return false;
 	}
 
    // Close the HttpRequest object:
    InternetCloseHandle(hHttpFile);

	// Parse the response:
	if (len == 0) {
		errmsg = "Invalid XMLRPC response: content size is 0";
		free(buf);		// 'buf' will always be NULL unless for some strange reason,
						// InternetReadFile() returns 'false' on the first pass.
		return false;
	}

	try {
		isFault = ! result.parseMethodResponse(buf);
		if (isFault) {
			XmlRpcValue possible = result["faultString"];
			if (possible.getType() == XmlRpcValue::TypeString)
				errmsg = std::string(possible);
			else errmsg = "unspecified error";
		}
	}
	catch (XmlRpcException err) {
		errmsg = err.getMessage();
	}
	free(buf);

	// Finished:
	return errmsg == "";
}


XmlRpcImplementation::~XmlRpcImplementation()
{
	if (hConnect)
		InternetCloseHandle(hConnect);
	if (hInternet)
		InternetCloseHandle(hInternet);
}


#if FALSE
//---------------------------- Unit testing: ----------------------
static void assert_failed(int line, char* filename, char* msg)
{
    printf("Assert failed|   %s:%d   %s\n", filename, line, msg);
}

#define assert_test(c)		if (c) ; else assert_failed(__LINE__,__FILE__,#c)

static void RoundTrip(XmlRpcValue &a)
{
	std::ostringstream ostr;
	a.toXml(ostr);
	std::string buf = ostr.str();
	XmlRpcValue b;
	const char* s = buf.c_str();
	b.fromXml(s);
	assert_test(a == b);
}


void XmlRpcUnitTest()
{
	try {
		XmlRpcValue result;
		char buf[327680];
		std::ifstream input("C:\\Documents && Settings\\edval\\Desktop\\Edval data\\debug.txt");
		input.read(buf, sizeof(buf));
		const char* s = buf;
		result.fromXml(s);
		for (int i=0; i < result.size(); i++) {
			XmlRpcValue el = result[i];
			std::string _AcademicYear = el["ACADEMIC_YEAR"];
			int AcademicYearId = el["ACADEMIC_YEAR_ID"];
		}
	} catch (XmlRpcException e) {
	}

	std::vector<XmlRpcValue> V;
	V.push_back(10);
	V.push_back(20);
	V.push_back(30);
	V.push_back(40);
	V.push_back(50);

	XmlRpcValue a = "Hello you";
	RoundTrip(a);

	XmlRpcValue harry;
	harry[0] = 56;
	harry[1] = 560;
	harry[2] = 115;
	harry[3] = 145;
	harry[4] = 35;

	//5
	XmlRpcValue binny((void*)XmlRpcUnitTest, 30);
	RoundTrip(binny);

	XmlRpcValue jenny;
	jenny["NAME"] = "Electric boots";
	jenny["code"] = "54121";
	jenny["status"] = true;

	//14
	a.clear();
	a["CHARGE"] = 505;
	a["SPIN"] = "foo";
	a["COLOUR"] = false;
	a["BENNY"] = harry;
	a["BINNY"] = binny;
	a["JENNY"] = jenny;
	a["EMPTY"] = "";
	RoundTrip(a);

	// Copy constructors:
	XmlRpcValue b;
	{
		XmlRpcValue copy = a;
		XmlRpcValue tmp = copy;
		b = tmp;
	}
	assert(a == b);
}
#endif
