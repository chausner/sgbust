#include <iostream>
#include <chrono>
#include "BlockGrid.h"
#include "BlocSolver.h"

int main()
{
	unsigned int smallestGroupSize;

	BlockGrid blockGrid("E:\\Users\\Christoph\\Documents\\Visual Studio 2015\\Projects\\Bloc\\boop.bgf", smallestGroupSize);


	BlocSolver solver;

	auto startTime = std::chrono::high_resolution_clock::now();

	solver.Solve(blockGrid, smallestGroupSize, 3000000, UINT_MAX, false, false);

	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
}