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

This project consists of 2 files:   "TimXmlRpc.h"  and  "TimXmlRpc.cpp".

Parts of this project have been taken from Chris Morley's "XmlRpc++" project,
in particular the API.  Chris's contribution is acknowledged by marking his
work with the token:  "/*ChrisMorley/".  Thanks, Chris!


-------------------------Sample app:-------------------------

#include <iostream>
#include "timXmlRpc.h"

void Test(std::string username, std::string password)
{
	XmlRpcValue args, result;

	XmlRpcClient Connection("https://www.edval.com.au:9001/test.php");
	args[0] = username;
	args[1] = Md5(username + "." + password);		// salted Md5
	if (not Connection.execute("tetun.aprende", args, result)) {
		std::cerr << "Error: " << Connection.getError() << std::endl;
	}
	else {
		std::cout << "The answer is: " << std::string(result) << std::endl;
	}
}


See 'SampleMain.cpp' for a more elaborate example.

*/



#include <string>
#include <vector>
#include <map>
#include <sstream>




typedef void (*XmlRpcCallback)(void* context, char* status);


	/* <Chris Morley> */
class XmlRpcException {
	std::string _message;

public:
	//! Constructor
	//!	 @param message	A descriptive error message
	XmlRpcException(const std::string message) :
			_message(message) {}

	//! Return the error message.
	const std::string& getMessage() const { return _message; }
};
	/* </Chris Morley> */


class XmlRpcValue {
public:
	/* <Chris Morley> */
	enum Type {
		TypeInvalid,
		TypeBoolean,
		TypeInt,
		TypeDouble,
		TypeString,
		TypeDateTime,
		TypeBase64,
		TypeArray,
		TypeStruct,
		TypeNil
	};

	// Non-primitive types
	typedef std::vector<char> BinaryData;
	typedef std::map<std::string, XmlRpcValue> ValueStruct;
	/* </Chris Morley> */

	class ValueArray {
		// tco> I'm implementing my own 'ValueArray' instead of the original
		// std::vector<> because resizing the std::vector<> calls 100's of 
		// constructors and destructors.  Using 'vector::reserve()' is not sufficient
		// to prevent these constructors/destructors from being called, because
		// the C++ standard requires constructors and destructors be called whenever
		// an object changes its address, as happens when the std::vector resizes.
		XmlRpcValue *A;
		int _size;
		int _allocated;

	public:
		ValueArray() { A = NULL; _size = _allocated = 0; }
		ValueArray(int n) { 
					A = NULL; _size = _allocated = 0;
					resize(n); }
		ValueArray(ValueArray &other);
		int size() { return _size; }
		void resize(int n);
		XmlRpcValue& operator[](int i) { return A[i]; }
		XmlRpcValue& at(int i) { return A[i]; }
		bool operator==(ValueArray &other);
		void push_back(XmlRpcValue &val) { int last = _size; resize(_size + 1); A[last] = val; }
		~ValueArray();
	};

	/* <Chris Morley> */

	//! Constructors
	XmlRpcValue() : _type(TypeInvalid) { u.asBinary = 0; }
	XmlRpcValue(bool value) : _type(TypeBoolean) { u.asBool = value; }
	XmlRpcValue(int value)	: _type(TypeInt) { u.asInt = value; }
	XmlRpcValue(double value)	: _type(TypeDouble) { u.asDouble = value; }
	XmlRpcValue(char value)	: _type(TypeString) { 
					u.asString = (char*)malloc(2); 
					u.asString[0] = value;
					u.asString[1] = '\0';
				}

  XmlRpcValue(std::string const& value) : _type(TypeString) 
	{ u.asString = _strdup(value.c_str()); }

	XmlRpcValue(const char* value)	: _type(TypeString)
	{ u.asString = _strdup(value); }

	XmlRpcValue(struct tm* value)	: _type(TypeDateTime) 
	{ u.asTime = new struct tm(*value); }

	XmlRpcValue(void* value, int nBytes)	: _type(TypeBase64)
	{
		u.asBinary = new BinaryData((char*)value, ((char*)value)+nBytes);
	}

	//! Copy
	XmlRpcValue(XmlRpcValue const& rhs) : _type(TypeInvalid) { *this = rhs; }

	//! Destructor (make virtual if you want to subclass)
	/*virtual*/ ~XmlRpcValue() { invalidate(); }

	//! Erase the current value
	void clear() { invalidate(); }

	// Operators
	XmlRpcValue& operator=(bool rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(int const& rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(double const& rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(const char* rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(char rhs) { return operator=(XmlRpcValue(rhs)); }

	XmlRpcValue& operator=(XmlRpcValue const& rhs);	//<-- don't use copy constructors if you can avoid them!
				// This does a deep copy. Often you can use references instead of using copy constructors.

	bool operator==(XmlRpcValue const& other) const;
	bool operator!=(XmlRpcValue const& other) const { return !(*this == other); }

	operator bool&()		{ assertTypeOrInvalid(TypeBoolean); return u.asBool; }
	operator int&()			{   
								// Tim> I've implemented a hack that automatically converts strings
								// to integers, to get around a bug in an upstream system.  
								// Not sure if you users want it in or out.
								if (_type == TypeString && u.asString[0] >= '0' && u.asString[0] <= '9') {
									static int hack;
									hack = atoi(u.asString);
									return hack;
								}
								assertTypeOrInvalid(TypeInt); return u.asInt; }
	operator char()			{ assertTypeOrInvalid(TypeString); return *u.asString; }
	//operator int&()			{ assertTypeOrInvalid(TypeInt); return u.asInt; }
	operator double&()		{ assertTypeOrInvalid(TypeDouble); return u.asDouble; }
	operator std::string*()	{ 
								if (_type == TypeInt) {
									char tmp[80];
									_itoa_s(u.asInt, tmp, 10);
									return &std::string(tmp);
								}
								assertTypeOrInvalid(TypeString); 
								return &std::string(u.asString); 
							}
	operator const char*()	{ assertTypeOrInvalid(TypeString); return u.asString; }
	operator BinaryData&()	{ assertTypeOrInvalid(TypeBase64); return *u.asBinary; }
	operator struct tm&()	{ assertTypeOrInvalid(TypeDateTime); return *u.asTime; }
	operator ValueStruct&()	{ assertTypeOrInvalid(TypeStruct); return *u.asStruct; }	// good for iterating thru fields

  XmlRpcValue const& operator[](int i) const { 
			assertArray(i+1); return u.asArray->at(i); 
	}
	XmlRpcValue& operator[](int i)	{ assertArray(i+1); return u.asArray->at(i); }

	XmlRpcValue& operator[](std::string const& k) { assertStruct(); return (*u.asStruct)[k]; }
	XmlRpcValue& operator[](const char* k) { assertStruct(); std::string s(k); return (*u.asStruct)[s]; }

	// Accessors
	//! Return true if the value has been set to something.
	bool valid() const { return _type != TypeInvalid; }

	//! Return the type of the value stored. \see Type.
	Type const &getType() const { return _type; }

	//! Return the size for string, base64, array, and struct values.
	int size() const;

	//! Set up this value as an array, if not already so.  This function is optional,
	// because an undefined value is converted to an array implicitly the first time
	// you index it with an integer e.g. arg[0] = "hello";   , however if there's a
	// chance your array will be zero length then this function is compulsory.
	void initAsArray() { assertArray(0); }

	//! Specify the size for array values. Array values will grow beyond this size if needed.
	void setSize(int size)		{ assertArray(size); }

	//! Check for the existence of a struct member by name.
	bool hasMember(const std::string& name) const;

	//! Decode xml. Destroys any existing value.
	void fromXml(const char* &s);

	//! Encode the Value in xml
	void toXml(std::ostringstream &ostr) const;

	// Formatting
	//! Return the format used to write double values.
	static std::string const& getDoubleFormat() { return _doubleFormat; }

	//! Specify the format used to write double values.
	static void setDoubleFormat(const char* f) { _doubleFormat = f; }

	bool parseMethodResponse(const char* s);
	void buildCall(const char* method, std::ostringstream &ostr) const;

protected:
	// Clean up
	void invalidate();

	// Type checking
	void assertTypeOrInvalid(Type t);
	void assertArray(int size) const;
	void assertArray(int size);
	void assertStruct();

	// XML decoding
	void boolFromXml(const char* &s);
	void intFromXml(const char* &s);
	void doubleFromXml(const char* &s);
	void stringFromXml(const char* &s);
	void timeFromXml(const char* &s);
	void binaryFromXml(const char* &s);
	void arrayFromXml(const char* &s);
	void structFromXml(const char* &s);

	// XML encoding
	void boolToXml(std::ostringstream &ostr) const;
	void intToXml(std::ostringstream &ostr) const;
	void doubleToXml(std::ostringstream &ostr) const;
	void stringToXml(std::ostringstream &ostr) const;
	void timeToXml(std::ostringstream &ostr) const;
	void binaryToXml(std::ostringstream &ostr) const;
	void arrayToXml(std::ostringstream &ostr) const;
	void structToXml(std::ostringstream &ostr) const;
	void nilToXml(std::ostringstream &ostr) const;

	// Format strings
	static std::string _doubleFormat;

	// Type tag and values
	Type _type;

	union {
		bool			asBool;
		int				asInt;
		double			asDouble;
		struct tm*		asTime;
		char*			asString;
		BinaryData*		asBinary;
		ValueArray*		asArray;
		ValueStruct*	asStruct;
	} u;
	
};
	/* </Chris Morley> */


class XmlRpcClient {
	class XmlRpcImplementation *secret;

public:
	enum protocol_enum { XMLRPC_AUTO=0, XMLRPC_HTTP=1, XMLRPC_HTTPS=2 };

	//! Construct a client and attempt to connect to the server at the specified host:port address
	//!	@param host The name of the remote machine hosting the server
	//!	@param port The port on the remote machine where the server is listening
	//!	@param object	An optional object name to be sent in the HTTP GET header
	XmlRpcClient(const char* server, int port, const char* object, protocol_enum protocol=XMLRPC_AUTO);

	//! Construct a client and attempt to connect to the server at the specified URI.
	//!	@param URI  (Commonly and previously known as "URL"): e.g. "https://www.edval.com.au:9001/test.php"
	XmlRpcClient(const char* URI);

	~XmlRpcClient() { close(); }

	//! Execute the named procedure on the remote server.
	//!	@param method The name of the remote procedure to execute
	//!	@param params An array of the arguments for the method
	//!	@param result The result value to be returned to the client
	//!	@return true if the request was sent and a result received 
	//!	 (although the result might be a fault).
	//!
	//! Currently this is a synchronous (blocking) implementation (execute
	//! does not return until it receives a response or an error). Use isFault()
	//! to determine whether the result is a fault response.
	bool execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result);
  bool execute(const char* method, XmlRpcValue::ValueArray const& params, XmlRpcValue& result);

	//! Returns true if the result of the last execute() was a fault response.
	bool isFault() const;

	// Set the details for a callback function
	void setCallback(XmlRpcCallback Callback, void* context);

	// ignore the certificate authority on subsequent execute()'s.
	void setIgnoreCertificateAuthority(bool value=true);

	// Get and set error messages:
	std::string getError();
	void setError(std::string);
	int getHttpErrorCode();

	//! Close the connection
	void close();
};
