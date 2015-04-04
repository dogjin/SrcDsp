/*-----------------------------------------------------------------------------
@file 

Definition of all DSP routines related to mutlirate filters
performing an upconversion.

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _UPSAMPLING_FILTER_FIR_H
#define _UPSAMPLING_FILTER_FIR_H

#include <cassert>
#include <vector>
#include "dsp_complex.h"



/*-----------------------------------------------------------------------------
FIR Filter


@tparam InType Type of the input signal. Can be float, double, complex, int...
@tparam OutType Type of the output signal
@tparam InternalType Type used internally for the computation
@tparam CoefType Type of the coefficients
@tparam N	Upsampling ratio

It is the responsibility of the caller to make sure that the different types 
work smoothly. Overflow and underflow conditions must not occur

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
class FilterUpsamplingFir
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
	unsigned top; 				///< Current insertion point in the history buffer 
	int leftShiftFactor;
};


/*-----------------------------------------------------------------------------
Upsampling FIR Filter Constructor

The constructor initializes the internal coefficient table and creates the internal
buffer for computations.

@param firCoeff Filter Coefficients

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::FilterUpsamplingFir(const std::vector<CoefType> &firCoeff)
		: coeff(firCoeff), top(0)
{
	// We verify that the number of coefficient is a multiple
	// of L
	assert(firCoeff.size() % L == 0);
	// The internal history buffer is sized according to the 
	// number of coefficients
	buffer.resize(firCoeff.size() / L);
	// Compute the scaling factor
	leftShiftFactor = static_cast<int>(round(log2(L)));

}


/*-----------------------------------------------------------------------------
Upsampling FIR Filter

The algorithm uses an internal buffer with a length equal to the number of coefficients
divided by the upsampling ratio \n
The current input value is inserted in the buffer at the correct location, then the convolution
is computed. The history buffer is processed with  a stride of 1 while the
coefficient buffer is processed with a stride of L\n

The user must make sure that the internal type is large enough to contain the
accumulated sum of the convolution operation\n
The user must make sure that the size of the output buffer is L times the size
of the input buffer

@param signal Input to the filter
@param filteredSignal Output of the filter. Must be the same size as signal

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
void FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal)
{
	#ifdef CPLUSPLUS11
	static_assert(signal.size() * L == filteredSignal.size(), "");
	#else
	assert(signal.size() * L  == filteredSignal.size());
	#endif

	InternalType y;  				// Output result
	unsigned  n;				// Counting indexes
	size_t histSize = buffer.size();	// Number of taps in the filter
	size_t inputSize = signal.size();	// Number of input samples


	for(unsigned j = 0; j < inputSize ; j++)
	{
	// This loop is executed for each of the input samples
		buffer[top] = signal[j];
		n = 0;
		
		// For each input sample, we compute L output samples
		for (size_t offset = 0; offset < L; ++offset)
		{
			n = offset;
			y = InternalType();

			// Process samples before and including Top
			for(int k = top; k >= 0; k--, n+= L)
			{
				y += coeff[n] * buffer[k];
			}
			// Process samples after Top
			for(unsigned k = histSize-1 ; k > top; k--, n+= L)
			{
				y += coeff[n]* buffer[k];
			}

			
			
			//filteredSignal[L*j + offset] = static_cast<OutType>(y);

			//filteredSignal[L*j + offset] = limitScale16(y,  15 - leftShiftFactor);
			filteredSignal[L*j + offset] = limitScale<OutType::value_type, InternalType::value_type>(y,  15 - leftShiftFactor);
		}
		top++;
		if(top >= histSize) top = 0;
	}

}

#endif
