/***********************************************************************//**
@file 

Definition of all DSP routines related to mutlirate filters
performing an upconversion.

@author Thierry Guichon
@date 2015
@copyright ORBCOMM

***************************************************************************/

#ifndef DSPTL_UPSAMPLING_FILTER_FIR_H
#define DSPTL_UPSAMPLING_FILTER_FIR_H

#include <cassert>
#include <vector>
#include "dsp_complex.h"

namespace dsptl
{

	/***********************************************************************//**
	Upsampling FIR filter
	
	@tparam InType Type of the input signal. Can be float, double, complex, int...
	@tparam OutType Type of the output signal
	@tparam InternalType Type used internally for the computation
	@tparam CoefType Type of the coefficients
	@tparam N	Upsampling ratio

	It is the responsibility of the caller to make sure that the different types
	work smoothly. Overflow and underflow conditions must not occur

	***************************************************************************/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
	class FilterUpsamplingFir
	{
	public:
		/// Constructor. Coefficients are defined. Size for the internal
		/// buffer is reserved based on the number of coefficients
		FilterUpsamplingFir(const std::vector<CoefType> &firCoeff = std::vector<CoefType>());
		// Change the coefficients of the filter
		void setCoefficients(const std::vector<CoefType> &firCoeff);
		// This function is called for each iteration of the filtering process
		void step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal, bool flush = false);
		// Version of step with an iterator as destination
		void step(const std::vector<InType> & signal, typename std::vector<OutType>::iterator  filteredSignal, bool flush = false);
		/// Reset the internal counters and buffers
		void reset()
		{
			top = 0;
			for (size_t index = 0; index < buffer.size(); ++index)
				buffer[index] = InType();
		}
		/// Return the number of coefficients excluding zero coefficients at the end
		int getLength() const {
			return length;
		}
		/// Return the number of coefficients including any zero coefficients at the end
		int getImpLength() const {
			return impLength;
		}
		/// Return the upsampling ratio implemented by the filter
		int getUpsamplingRatio() const {
			return L;
		}


	private:
		std::vector<CoefType> coeff;		///< Coefficients
		std::vector<InType> buffer;  	///< History buffer
		unsigned top; 					///< Current insertion point in the history buffer 
		int leftShiftFactor;			///< Number of left shifts to operate on the output. This is linked to the upsampling ratio
		unsigned length;				///< Number of coefficients of the filter excluding the null coeff at the end
		unsigned impLength;				///< Number of coefficients of the filter including the null coeff at the end
	};


	/***********************************************************************//**
	Upsampling FIR Filter Default Constructor

	If coefficients are provided, the constructor initializes the internal coefficient table
	and creates the internal buffer for computations.

	@param firCoeff Filter Coefficients

	***************************************************************************/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
	FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::FilterUpsamplingFir(const std::vector<CoefType> &firCoeff )
		: top(0), length(0), impLength(0)
	{
		if (!firCoeff.empty())
			setCoefficients(firCoeff);			

	}

	/***********************************************************************//**
	Upsampling FIR Filter Constructor

	Initializes the internal coefficient table and creates the internal
	buffer for computations.

	@param firCoeff Filter Coefficients

	***************************************************************************/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
	void FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::setCoefficients(const std::vector<CoefType> &firCoeff)
	{
		assert(!firCoeff.empty());
		// We verify that the number of coefficient is a multiple
		// of L
		assert(firCoeff.size() % L == 0);

		coeff = firCoeff;
		// The internal history buffer is sized according to the 
		// number of coefficients
		buffer.resize(firCoeff.size() / L);
		// Compute the scaling factor
		leftShiftFactor = static_cast<int>(round(log2(L)));
		// Compute the length of the filter. i.e. the numbers of coefficientss
		length = impLength = coeff.size();
		while (coeff[length -1 ] == 0) --length ;


	}


	/***********************************************************************//**
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
	@param filteredSignal Output of the filter. Must be L times the size of the input signal plus any space
	required for flushing.
	@param flush If true the history buffer is fully flushed in the output buffer

	***************************************************************************/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
	void FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::step(const std::vector<InType> & signal, std::vector<OutType> & filteredSignal, bool flush)
	{
#ifdef CPLUSPLUS11
		static_assert(signal.size() * L == filteredSignal.size(), "");
#else
		if (!flush)
			assert(signal.size() * L == filteredSignal.size());
#endif
		assert(!coeff.empty());

		InternalType y;  				// Output result
		unsigned  n;				// Counting indexes
		size_t histSize = buffer.size();	// Number of taps in the filter
		size_t inputSize = signal.size();	// Number of input samples


		for (unsigned j = 0; j < inputSize; j++)
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
				for (int k = top; k >= 0; k--, n += L)
				{
					y += coeff[n] * buffer[k];
				}
				// Process samples after Top
				for (unsigned k = histSize - 1; k > top; k--, n += L)
				{
					y += coeff[n] * buffer[k];
				}

				// The following line has been replaced synchronously with the addtion of an overload of the function limitScale in 
				// order to hangle the case where the types are not complex
				//filteredSignal[L*j + offset] = limitScale<typename OutType::value_type, typename InternalType::value_type>(y, 15 - leftShiftFactor);
				filteredSignal[L*j + offset] = limitScale<OutType,  InternalType>(y, 15 - leftShiftFactor);
			}

			top++;
			if (top >= histSize) top = 0;
		}

		if (flush)
		{
			// We flush with length / L zeros
			for (unsigned j = inputSize; j < (inputSize + length / L); j++)
			{
				// This loop is executed for each of the input samples
				buffer[top] = InType{};
				n = 0;

				// For each input sample, we compute L output samples
				for (size_t offset = 0; offset < L; ++offset)
				{
					n = offset;
					y = InternalType();

					// Process samples before and including Top
					for (int k = top; k >= 0; k--, n += L)
					{
						y += coeff[n] * buffer[k];
					}
					// Process samples after Top
					for (unsigned k = histSize - 1; k > top; k--, n += L)
					{
						y += coeff[n] * buffer[k];
					}

					// The following line has been replaced synchronously with the addtion of an overload of the function limitScale in 
					// order to hangle the case where the types are not complex
					//filteredSignal[L*j + offset] = limitScale<typename OutType::value_type, typename InternalType::value_type>(y, 15 - leftShiftFactor);
					filteredSignal[L*j + offset] = limitScale<OutType, InternalType>(y, 15 - leftShiftFactor);
				}

				top++;
				if (top >= histSize) top = 0;
			}
		}

	}
	
	/**
	Version of the step function which takes an iterator as location of the output signal
	@TODO Modify the function which takes a vector reference as input so that it calls this one. It will
	require to modify the handling of the shift factor
	*/
	template<class InType, class OutType, class InternalType, class CoefType, unsigned L>
	void FilterUpsamplingFir<InType, OutType, InternalType, CoefType, L>::step(const std::vector<InType> & signal, typename std::vector<OutType>::iterator  filteredSignal, bool flush)
	{
		assert(!coeff.empty());
		int ShiftFactor = 0; // This will need to be modified in order to accomodate the behavior of the version
		// of the step function which takes a vector as input.

		InternalType y;  				// Output result
		unsigned  n;				// Counting indexes
		size_t histSize = buffer.size();	// Number of taps in the filter
		size_t inputSize = signal.size();	// Number of input samples


		for (unsigned j = 0; j < inputSize; j++)
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
				for (int k = top; k >= 0; k--, n += L)
				{
					y += coeff[n] * buffer[k];
				}
				// Process samples after Top
				for (unsigned k = histSize - 1; k > top; k--, n += L)
				{
					y += coeff[n] * buffer[k];
				}

				// The following line has been replaced synchronously with the addtion of an overload of the function limitScale in 
				// order to hangle the case where the types are not complex
				//filteredSignal[L*j + offset] = limitScale<typename OutType::value_type, typename InternalType::value_type>(y, 15 - leftShiftFactor);
				filteredSignal[L*j + offset] = limitScale<OutType,  InternalType>(y, shiftFactor);
			}

			top++;
			if (top >= histSize) top = 0;
		}

		if (flush)
		{
			// We flush with length / L zeros
			for (unsigned j = inputSize; j < (inputSize + length / L); j++)
			{
				// This loop is executed for each of the input samples
				buffer[top] = InType{};
				n = 0;

				// For each input sample, we compute L output samples
				for (size_t offset = 0; offset < L; ++offset)
				{
					n = offset;
					y = InternalType();

					// Process samples before and including Top
					for (int k = top; k >= 0; k--, n += L)
					{
						y += coeff[n] * buffer[k];
					}
					// Process samples after Top
					for (unsigned k = histSize - 1; k > top; k--, n += L)
					{
						y += coeff[n] * buffer[k];
					}

					// The following line has been replaced synchronously with the addtion of an overload of the function limitScale in 
					// order to hangle the case where the types are not complex
					//filteredSignal[L*j + offset] = limitScale<typename OutType::value_type, typename InternalType::value_type>(y, 15 - leftShiftFactor);
					filteredSignal[L*j + offset] = limitScale<OutType, InternalType>(y, shiftFactor);
				}

				top++;
				if (top >= histSize) top = 0;
			}
		}

	}
} // End of namespace

#endif
