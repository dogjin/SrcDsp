/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which allow file storage of samples
in specific format
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _FILE_SAVING_H
#define _FILE_SAVING_H

#include <cassert>
#include <vector>
#include <complex>
#include <fstream>


// Uncomment the following line to include the C++ specific syntax
//#define CPLUSPLUS11

namespace dsptl
{
	
	/*-----------------------------------------------------------------------------
	Saves the container data in the specified file as binary data

	@tparam Type Conatiner in which the samples are stored
	------------------------------------------------------------------------------*/
	template<class Container>
	void saveBinarySamples(Container & in, std::ofstream & os)
	{
		os.write(reinterpret_cast<char *>(in.data()), in.size() * sizeof(typename Container::value_type));
	}

	/*-----------------------------------------------------------------------------
	Saves the vector data in the specified file as binary data

	@tparam Type Container in which the samples are stored
	------------------------------------------------------------------------------*/
#ifdef UNDEFINED
	template<class Container, class Type>
	void saveBinarySamples(const Container<std::complex<Type> > & in, std::ofstream & os)
	{
		os.write(reinterpret_cast<char *>(in.data()), in.size() * sizeof(Type));
	}
	
#endif







	/*-----------------------------------------------------------------------------
	Saves the vector data in the specified file as ASCII data

	@tparam Type Values stored in the container
	------------------------------------------------------------------------------*/
	template<class Container>
	void saveAsciiSamples(const Container & in, std::ofstream & os)
	{
		typename Container::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			os << *it << '\n';
		}
	}

	void saveAsciiSamples(const std::vector<int8_t> & in, std::ofstream & os)
	{
		std::vector<int8_t>::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			// Forces the interpretation as a number not as a character
			os << static_cast<int>(*it) << '\n';
		}
	}


	void saveAsciiSamples(const std::vector<std::complex<int8_t> > & in, std::ofstream & os)
	{
		std::vector<std::complex<int8_t> >::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			// Forces the interpretation as a number not as a character
			os << '(' << static_cast<int>(it->real()) << ',' << static_cast<int>(it->imag()) << ')' << '\n';
		}
	}

	/*-----------------------------------------------------------------------------
	Saves the vector data in the specified file as ASCII data

	@tparam Type Values stored in the container
	------------------------------------------------------------------------------*/
	template<class Type, template<class> class Container>
	void saveAsciiSamples(const Container<typename std::complex<Type> > & in, std::ofstream & os)
	{/*
		Container::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			os << *it.real() << '\n' << *it.imag() << '\n';
		}*/
	}


} // End of namespace

#endif
