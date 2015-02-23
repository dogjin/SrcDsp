#include "buffers.h"
#include <vector>
#include <iostream>
#include <iterator>
#include <csignal>

template <class T>
bool testFifoWithTimeTrack();
bool quitFlag = false;

int main()
{
	
	testFifoWithTimeTrack<double>();

	return 0;
}


void quitInterrupt(int sig)
{
	quitFlag = true;
}

template <class T>
bool testFifoWithTimeTrack()
{
	using namespace dsptl;

	// variables declaration
	FifoWithTimeTrack<T, 15> fifo;
	std::vector<T> input;
	std::vector<T> output;
	T value{};
	size_t nbrToAdd{};
	size_t nbrToRead{};
	bool retval;
	uint32_t timeFirstElt;

	signal(SIGINT, quitInterrupt);
	std::cout << "+++++ Initial State\n";
	fifo.dumpInfo();

	// At each iteration, we add elements and read some other ones
	while (!quitFlag)
	{
		// Get info from user
		std::cout << "-------  nbrToAdd, nbrToRead, timeFirstElt : " << '\n';
		std::cin >> nbrToAdd >> nbrToRead >> timeFirstElt;

		// Add  elements 
		//
		std::cout << "+++++ Add " <<  nbrToAdd <<" elements\n";
		input.assign(nbrToAdd , T{});
		for (auto &elt:input)
		{
			value += static_cast<T>(1);
			elt = value;
		}
		fifo.write(input);
		fifo.dumpInfo();

		std::cout << "+++++ Read " <<  nbrToRead <<" elements\n";
		output.assign(nbrToRead,T{});
		retval = fifo.read(output, timeFirstElt);
		std::cout << "Read return value: " <<  retval  <<"\n";
		std::copy(output.cbegin(), output.cend(), std::ostream_iterator<T>(std::cout," \n"));
	}
}
