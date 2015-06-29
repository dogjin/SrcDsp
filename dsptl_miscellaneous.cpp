/***********************************************************************//**
@file

Miscellaneous utilities for signal processing applications

@author Thierry Guichon
@date 2015
@copyright ORBCOMM

***************************************************************************/


#include "dsptl_miscellaneous.h"


/**************************************************************************//**

Convert a frequency value given in rad per samples into a value in Hd           
******************************************************************************/
double dsptl::toFreqHz(double freqRadPerSample, double samplingFreqHz)
{
	return freqRadPerSample / 2.0 / dsptl::pi * samplingFreqHz;
}

/**************************************************************************//**
Convert a frequency value given in Hz into a value in radians per sample        
******************************************************************************/	
double dsptl::toFreqRadPerSample(double freqHz, double samplingFreqHz)
{
	return 2.0 * dsptl::pi * freqHz / samplingFreqHz;
}