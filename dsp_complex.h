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
std::complex<int16_t> limitScale16(std::complex<int32_t> z, unsigned shift);


namespace dsptl
{



} //end of namespace

#endif