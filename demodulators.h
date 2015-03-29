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
		/// Initial frequency of the loop in rad/samples
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
		struct {
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

#if 0
	/*-----------------------------------------------------------------------------
	Constructor

	Create the sine and phase lookup table

	------------------------------------------------------------------------------*/
	DemodulatorOqpsk<int16_t>::DemodulatorOqpsk():
		twoPi(8192), onePi(4096), halfPi(2048), maxAmp(128)
	{
		// Build the phase lookup table
		int16_t phase;
		phaseLUT.assign(maxAmp * maxAmp, {});
		for (int im = 0; im < maxAmp ; ++im)
			for (int re = 0; re < maxAmp; ++re)
			{
				auto tmp = atan2(im, re) * onePi / dsptl::pi;
				auto tmp2 = round(atan2(im, re) * onePi / dsptl::pi);
				phase = static_cast<int16_t>(round(atan2(im, re) * onePi / dsptl::pi));
				phaseLUT[im * maxAmp + re] = phase;
			}

		// Build the sine lookup table
		sineLUT.assign(twoPi, {});
		for (int index = 0; index < twoPi ; ++index)
		{
			double phase = index * (dsptl::pi / static_cast<double>(onePi));
			sineLUT[index] = static_cast<int16_t>(sin(phase) * INT16_MAX);
		}
		reset();
	
	}


	int16_t DemodulatorOqpsk<int16_t>::quickPhase(int16_t re, int16_t im)
	{

		// Determine which quadrant is used
		bool quad2 = re <= 0 && im > 0;
		bool quad3 = re < 0 && im <= 0;
		bool quad4 = re >= 0 && im < 0;
		
		// rotate the input to the first quadrant
		re = abs(re);
		im = abs(im);

		// Divide by 2 to fit it in the range of the LUT
		while (re >= maxAmp || im >= maxAmp)
		{
			re = static_cast<int32_t>((re + 1) / 2);
			im = static_cast<int32_t>((im + 1) / 2);
		}
		// Read the look up table
		int32_t addr = maxAmp * im + re;
		int16_t phase = phaseLUT[addr];

		// Correct the value to compensate for the rotation
		if (quad2)
			phase = onePi - phase;
		else if (quad3)
			phase = phase - onePi;
		else if (quad4)
			phase = -phase;

		return phase;
	}

	/*-----------------------------------------------------------------------------
	Rotate the input point (inRe,inIm) by the phase and write the result in (outRe, outIm)

	@param inRe Real part of the vector to rotate
	@param inIm Imaginary part of the vector to rotate
	@param outRe Real part of the result
	@param outIm Imaginary part of the result
	@param phase Amount to rotate expressed as an integer between  0 and twoPi -1

	Internally, the rotation is done with 32 bits arithmetic then cast to a 16 bit
	result. \n
	The cos and sin are extracted from the look-up table
	------------------------------------------------------------------------------*/
	void DemodulatorOqpsk<int16_t>::phaseShift(int16_t inRe, int16_t inIm, int16_t & outRe, int16_t & outIm, int16_t phase)
	{
		assert(phase >= 0 && phase < twoPi);
		int32_t s = sineLUT[phase];
		int16_t phaseCos = (phase + halfPi) % twoPi;
		int32_t c = sineLUT[phaseCos];
		int32_t tmpI = inRe * c + inIm * s;
		int32_t tmpQ = inIm*c - inRe * s;

		outRe = static_cast<int16_t>((tmpI + 16384) >> 15);
		outIm = static_cast<int16_t>((tmpQ + 16384) >> 15);


	}

	/*-----------------------------------------------------------------------------
	Sets the reference. The reference is the bit samples of the sync word after
	removal of the modulation
	------------------------------------------------------------------------------*/
	void DemodulatorOqpsk<int16_t>::setReference(std::vector<std::complex<int16_t>> ref)
	{
		Iref.assign(ref.size(), {});
		Qref.assign(ref.size(), {});
		for (size_t index = 0; index < ref.size(); ++index)
		{
			Iref[index] = ref[index].real();
			Qref[index] = ref[index].imag();

		}



	}

	/*-----------------------------------------------------------------------------
	Demodulate the provided input. The input consists of one sample for each
	output bit.

	@param in Bit samples of the waveform to demodulate
	@param error Sum of the magnitude of the phase error

	------------------------------------------------------------------------------*/

	std::vector<int8_t> DemodulatorOqpsk<int16_t>::step(std::vector<std::complex<int16_t>> in, int32_t & error)
	{
		std::vector<int8_t> softBits;
		size_t numIn = in.size();

		// Initialize local variables
		auto bitCnt = stateVar.bitCnt;
		auto mod2Cnt = stateVar.mod2Cnt;
		auto phaseAcc = stateVar.phase;
		auto bit1 = stateVar.bit1;
		auto bit2 = stateVar.bit2;
		auto Iprev = stateVar.Iprev;
		auto Qprev = stateVar.Qprev;

		// Allocate vector for softbit values
		// There are as many output softbit than there are input values, except for the
		// first time that the function is called.
		if (bitCnt == 0 && !bitSyncPattern.empty())
		{
			// It is assumed that the number of input values is higher than 
			// the size of the bitSyncPattern
			assert(numIn > bitSyncPattern.size());
			softBits.assign(numIn - bitSyncPattern.size(), {});
		}
		else
			softBits.assign(numIn, {});

		int softCnt{}; // Number of output soft decisions
		auto errAcc = int32_t{};

		// Loop processing
		for (unsigned k = 0; k < numIn; ++k)
		{
			// phaseAcc = 0 // DEBUG , forces open loop
			++bitCnt;
			// Select the next sample to process depending if we have a sync word or not and if
			// it is the beginning of a frame
			int16_t I, Q, reErrVec, imErrVec, samp, bit0, reSig, imSig;
			int32_t tmp32;
			if (!bitSyncPattern.empty() && bitCnt < bitSyncPattern.size())
			{
				I = Iref[k];
				Q = Qref[k];
				// Phase correction
				phaseShift(I, Q, I, Q, phaseAcc);
				reErrVec = I;
				imErrVec = Q;
			}
			else
			{
				I = in[k].real();
				Q = in[k].imag();
				phaseShift(I, Q, I, Q, phaseAcc);
				if (!bitSyncPattern.empty() && bitCnt == bitSyncPattern.size())
				{
					// We transition from the preamble to the data size
					reErrVec = 0;
					imErrVec = 0;

				}
				else
				{
					if (mod2Cnt == 0)
					{
						// Eye  opening in I branch
						samp = I;  // Soft bit decision
						bit0 = 2 * (samp > 0) - 1; // hard decision
						// REconstruct previous complex signal
						reSig = g2* (bit2 + bit0);
						imSig = g1*bit1;

					}
					else
					{
						// Eye opening in Q branch -- Odd bit interval
						samp = Q;
						bit0 = 2 * (samp > 0) - 1;
						// Reconstruct previous complex signal
						reSig = g1*bit1;
						imSig = g2*(bit2 + bit0);
					}
					// Make sure that samp is between -128 and 127 . If not saturate
					if (samp > 127) samp = 127;
					else if (samp < -128) samp = -128;

					bit2 = bit1;
					bit1 = bit0;
					// Store the soft decision bit
					softBits[softCnt++] = static_cast<int8_t>(samp);
					// Compute error vector
					tmp32 = Iprev * reSig + Qprev * imSig;
					reErrVec = static_cast<int16_t>((tmp32 + 16384) / 32768);
					tmp32 = Qprev*reSig - Iprev*imSig;
					imErrVec = static_cast<int16_t>((tmp32 + 16384) / 32768);

				}
			}
			// Store the current phase corrected sample
			Iprev = I;
			Qprev = Q;
			// Extract the phase error
			int16_t err = quickPhase(reErrVec, imErrVec);
			// Accumulate the magnitude of the phase error
			errAcc += err > 0 ? err : -err;
			// Apply PLL loop
			tmp32 = b0*err;
			auto freqCorr = static_cast<int16_t>((tmp32 + 32768) / 65536);
			// The phase error is in the range -4096 to 4096 but the accumulated phase is in the range 0 to 8191
			int16_t step = intialFreqEst + freqCorr;
			auto tmp = phaseAcc + step + twoPi;  // force the value to be positive
			phaseAcc = (tmp % twoPi);
			// Toggle between even and odd bit
			mod2Cnt = (mod2Cnt + 1) % 2;

		} // End of bit processing

		error = errAcc;
		// State variables are updated
		stateVar.bitCnt = bitCnt;
		stateVar.mod2Cnt = mod2Cnt;
		stateVar.phase = phaseAcc;
		stateVar.bit1 = bit1;
		stateVar.bit2 = bit2;
		stateVar.Iprev = Iprev;
		stateVar.Qprev = Qprev;

		return softBits;
	}

	/*-----------------------------------------------------------------------------
	Clears all the state variables of the demodulator. This is typically done at the
	beginning of each frame. \n
	It is also important to call this function after setting the reference vector

	------------------------------------------------------------------------------*/

	void DemodulatorOqpsk<int16_t>::reset()
	{
		stateVar.reset();
		int num = bitSyncPattern.size();
		rightShift = 0;
		if (!bitSyncPattern.empty())
		{
			// We set bit 1 and bit 2 respectively to the last bit and the last before last
			// bit of the sync pattern
			assert(num >= 2);
			stateVar.bit1 = 2 * bitSyncPattern[num - 1] - 1;
			stateVar.bit2 = 2 * bitSyncPattern[num - 2] - 1;

		}

	}

#endif

} // end of namespace


#endif