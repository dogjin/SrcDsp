/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which perform correlation
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#ifndef CORRELATORS_H
#define CORRELATORS_H


#include <vector>
#include <complex>
#include <array>
#include <cstdint>


namespace dsptl
{

	/*-----------------------------------------------------------------------------
	Implements a fixed pattern complex correlator

	The design is done so that 14 bits input values and 13 bits coefficients do not
	saturate.\n



	@tparam InType Data type of the input signal
	@tparam CompType Internal computation type
	@tparam N Number of points of the correlation pattern
	@tparam S Stride used to scan the input vector


	------------------------------------------------------------------------------*/
	
	template<class InType = int16_t, class CompType = int32_t, size_t N = 32, size_t S = 4 >
	class FixedPatternCorrelator
	{


	public:
		FixedPatternCorrelator();
		bool step(const std::vector <std::complex<InType> > &in, size_t & corrIndex);
		void setPattern(const std::array<std::complex<CompType>, N > &in, double thresholdCoeff = 0.8 );
		void reset();


	private:
		std::array < std::complex<CompType>, N*S> history;
		std::array < std::complex<CompType>, N > coeffs;
		std::array < std::complex<InType>, N > bitSamples;

		uint32_t coeffsEnergy;
		uint32_t corrValue[3] ;
		uint32_t energyValue[3] ;
		double thresholdCoeff;
		size_t top;

	
	};


	/*-----------------------------------------------------------------------------
	Copy locally the array of correlation values

	@param[in] in Correlation pattern

	The function also compute and stores the energy of the correlation signal

	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	FixedPatternCorrelator<InType, CompType, N, S >::FixedPatternCorrelator()
	{
		reset();

	}

	/*-----------------------------------------------------------------------------
	Reset all internals of the correlator

	Correlator coefficients and associated energy are not modified


	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	void FixedPatternCorrelator<InType, CompType, N, S >::reset()
	{
		top = 0;
		corrValue[0] = corrValue[1] = corrValue[2] = 0;
		energyValue[0] = energyValue[1] = energyValue[2] = 0;
		for (size_t k = 0; k < history.size(); ++k)
			history[k] = {};
		for (size_t k = 0; k < bitSamples.size(); ++k)
			bitSamples[k] = {};

	}

	/*-----------------------------------------------------------------------------
	Copy locally the array of correlation values

	@param[in] in Correlation pattern

	The function also compute and stores the energy of the correlation signal

	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	void FixedPatternCorrelator<InType, CompType, N, S >::setPattern(const std::array<std::complex<CompType>, N > &in, double thresholdCoeff)
	{
		coeffs = in;
		coeffsEnergy = 0;
		for (size_t index = 0; index < N; ++index)
		{
			// A scaling factor of 2 is applied to keep the energy within a 32 bit unsigned value
			coeffsEnergy += (coeffs[index].real() * coeffs[index].real() + coeffs[index].imag() * coeffs[index].imag());
		}

		double tmp = thresholdCoeff * sqrt(coeffsEnergy);

		assert(coeffsEnergy <= 1073217600); // Each coeffs value must be less than 13 bits.

	}


	/*-----------------------------------------------------------------------------
	Each S element of the input vector is copied in the history buffer at the right
	location. The inner product between the history and the coeffs buffer is then 
	computed

	@param[in] in Samples to correlate
	@param[out] corrIndex Index related to the input buffer at which the correlation occurred
	The value only makes sense if the function returned true


	@return true if correlation peak has been detected; false otherwise
	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	bool FixedPatternCorrelator<InType, CompType, N, S >::step(const std::vector < std::complex<InType> > &in, size_t & corrIndex)
	{
		size_t inSize = in.size();
		int historySize = static_cast<int>(history.size());
		std::complex< CompType> tmp;
		int k;
		int hIndex;
		bool syncFound = false;


		// Iteration over each element of the input buffer
		for (size_t index = 0; index < inSize; index++)
		{
			// Each element is copied into the history buffer
			history[top] = in[index];
			tmp = std::complex<CompType>{};

			energyValue[2] = energyValue[1];
			energyValue[1] = energyValue[0];
			energyValue[0] = 0;

			// We iterate with a stride S on the history buffer
			for (k = 0; (hIndex = top - k*S) >=0 ; ++k)
			{
				tmp += history[hIndex] * coeffs[N-1-k];
				energyValue[0] += history[hIndex].real() * history[hIndex].real() + history[hIndex].imag() *  history[hIndex].imag();
			}
			for (k = 0; (hIndex = top + (k + 1 )*S) < historySize ; ++k)
			{
				tmp += history[hIndex] * coeffs[k];
				energyValue[0] += history[hIndex].real() * history[hIndex].real() + history[hIndex].imag() *  history[hIndex].imag();
			}

			// Store the magnitude of the correlation value
			corrValue[2] = corrValue[1];
			corrValue[1] = corrValue[0];
			corrValue[0] = tmp.real()* tmp.real() + tmp.imag() * tmp.imag();




			// Have we passed a peak
			if (corrValue[1] >= corrValue[2] && corrValue[1] >= corrValue[0])
			{
				// Do we exceed the threshold
				double corr = corrValue[2];
				
				if (true)
				{
					// We have found a peak
					// -1 to refer to the previous sample
					corrIndex = index - 1;  
					// We extract the bit samples
					for (int k = 0; (hIndex = top -1 - k*S) >= 0; ++k)
					{
						bitSamples[N - 1 - k] = history[hIndex];
					}
					for (int k = 0; (hIndex = top -1 + (k + 1)*S) < historySize; ++k)
					{
						bitSamples[k] = history[hIndex];
					}
					syncFound = true;
					break;

				}
			}


			top = ++top % historySize;
			

		}


		return syncFound;
	};



} // End of namespace 




#endif