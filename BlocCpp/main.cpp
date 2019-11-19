#include <iostream>
#include <chrono>
#include "BlockGrid.h"
#include "BlocSolver.h"

int main()
{
	unsigned int smallestGroupSize;

	BlockGrid blockGrid("F:\\Users\\Christoph\\Documents\\Visual Studio 2015\\Projects\\Bloc\\fast.bgf", smallestGroupSize);
	blockGrid.Print();

	BlocSolver solver;

	solver.MaxDBSize = 10000000;
	solver.MaxDepth = std::nullopt;
	solver.DontAddToDBLastDepth = false;

	auto startTime = std::chrono::steady_clock::now();

	solver.Solve(blockGrid, smallestGroupSize);

	auto endTime = std::chrono::steady_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
}