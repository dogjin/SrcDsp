#include "buffers.h"
#include <vector>


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

	T value{};
	size_t nbrToAdd{};

	std::cout << "+++++ Initial State\n";
	fifo.dumpInfo();

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
	fifo.dumpInfo();

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
	fifo.dumpInfo();

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
	fifo.dumpInfo();

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
	fifo.dumpInfo();

	std::cout << "+++++ Fifo reset\n";
	fifo.reset();
	fifo.dumpInfo();
}
