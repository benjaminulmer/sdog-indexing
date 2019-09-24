#pragma once

#include <bitset>
#include <functional>
#include <iostream>
#include <tuple>

constexpr unsigned int INDEX_WIDTH = 64;
constexpr double GRID_RAD = 1.0;

typedef uint_fast64_t Index;
typedef uint_fast32_t DimIndex;
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

	bool operator==(const Range& rhs) {
		return radMin == rhs.radMin &&
		       radMax == rhs.radMax &&
		       latMin == rhs.latMin &&
		       latMax == rhs.latMax &&
		       lngMin == rhs.lngMin &&
		       lngMax == rhs.lngMax;
	}
	bool operator!=(const Range& rhs) {
		return !(*this == rhs);
	}
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
//class ModifiedSimple : public IndexOperations {
//
//};
//
//
//class ModifiedEfficient : public IndexOperations {
//
//public:
//	ModifiedEfficient();
//	ModifiedEfficient(double radPower, double latScale);
//	ModifiedEfficient(InterpFunc rad, InterpFunc lat);
//
//	Index pointToIndex(const Point& p, int k) const;
//	Range indexToRange(Index index) const;
//
//private:
//	InterpFunc rad;
//	InterpFunc lat;
//};