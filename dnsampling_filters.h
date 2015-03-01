/*-----------------------------------------------------------------------------
@file 

Definition of all DSP routines related to mutlirate filters
performing a downconversion

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _DNSAMPLING_FILTER_FIR_H
#define _DNSAMPLING_FILTER_FIR_H

#include <cassert>
#include <vector>


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

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	class FilterDnsamplingFir
	{
	public:
		/// Constructor. Coefficients are defined. Size for the internal
		/// buffer is reserved based on the number of coefficients
		FilterUpsamplingFir(const std::vector<CoefType> &firCoeff);
		// This function is called for each iteration of the filtering process
		void step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal);
		// Reset the internal counters and buffers
		void reset()
		{
			top = 0;
			for (size_t index = 0; index < buffer.size(); ++index)
				buffer[index] = InType();
		}
		
	private:
		std::vector<CoefType> coeff;		///< Coefficients
		std::vector<InType> buffer;  	///< History buffer
	};


	/*-----------------------------------------------------------------------------
	Upsampling FIR Filter Constructor

	The constructor initializes the internal coefficient table and creates the internal
	buffer to maintain history from one call to the other.\n
	There are no constraints on the number of coefficients

	@param firCoeff Filter Coefficients

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	FilterDnsamplingFir<InType, OutType, InternalType, CoefType, M>::FilterDnsamplingFir(const std::vector<CoefType> &firCoeff)
			: coeff(firCoeff)
	{

		// The internal history buffer is sized according to the 
		// number of coefficients
		buffer.resize(firCoeff.size());
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
	large compared to the number of coefficients of the filter.\


	The user must make sure that the internal type is large enough to contain the
	accumulated sum of the convolution operation\n
	The user must make sure that the size of the input buffer is a multiple of M and that 
	the size of the output buffer is 1/M times the size of the input buffer

	@param input Input to the filter
	@param filteredSignal Output of the filter. Must be the same size as signal

	------------------------------------------------------------------------------*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned M>
	void FilterUpsamplingFir<InType, OutType, InternalType, CoefType, M>::step(const std::vector<InType> & input, std::vector<OutType> & filteredSignal)
	{

		// There should be M times more samples at the input than the output
		assert(filteredSignal.size() * M == input.size());

		InternalType y;  				// Output result
		size_t histSize = buffer.size();	// Number of taps in the filter
		size_t inputSize = input.size();	// Number of input samples
		
		for(unsigned j = 0; j < inputSize ; j+= M)
		{
		// This loop is executed for each M of the input samples
		y = InternalType{};
		// Two scenarios must be considered
		// A: need for history data
		// B: no need for history data
		
		// The sample location is within the N-1 first samples
		if( j < N-1)
		{
			// History is needed
			for(int k = 0; k <= j; ++k)
				y += coeff[k] * input[j - k];
			for(int k = j+1 , int p = 0; k < N ; ++k, ++p)
				y += coeff[k] * history[N -2 -p];
		}
		else
		{
			// History is not needed
			for(int k = 0; k < N; ++k)
				y += coeff[k] * input[j - k];
			// copy the result to its destination
			filteredSignal[outIndex++] = static_cast<OutType>(y);
		}
		
		// We copy N-1 samples of historical data for the next iteration
		std::copy(&input[inputSize - N + 1], &input[inputSize],&buffer[0]);
		
		}

	}

} // end of namespace

#endif
