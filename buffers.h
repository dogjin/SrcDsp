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
#include "pthread.h"

namespace dsptl
{


/***********************************************************************//**
FIFO where the write always write. There is no notion of being full
as far as write is concerned.\n
The read occurs at the location specified in the read call.\n
The buffer keeps a time index which relates to the aboslute time at which the value was collected.
 This time index rolls over when it has
exceeded its maximum value (this is a 64 bit unsigned)\n

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
	FifoWithTimeTrack():
	writePtr(0), timeStart(0), timeEnd(0), rolloverFlag(false), storage(N)
	{ pthread_mutex_init(&mx, nullptr);}
	// Destructor
	~FifoWithTimeTrack()
	{ pthread_mutex_destroy(&mx);}
	// Write values at the back of the fifo
	void write(std::vector<T>& in);
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

private:
	// Location to write the next value
	size_t writePtr;
	// Time index of the earliest value wavailable in the buffer
	uint64_t timeStart;
	// Time index of the latest value available in the buffer
	uint64_t timeEnd;
	// Flag indicating that timeEnd has rolled over but that
	// timeStart has not yet rolled over.
	bool rolloverFlag;
	// FIFO storage
	std::vector<T> storage;
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
};


/******************************************************************************
 * Write the elements in the provided vector in the fifo
 *
 * @param in vector of elements to write. Any existing data is overwritten. The input
 * size must be less than the size of the fifo
 *
 * A true rollover flag indicates that the timeEnd has rolled over but that the
 * time start has not rolled over yet.
 * 
 * 
 * ***************************************************************************/
template <class T, size_t N>
void FifoWithTimeTrack<T,N>::write(std::vector<T> & in)
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

	// The time index of the most recent value is updated
	uint64_t diff = UINT64_MAX - timeEnd;	if(diff >= inSize)
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


/******************************************************************************
 * Write the internal state of the fifo to the standard output
 *
 * @param data Flag indicating wherther the elements are dumped or not
 * 
 * ***************************************************************************/
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

/******************************************************************************
 * Reset the state of the fifo. Internal values are not cleared.\n
 * 
 * ***************************************************************************/

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

/******************************************************************************
 * Retrieves a range of values from the FIFO. The first value is the value
 * associated with the start timestamp. The number of values is indicated by
 * the size of the vector passed as parameter.
 *
 * param out vector in which the values retrieved will be stored
 * param start timestamp of the first value. If this value is zero, 
 * the buffer is filled with the first value available and the value of
 * start is updated approprietly
 *
 * @return true if no error, false otherwise
 *
 * ***************************************************************************/
template <class T, size_t N>
bool  FifoWithTimeTrack<T,N>::read(std::vector<T>& out, uint64_t & start )
{
	// Rollover conditions  of timeStart and timeEnd are not handled
	// Error if the range requested is beyond the existing range
	assert(out.size() != 0);
	if(start == 0)
		start = timeStart;
	if(start <  timeStart || (start + out.size()-1) > timeEnd)
		return true;
	else
	{
		// ------ CRITICAL SECTION START -------
	
		pthread_mutex_lock(&mx);

		size_t end = start + out.size() - 1;
		size_t startPtr = (writePtr + N - (timeEnd - start) - 1 ) % N;  
		size_t endPtr  = (writePtr + N - (timeEnd - end) - 1) % N;  
		//std::cout << "WritePtr " << writePtr << '\n';
		//std::cout << "TimeEnd " << timeEnd << '\n';
		//std::cout << "Start " << start << '\n';
		//std::cout << "End  " << end << '\n';
		//std::cout << "StartPtr  " << startPtr << '\n';
		//std::cout << "EndPtr  " << endPtr << '\n';
	
		pthread_mutex_unlock(&mx);
	
		// ------ CRITICAL SECTION END -------

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
}

/******************************************************************************
 * Return the number of values currently stored in the FIFO.\n
 * 
 * ***************************************************************************/

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


} // End of dsptl namespace


#endif



