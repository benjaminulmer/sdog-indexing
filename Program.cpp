#include "Program.h"

#include <chrono>
#include <iostream>
#include <random>


void Program::testPointToIndex() {

	SimpleOperations so;
	EfficientOperations eo;

	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_real_distribution<> radDist(0.0, GRID_RAD);
	std::uniform_real_distribution<> latLngDist(0.0, M_PI_2);
	std::uniform_int_distribution<> levelDist(1, 21);

	int numPoints = 10000000;
	std::vector<Point> points;

	for (int i = 0; i < numPoints; i++) {
		points.push_back(Point(radDist(eng), latLngDist(eng), latLngDist(eng)));
	}


	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	int errorCount = 0;
	for (const Point& p : points) {

		Index si;// = so.pointToIndex(p, 20);
		Index ei = so.pointToIndex(p, 4);

		if (si != ei) {
			errorCount++;
			//std::cout << "Error on trial #" << i << std::endl;
			//std::cout << p << std::endl;
			//std::cout << "Simple: " << si << std::endl;
			//std::cout << "Effcnt: " << ei << std::endl;
			//std::cout << std::endl;
		}
	}

	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::duration<float> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);

	std::cout << "Finished in  " << dTimeS.count() << "s" << std::endl;
	std::cout << errorCount << " errors" << std::endl;
}


void Program::testIndexToRange() {

	SimpleOperations so;
	EfficientOperations eo;

	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_real_distribution<> radDist(0.0, GRID_RAD);
	std::uniform_real_distribution<> latLngDist(0.0, M_PI_2);
	std::uniform_int_distribution<> levelDist(1, 21);

	int numIndices = 10000000;
	std::vector<Index> indices;
	std::vector<Point> points;

	for (int i = 0; i < numIndices; i++) {
		Point p(radDist(eng), latLngDist(eng), latLngDist(eng));
		points.push_back(p);
		indices.push_back(eo.pointToIndex(p, 20));
	}


	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

	int errorCount = 0;
	for (const Index& i : indices) {

		Range sr = eo.indexToRange(i);
		Range er;// = eo.indexToRange(i);

		if (sr != er) {
			errorCount++;
			//std::cout << "Error on trial #" << i << std::endl;
			//std::cout << i << std::endl;
			//std::cout << "Simple: " << sr << std::endl;
			//std::cout << "Effcnt: " << er << std::endl;
			//std::cout << std::endl;
		}
	}

	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::chrono::duration<float> dTimeS = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);

	std::cout << "Finished in  " << dTimeS.count() << "s" << std::endl;
	std::cout << errorCount << " errors" << std::endl;
}
