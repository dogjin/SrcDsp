/***********************************************************************//**
@file

Definition of all objects and utilities related to buffers. \n
This include specialized buffers like fifos or queues

@author Thierry Guichon
@date 2015
@copyright ORBCOMM

***************************************************************************/


#ifndef DSP_BUFFERS_H
#define DSP_BUFFERS_H

#include <cstddef>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <iterator>
#include <utility>
#include "pthread.h"
#include <iostream>

namespace dsptl
{


/***********************************************************************//**
FIFO where the write always write. There is no notion of being full
as far as write is concerned.\n
The read occurs at the location specified in the read call.\n

The buffer keeps a timePoint value which relates to the aboslute time at which the value was collected.
The timePoint is a 64 bits unsigned integer. At a sampling rate of 38400 sps it will roll over in
many centuries, consequently the roll-over of the timePoint is not fully implemented
(Partial implementation can be seen but this not fully operational).\n


The correspondence between the time index and the absolute system time can be 
retrieved by calling getAbsoluteTime(). This only works if the sampling
frequency in Hz was correctly passed to the constructor.

The number of values added into the fifo in a single write operation
MUST BE LESS than the size of the FIFO

The class is meant to work with one thread which calls read() and one
thread which calls write(). The read and the write are assumed to access
different parts of the fifo.

 @tparam T Type of the values stored in the buffer
 @tparam N Number of elements stored in the FIFO

***************************************************************************/

template <class T, size_t N>
class FifoWithTimeTrack
{
public:
	// Constructor
	FifoWithTimeTrack(double samplingFrequencyArg = 0):
	writePtr(0), timeStart(0), timeEnd(0), rolloverFlag(false), storage(N), samplingFrequency(samplingFrequencyArg)
	{ pthread_mutex_init(&mx, nullptr);}
	// Destructor
	~FifoWithTimeTrack()
	{ pthread_mutex_destroy(&mx);}
	// Write values at the back of the fifo
	void write(std::vector<T>& in, unsigned int seconds = 0 , double fracSeconds = 0);
	// Return the desired value in the provided buffer
	bool  read(std::vector<T>& out, uint64_t & start);
	// Return the number of values currently stored in the FIFO
	size_t count();
	/// Reset the pointers and counters of the FIFO as if the FIFO
	/// was just created
	void reset();
	/// Write debugging info to the standard output
	// This is mainly for debuggin purposes
	void dumpInfo(bool dumpData = false) ;
	/// Returns the absolute time associated with a timePoint value and a fraction of a timePoint
	std::pair<unsigned int, double> getAbsoluteTime(uint64_t timePoint, double fracTimePoint);

private:
	// Location to write the next value
	size_t writePtr;
	// Time index of the earliest value available in the buffer
	uint64_t timeStart;
	// Time index of the latest value available in the buffer
	uint64_t timeEnd;
	// Flag indicating that timeEnd has rolled over but that
	// timeStart has not yet rolled over.
	bool rolloverFlag;
	// FIFO storage
	std::vector<T> storage;
	// Sampling frequency. This is used purely to return a sample
	// time if required
	double samplingFrequency;
	// Mutex used for contention prevention
	pthread_mutex_t mx;
	// Guarded lock
	class lock_guard
	{
	public:
		lock_guard(pthread_mutex_t * pmx):_mx(pmx)
			{pthread_mutex_lock(_mx);}
		~lock_guard(){pthread_mutex_unlock(_mx);}
	private:
		pthread_mutex_t * _mx;
	};
	
	/// Struture which associates an absolute time with a sample number
	/// The absolute time is represented by the number of seconds and the
	/// number of fractional seconds
	struct timeReference_t
	{
		uint64_t timePoint;
		std::pair<unsigned int , double> absoluteTime;
	} timeReference;


};


/***********************************************************************//**
Write the elements in the provided vector in the fifo

 @param in vector of elements to write. Any existing data is overwritten. The input
size must be less than the size of the fifo
@param seconds Absolute time of the first sample of the buffer in seconds
@param fracSeconds Fractional seconds part of the absolute time of the first
sample of the in buffer. 

A true rollover flag indicates that the timeEnd has rolled over but that the
time start has not rolled over yet.


***************************************************************************/
template <class T, size_t N>
void FifoWithTimeTrack<T,N>::write(std::vector<T> & in, unsigned int seconds, double fracSeconds)
{
	size_t upToTop = N - writePtr; // Nbr locations until top
	size_t inSize = in.size();
	assert(inSize < N);

	// The data copying is not in the critical section because
	// it is assumed that different sections of the vector are
	// accessed by the different threads
	if(inSize <= upToTop)
		// Everything fits between the writePtr and the top
		std::copy(in.cbegin(), in.cend(), &storage[writePtr]);
	else
	{
		// Part is written from writePtr to the top. The remainder
		// is written at the beginning of the FIFO
		std::copy(in.cbegin(), in.cbegin() + upToTop, &storage[writePtr]);
		std::copy(in.cbegin() + upToTop, in.cend(), &storage[0]);
	}

	// ------ CRITICAL SECTION START -------
	
	pthread_mutex_lock(&mx);
	// Update writePtr
	writePtr = (writePtr + inSize) % N;

	// Difference between the current timePoint value and its maximum value
	uint64_t diff = UINT64_MAX - timeEnd;
	
	// The case when timeEnd has reached the maximum time point value is ignored

	// We associate the time of the first sample we just receive with the
	// timePoint one above the current timeEnd.
	timeReference.timePoint = timeEnd + 1 ;
	timeReference.absoluteTime.first = seconds;
	timeReference.absoluteTime.second = fracSeconds;


	// The time index of the most recent value is updated
	if(diff >= inSize)
		// timeEnd can be incremented safely
		timeEnd += inSize;
	else
	{
		// timeEnd must rollover
		timeEnd = inSize-diff;
		rolloverFlag = true;
	}

	// The time index of the oldest value is updated
	if(!rolloverFlag)
		if((timeEnd - timeStart +1 ) > N)
			timeStart = timeEnd - N + 1;
		else
			timeStart = 1;
	else
	{
		// timeEnd has rolled over. 
		uint64_t diff = UINT64_MAX - timeStart;
		if(diff >= inSize)
			timeStart += inSize;
		else
		{
			// timeStart must rollover
			timeStart = inSize - diff;
			// Both have rolled over. 
		}

			rolloverFlag = false;	
	}
	
	pthread_mutex_unlock(&mx);

	// ------ CRITICAL SECTION END -------


}


/***********************************************************************//**
Write the internal state of the fifo to the standard output

 @param data Flag indicating wherther the elements are dumped or not

***************************************************************************/
template <class T, size_t N>
void FifoWithTimeTrack<T,N>::dumpInfo(bool dumpData) 
{
	// ------ CRITICAL SECTION START -------
	
	pthread_mutex_lock(&mx);

	std::cout << "writePtr: " << writePtr << '\n';
	std::cout << "timeStart : " << timeStart << '\n';
	std::cout << "timeEnd : " << timeEnd << '\n';
	std::cout << "rolloverFlag : " << rolloverFlag << '\n';
	size_t index = 0;
	if (dumpData)
	{
		for (auto it = storage.begin();it != storage.end(); ++it)
			std::cout << "index: " << index++ << " Value: " << *it << '\n';
	}

	pthread_mutex_unlock(&mx);
	
	// ------ CRITICAL SECTION END -------
}

/***********************************************************************//**
Reset the state of the fifo. Internal values are not cleared.\n

***************************************************************************/

template <class T, size_t N>
void FifoWithTimeTrack<T,N>::reset()
{
	// ------ CRITICAL SECTION START -------
	
	pthread_mutex_lock(&mx);
	
	writePtr = 0;
	timeStart = 0;
	timeEnd = 0;
	rolloverFlag = false; 

	pthread_mutex_unlock(&mx);
	
	// ------ CRITICAL SECTION END -------
}

/***********************************************************************//**
Retrieves a range of values from the FIFO. The first value is the value
associated with the start timestamp. The number of values is indicated by
the size of the vector passed as parameter.

 @param out vector in which the values retrieved will be stored
 @param start timestamp of the first value. If this values is less than the
 timeStart representing the first available sample in the buffer, it is adjusted
 to timeStart and a warnning is isssued.

 @return true an error occurred ( for example if the requested range goes beyond
 the number of samples currently available in the buffer) , false if no error.

*****************************************************************************/
template <class T, size_t N>
bool  FifoWithTimeTrack<T,N>::read(std::vector<T>& out, uint64_t & start )
{
	// Rollover conditions  of timeStart and timeEnd are not handled
	// Error if the range requested is beyond the existing range
	assert(out.size() != 0);

	// Local pointers to the buffers indicating the first and the last
	// sample to read. The pointers make a closed range.
	size_t startPtr;  
	size_t endPtr ;  
	
	// The block below is intended to clearly define the critical section
	{
		// ------ CRITICAL SECTION START -------
		lock_guard lck(&mx);
		
		if(start <  timeStart)
		{
			std::cerr << "******* REQUESTED START BEFORE FIRST AVAILABLE SAMPLE *****";
			// The start is adjusted to the minimum value available
			start = timeStart;
			//return true;
		}
		if ((start + out.size()-1) > timeEnd)
		{
			return true;
		}

		//std::cerr << "K";
		// The code below is only performed if the timeStart and timeEnd
		// were correct
		size_t end = start + out.size() - 1;
		startPtr = (writePtr + N - (timeEnd - start) - 1 ) % N;  
		endPtr  = (writePtr + N - (timeEnd - end) - 1) % N;  
		//std::cout << "WritePtr " << writePtr << '\n';
		//std::cout << "TimeEnd " << timeEnd << '\n';
		//std::cout << "Start " << start << '\n';
		//std::cout << "End  " << end << '\n';
		//std::cout << "StartPtr  " << startPtr << '\n';
		//std::cout << "EndPtr  " << endPtr << '\n';
		
		// ------ CRITICAL SECTION END -------
	}	

	// The following code is not in the critical section because it is
	// assumed that we never read and write at the same location simultaneously
	
	assert(endPtr < N);
	assert(startPtr < N);

	if (endPtr >= startPtr)
		std::copy(&storage[startPtr], &storage[endPtr + 1], out.begin());
	else
	{	
		auto  it = std::copy(&storage[startPtr], &storage[N], out.begin());
		//auto diff = std::distance(out.begin(),it);
		//std::cout << "Distance " << diff << '\n';
		//diff = std::distance(it,out.end());
		//std::cout << "Distance to end" << diff << '\n';
		//assert(it != out.end()); 
		it = std::copy(&storage[0], &storage[endPtr+1], it);
		//diff = std::distance(out.begin(),it);
		//std::cout << "Distance " << diff << '\n';
	}
		
	return false;
	
}

/***********************************************************************//**
Return the number of values currently stored in the FIFO.\n

***************************************************************************/

template <class T, size_t N>
size_t  FifoWithTimeTrack<T,N>::count()
{
	// ------ CRITICAL SECTION START -------
	
	lock_guard lck(&mx);
	
	if(!rolloverFlag)
		return (timeEnd - timeStart) + 1;
	else
		// The assumptions are :
		//	timeEnd is much smaller than the max value of size_t
		//	(UINT64_MAX - timeStart) is much smaller than the size of size_t
		return (UINT64_MAX - timeStart) + timeEnd + 1;

	
	// ------ CRITICAL SECTION END -------
}


/***********************************************************************//**
Return a number of seconds and fractional seconds associated with the time point
and the fractional timePoint specified as parameter. 

 @param timePoint Indicate at what point in time the time should be computed.
 The timePoint value is typically between timeStart and timeEnd although it may
 occasionally be beyond these limits.
 @param fracTimePoint Fractional time point. This indicates that the time that we
 want lies between two samples.
 
 @return A pair of values where the first value is the number of full seconds and the
 second number is the fractional second.
 
 The routine does not verified if the timePoint is between timeStart and timeEnd, but
 it will give its best estimate of the time
***************************************************************************/

template <class T, size_t N>
std::pair<unsigned int, double> FifoWithTimeTrack<T,N>::getAbsoluteTime(uint64_t timePoint, double fracTimePoint)
{

	// Set to 1 to enable information to be displayed on the standard output
	#define GET_ABSOLUTE_TIME_DEBUG_INFO 0
	
	uint32_t seconds = 0U;
	double fracSeconds = 0.0;
	

	// ------ CRITICAL SECTION START -------
	{
		lock_guard lck(&mx);
		
		#if GET_ABSOLUTE_TIME_DEBUG_INFO
		std::cout << "\nFunction call getAbsoluteTime: \n";
		std::cout << "\tInput timePoint: " <<  timePoint << " fracTimePoint: " << fracTimePoint << '\n';
		std::cout << "\tReference timePoint: " << timeReference.timePoint 
					<< " Absolute Time seconds: " << timeReference.absoluteTime.first
					<< " Absolute Time fracSeconds: " << timeReference.absoluteTime.second << '\n';
		#endif
		
		// Compute the time difference between timePoint and the timePoint of the reference
		int64_t sampleDiff = timePoint - timeReference.timePoint;
		//std::cout << "Samplediff " << sampleDiff << '\n';
		double timeDiff = sampleDiff / samplingFrequency;
		//std::cout << "TimeDiff " << timeDiff << '\n';
		auto timeDiffInt = static_cast<int32_t>(floor(timeDiff));
		//std::cout << "timeDiffInt " << timeDiffInt << '\n';		
		// TimeDiffFrac is the positive difference between the floored value and the inial value
		double timeDiffFrac = timeDiff - floor(timeDiff);
		//std::cout << "timeDiffFrac " << timeDiffFrac << '\n';			
		assert(timeDiffFrac >= 0);
		// Compute the integer part of the number of seconds
		seconds = timeReference.absoluteTime.first + timeDiffInt;
		//std::cout << "seconds " << seconds << '\n';			
		
		// Compute the fractional seconds
		if(sampleDiff >= 0)
			fracSeconds = timeReference.absoluteTime.second + timeDiffFrac + (fracTimePoint / samplingFrequency);
		else
			fracSeconds = timeReference.absoluteTime.second + timeDiffFrac + (fracTimePoint / samplingFrequency);
		//std::cout << "fracSeconds " << fracSeconds << '\n';			
		
		assert(fracSeconds >= 0);
		
		// We adjust the values to make sure that the fractional part is between 0 and 1
		auto tmp = static_cast<int32_t>(fracSeconds);
		fracSeconds -= tmp;
		seconds += tmp;

	}
	// ------ CRITICAL SECTION END -------

	
	#if GET_ABSOLUTE_TIME_DEBUG_INFO
	std::cout << "\tOutput seconds: " <<  seconds << " fracSeconds: " << fracSeconds << '\n';
	#endif

	
	return 	std::make_pair(seconds, fracSeconds);

}



} // End of dsptl namespace

#endif



