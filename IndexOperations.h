#pragma once

#include <bitset>
#include <functional>
#include <iostream>
#include <tuple>


constexpr double GRID_RAD = 1.0;
constexpr int INDEX_WIDTH = 64;

typedef std::bitset<INDEX_WIDTH> Index;
typedef unsigned __int64 uint64;
typedef std::function<double(double, double, double)> InterpFunc;


struct Point {
	Point() = default;
	Point(double rad, double lat, double lng) :
		rad(rad),
		lat(lat),
		lng(lng)
	{}

	double rad;
	double lat;
	double lng;

	friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

struct Range {
	Range() = default;
	Range(double radMin, double radMax, double latMin, double latMax, double lngMin, double lngMax) :
		radMin(radMin),
		radMax(radMax),
		latMin(latMin),
		latMax(latMax),
		lngMin(lngMin),
		lngMax(lngMax)
	{}

	double radMin, radMax;
	double latMin, latMax;
	double lngMin, lngMax;

	friend std::ostream& operator<<(std::ostream& os, const Range& r);
};


class IndexOperations {

public:
	virtual Index pointToIndex(const Point& p, int k) const = 0;
	virtual Range indexToRange(Index index) const = 0;
};


class SimpleOperations : public IndexOperations {

public:
	Index pointToIndex(const Point& p, int k) const;
	Range indexToRange(Index index) const;
};


class EfficientOperations : public IndexOperations {

public:
	Index pointToIndex(const Point& p, int k) const;
	Range indexToRange(Index index) const;
};

//
//class MappedOperations : public IndexOperations {
//
//public:
//	MappedOperations();
//	MappedOperations(double radPower, double latScale);
//	MappedOperations(InterpFunc rad, InterpFunc lat);
//
//	Index pointToIndex(const Point& p, int k) const;
//	Range indexToRange(Index index) const;
//
//private:
//	InterpFunc rad;
//	InterpFunc lat;
//};