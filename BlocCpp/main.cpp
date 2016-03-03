#include <iostream>
#include <chrono>
#include "BlockGrid.h"
#include "BlocSolver.h"

#include <vector>

int main()
{
	unsigned int smallestGroupSize;

	BlockGrid blockGrid("E:\\Users\\Christoph\\Documents\\Visual Studio 2015\\Projects\\Bloc\\wikipedia.bgf", smallestGroupSize);

	//for (char c : "AEEICBFG")
	//	if (c != '\0')
	//        blockGrid.RemoveGroup(blockGrid.GetGroups(smallestGroupSize)[c - 'A']);

	BlocSolver solver;

	auto startTime = std::chrono::high_resolution_clock::now();

	solver.Solve(blockGrid, smallestGroupSize, 3000000, 15, false, false);

	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
}