#ifndef DSP_BUFFERS_H
#define DSP_BUFFERS_H

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
 * exceeded its maximum value (this is a 64 bit unsigned)
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
	writePtr(0), timeStart(0), timeEnd(0), rollOverFlag(false), storage(N)
	{}
	// Write values at the back of the fifo
	write(std::vector<T>& in);
	// Return the desired value in the provided buffer
	read(std::vector<T>& out, uint64_t start, uint64_t end);
	// Return the number of values currently stored in the FIFO
	size_t count();
	/// Reset the pointers and counters of the FIFO
	void reset();

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
}


template <class T, size_t N>
FifoWithTimeTrack<T,N>::write(std::vector<T> in)
{
	size_t upToTop = N - writePtr; // Nbr locations until top
	if(in.size() <= upToTop
		// Everything fits between the writePtr and the top
		std::copy(in.cbegin(), in.cend(), &storage[writePtr]);
	else
	{
		// Part is written from writePtr to the top. The remainder
		// is written at the beginning of the FIFO
		std::copy(in.cbegin(), in.cbegin() + upToTop, writePtr);
		std::copy(in.cbegin() + upToTop, in.cend(), &storage[0]);
	}
	// Update writePtr
	writePtr = (writePtr + in.size()) % N;
	// Update timeEnd - timeEnd corresponds to "writePtr - 1"
	
	



}

} // End of dsptl namespace
#endif
