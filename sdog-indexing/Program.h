#pragma once

#include "IndexOperations.h"

#include <random>


class Program {

public:
	void start();

private:
	int comparePointToIndex(const std::vector<Point>& points, int k, const IndexOperations* io1, const IndexOperations* io2, bool log);
	int compareIndexToRange(const std::vector<Index>& indices, const IndexOperations* io1, const IndexOperations* io2, bool log);

	double timePointToIndex(const std::vector<Point>& points, int k, const IndexOperations* io);
	double timeIndexToRange(const std::vector<Index>& indices, const IndexOperations* io);

	std::vector<Point> generateRandomPoints(int n);
	std::vector<Index> generateRandomIndices(int n, int k);
	std::vector<Index> generateIndicesFromPoints(const std::vector<Point>& points, int k);
};

