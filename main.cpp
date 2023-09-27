#include "MergeSort.h"
#include "ThreadPool.h"
#include <Windows.h>

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "Russian");

	int numCores = coreNumber();
	std::cout << "\nКоличество ядер CPU: " << numCores << std::endl;
	int maxThreads = numCores;

	std::vector<int> largeVector;
	const long long size = 4'000'000;

	largeVectorFill(largeVector, size);

	ThreadPool pool;

	mergeSort(largeVector, 0, largeVector.size() - 1, pool);

	printArray(largeVector);

	return 0;
}