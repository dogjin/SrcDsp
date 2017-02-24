/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which allow file storage of samples
in specific format



------------------------------------------------------------------------------*/

#ifndef _DSPTL_FILE_SAVING_H
#define _DSPTL_FILE_SAVING_H

#include <cassert>
#include <vector>
#include <complex>
#include <fstream>



namespace dsptl
{
	
	/**
	Saves a container of real data in a file. Each data point is entered as a
	binary value.

    @tparam Type Value type of the container. Must be a numeric type
    @tparam Container Container type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template<class Type, class Allocator, template<class, class> class Container>
	void saveBinarySamples(Container<Type, Allocator> & in, std::ofstream & os)
	{
		// In order to potentially deal with containers which do not have continuous storage, we iterate
		// over all the elements
		for(auto &e:in)
		{
			os.write(reinterpret_cast<char *>(&e), sizeof(Type));
		}
	}

	/**
	Saves a vector of  real data in a file. Each data point is entered as a
	binary value. The fact that vector uses contiguous memory is used

    @tparam Type Value type of the vector. Must be a numeric type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template<class Type >
	void saveBinarySamples( std::vector<Type> & in, std::ofstream & os)
	{
		// We use the fact that the vector has a contiguous memory storage
		os.write(reinterpret_cast<char *>(in.data()), in.size() * sizeof(Type));
	}

	
	/**
	Saves a container of complex data in a file. Each I and Q point is entered as a
	binary value. The I and Q values are interleaved

    @tparam Type Value type of the container. Must be a numeric type
    @tparam Container Container type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template<class Type, class Allocator, template<class, class> class Container>
	void saveBinarySamples(Container<std::complex<Type>, Allocator> & in, std::ofstream & os)
	{
		// In order to potentially deal with containers which do not have continuous storage, we iterate
		// over all the elements
		// However, we also assume here that each complex real and imaginary parts are stored contiguously in 
		// memory
		static_assert(sizeof(std::complex<Type>) == 2 * sizeof(Type), "");
		for(auto &e:in)
		{
			os.write(reinterpret_cast<char *>(&e), sizeof(std::complex<Type>));
		}
	}

	/**
	Saves a vector of  complex  data in a file. Each I and Q is entered as a
	binary value. The fact that vector uses contiguous memory is used. It is also
	assumed that the complex element is itself stored contiguously in memory.

    @tparam Type Value type of each element of the vector. Must be a numeric type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template<class Type >
	void saveBinarySamples( std::vector<std::complex<Type>> & in, std::ofstream & os)
	{
		// We use the fact that the vector has a contiguous memory storage and the assumption that
		// complex<Type> are stored in contiguous memory
		static_assert(sizeof(std::complex<Type>) == 2*sizeof(Type), "");
		os.write(reinterpret_cast<char *>(in.data()), in.size() * sizeof(std::complex<Type>));
	}



	/**
	Saves a container of real data in a file. Each data point is formatted
    ASCII and is on its own line.	

    @tparam Type Value type of the container. Must be a numeric type
    @tparam Container Container type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template<class Type,class Allocator, template<class,class> class Container>
	void saveAsciiSamples(const Container<Type, Allocator> & in, std::ofstream & os)
	{
		typename Container<Type, Allocator>::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			os << *it << '\n';
		}
	}

	/**
	Saves a std::vector<int8_t> data in a file. Each data point is formatted
    ASCII and is on its own line.	

    @tparam Type Value type of the container. Must be a numeric type
    @tparam Container Container type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	void saveAsciiSamples(const std::vector<int8_t> & in, std::ofstream & os)
	{
		std::vector<int8_t>::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			// Forces the interpretation as a number not as a character
			os << static_cast<int>(*it) << '\n';
		}
	}


	/**
	Saves a container of complex data in a file as interleaved I and Q. Each value is
	on a separate line.	

	@tparam Type Type used in the complex values used in the container
    @tparam Container Container type

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	template< class Type, class Allocator, template<class, class> class Container>
	void saveAsciiSamples(const Container<typename std::complex<Type>,Allocator > & in, std::ofstream & os)
	{
		typename Container<std::complex<Type>, Allocator>::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			os << it->real() << '\n' << it->imag() << '\n';
		}
	}

	/**
	Saves a container of complex<int8_t> data in a file as interleaved I and Q. Each value is
	on a separate line.	

	@param in Container to be iterated to write the values in a file
	@param os Stream to use to write the  output	
	*/
	void saveAsciiSamples(const std::vector<std::complex<int8_t> > & in, std::ofstream & os)
	{
		std::vector<std::complex<int8_t> >::const_iterator it;
		for (it = in.cbegin(); it != in.cend(); ++it)
		{
			// Forces the interpretation as a number not as a character
			os << static_cast<int>(it->real()) << '\n' << static_cast<int>(it->imag()) << '\n';
		}
	}
	
} // End of namespace

#endif
