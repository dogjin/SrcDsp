/*-----------------------------------------------------------------------------
@file

Declaration of function and templates related to the use of complex integer
numbers.

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef DSP_COMPLEX_H
#define DSP_COMPLEX_H
#include <cstdint>
#include <complex>


std::complex<int32_t> operator*(std::complex<int32_t> a, std::complex<int16_t> b);
std::complex<int32_t> operator*(std::complex<int16_t> a, std::complex<int32_t> b);
std::complex<int32_t> scale32(std::complex<int32_t> z, unsigned shift);
std::complex<uint32_t> scale32(std::complex<uint32_t> z, unsigned shift);
std::complex<int16_t> limitScale16(std::complex<int32_t> z, unsigned shift);

#if 0

std::complex<uint32_t> scale(std::complex<uint32_t> z, int shift)
{	
	return std::complex<uint32_t>(z.real() >> shift, z.imag() >> shift);
}


inline std::complex<int32_t> addMutiplyScale(std::complex<int32_t> a, int32_t b, int scale = 0)
{
	int32_t r = a.real() *b >> scale;
	int32_t i = a.imag() * b >> scale;
}

inline std::complex<int32_t> addMutiplyScale(std::complex<int32_t> a, std::complex<int32_t> b, int scale = 0)
{
	int32_t r = (a.real() * b.real() - a.imag() * b.imag()) >> scale;
	int32_t i = (a.real() * b.imag() + a.imag() * b.real()) >> scale;
}

#endif

namespace dsptl
{



} //end of namespace

#endif