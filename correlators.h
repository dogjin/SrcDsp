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


namespace dsptl
{

	/*-----------------------------------------------------------------------------
	Implements a fixed pattern complex correlator


	@tparam InType Data type of the input signal
	@tparam CompType Internal computation type
	@tparam N Number of points of the correlation pattern
	@tparam S Stride used to scan the input vector


	------------------------------------------------------------------------------*/
	template<class InType, class CompType, size_t N , size_t S >
	class FixedPatternCorrelation
	{


	public:
		bool step(std::vector < const std::complex<InType> &in, size_t & corrIndex);


	private:
		std::array < std::complex<CompType>, N*S> history;
		std::array < std::complex<CompType>, N > coeffs;
		size_t top;

	
	};


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
	bool FixedPatternCorrelation<InType, CompType, N, S >::step(const std::vector < std::complex<InType> &in, size_t & corrIndex)
	{
		size_t inSize = in.size();
		for (size_t index = 0; index < inSize; index++)
		{
			// 
			history[top] = in[index];
			// We iterate
			for (size_t k = top, size_t j = 0 ; k>=0; k-=S, --j)
			{
				result = history[k] * coeffs[j];
			}
			for (size_t k = top + , size_t j = 0; k >= 0; k -= S, --j)
			{
				result = history[k] * coeffs[j];
			}


		}



	};



} // End of namespace 




#endif