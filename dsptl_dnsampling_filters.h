/*-----------------------------------------------------------------------------
@file 

Definition of all DSP routines related to mutlirate filters
performing a downconversion

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember

-----------------------------------------------------------------------------*/

#ifndef _DNSAMPLING_FILTER_FIR_H
#define _DNSAMPLING_FILTER_FIR_H

#include <cassert>
#include <vector>
#include "dsp_complex.h"
#include <cmath>


namespace dsptl
{


	/*-----------------------------------------------------------------------------
	FIR Downsampling filter


	@tparam InType Type of the input signal. Can be float, double, complex, int...
	@tparam OutType Type of the output signal
	@tparam InternalType Type used internally for the computation
	@tparam CoefType Type of the coefficients
	@tparam M	Downsampling ratio

	It is the responsibility of the caller to make sure that the different types 
	work smoothly. Overflow and underflow conditions must not occur

	@note The class is currently written to support int32_t coefficients, int16_t inputs
	and int16_t outputs. Internal precision is int32_t.

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	class FilterDnsamplingFir
	{
	public:
		/// Constructor. Coefficients are defined. Size for the internal
		/// buffer is reserved based on the number of coefficients
		FilterDnsamplingFir();
		FilterDnsamplingFir(const std::vector<CoefType> &firCoeff);
		// This function is called for each iteration of the filtering process
		void step(const std::vector<InType> & input, std::vector<OutType> & filteredSignal);
		// Reset the internal counters and buffers
		void reset()
		{
			for (size_t index = 0; index < history.size(); ++index)
				history[index] = InType();
		}
		void setCoeffs(const std::vector<CoefType>&);
		/// Set the gain of the upsampler in terms of left shift. The default gain is 
		/// about 0 dB
		void setLeftShiftBy2(int leftShiftBy2) {leftShift = leftShiftBy2;}
		
	private:
		std::vector<CoefType> coeff;		///< Coefficients
		std::vector<InType> history;  	///< History buffer
		unsigned coeffScaling;  // Can be used to scale back the result 
		int leftShift;          ///< Amount of left shift to perform on the decimated output related to the 0 dB
	};


	/*-----------------------------------------------------------------------------
	Upsampling FIR Filter Constructor

	This is the default constructor which create an object which is not initialized.
	For example the filter coefficients are not setup. They van be setup by calling 
	setCoeffs()

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	FilterDnsamplingFir<InType, OutType, InternalType, CoefType, M>::FilterDnsamplingFir()
	{};
	
	/*-----------------------------------------------------------------------------
	Upsampling FIR Filter Constructor

	The constructor initializes the internal coefficient table and creates the internal
	buffer to maintain history from one call to the other.\n
	There are no constraints on the number of coefficients

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	FilterDnsamplingFir<InType, OutType, InternalType, CoefType, M>::FilterDnsamplingFir
	(
		const std::vector<CoefType> &firCoeff ///< Vector of real coefficients
	)
	{
		setCoeffs(firCoeff);

	}


	/*-----------------------------------------------------------------------------
	Sets the coeffficients of the filter.

	The function initializes the internal coefficient table and creates the internal
	buffer to maintain history from one call to the other.\n
	There are no constraints on the number of coefficients

	@param firCoeff Filter Coefficients

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	void FilterDnsamplingFir<InType, OutType, InternalType, CoefType, M>::setCoeffs
	(
		const std::vector<CoefType> &firCoeff ///< vector of real coefficients
	)
	{
		// The number of coefficients msut always be a multiple of the 
		// decimation ratio
		assert(firCoeff.size() % M == 0);
		coeff.assign(firCoeff.begin(),firCoeff.end());

		// The internal history buffer is sized according to the 
		// number of coefficients
		history.resize(firCoeff.size()-1);
		// bit growth due to coefficient  and number of taps
		double sumMagnitude = 0;
		for (size_t index = 0; index < coeff.size(); ++index)
			sumMagnitude += abs(coeff[index]);
		coeffScaling = static_cast<int>(floor(log2(sumMagnitude)));
		leftShift = 0;
	}

	/*-----------------------------------------------------------------------------
	Downsampling FIR Filter

	M is the decimation ratio
	N is the number of coefficients of the filter ( high sampling rate)
	L is the number of samples in the input vector

	The algorithm computes a convolution sum every M samples where M is the decimation
	ratio.\n
	An internal buffer keeps the N-1 last samples from the previous call. \n
	No copy of the input samples is made except for the last N-1.\n

	The algorithm performs well as long as the number of input samples to process is
	large compared to the number of coefficients of the filter.\n


	The user must make sure that the internal type is large enough to contain the
	accumulated sum of the convolution operation\n
	The user must make sure that the size of the input buffer is a multiple of M and that 
	the size of the output buffer is 1/M times the size of the input buffer.\n
	The output is always scaled to maintain a gain of between 0 and 6dB

	@tparam InType Type of the input of the filter. If the input is complex, it must be
	specified as std::complex<...>
	@tparam OutType Type of the output of the filter. If  The output is complex, it must be specified as 
	std::complex<>
	@tparam InternalType Type used internally for the computation. If the type is complex, it
	must be specified as std::complex<...>
	@tparam CoefType Type fo the coefficients. The coefficients are assumed to be real (not complex)
	@tparam N Decimation ratio that the filter will implement

	@param input Input to the filter
	@param filteredSignal Output of the filter. Must be the same size as signal


	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	void FilterDnsamplingFir<InType, OutType, InternalType, CoefType, M>::step
	(
		const std::vector<InType> & input, 
		std::vector<OutType> & filteredSignal
	)
	{

		// There should be M times more samples at the input than the output
		assert(filteredSignal.size() * M == input.size());

		InternalType y;  				// Output result
		int N = coeff.size();	// Number of taps in the filter
		int inputSize = input.size();	// Number of input samples
		int outIndex;
		
		for(int j = 0; j < inputSize ; j+= M)
		{
			// This loop is executed for each M of the input samples
			y = InternalType{};
			outIndex = j / M;
			// Two scenarios must be considered
			// A: need for history data
			// B: no need for history data
		
			// The sample location is within the N-1 first samples
			if( j < N-1)
			{
				// History is needed
				for(int k = 0; k <= j; ++k)
					y += coeff[k] * input[j - k];
				for(int k = j+1 , p = 0; k < N ; ++k, ++p)
					y += coeff[k] * history[N -2 -p];
			}
			else
			{
				// History is not needed
				for(int k = 0; k < N; ++k)
					y += coeff[k] * input[j - k];
			}		
			// copy the result to its destination
			// By default, the data is scaled to provide a gain of about 0 dB
			// A non zero value of leftShift increases the gain by 2^leftShift
			filteredSignal[outIndex++] = limitScale16(y, coeffScaling - leftShift);
		}
		// We copy N-1 samples of historical data for the next iteration
		for (int k = 0; k < N - 1; ++k)
			history[k] = input[inputSize - N + 1 +k];
	}


} // end of namespace

#endif
