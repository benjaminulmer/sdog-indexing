#include "Program.h"

#include <chrono>
#include <fstream>
#include <iostream>


void Program::testOperations(int n, int k) {

	std::vector<Point> points = generateRandomPoints(n);
	std::vector<Index> indices = generateIndicesFromPoints(points, k);

	SimpleOperations simple;
	SimpleOperations simpleVol(true);
	EfficientOperations efficient;
	ModifiedEfficient efficientVol;

	std::cout << "testing " << n << " times at k = " << k << std::endl;

	int numPtoIErrors = comparePointToIndex(points, k, &simple, &efficient, false);
	std::cout << "PtoI non: " << numPtoIErrors << std::endl;

	int numVolPtoIErrors = comparePointToIndex(points, k, &simpleVol, &efficientVol, false);
	std::cout << "PtoI vol: " << numVolPtoIErrors << std::endl;

	if (numPtoIErrors == 0) {
		int numItoRErrors = compareIndexToRange(indices, &simple, &efficient, false);
		std::cout << "ItoR non: " << numItoRErrors << std::endl;
	}
	if (numVolPtoIErrors == 0) {
		int numVolItoRErrors = compareIndexToRange(indices, &simpleVol, &efficientVol, false);
		std::cout << "ItoR vol: " << numVolItoRErrors << std::endl;
	}
}


void Program::benchmarkAll(int n, int maxK) {

	std::ofstream out("results-10mil.csv");
	out << "Simple PtoI,Simple Volume PtoI,Efficient PtoI,Efficient Volume PtoI,Simple ItoR,Simple Volume ItoR,Efficient ItoR,Efficient Volume ItoR" << std::endl;

	SimpleOperations simple;
	SimpleOperations simpleVol(true);
	EfficientOperations efficient;
	ModifiedEfficient efficientVol;

	std::vector<Point> points = generateRandomPoints(n);

	for (int k = 1; k <= maxK; k++) {

		std::vector<Index> indices = generateIndicesFromPoints(points, k);
		std::cout << "starting " << k << "...";

		out << timePointToIndex(points, k, &simple) << ",";
		out << timePointToIndex(points, k, &simpleVol) << ",";
		out << timePointToIndex(points, k, &efficient) << ",";
		out << timePointToIndex(points, k, &efficientVol) << ",";

		out << timeIndexToRange(indices, &simple) << ",";
		out << timeIndexToRange(indices, &simpleVol) << ",";
		out << timeIndexToRange(indices, &efficient) << ",";
		out << timeIndexToRange(indices, &efficientVol) << std::endl;

		std::cout << " done" << std::endl;
	}

	out.close();
}


int Program::comparePointToIndex(const std::vector<Point>& points, int k, const IndexOperations* io1, const IndexOperations* io2, bool log) {

	int errorCount = 0;
	for (const Point& p : points) {

		Index i1 = io1->pointToIndex(p, k);
		Index i2 = io2->pointToIndex(p, k);

		if (i1 != i2) {
			errorCount++;
			if (log) {
				std::cout << "Error" << std::endl;
				std::cout << p << std::endl;
				std::cout << "IO 1: " << std::bitset<64>(i1) << std::endl;
				std::cout << "IO 2: " << std::bitset<64>(i2) << std::endl;
				std::cout << std::endl;
			}
		}
	}

	if (log) {
		std::cout << "Finished with  " << errorCount << " errors out of " << points.size() << " tests" << std::endl;
	}
	return errorCount;
}


int Program::compareIndexToRange(const std::vector<Index>& indices, const IndexOperations* io1, const IndexOperations* io2, bool log) {


	int errorCount = 0;
	for (const Index& i : indices) {

		Range r1 = io1->indexToRange(i);
		Range r2 = io2->indexToRange(i);

		if (r1 != r2) {
			errorCount++;
			if (log) {
				std::cout << "Error" << std::endl;
				std::cout << std::bitset<64>(i) << std::endl;
				std::cout << "IO 1: " << r1 << std::endl;
				std::cout << "IO 2: " << r2 << std::endl;
				std::cout << std::endl;
			}
		}
	}

	if (log) {
		std::cout << "Finished with  " << errorCount << " errors out of " << indices.size() << " tests" << std::endl;
	}
	return errorCount;
}


double Program::timePointToIndex(const std::vector<Point>& points, int k, const IndexOperations* io) {

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	for (const Point& p : points) {
		Index i = io->pointToIndex(p, k);
	}

	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::duration<double> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);

	return dTimeS.count();
}


double Program::timeIndexToRange(const std::vector<Index>& indices, const IndexOperations* io) {
	
	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	for (const Index& i : indices) {
		Range r = io->indexToRange(i);
	}

	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::duration<double> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);

	return dTimeS.count();
}


std::vector<Point> Program::generateRandomPoints(int n) {

	std::random_device rd;
	std::mt19937 eng(rd());

	std::uniform_real_distribution<> radDist(0.0, GRID_RAD);
	std::uniform_real_distribution<> latLngDist(0.0, M_PI_2);

	std::vector<Point> points;
	for (int i = 0; i < n; i++) {
		points.push_back(Point(radDist(eng), latLngDist(eng), latLngDist(eng)));
	}

	return points;
}


std::vector<Index> Program::generateRandomIndices(int n, int k) {
	return generateIndicesFromPoints(generateRandomPoints(n), k);
}


std::vector<Index> Program::generateIndicesFromPoints(const std::vector<Point>& points, int k) {
	
	std::vector<Index> indices;
	EfficientOperations eo;

	for (const Point& p : points) {
		indices.push_back(eo.pointToIndex(p, k));
	}

	return indices;
}
