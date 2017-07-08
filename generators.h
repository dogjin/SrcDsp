/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines related waveform generators
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _GENERATOR_H
#define _GENERATOR_H

#include <cassert>
#include <vector>
#include <complex>
#include <array>
#include <cmath>
#include <iostream>
#include "constants.h"

// Uncomment the following line to include the C++ specific syntax
//#define CPLUSPLUS11

namespace dsptl
{


	/*-----------------------------------------------------------------------------
	Sinewave generator. Generates sinewaves

	@tparam OutType Type of the output signal

	@param frequency Sinewave frequency in normalized units between 0 and 1
	@param amplitude Peak amplitude of the sinewave

	The computation are done internally as double and converted to the desired type
	with the desired amplitude\n
	The class has a specialization for the complex output type

	Usage
	@arg create an object of the class
	@arg call the step function with a reference to the vector to be filled with the sinewave

	------------------------------------------------------------------------------*/
	template<class OutType>
	class GenSine
	{
	public:
		/// Constructor. 
		GenSine(double frequency, OutType amplitude) : ampl(amplitude)
		{
			freq = frequency * pi;
			phase = 0;
		}
		// Generate a number of samples equal to the size of the buffer given
		void step(std::vector<OutType>& out)
		{
			for (unsigned i = 0; i < out.size(); ++i)
			{
				phase += freq;
				// Limit the phase between 0 and 2pi
				if (phase >= 2 * pi) phase -= 2 * pi;
				if (phase < 0 ) phase += 2* pi;
				// Compute the sample for this phase
				out[i] = static_cast<OutType>(cos(phase) * ampl);
			}

		}
		/// reset the internal phase of the generator
		void reset()
		{
			phase = 0;
		}

	private:
		OutType ampl; // output level
		double freq; //rad_per_sample
		double phase;  //radians
	};

	template<class OutType>
	class GenSine<std::complex<OutType> >
	{
	public:
		/// Constructor. 
		GenSine(double frequency, OutType amplitude) :  ampl(amplitude)
		{
			phase = 0;
			freq = frequency * pi;
			std::cout << "Frequency" << freq << "rad/samples\n";
		}
		// Generate a number of samples equal to the size of the buffer given
		void step(std::vector<std::complex<OutType> >& out)
		{
			for (unsigned i = 0; i < out.size(); ++i)
			{
				phase += freq;
				// We maintain the phase between 0 and 2pi
				if (phase >= 2* pi) phase -= 2 * pi;
				if (phase < 0 ) phase += 2*pi;
				// Compute the sample for this phase value
				out[i] = std::complex<OutType>(static_cast<OutType>(cos(phase) * ampl), static_cast<OutType>(sin(phase) * ampl));
			}

		}
		/// reset the internal phase of the generator
		void reset()
		{
			phase = 0;
		}

	private:
		OutType ampl; // output level
		double freq; //rad_per_sample
		double phase;  //radians
	};



	/*-----------------------------------------------------------------------------
	Create a table of cosine values. The values correspond to one quadrant of the
	full circle. The length of the quadrant is indicated by the length template argument.
	This function is intended to be a constexpr function

	@tparam OutType Type of the output signal
	@tparam length  Number of points in the quadrant

	@param amplitude Peak amplitude of the waveform

	The computation are done internally as double and converted to the desired type
	with the desired amplitude\n.
   In order to support floating point no rounding is done, only a static cast to the 
	desired type is perfomed.   

	------------------------------------------------------------------------------*/
	template<typename Type, size_t length>
		std::array<Type,length> makeCosTable(Type amplitude)
		{
			const int QS = length;
			std::array<Type, QS> cosTable;
			int phase = 0;
			for(auto &e:cosTable)
			{
				e = static_cast<Type>(lround(amplitude * cos(phase * 2 * dsptl::pi / (4 * QS))));
				phase ++;
			}
			return cosTable;
		}



} // End of namespace

#endif
