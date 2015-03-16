/**************************************************************************//**
@file

Defintion of miscellaneous routines and classes to supporth the dsptl library

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember

******************************************************************************/


#ifndef DSPTL_MISCELLANEOUS_H
#define  DSPTL_MISCELLANEOUS_H

#include <complex>
#include <cmath>

/**************************************************************************//**
Estimate the frequency in radians per samples of a sequence of complex values\n

The frequency is estimated by measuring the phase difference between samples 
separated by L. As many phase differences as possible, based on the input
buffer length are computed.

@tparam InType The input vector shall contain complex<InType> values
@tparam L Interval between samples used to compute a phase difference.

@param in Input vector of complex values. The size of the vector must be at least L

@return Frequency in radians per sample

******************************************************************************/
template<class InType, int L>
float estimateFreq(std::vector<std::complex<InType>> in)
{
	double sumPhase = 0;
	int cnt = 0;
	for (size_t index = 0; (index + L) < in.size(); index += L)
	{
		double tmpI, tmpQ;
		tmpI = (in[index].real() * in[index + L].real() + in[index].imag() * in[index + L].imag());
		tmpQ = (in[index].imag() * in[index + L].real() - in[index].real() * in[index + L].imag());
		double phase = atan2(tmpQ, tmpI);
		sumPhase += phase;
		++cnt;
	}
	return static_cast<float>(sumPhase / (L * cnt));
}


#endif