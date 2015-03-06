/*-----------------------------------------------------------------------------
@file 

Definition of all DSP routines related to filters.
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef _FILTER_FIR_H
#define _FILTER_FIR_H

#include <cassert>
#include <vector>
#include "dsp_complex.h"


// Uncomment the following line to include the C++ specific syntax
//#define CPLUSPLUS11


/*-----------------------------------------------------------------------------
FIR Filter


@tparam InType Type of the input signal. Can be float, double, complex, int...
@tparam OutType Type of the output signal
@tparam InternalType Type used internally for the computation
@tparam CoefType Type of the coefficients

It is the responsibility of the caller to make sure that the different types 
work smoothly. Overflow and underflow conditions must not occur

The filter has been verified not to saturate with a sine input of 32000 and a
set of coefficients in coeffRrc

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
class FilterFir
{
public:
	/// Constructor. Coefficients are defined. Size for the internal
	/// buffer is reserved based on the number of coefficients
	FilterFir(const std::vector<CoefType> &firCoeff);
	// This function is called for each iteration of the filtering process
	void step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal);
	void reset();
	
private:
	std::vector<CoefType> coeff;		///< Coefficients
	std::vector<InternalType> buffer;  	///< History buffer
	unsigned top; 							///< Current insertion point in the history buffer 
	int coeffScaling;
};


/*-----------------------------------------------------------------------------
FIR Filter Constructor

The constructor initializes the internal coefficient table and creates the internal
buffer for computations.

@param firCoeff Filter Coefficients

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
FilterFir<InType, OutType, InternalType, CoefType>::FilterFir(const std::vector<CoefType> &firCoeff)
		: coeff(firCoeff), top(0)
{
	buffer.resize(firCoeff.size());
	// Compute the energy in the coefficients
	// bit growth due to coefficient  and number of taps
	double sumMagnitude = 0;
	for (size_t index = 0; index < coeff.size(); ++index)
		sumMagnitude += abs(coeff[index]);
	coeffScaling = static_cast<int>(floor(log2(sumMagnitude)));
}

/*-----------------------------------------------------------------------------
Reset the internal state of the filter. All history is cleared.


------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
void FilterFir<InType, OutType, InternalType, CoefType>::reset()
{
	for (size_t index = 0; index < buffer.size(); ++index)
	{
		buffer[index] = InternalType{};
	}
}


/*-----------------------------------------------------------------------------
FIR Filter

The algortihm uses an internal buffer with a length equal to the number of coefficients.
The current input value is inserted in the buffer at the correct location, then the convolution
is computed. \n
The user must make sure that the internal type is large enough to contain the
accumulated sum of the convolution operation
Filtering can be made in place.

@param signal Input to the filter
@param filteredSignal Output of the filter. Must be the same size as signal

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
void FilterFir<InType, OutType, InternalType, CoefType>::step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal)
{
	#ifdef CPLUSPLUS11
	static_assert(signal.size() == filteredSignal.size(), "");
	#else
	assert(signal.size() == filteredSignal.size());
	#endif

	InternalType y;  							// Internal computation type
	unsigned  n;						// Counting indexes
	size_t numTaps = buffer.size();		// Number of taps in the filter
	size_t inputSize = signal.size();		// Number of input samples


	for(unsigned j = 0; j < inputSize ; j++)
	{
	// This loop is executed for each of the input samples
		buffer[top] = signal[j];
		y = InternalType();
		n = 0;

		// Process samples before and including Top
		for(int k = top; k >= 0; k--)
		{
			y += coeff[n++] * buffer[k];
			//y = scale32(y, 1);
		}
		// Process samples after Top
		for(unsigned k = numTaps-1; k > top; k--)
		{
			y += coeff[n++] * buffer[k];
			//y = scale32(y, 1);
		}
		filteredSignal[j] = limitScale16(y, coeffScaling);

		top++;
		if(top >= numTaps) top = 0;
	}
}

#endif