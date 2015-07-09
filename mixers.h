#ifndef MIXERS_H
#define MIXERS_H


#include <cstdint>
#include <complex>
#include "generators.h" // for pi
#include "dsp_complex.h"

namespace dsptl  // used to be dsptl_private but issue with gcc 453
{

	// BASE CLASS

	/*-----------------------------------------------------------------------------
	Base class for the implementation of the mixers

	@tparam InType Data type of the input signal
	@tparam OutType  Data type of the output signal
	@tparam PhaseType Type of the phase
	@tparam N Number of points for the sine table
	
	Upon construction, the frequency and the phase are initialized to zero
	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class PhaseType, unsigned N = 4096>
	class _Mixer
	{
	public:
		_Mixer():freq(), phi(), nominalFreq() {};
		void setFrequency(float loFreq);
		void reset(float loFreq = 0);
		void adjustFrequency(float loFreq = 0);
	protected:
		PhaseType phi;		///< Phase to be used to multiply the next sample
		PhaseType freq;    ///< Frequency in radian per sample (freq = N represents 2pi rad.samples)
		float nominalFreq; ///< normalized frequency between -1 and 1; This is used to maintain accuracy during frequency adjustments
		std::vector<PhaseType> ptable;   /// Sine table representing 0 to 2pi

	};


	/***********************************************************************//**
	Sets the internal frequency of the mixer

	@param loFreq Frequency of the local oscillator  in normalized frequency (-1 to 1)

	***************************************************************************/
	template<class InType, class OutType, class PhaseType, unsigned N >
	void _Mixer<InType, OutType, PhaseType, N>::setFrequency(float loFreq)
	{
		assert(loFreq <= 1 && loFreq >= -1);
		nominalFreq = loFreq;
		if (loFreq >= 0)
			freq = static_cast<PhaseType>(round(loFreq * N / 2));
		else
		{
			freq = static_cast<PhaseType>(round(N - round(-loFreq * N / 2)));
			// For very small negative values of loFreq, freq becomes 4096
			// We force it to the equivalent value of 0
			if (freq == N) freq = 0;
		}

		assert(freq >= 0 && freq < N);
	}

	/***********************************************************************//**
	Resets the state of the mixer. The frequency of the mixer is not affected

	***************************************************************************/
	template<class InType, class OutType, class PhaseType, unsigned N >
	void _Mixer<InType, OutType, PhaseType, N>::reset(float loFreq)
	{
		phi = PhaseType{};
	}

	/***********************************************************************//**
	Adjust the frequency of the mixer by the value provided. The adjustement is 
	done with continuous phase.\n

	@param adjustFreq Value of the adjustment in normalized frequency (-1 to +1)

	***************************************************************************/
	template<class InType, class OutType, class PhaseType, unsigned N >
	void _Mixer<InType, OutType, PhaseType, N>::adjustFrequency(float adjustFreq)
	{
		nominalFreq += adjustFreq;
		if (nominalFreq > 1) nominalFreq -= 2;
		if (nominalFreq < -1) nominalFreq += 2;
		setFrequency(nominalFreq);
	}

}

	
namespace dsptl
{


	//  DERIVED CLASS AND SPECIALIZATION


	/*-----------------------------------------------------------------------------
	Primary template 

	@tparam InType Data type of the input signal
	@tparam OutType  Data type of the output signal
	@tparam PhaseType Type of the phase.
	@tparam N Number of points for the sine table


	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class PhaseType, unsigned N>
	class Mixer;

	/*-----------------------------------------------------------------------------
	Mixer Specialization

	Input and output are 16 bits. Look up table is also 16 bits


	------------------------------------------------------------------------------*/
	template<unsigned N >
	class Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N > : public dsptl::_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >
	{
	public:
		Mixer();
		void step(std::vector<std::complex<int16_t>> & in, std::vector<std::complex<int16_t>> & out);

	};


	/*-----------------------------------------------------------------------------
	Mixer Specialization

	Input and output are 16 bits. Look up table is also 16 bits

	constructor


	------------------------------------------------------------------------------*/
	template<unsigned N >
	Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::Mixer()
	{
		// Build the phase lookup table.
		// The table is a sine table representing 0 to 2pi
		// The maximum amplitude is given by the value of max
		const int16_t max = (INT16_MAX >> 1);
		for (size_t k = 0; k < N; ++k)
			// full qualification of the _Mixer was added to remedy a bug in gcc 453
			_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::ptable.push_back( static_cast<int16_t>(max * sin(2 * dsptl::pi * static_cast<double>(k) / N)));

	};

	/*-----------------------------------------------------------------------------
	Performs the mixing between the input samples and the local oscillator

	@param in
	@param out
	------------------------------------------------------------------------------*/
	template <unsigned N >
	void Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::step(std::vector<std::complex<int16_t>> & in, std::vector<std::complex<int16_t>> & out)
	{
		// Full qualification of the base class members is due to a bug in gcc453
		for (size_t k = 0; k < in.size(); ++k)
		{		
			// To maintain a gain of 1 , the output scaling must correspond to the amplitude of the local oscillator
			out[k] = limitScale16(in[k] * std::complex<int32_t>(_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::ptable[(_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::phi + N / 4) % N],
							_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::ptable[_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::phi]), 14);
			_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::phi = (_Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::phi + _Mixer<std::complex<int16_t>, std::complex<int16_t>, int16_t, N >::freq) % N;
			
#if 0
			// Same as above without the full qualification
			// To maintain a gain of 1 , the output scaling must correspond to the amplitude of the local oscillator
			out[k] = limitScale16(in[k] * std::complex<int32_t>(this->ptable[(this->phi + N / 4) % N],
							this->ptable[this->phi]), 14);

			this->phi = (this->phi + this->freq) % N;
#endif
		}
	}

} // end of namespace





#endif