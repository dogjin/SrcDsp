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
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include "dsp_complex.h"


//#define CREATE_DEBUG_FILES

namespace dsptl
{

	/*-----------------------------------------------------------------------------
	Implements a fixed pattern complex correlator

	The design is done so that 14 bits input values and 13 bits coefficients do not
	saturate.\n

	The macro CREATE_DEBUG_FILES can be defined so that the routine creates debugging files.
	When debugging files are created, the routine will not exit after the first found correlation
	point.



	@tparam InType Data type of the input signal
	@tparam CompType Internal computation type
	@tparam N Number of points of the correlation pattern
	@tparam S Stride used to scan the input vector


	------------------------------------------------------------------------------*/
	
	template<class InType = int16_t, class CompType = int32_t, size_t N = 32, size_t S = 4 >
	class FixedPatternCorrelator
	{
		/// Internal status of the correlator. This is mainly done in order to
		/// have visibility in the correlator operation from the calling code
		struct CorrState 
		{
			static const int Nelements = 3;
			float InputEnergy;
			uint32_t coeffsEnergy;
			int coeffScaling;
			uint32_t energyValue[Nelements] ;
			uint32_t corrValue[Nelements] ;
			double thresholdFactor;
			std::string prettyString()
			{	std::ostringstream os;
				os << "Input Energy: " << InputEnergy << '\n';
				os << "Coeffs Energy: " << coeffsEnergy << '\n';
				os << "Coeff Scaling: " << coeffScaling << '\n';	
				os << "Threshold Factor: " << thresholdFactor << '\n';
				for(int index = 0; index < Nelements; ++index)
					os << "Energy Value " << index << ": " << energyValue[index] << '\n';
				for(int index = 0; index < Nelements; ++index)
					os << "CorrValue " << index << ": " << corrValue[index] << '\n';

				return os.str();
			}
		};


	public:
		FixedPatternCorrelator();
		bool step(const std::vector <std::complex<InType> > &in, int & corrIndex);
		void setPattern(const std::array<std::complex<CompType>, N > &in, double thresholdCoeff = 0.8 );
		void reset();
		std::vector<std::complex<InType>> getRefBitSamples();
		CorrState getStatus(){return state;};

	private:
		std::array < std::complex<CompType>, N*S> history;
		std::array < std::complex<CompType>, N > coeffs;
		std::vector < std::complex<InType> > bitSamples;

		//uint32_t state.coeffsEnergy;
		//uint32_t corrValue[3] ;
		//uint32_t energyValue[3] ;
		//double thresholdFactor;
		size_t top;
		//int state.coeffScaling;
		uint32_t cntProcessedSamples; // Number of samples processed until detection
		CorrState state;

		// Debugging routines
		#ifdef CREATE_DEBUG_FILES
		std::ofstream fenergy;
		std::ofstream fcorr;
		std::ofstream fthreshold;
		#endif

	
	};


	/*-----------------------------------------------------------------------------
	Constructor

	Internal variables are initialized. Files are created if necessary.
	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
		FixedPatternCorrelator<InType, CompType, N, S >::FixedPatternCorrelator()
	:top(0)
	{
		bitSamples.assign(N, {});
		reset();
		#ifdef CREATE_DEBUG_FILES
		fenergy.open("debug_corr_energy.dat");
		fcorr.open("debug_corr_values.dat");
		fthreshold.open("debug_corr_threshold.dat");
		#endif
	}

	/*-----------------------------------------------------------------------------
	Reset all internals of the correlator

	Correlator coefficients and associated energy are not modified


	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	void FixedPatternCorrelator<InType, CompType, N, S >::reset()
	{
		top = 0;
		state.corrValue[0] = state.corrValue[1] = state.corrValue[2] = 0;
		state.energyValue[0] = state.energyValue[1] = state.energyValue[2] = 0;
		for (size_t k = 0; k < history.size(); ++k)
			history[k] = {};
		for (size_t k = 0; k < bitSamples.size(); ++k)
			bitSamples[k] = {};
		cntProcessedSamples = 0;

	}

	/*-----------------------------------------------------------------------------
	Copy locally the array of correlation values

	@param[in] in Correlation pattern

	The coefficients should be passed as non conjuguate values (i.e. as a replica of 
	the desired signal) because the routine conjuguates them before storage.\n

	The function also compute and stores the energy of the correlation signal

	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N, size_t S >
	void FixedPatternCorrelator<InType, CompType, N, S >::setPattern(const std::array<std::complex<CompType>, N > &in, double thresholdCoeff)
	{
		coeffs = in;

		// We conjuguate the coefficients
		for (auto it = coeffs.begin(); it != coeffs.end(); ++it)
		{
			*it = std::complex<CompType>(it->real(), -it->imag());
		}

		// We compute the energy in the coefficients
		state.coeffsEnergy = 0;
		double tmp = 0;
		for (size_t index = 0; index < N; ++index)
		{
			tmp += (coeffs[index].real() * coeffs[index].real() + coeffs[index].imag() * coeffs[index].imag());
		}
		assert(tmp <= 1073217600); // Each coeffs value must be less than 13 bits.

		state.coeffsEnergy = static_cast<uint32_t>(tmp);
		state.thresholdFactor = thresholdCoeff * sqrt(state.coeffsEnergy);

		state.coeffScaling = static_cast<int>(floor(log2(sqrt(state.coeffsEnergy))));



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
	bool FixedPatternCorrelator<InType, CompType, N, S >::step(const std::vector < std::complex<InType> > &in, int & corrIndex)
	{
		int inSize = in.size();
		int historySize = static_cast<int>(history.size());
		std::complex< CompType> tmp;
		int k;
		int hIndex;
		bool syncFound = false;


		// Iteration over each element of the input buffer
		for (int index = 0; index < inSize; index++)
		{
			++cntProcessedSamples;
			// Each element is copied into the history buffer
			history[top] = in[index];
			tmp = std::complex<CompType>{};

			state.energyValue[2] = state.energyValue[1];
			state.energyValue[1] = state.energyValue[0];
			state.energyValue[0] = 0;

			// We iterate with a stride S on the history buffer
			for (k = 0; (hIndex = top - k*S) >=0 ; ++k)
			{
				tmp += history[hIndex] * coeffs[N-1-k];
				state.energyValue[0] += history[hIndex].real() * history[hIndex].real() + history[hIndex].imag() *  history[hIndex].imag();
			}
			for (k = 0; (hIndex = top + (k + 1 )*S) < historySize ; ++k)
			{
				tmp += history[hIndex] * coeffs[k];
				state.energyValue[0] += history[hIndex].real() * history[hIndex].real() + history[hIndex].imag() *  history[hIndex].imag();
			}

			tmp = scale32(tmp, state.coeffScaling);  // V2 dimension
			state.energyValue[0] = state.energyValue[0] >> (state.coeffScaling/2);  // V2 dimension

			// Store the squared magnitude of the correlation values
			state.corrValue[2] = state.corrValue[1];
			state.corrValue[1] = state.corrValue[0];
			state.corrValue[0] = (tmp.real() >> 2)*(tmp.real()>>2) + (tmp.imag()>>2) * (tmp.imag()>>2);

			// DEBUG ONLY
			#ifdef CREATE_DEBUG_FILES
			fenergy << sqrt(state.energyValue[0]) << '\n';
			fcorr << sqrt(state.corrValue[0]) << '\n';
			fthreshold << sqrt(state.energyValue[0]) * 2.5 << '\n';
			#endif



			// Is the middle point (index 1) a peak?
			if (state.corrValue[1] > state.corrValue[2] && state.corrValue[1] > state.corrValue[0])
			{
				// Has the middle point (index 1) exceeded the threshold?
				double corr = sqrt(state.corrValue[1]); // magnitude of the correlation
				double energy = sqrt(state.energyValue[1]); // magnitude of the signal energy   
				// Initial values before debugging were 2.5 and 200
				if (corr > energy * 2.7 && energy > 300)
				{
					// Index 1 is a peak which exceeded the threshold
					// -1 to refer to the previous sample
					corrIndex = index - 1;
					//  We extract the bit samples corresponding to the found correlation
					// top represents the location of the last input sample process
					// top - 1 modulo historySize is the location of the sample at which the peak occurred
					// From this peak correlation sample, extract every S bit samples from the history buffer.
					// We iterate with a stride S on the history buffer
					size_t newTop;
					if (top > 0) newTop = top - 1;
					else newTop = historySize - 1;
					for (int k = 0; (hIndex = newTop - k*S) >= 0; ++k)
					{
						bitSamples[N - 1 - k] = history[hIndex];
					}
					for (int k = 0; (hIndex = newTop + (k + 1)*S) < historySize; ++k)
					{
						bitSamples[k] = history[hIndex];
					}
					syncFound = true;
					//#ifndef CREATE_DEBUG_FILES
					break;
					//#endif
				}
			}

			top = (top + 1) % historySize;
			

		}


		return syncFound;
	}

	/**************************************************************************//**
	Return the bitSamples corresponding to a successful correlation. The number of
	bit samples returned is the same as the number of bits used to compute each 
	correlation point.

	******************************************************************************/
	template<class InType, class CompType, size_t N, size_t S >
	std::vector<std::complex<InType>> FixedPatternCorrelator<InType, CompType, N, S >::getRefBitSamples()
	{
		return bitSamples;

	}


} // End of namespace 


#ifdef CREATE_DEBUG_FILES
#undef  CREATE_DEBUG_FILES
#endif


#endif
