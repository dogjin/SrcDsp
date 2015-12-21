/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which perform symbol mapping and
modulation.

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef DSPTL_MODULATORS_H
#define DSPTL_MODULATORS_H


#include <complex>
#include <cstdint>
#include <vector>
#include <cassert>

namespace dsptl_private
{
	/*-----------------------------------------------------------------------------
	Modulator nominal values for different type of output

	------------------------------------------------------------------------------*/
	template<class T>
	struct ModAmplitude;

	template<>
	struct ModAmplitude < int16_t >
	{
		ModAmplitude(){ value = 8192; }
		int16_t value;
	};

	template<>
	struct ModAmplitude < int8_t >
	{
		static const int8_t value = 32;
	};

	template<>
	struct ModAmplitude < int32_t >
	{
		static const int32_t value = 8192;
	};

	template<>
	struct ModAmplitude < float >
	{
		ModAmplitude(){ value = 0.707f; }
		float value;
	};
	template<>
	struct ModAmplitude < double >
	{
		ModAmplitude(){ value = 0.707; }
		double value ;
	};
}

namespace dsptl
{

	/*-----------------------------------------------------------------------------
	Mapping of bits in a sequence of complex symbols for SDPSK

	Takes a bit pattern and generates the modulation symbols for an SDPSK modulation\n
	The output of the modulator is always complex with the type specified as template
	parameter.\n
	There are as many output symbols as the number of input bits

	@tparam T Each output sample is of the type complex<T>


	------------------------------------------------------------------------------*/
	template<class T>
	class SymbolMapperSdpsk
	{
	public:
		/// Default constructor
		SymbolMapperSdpsk()
		{
			//static_assert(std::is_signed<T>::value);
			T amplitude;
			amplitude = dsptl_private::ModAmplitude < T > {}.value;

			map[0] = { amplitude, amplitude };
			map[1] = { static_cast<T>(-amplitude), amplitude };
			map[2] = {  static_cast<T>(-amplitude),  static_cast<T>(-amplitude) };
			map[3] = { amplitude,  static_cast<T>(-amplitude) };
			reset();

		}

		/// Performs the modulation as required.
		/// The input bits can be either 0,1 or -1,1
		void step(const std::vector<uint8_t> & bits , std::vector<std::complex<T> > & out)
		{
			const int N = 4;
			assert(bits.size() == out.size());
			for (size_t i = 0; i < bits.size(); i++)
			{
				// Generates a state from 0 to 3
				state += (bits[i] > 0)? 1 : (N-1);
				state = state % N;
				// Map the state to the correct amplitude
				std::complex<T> tmp = map[state];
				out[i] = tmp;
				
			}
		}

		/// Resets the state to a known value
		void reset()
		{
			state = 0;
		}


	private:
		int state;
		std::complex<T> map[4];


	};


	/*-----------------------------------------------------------------------------
	Mapping of bits into symbols for QPSK or OQPSK modulations

	Takes a bit pattern and generates the modulation symbols for a non-differential quadrature
	modulation\n
	The output of the symbol mapping is complex<T>.\n
	The number of bits at the input of the step() function must be even and the
	number of returned symbols is 1/2 the number of input bits

	@tparam T Each output symbol is of the type complex<T>


	------------------------------------------------------------------------------*/
	template<class T>
	class SymbolMapperQpsk
	{
	public:
		/// Default constructor
		SymbolMapperQpsk()
		{
			//static_assert(std::is_signed<T>::value);
			T amplitude;
			amplitude = dsptl_private::ModAmplitude < T > {}.value;
			// Create the constellation map Gray encoding is used
			map[0] = { static_cast<T>(-amplitude), static_cast<T>(-amplitude)};
			map[1] = { static_cast<T>(-amplitude), static_cast<T>(amplitude) };
			map[3] = {  static_cast<T>(amplitude),  static_cast<T>(amplitude) };
			map[2] = {  static_cast<T>(amplitude) ,  static_cast<T>(-amplitude) };

			reset();

		}

		/// Performs the modulation as required
		/// The bits vector can either consists of 0, 1 or -1, 1
		void step(const std::vector<uint8_t> & bits , std::vector<std::complex<T> > & out)
		{
			assert(bits.size() == 2 * out.size());

			// Bits are grouped 
			for (size_t i = 0; i < bits.size() /2 ; i++)
			{
				// The sequence of bits 00110110 become the symbols 0, 3, 1, 2
				state = ((bits[2 * i] > 0 ?1:0) << 1) | (bits[2 * i + 1] > 0? 1:0);

				// Map the state to the correct amplitude
				std::complex<T> tmp = map[state];
				out[i] = tmp;
				
			}
		}

		/// Resets the state to a known value
		void reset()
		{
			state = 0;
		}


	private:
		int state;
		std::complex<T> map[4];


	};


}

#endif

