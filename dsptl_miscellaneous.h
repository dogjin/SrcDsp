/***********************************************************************//**
@file

Miscellaneous utilities for signal processing applications

@author Thierry Guichon
@date 2015
@copyright ORBCOMM

***************************************************************************/


#ifndef DSPTL_MISCELLANEOUS_H
#define  DSPTL_MISCELLANEOUS_H

#include <complex>
#include <cmath>
#include <ostream>
#include <sstream>
#include <iomanip>
#include "constants.h"
#include <vector>

/**************************************************************************//**
Estimate the frequency in radians per samples of a sequence of complex values\n

The frequency is estimated by measuring the phase difference between samples 
separated by L. As many phase differences as possible, based on the input
buffer length are computed./n
The algorithm implemented is only valid if the phase difference between two
samples compared is always between -pi and +pi which is equivalent to say that
the frequency of the input signal must be less than pi/L rad/samples

@tparam InType The input vector shall contain complex<InType> values
@tparam L Interval between samples used to compute a phase difference.

@param in Input vector of complex values. The size of the vector must be at least L

@return Frequency in radians per sample at the sampling frequency of the input
data (sampling frequency of the in vector)

******************************************************************************/
template<class InType, int L>
float estimateFreq(std::vector<std::complex<InType>> in)
{
	double phase = 0;
	double tmpI{}, tmpQ{};

	for (size_t index = 0; (index + L) < in.size(); ++index )
	{
		// Correlation between one sample and the sample L samples after
		tmpI += (in[index].real() * in[index + L].real() + in[index].imag() * in[index + L].imag());
		tmpQ += (in[index].imag() * in[index + L].real() - in[index].real() * in[index + L].imag());
	}
	
	phase = atan2(tmpQ, tmpI);
	return static_cast<float>(phase / L );
}

template<class InType, int Threshold>
int estimateShiftFactor(InType in)
{
	int shift = 0;
	while (in > Threshold)
	{
		in >>= 1;
		++shift;
	}
	return shift;
}

/**************************************************************************//**
Take a vector representing a sequence of bits and create a representation 
string with hexadecimal characters\n
Input values > 0 are considered to be 1. Input values <= 0 are considered to 
be zeros\n

If the bit pattern is not a multiple of 8 bits, addtional bits 0 are added on the right
and the number of added bit is indicated at the end of the string by -- # added bits


@param bits      vector of the bits received
@param order     Define how the bytes are created from the bits. If true, the first bit
of the vector becomes the Most significant bit of the first byte. If false, this bit is the
least significant bit of the first byte.
           



******************************************************************************/
template<class InType>
std::string bits2HexStr(std::vector<InType> in, bool firstBitIsMsbOfByte)
{
	const int bitsInByte = 8;
	//Create a string stream for formatting
	std::ostringstream os;
	os.fill('0');
	os.setf(std::ios::hex, std::ios::basefield);
	// How many extra bits will be needed?
	int extraBits = bitsInByte - (in.size() % bitsInByte);
	if (extraBits == bitsInByte) extraBits = 0;


	//We take the bits one by one and we pack them
	auto it = in.cbegin();
	uint8_t byte = 0;
	unsigned shift = 0;
	for (auto it = in.cbegin(); it != in.cend(); ++it)
	{
		if (firstBitIsMsbOfByte)
		{
			byte |= (*it > 0 ? 1 : 0) << (7 - shift);

		}
		else
		{
			byte |= (*it > 0 ? 1 : 0) << shift;

		}
		shift = ((shift + 1) % bitsInByte);
		if (shift == 0)
		{
			// A full byte has been collected
			os << std::setw(2) << static_cast<uint16_t>(byte);
			byte = 0;
		}
	}

	//  Take care of the last byte
	//  There  is no need to explicitly add the zeros because the byte was initialized
	// to zero
	if (shift != 0)
			os << std::setw(2) << static_cast<uint16_t>(byte);

	// Add the information regarding the extra bits
	os << "--" << std::setw(2) <<  extraBits;
	return os.str();


}

namespace dsptl
{

// Convert a frequency value given in rad per samples into a value in Hd           
double toFreqHz(double freqRadPerSample, double samplingFreqHz);

// Convert a frequency value given in Hz into a value in radians per sample        
double toFreqRadPerSample(double freqHz, double samplingFreqHz);
	
}




#endif
