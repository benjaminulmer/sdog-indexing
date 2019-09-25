#pragma once

#include <bitset>
#include <functional>
#include <iostream>
#include <tuple>


enum class SdogCellType {
	SG,
	LG,
	NG,
	INVALID
};


constexpr unsigned int INDEX_WIDTH = 64;
constexpr double GRID_RAD = 1.0;


typedef uint_fast64_t Index;
typedef uint_fast32_t DimIndex;
typedef std::function<double(double, double, double)> InterpFunc;
typedef std::function<double(double, double, SdogCellType)> SplitFunc;


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
		return radMin - rhs.radMin <= abs(0.000000001) &&
		       radMax - rhs.radMax <= abs(0.000000001) &&
		       latMin - rhs.latMin <= abs(0.000000001) &&
		       latMax - rhs.latMax <= abs(0.000000001) &&
		       lngMin - rhs.lngMin <= abs(0.000000001) &&
		       lngMax - rhs.lngMax <= abs(0.000000001);
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
	SimpleOperations();
	SimpleOperations(bool volume);
	SimpleOperations(double radPower, double latScale);
	SimpleOperations(SplitFunc radFunc, SplitFunc latFunc);

	Index pointToIndex(const Point& p, int k) const;
	Range indexToRange(Index index) const;

private:
	SplitFunc radFunc;
	SplitFunc latFunc;

};


class EfficientOperations : public IndexOperations {

public:
	Index pointToIndex(const Point& p, int k) const;
	Range indexToRange(Index index) const;
};


class ModifiedEfficient : public IndexOperations {

public:
	ModifiedEfficient();
	ModifiedEfficient(double radPower, double latScale);
	ModifiedEfficient(InterpFunc radFunc, InterpFunc latFunc);

	Index pointToIndex(const Point& p, int k) const;
	Range indexToRange(Index index) const;

private:
	InterpFunc radFunc;
	InterpFunc latFunc;
};