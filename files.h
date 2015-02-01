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

	@tparam Type Conatiner in which the samples are stored
	------------------------------------------------------------------------------*/
	template<class Type>
	void saveBinarySamples(const std::vector<Type> & in, std::ofstream & os)
	{
		os.write(reinterpret_cast<char *>(in.data()), in.size() * sizeof(Type));
	}

	
	/*-----------------------------------------------------------------------------
	Saves the vector data in the specified file as ASCII data

	@tparam Type Vakues stored in the container
	------------------------------------------------------------------------------*/
	template<class Type>
	void saveAsciiSamples(const std::vector<Type> & in, std::ofstream & os)
	{
		typename std::vector<Type>::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			os << *it << '\n';
		}
	}

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

} // End of namespace

#endif
