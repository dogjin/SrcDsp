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
#include <vector>
#include <limits>
#include <type_traits>


std::complex<int32_t> operator*(std::complex<int32_t> a, std::complex<int16_t> b);
std::complex<int32_t> operator*(std::complex<int16_t> a, std::complex<int32_t> b);
std::complex<int32_t> scale32(std::complex<int32_t> z, unsigned shift);
std::complex<uint32_t> scale32(std::complex<uint32_t> z, unsigned shift);
std::complex<int16_t> limitScale16(std::complex<int32_t> z, unsigned shift);

/***************************************************************************//**
Apply the specified right shift to the value z then limit the value of  z to the 
maximum value of the type of the output type.\n

This routine is only applicable when both the input and output  types are
integers.

tparam T The input if of type T (T is not deducible)
tparam U The output is of type U

param z Value to scale and limit


*******************************************************************************/
template <class T, class U, typename  std::enable_if<std::numeric_limits<T>::is_integer,int>::type * = nullptr>
T limitScale(U z, unsigned shift)
{
	// Verify that we are working with complex of integers value.
	static_assert(std::numeric_limits<U>::is_integer && std::numeric_limits<T>::is_integer, "");
	// Verify that U has at least as many bits than T
	static_assert(std::numeric_limits<U>::digits >= std::numeric_limits<T>::digits, "");
	U a;

	a = z >> shift;
	// Limit the real part
	if (a > std::numeric_limits<T> :: max() )
		a = std::numeric_limits<T> :: max();
	else if (a < std::numeric_limits<T>::lowest())
		a = std::numeric_limits<T> :: lowest();

	return a;

}



/***************************************************************************//**
Apply the specified right shift to the complex z then limit the real part and
imaginary part of z to the maximum value of the type of the output complex.\n
The enable_if restrict this function to be instantiated only when U and T are not
integer (ie they are complex)

This routine is only applicable when both the input and output complex types are
integers.

tparam T The input complex type (T is not deducible)
tparam U The output complex type 

param z Value to scale and limit


*******************************************************************************/
template <class T, class U, typename std::enable_if<!std::numeric_limits<T>::is_integer, int>::type* = nullptr>
T limitScale(U z, unsigned shift)
{
	// Verify that we are working with complex of integers value.
	static_assert(std::numeric_limits<typename U::value_type>::is_integer && std::numeric_limits<typename T::value_type>::is_integer, "");
	// Verify that U has at least as many bits than T
	static_assert(std::numeric_limits<typename U::value_type>::digits >= std::numeric_limits<typename T::value_type>::digits, "");
	typename U::value_type a, b;

	a = z.real() >> shift;
	// Limit the real part
	if (a > std::numeric_limits<typename T::value_type> :: max() )
		a = std::numeric_limits<typename T::value_type> :: max();
	else if (a < std::numeric_limits<typename T::value_type>::lowest())
		a = std::numeric_limits<typename T::value_type> :: lowest();

	b = z.imag() >> shift;
	// Limit the imaginary part
	if (b > std::numeric_limits<typename T::value_type> :: max() )
		b = std::numeric_limits<typename T::value_type> :: max();
	else if (b < std::numeric_limits<typename T::value_type>::lowest())
		b = std::numeric_limits<typename T::value_type> :: lowest();

	return T(a,b);

}


#if 0
/***************************************************************************//**
Apply the specified right shift to the complex z then limit the real part and
imaginary part of z to the maximum value of the type of the output complex.\n

This routine is only applicable when both the input and output complex types are
integers.

tparam T The input complex is of type complex<T> (T is not deducible)
tparam U The output complex is of type complex<U>

param z Value to scale and limit


*******************************************************************************/
template <class T, class U>
std::complex<T> limitScale(std::complex<U> z, unsigned shift)
{
	// Verify that we are working with complex of integers value.
	static_assert(std::numeric_limits<U>::is_integer && std::numeric_limits<T>::is_integer, "");
	// Verify that U has at least as many bits than T
	static_assert(std::numeric_limits<U>::digits >= std::numeric_limits<T>::digits, "");
	U a, b;

	a = z.real() >> shift;
	// Limit the real part
	if (a > std::numeric_limits<T> :: max() )
		a = std::numeric_limits<T> :: max();
	else if (a < std::numeric_limits<T>::lowest())
		a = std::numeric_limits<T> :: lowest();

	b = z.imag() >> shift;
	// Limit the imaginary part
	if (b > std::numeric_limits<T> :: max() )
		b = std::numeric_limits<T> :: max();
	else if (b < std::numeric_limits<T>::lowest())
		b = std::numeric_limits<T> :: lowest();

	return std::complex<T>(a,b);

}

#endif

/**
 * Multiply 2 complex numbers of int16_t then scale the output according to the
 * right shift specified
 *
 * @param a : first operand
 * @param b : second operand
 * @param rightShift: Number of bit right shift to perform
 *
 * @return Product of the operand shifted by rightShift bits
 */

inline std::complex<int16_t> multiplyShift(std::complex<int16_t> a, std::complex<int16_t> b, int rightShift)
{
	// This assert verifies that usual arithmetic conversion is performed from int16 to at least int32
	static_assert(sizeof(int) >= sizeof(int32_t),"");
	int32_t re = real(a)* real(b) - imag(a) * imag(b);
	int32_t im = imag(a) * real(b) + imag(b) * real(a);
	re >>= rightShift;
	im >>= rightShift;
	return std::complex<int16_t>(static_cast<int16_t> (re), static_cast<int16_t> (im));
}

/**
 * Multiply 1 complex numbers of int16_t by a real int16_t then scale the output according to the
 * right shift specified
 *
 * @param a : first operand
 * @param b : second operand
 * @param rightShift: Number of bit right shift to perform
 *
 * @return Product of the operand shifted by rightShift bits
 */

inline std::complex<int16_t> multiplyShift(std::complex<int16_t> a, int16_t b, int rightShift)
{
	// This assert verifies that usual arithmetic conversion is performed from int16 to at least int32
	static_assert(sizeof(int) >= sizeof(int32_t),"");
	int32_t re = real(a) * b;
	int32_t im = imag(a) * b ;
	re >>= rightShift;
	im >>= rightShift;
	return std::complex<int16_t>(static_cast<int16_t> (re), static_cast<int16_t> (im));
}



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

	template <class OutType, class InType>
	OutType sumPower(std::vector<std::complex<InType>> in)
	{
		OutType out{};
		for (size_t index = 0; index < in.size(); ++index)
		{
			out = in[index].real()*in[index].real() + in[index].imag()*in[index].imag();
		}
		return out;
	}

} //end of namespace

#endif
