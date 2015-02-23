#include "buffers.h"
#include <vector>
#include <iostream>
#include <iterator>

template <class T>
bool testFifoWithTimeTrack();

int main()
{
	
	testFifoWithTimeTrack<double>();

	return 0;
}

template <class T>
bool testFifoWithTimeTrack()
{
	using namespace dsptl;

	FifoWithTimeTrack<T, 15> fifo;
	std::vector<T> input;
	std::vector<T> output;

	T value{};
	size_t nbrToAdd{};
	size_t nbrToRead{};

	std::cout << "+++++ Initial State\n";
	fifo.dumpInfo(true);

	// Add a lot of elements
	size_t addedNbr{};
	for (int index =0; index < 23 ; ++index)
	{
		nbrToAdd = 14;
		input.assign(nbrToAdd , T{});
		for (auto &elt:input)
		{
			value += static_cast<T>(1);
			elt = value;
		}
		fifo.write(input);
		addedNbr += nbrToAdd;
	}

	std::cout << "+++++ Add " <<  addedNbr <<" elements\n";
	fifo.dumpInfo(true);

	// Add 10 elements
	nbrToAdd = 10;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	// Add 5 elements
	nbrToAdd = 5;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	// Add 7 elements
	nbrToAdd = 7;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	size_t cnt = fifo.count();
	std::cout << " Fifo count " << cnt << '\n';
	
	std::cout << "+++++ Fifo reset\n";
	fifo.reset();
	fifo.dumpInfo(true);

	//------------------------------   Read elements
	
	// Reset value stored in the elements
	value = 0;	
	bool retval;
	uint32_t timeFirstElt ;
	// Add 7 elements
	nbrToAdd = 7;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	nbrToRead = 3;
	timeFirstElt = 4;
	std::cout << "+++++ Read " <<  nbrToRead <<" elements\n";
	output.assign(nbrToRead,T{});
	retval = fifo.read(output, timeFirstElt);
	std::cout << "Read return value: " <<  retval  <<"\n";
	std::copy(output.cbegin(), output.cend(), std::ostream_iterator<T>(std::cout," \n"));

	// Add 10 elements Read 15
	//
	nbrToAdd = 10;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	nbrToRead = 15;
	timeFirstElt = 3;
	std::cout << "+++++ Read " <<  nbrToRead <<" elements\n";
	output.assign(nbrToRead,T{});
	retval = fifo.read(output, timeFirstElt);
	std::cout << "Read return value: " <<  retval  <<"\n";
	std::copy(output.cbegin(), output.cend(), std::ostream_iterator<T>(std::cout," \n"));


	// Add 4 elements Read 15
	//
	nbrToAdd = 4;
	std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
	input.assign(nbrToAdd , T{});
	for (auto &elt:input)
	{
		value += static_cast<T>(1);
		elt = value;
	}
	fifo.write(input);
	fifo.dumpInfo(true);

	nbrToRead = 4;
	timeFirstElt = 6;
	std::cout << "+++++ Read " <<  nbrToRead <<" elements\n";
	output.assign(nbrToRead,T{});
	retval = fifo.read(output, timeFirstElt);
	std::cout << "Read return value: " <<  retval  <<"\n";
	std::copy(output.cbegin(), output.cend(), std::ostream_iterator<T>(std::cout," \n"));
}
