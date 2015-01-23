/*-----------------------------------------------------------------------------
@file 

Definition of all DSP routines related to filters.
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

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

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
class FilterFir
{
	/// Constructor. Coefficients are defined. Size for the internal
	/// buffer is reserved based on the number of coefficients
	FilterFir(const std::vector<CoefType> &firCoeff);
	// This function is called for each iteration of the filtering process
	void step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal)
	
private:
	std::vector<CoefType> coeff;		///< Coefficients
	std::vector<InternalType> buffer;  	///< History buffer
	unsigned top; 							///< Current insertion point in the history buffer 
}


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
	buffer.resize(size(firCoeff));
}



/*-----------------------------------------------------------------------------
FIR Filter

The algortihm uses an internal buffer with a length equal to the number of coefficients.
The current input value is inserted in the buffer at the correct location, then the convolution
 operation is computed. \n
The user must make sure that the internal type is large enough to contain the
accumulated sum of the convolution operation

@param signal Input to the filter
@param filteredSignal Output of the filter. Must be the same size as signal

------------------------------------------------------------------------------*/
template<class InType, class OutType, class InternalType, class CoefType>
FilterFir<InType, OutType, InternalType, CoefType>::step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal)
{
	#ifdef CPLUSPLUS11
	static_assert(size(signal) == size(filteredSignal), "");
	#else
	assert(size(signal) == size(filteredSignal));
	#end
	OutType y;  							// Output result
	unsigned j, k, n;						// Counting indexes
	size_t numTaps = size(buffer);		// Number of taps in the filter
	size_t inputSize = size(signal);		// Number of input samples


	for(j = 0; j < inputSize ; j++)
	{
	// This loop is executed for each of the input samples
		buffer[top] = signal[j];
		y = OutType();
		n = 0;

		// Process samples before and including Top
		for(k = Top; k >= 0; k--)
		{
			y += FirCoeff[n++] * buffer[k];
		}
		// Process samples after Top
		for(k = numTaps-1; k > top; k--)
		{
			y += coeff[n++] * buffer[k];
		}
		filteredSignal[j] = y;

		top++;
		if(top >= NumTaps) top = 0;
	}
}
