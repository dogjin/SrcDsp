/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which perform demodulation

The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember

------------------------------------------------------------------------------*/

#ifndef DEMODULATORS_H
#define DEMODULATORS_H




#include <complex>
#include <cstdint>
#include <vector>
#include "generators.h"

namespace dsptl_private
{

}

namespace dsptl
{

	/*-----------------------------------------------------------------------------
	OQPSK Demodulator

	Primary template

	@tparam T Type of the input data to the demodulator

	------------------------------------------------------------------------------*/
	template<class InType>
	class DemodulatorOqpsk;



	/*-----------------------------------------------------------------------------
	OQPSK Demodulator

	Implementation for signed byte as input.
	The output of the demodulator is a vector of softbits. Each softbit is a signed
	8 bits value.

	------------------------------------------------------------------------------*/
	template<>
	class DemodulatorOqpsk<int8_t>
	{
	public:
		DemodulatorOqpsk();

	private:
		int16_t quickPhase(std::complex<int8_t> z);


		const int32_t maxAmp;
		const int16_t twoPi;
		const int16_t onePi;
		const int16_t halfPi;
		/// Phase look up table with 8192 entries
		/// Each entry corresponds to an integer value of the real and imaginary part
		/// This should become static to prevent duplication
		std::vector<int16_t> phaseLUT;
		std::vector<int16_t> sineLUT;
		


	};

	/*-----------------------------------------------------------------------------
	Constructor

	Create the sine and phase lookup table

	------------------------------------------------------------------------------*/
	DemodulatorOqpsk<int8_t>::DemodulatorOqpsk():
		twoPi(8192), onePi(4096), halfPi(2048), maxAmp(128)
	{
		// Build the phase lookup table
		int16_t phase;
		phaseLUT.assign(maxAmp * maxAmp, {});
		for (int im = 0; im < maxAmp ; ++im)
			for (int re = 0; re < maxAmp; ++re)
			{
				phase = static_cast<int16_t>(round(atan2(im, re) * onePi / dsptl::pi));
				phaseLUT[im * maxAmp + re] = phase;
			}

		// Build the sine lookup table
		sineLUT.assign(twoPi, {});
		for (int index = 0; index < twoPi ; ++index)
		{
			int16_t phase = static_cast<int16_t>(index * (dsptl::pi / static_cast<double>(onePi)));
			sineLUT[index] = static_cast<int16_t>(sin(phase) * INT16_MAX);
		}
	
	}


	int16_t DemodulatorOqpsk<int8_t>::quickPhase(std::complex<int8_t> z)
	{

		// Determine which quadrant is used
		bool quad2 = z.real() <= 0 && z.imag() > 0;
		bool quad3 = z.real() < 0 && z.imag() <= 0;
		bool quad4 = z.real() >= 0 && z.imag() < 0;
		
		// rotate the input to the first quadrant
		int32_t re = abs(z.real());
		int32_t im = abs(z.imag());

		// Divide by 2 to fit it in the range of the LUT
		while (re >= maxAmp || im >= maxAmp)
		{
			re = static_cast<int32_t>(floor((re + 1) / 2));
			im = static_cast<int32_t>(floor((im + 1) / 2));
		}
		// Read the look up table
		int32_t addr = maxAmp * im + re;
		int16_t phase = phaseLUT[addr];

		// Correct the value to compensate for the rotation
		if (quad2)
			phase = onePi - phase;
		else if (quad3)
			phase = phase - onePi;
		else if (quad4)
			phase = -phase;

		return phase;
	}






}


#endif