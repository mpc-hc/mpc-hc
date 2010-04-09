#ifndef _PODTYPES_H_
#define _PODTYPES_H_

template<class T> struct isPOD {enum {is=false};};
template<> struct isPOD<bool> {enum {is=true};};

template<> struct isPOD<char> {enum {is=true};};

template<> struct isPOD<signed char> {enum {is=true};};
template<> struct isPOD<short int> {enum {is=true};};
template<> struct isPOD<int> {enum {is=true};};
template<> struct isPOD<long int> {enum {is=true};};
template<> struct isPOD<__int64> {enum {is=true};};

template<> struct isPOD<unsigned char> {enum {is=true};};
template<> struct isPOD<unsigned short int> {enum {is=true};};
template<> struct isPOD<unsigned int> {enum {is=true};};
template<> struct isPOD<unsigned long int> {enum {is=true};};
template<> struct isPOD<unsigned __int64> {enum {is=true};};

template<> struct isPOD<float> {enum {is=true};};
template<> struct isPOD<double> {enum {is=true};};
template<> struct isPOD<long double> {enum {is=true};};

#if defined(__INTEL_COMPILER) || defined(__GNUC__) || (_MSC_VER>=1300)
template<> struct isPOD<wchar_t> {enum {is=true};};
template<class Tp> struct isPOD<Tp*> {enum {is=true};};
#endif

template<class A> struct allocator_traits {enum {is_static=false};};

#endif
