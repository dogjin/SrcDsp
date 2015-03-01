/*-----------------------------------------------------------------------------
@file

Definition of complex integer functions to supplement the standard library

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/
#include <complex>
#include <cstdint>
#include "dsp_complex.h"


/*---------------------------------------------------------------------------- -
Complex multiplication between a 16 bits complex and a 32 bits complex.
Result is stored in a 32 bits complex.
------------------------------------------------------------------------------*/
std::complex<int32_t> operator*(std::complex<int32_t> a, std::complex<int16_t> b)
{
	int32_t r, i;
	r = a.real() * b.real() - a.imag() * b.imag();
	i = a.imag() * b.real() + b.imag() * a.real();
	return std::complex<int32_t>(r, i);
}

std::complex<int32_t> operator*(std::complex<int16_t> a, std::complex<int32_t> b)
{
	int32_t r, i;
	r = a.real() * b.real() - a.imag() * b.imag();
	i = a.imag() * b.real() + b.imag() * a.real();
	return std::complex<int32_t>(r, i);
}

/*---------------------------------------------------------------------------- -
Apply the right shift specified as parameter then store the result in a 32 bits 
complex.
------------------------------------------------------------------------------*/
std::complex<int32_t> scale32(std::complex<int32_t> z, unsigned shift)
{
	return std::complex<int32_t>(z.real() >> shift, z.imag() >> shift);
}

/*---------------------------------------------------------------------------- -
Apply the right shift specified as parameter then store the result in a 16 bits
complex.\n
The value is limited before storage to the maximum value of a 16 bits integer.
------------------------------------------------------------------------------*/

std::complex<int16_t> limitScale16(std::complex<int32_t> z, unsigned shift)
{
	int32_t a, b;
	a = z.real() >> shift;
	if (abs(a) > INT16_MAX)
		a = a > 0 ? INT16_MAX : -INT16_MAX;
	b = z.imag() >> shift;
	if (abs(b) > INT16_MAX)
		b = b > 0 ? INT16_MAX : -INT16_MAX;
	return std::complex<int16_t>(a,b);
}