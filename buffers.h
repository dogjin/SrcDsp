#ifndef DSP_BUFFERS_H
#define DSP_BUFFERS_H

#include <cstddef>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>

namespace dsptl
{

/******************************************************************************
 * @file
 *
 * Definition of all objects and utilities related to buffers. \n
 * This include specialized buffers like fifos or queues
 *
 * ***************************************************************************/


/******************************************************************************
 * FIFO where the write always write. There is no notion of being full
 * as far as write is concerned.\n
 * The read occurs at the location specified in the read call.\n
 * The buffer keeps a time index which relates to the aboslute time
 * at which the value was collected. This time index rolls over when it has
 * exceeded its maximum value (this is a 64 bit unsigned)\n
 *
 * The number of values added into the fifo in a single write opertion
 * MUST BE LESS than the size of the FIFO
 *
 * @tparam T Type of the values stored in the buffer
 * @tparam N Number of elements stored in the FIFO
 *
 * ***************************************************************************/
template <class T, size_t N>
class FifoWithTimeTrack
{
public:
	// Constructor
	FifoWithTimeTrack():
	writePtr(0), timeStart(0), timeEnd(0), rolloverFlag(false), storage(N)
	{}
	// Write values at the back of the fifo
	void write(std::vector<T>& in);
	// Return the desired value in the provided buffer
	void read(std::vector<T>& out, uint64_t start, uint64_t end);
	// Return the number of values currently stored in the FIFO
	size_t count();
	/// Reset the pointers and counters of the FIFO as if the FIFO
	/// was just created
	void reset();
	/// Write debuugin info to the standard output
	// This is mainly for debuggin purposes
	void dumpInfo();

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
};


template <class T, size_t N>
void FifoWithTimeTrack<T,N>::write(std::vector<T> & in)
{
	size_t upToTop = N - writePtr; // Nbr locations until top
	size_t inSize = in.size();
	assert(inSize < N);
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
	// Update writePtr
	writePtr = (writePtr + inSize) % N;

	// The time index of the most recent value is updated
	uint64_t diff = UINT64_MAX - timeEnd;
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
	



}


template <class T, size_t N>
void FifoWithTimeTrack<T,N>::dumpInfo()
{
	std::cout << "writePtr: " << writePtr << '\n';
	std::cout << "timeStart : " << timeStart << '\n';
	std::cout << "timeEnd : " << timeEnd << '\n';
	std::cout << "rolloverFlag : " << rolloverFlag << '\n';
	size_t index = 0;
	for (auto& elt:storage)
		std::cout << "index: " << index++ << " Value: " << elt << '\n';
}

template <class T, size_t N>
void FifoWithTimeTrack<T,N>::reset()
{
	writePtr = 0;
	timeStart = 0;
	timeEnd = 0;
	rolloverFlag = false; 
}
} // End of dsptl namespace
#endif
