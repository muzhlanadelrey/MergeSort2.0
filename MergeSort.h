#pragma once
#include <iostream>
#include <vector>

void largeVectorFill(std::vector<int>& largeVector, long long size);
void merge(std::vector<int>& array, int left, int middle, int right);
void mergeSort(std::vector<int>& array, int left, int right, ThreadPool& pool);
void printArray(const std::vector<int>& array);
int coreNumber();