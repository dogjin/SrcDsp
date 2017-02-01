/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which perform demodulation

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember

------------------------------------------------------------------------------*/

#ifndef DEMODULATORS_H
#define DEMODULATORS_H




#include <complex>
#include <cstdint>
#include <vector>
#include "constants.h"

namespace dsptl_private
{

}

namespace dsptl
{

	/*-----------------------------------------------------------------------------
	OQPSK Demodulator

	Primary template

	@tparam T Type of the input data to the demodulator

	------------------------------------------------------------------------------*/
	template<class InType>
	class DemodulatorOqpsk;



	/*-----------------------------------------------------------------------------
	OQPSK Demodulator

	Implementation for signed byte as input.
	The output of the demodulator is a vector of softbits. Each softbit is a signed
	8 bits value.

	------------------------------------------------------------------------------*/
	template<>
	class DemodulatorOqpsk<int16_t>
	{
	public:
		DemodulatorOqpsk();
		std::vector<int8_t> step(std::vector<std::complex<int16_t>> in, int32_t & error);
		void reset();
		void setSyncPattern(std::vector<int8_t> bits){ bitSyncPattern = bits; };
		/// Initial frequency of the loop. The input parameter is  in rad/samples with a sampling
		/// rate equal to the bit rate. It is converted to an integer increment value within the NCO phase lookup 
		/// table
		void setInitialFrequency(float f) { intialFreqEst = static_cast<int16_t>(round(f * onePi / dsptl::pi)); }
		/// Initial phase of the loop in radians
		void setInitialPhase(float p) { initialPhase = static_cast<int16_t>(round(p * onePi / dsptl::pi)); }
		/// Sets the reference vector which contains the I and Q of the preamble after removing the modulation
		void setReference(std::vector<std::complex<int16_t>> ref);
		/// Sets how many right shift of the input must be done to keep the input within 8 bits
		void setInputShift(int shift) { rightShift = shift; }
		/// Returns the averaged frequency in radians per sample of the last run of the demodulator
		float getMeasuredFrequency() {return static_cast<float>((accumulatedFrequency >> freqShift) * dsptl::pi / onePi);}


	private:
		struct StateVar{
			unsigned bitCnt;
			int mod2Cnt;
			int16_t phase;
			int bit1;
			int bit2;
			int16_t Iprev;
			int16_t Qprev;
			void reset()
			{
				bitCnt = 0; mod2Cnt = 0; phase = 0; bit1 = 0; bit2 = 0; Iprev = 0; Qprev = 0;
			}
		} stateVar;


	private:
		int16_t quickPhase(int16_t re, int16_t im);
		void phaseShift(int16_t inRe, int16_t inIm, int16_t & outRe, int16_t & outIm, int16_t phase);
		/// Sync pattern represented as an array of 0 and 1. If empty, it is assumed that there is
		/// no sync word
		std::vector<int8_t> bitSyncPattern;
		/// Reference bit samples. This samples correspond to the sync pattern in which the
		/// modulation has been removed.
		/// If empty, it is assumed that there is no sync word
		std::vector<int16_t> Iref;
		std::vector<int16_t> Qref;
		/// Initial frequency estimate
		int16_t intialFreqEst;
		/// Initial phase between 0 and twoPi
		int16_t initialPhase;
		/// Coefficients for demodulation
		static const int32_t g1 = 19333; // to prevent old compiler error
		static const int32_t g2 = 13107;
		/// Gain factor for PLL
		static const int32_t b0 = 8000;
		/// Number of samples used to compute an average frequency offset
		static const int nbrFreqSamples = 32;  // must be 2^freqShift
		int32_t accumulatedFrequency;
		static const int freqShift = 5; // Number of right shift to apply to accumulatedFrequency
		/// Number of right shift to perform on the input signal to keep it under 8 bits
		int rightShift ;

		const int32_t maxAmp;
		const int16_t twoPi;
		const int16_t onePi;
		const int16_t halfPi;
		/// Phase look up table with 8192 entries
		/// Each entry corresponds to an integer value of the real and imaginary part
		/// This should become static to prevent duplication
		std::vector<int16_t> phaseLUT;
		std::vector<int16_t> sineLUT;
		


	};


} // end of namespace


#endif
