#include "IndexOperations.h"

#include <libmorton/morton.h>

#include <algorithm>
#include <cmath>


std::ostream& operator<<(std::ostream& os, const Point& p) {
	os << p.rad << ", " << p.lat << ", " << p.lng;
	return os;
}


std::ostream& operator<<(std::ostream& os, const Range& r) {
	os << "Rad [" << r.radMin << ", " << r.radMax << "] ";
	os << "Lat [" << r.latMin << ", " << r.latMax << "] ";
	os << "Lng [" << r.lngMin << ", " << r.lngMax << "]";
	return os;
}


SimpleOperations::SimpleOperations() {

	auto mid = [&](double max, double min, SdogCellType type) {
		return (max + min) / 2.0;
	};
	radFunc = mid;
	latFunc = mid;
}


SimpleOperations::SimpleOperations(bool volume) {

	if (volume) {
		auto r = [&](double max, double min, SdogCellType type) {
			if (type == SdogCellType::NG || type == SdogCellType::LG) {
				return cbrt((max * max * max + min * min * min) / 2.0);
			}
			else {
				return (max + min) / 2.0;
			}
		};
		radFunc = r;

		auto l = [&](double max, double min, SdogCellType type) {
			if (type == SdogCellType::SG || type == SdogCellType::LG) {
				return asin(0.75 * sin(max) + 0.25 * sin(min));
			}
			else {
				return asin((sin(max) + sin(min)) / 2.0);
			}
		};
		latFunc = l;
	}
	else {
		SimpleOperations();
	}
}


SimpleOperations::SimpleOperations(double radPower, double latScale) {

	auto r = [&](double max, double min, SdogCellType type) {
		if (type == SdogCellType::NG || type == SdogCellType::LG) {
			return pow((max * max * max + min * min * min) / 2.0, 1.0 / radPower);
		}
		else {
			return (max + min) / 2.0;
		}
	};
	radFunc = r;

	auto l = [&](double max, double min, SdogCellType type) {
		if (type == SdogCellType::SG || type == SdogCellType::LG) {
			return asin(0.75 * sin(max) + 0.25 * sin(min));
		}
		else {
			return latScale * asin(0.5 * (1.0 / latScale) * sin(max) + 0.5 * (1.0 / latScale) * sin(min));
		}
	};
	latFunc = l;
}


SimpleOperations::SimpleOperations(SplitFunc radFunc, SplitFunc latFunc) :
	radFunc(radFunc),
	latFunc(latFunc)
{}


Index SimpleOperations::pointToIndex(const Point& p, int k) const {

	Range r;
	r.radMin = 0.0;
	r.radMax = GRID_RAD;
	r.latMin = 0.0;
	r.latMax = M_PI_2;
	r.lngMin = 0.0;
	r.lngMax = M_PI_2;

	Index index;
	index = 1;

	// Loop for desired number of levels and determine which child point is in for each itteration
	SdogCellType curType = SdogCellType::SG;
	for (int i = 0; i < k; i++) {

		unsigned int childCode = 0;
		double radMid = radFunc(r.radMax, r.radMin, curType);
		double latMid = latFunc(r.latMax, r.latMin, curType);
		double lngMid = (r.lngMin + r.lngMax) / 2.0;

		if (curType == SdogCellType::NG) {

			if (p.rad > radMid) {
				r.radMin = radMid;
			}
			else {
				childCode += 4;
				r.radMax = radMid;
			}
			if (p.lat < latMid) {
				r.latMax = latMid;
			}
			else {
				childCode += 2;
				r.latMin = latMid;
			}
			if (p.lng < lngMid) {
				r.lngMax = lngMid;
			}
			else {
				childCode += 1;
				r.lngMin = lngMid;
			}
			// type doesn't change
		}
		else if (curType == SdogCellType::LG) {

			if (p.rad > radMid) {
				r.radMin = radMid;
			}
			else {
				r.radMax = radMid;
				childCode += 4;
			}
			if (p.lat < latMid) {
				r.latMax = latMid;
				curType = SdogCellType::NG;

				if (p.lng < lngMid) {
					r.lngMax = lngMid;
				}
				else {
					childCode += 1;
					r.lngMin = lngMid;
				}
			}
			else {
				childCode += 2;
				r.latMin = latMid;
				// type doesn't change
			}
		}
		else {// curType == SdogCellType::SG

			if (p.rad > radMid) {

				r.radMin = radMid;

				if (p.lat < latMid) {
					r.latMax = latMid;
					curType = SdogCellType::NG;

					if (p.lng < lngMid) {
						childCode = 0;
						r.lngMax = lngMid;
					}
					else {
						childCode = 1;
						r.lngMin = lngMid;
					}
				}
				else {
					childCode = 2;
					r.latMin = latMid;
					curType = SdogCellType::LG;
				}
			}
			else {
				childCode = 4;
				r.radMax = radMid;
				// type doesn't change
			}
		}
		index <<= 3;
		index |= childCode;
	}
	return index;
}


Range SimpleOperations::indexToRange(Index index) const {

	// Find width of index
	Index copy = index;
	int width = 0;
	while (copy >>= 1) {
		width++;
	}

	Range r;
	r.radMin = 0.0;
	r.radMax = GRID_RAD;
	r.latMin = 0.0;
	r.latMax = M_PI_2;
	r.lngMin = 0.0;
	r.lngMax = M_PI_2;

	int k = width / 3;

	// Loop for each char in code and determine properties based on code
	SdogCellType type = SdogCellType::SG;
	for (int i = k - 1; i >= 0; i--) {

		DimIndex code = (index & (7ll << (i * 3))) >> (i * 3);

		double radMid = radFunc(r.radMax, r.radMin, type);
		double latMid = latFunc(r.latMax, r.latMin, type);
		double lngMid = (r.lngMin + r.lngMax) / 2.0;

		if (type == SdogCellType::NG) {

			if (code == 0) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMax = lngMid;
			}
			else if (code == 1) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMin = lngMid;
			}
			else if (code == 2) {
				r.radMin = radMid;
				r.latMin = latMid;
				r.lngMax = lngMid;
			}
			else if (code == 3) {
				r.radMin = radMid;
				r.latMin = latMid;
				r.lngMin = lngMid;
			}
			else if (code == 4) {
				r.radMax = radMid;
				r.latMax = latMid;
				r.lngMax = lngMid;
			}
			else if (code == 5) {
				r.radMax = radMid;
				r.latMax = latMid;
				r.lngMin = lngMid;
			}
			else if (code == 6) {
				r.radMax = radMid;
				r.latMin = latMid;
				r.lngMax = lngMid;
			}
			else if (code == 7) {
				r.radMax = radMid;
				r.latMin = latMid;
				r.lngMin = lngMid;
			}
			else {
				type = SdogCellType::INVALID;
				break;
			}
			// type doesn't change
		}
		else if (type == SdogCellType::LG) {

			if (code == 0) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMax = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 1) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMin = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 2) {
				r.radMin = radMid;
				r.latMin = latMid;
				// type doesn't change
			}
			else if (code == 4) {
				r.radMax = radMid;
				r.latMax = latMid;
				r.lngMax = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 5) {
				r.radMax = radMid;
				r.latMax = latMid;
				r.lngMin = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 6) {
				r.radMax = radMid;
				r.latMin = latMid;
				// type doesn't change
			}
			else {
				type = SdogCellType::INVALID;
				break;
			}
		}
		else {// type == CellType::SG

			if (code == 0) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMax = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 1) {
				r.radMin = radMid;
				r.latMax = latMid;
				r.lngMin = lngMid;
				type = SdogCellType::NG;
			}
			else if (code == 2) {
				r.radMin = radMid;
				r.latMin = latMid;
				type = SdogCellType::LG;
			}
			else if (code == 4) {
				r.radMax = radMid;
				// type doesn't change
			}
			else {
				type = SdogCellType::INVALID;
				break;
			}
		}
	}
	return r;
}


Index EfficientOperations::pointToIndex(const Point& p, int k) const {

	// Find percentage distance in each coordinate
	double radP = 1.0 - (p.rad / GRID_RAD);
	double latP = p.lat / M_PI_2;
	double lngP = p.lng / M_PI_2;

	// Modifiers to account for degenerate subdivision
	int latD = std::min((int)floor(-log2(1.0 - radP)), k);
	int lngD = std::min(latD + (int)floor(-log2(1.0 - latP)), k);

	// Find index in each coordinate
	// 1ll << k == 2^k
	DimIndex radI = (DimIndex)floor((1ll << k) * radP);
	DimIndex latI = (DimIndex)floor((1ll << (k - latD)) * latP);
	DimIndex lngI = (DimIndex)floor((1ll << (k - lngD)) * lngP);

	// Interleave Morton Code and set 1 bit at beginning to mark start of index
	return libmorton::morton3D_64_encode(lngI, latI, radI) + (1ll << (k * 3));
}


Range EfficientOperations::indexToRange(Index index) const {

	// Find width of index
	Index copy = index;
	int width = 0;
	while (copy >>= 1) {
		width++;
	}

	index ^= 1ll << width;

	DimIndex radI, latI, lngI;
	libmorton::morton3D_64_decode(index, lngI, latI, radI);

	Range r;
	int k = width / 3;

	r.radMax = radI / (double)(1ll << k);
	r.radMin = (radI + 1.0) / (double)(1ll << k);

	// Modifier to account for degenerate subdivision
	int latD = std::min((int)floor(-log2(1.0 - r.radMax)), k);

	r.latMin = latI / (double)(1ll << (k - latD));
	r.latMax = (latI + 1.0) / (double)(1ll << (k - latD));

	// Modifier to account for degenerate subdivision
	int lngD = std::min(latD + (int)floor(-log2(1.0 - r.latMin)), k);

	r.lngMin = lngI / (double)(1ll << (k - lngD));
	r.lngMax = (lngI + 1.0) / (double)(1ll << (k - lngD));

	// Put bounds into coordinate domain as opposed to parameter
	r.radMin = (1.0 - r.radMin) * GRID_RAD;
	r.radMax = (1.0 - r.radMax) * GRID_RAD;
	r.latMin *= M_PI_2;
	r.latMax *= M_PI_2;
	r.lngMin *= M_PI_2;
	r.lngMax *= M_PI_2;

	return r;
}


ModifiedEfficient::ModifiedEfficient() {

	auto r = [&](double max, double min, double d) {
		return GRID_RAD * cbrt(d * max*max*max + (1.0 - d) * min*min*min);
	};
	radFunc = r;

	auto l = [&](double max, double min, double d) {
		return asin(d * sin(max) + (1.0 - d) * sin(min));
	};
	latFunc = l;
}


ModifiedEfficient::ModifiedEfficient(double radPower, double latScale) {

	auto r = [&](double max, double min, double d) {
		return GRID_RAD * pow(d * pow(max, radPower) + (1.0 - d) * pow(min, radPower), 1 / radPower);
	};
	radFunc = r;

	auto l = [&](double max, double min, double d) {
		return latScale * asin(d * (1.0 / latScale) * sin(max) + (1.0 - d) * (1.0 / latScale) * sin(min));
	};
	latFunc = l;
}


ModifiedEfficient::ModifiedEfficient(InterpFunc radFunc, InterpFunc latFunc) :
	radFunc(radFunc),
	latFunc(latFunc)
{}


Index ModifiedEfficient::pointToIndex(const Point& p, int k) const {

	// Find percentage distance in each coordinate
	double radP = 1.0 - (p.rad / GRID_RAD);
	double latP = sin(p.lat);
	double lngP = p.lng / M_PI_2;

	double radExp = -log2(1.0 - radP);
	double latExp = -log2(sqrt(1.0 - latP));

	// Modifiers to account for degenerate subdivision
	int latD = std::min((int)floor(radExp), k);
	int lngD = std::min(latD + (int)floor(latExp), k);

	// Max and mins for mapping technique
	double rMax = pow(2.0, -floor(radExp));
	double rMin = pow(2.0, -ceil(radExp));

	double lsMin = 1.0 - (pow(2.0, -floor(latExp)));
	double lsMax = 1.0 - (pow(2.0, -ceil(latExp)));

	// Handle exact logarithm (latitude on split)
	if (lsMax != lsMin) {
		double lvMin = 2.0 * lsMin - lsMin * lsMin;
		double lvMax = 2.0 * lsMax - lsMax * lsMax;

		double latNGP = (latP - lvMin) / (lvMax - lvMin);
		latP = latNGP * lsMax + (1.0 - latNGP) * lsMin;
	}
	else {
		latP = lsMax;
	}

	// Handle exact logarithm (radius on split)
	if (rMax != rMin) {
		double radPi = 1.0 - radP;
		double radNGP = (radPi * radPi * radPi - rMin * rMin * rMin) / (rMax * rMax * rMax - rMin * rMin * rMin);

		radP = 1.0 - (radNGP * rMax + (1.0 - radNGP) * rMin);
	}
	else {
		radP = rMax;
	}

	// Find index in each coordinate
	// 1ll << k == 2^k
	DimIndex radI = (DimIndex)floor((1ll << k) * radP);
	DimIndex latI = (DimIndex)floor((1ll << (k - latD)) * latP);
	DimIndex lngI = (DimIndex)floor((1ll << (k - lngD)) * lngP);

	// Interleave Morton Code and set 1 bit at beginning to mark start of index
	return libmorton::morton3D_64_encode(lngI, latI, radI) + (1ll << (k * 3));
}


Range ModifiedEfficient::indexToRange(Index index) const {

	// Find width of index
	Index copy = index;
	int width = 0;
	while (copy >>= 1) {
		width++;
	}

	index ^= 1ll << width;

	DimIndex radI, latI, lngI;
	libmorton::morton3D_64_decode(index, lngI, latI, radI);

	Range r;
	int k = width / 3;

	r.radMax = radI / (double)(1ll << k);
	r.radMin = (radI + 1.0) / (double)(1ll << k);

	// Max and min for mapping technique
	double radExp = -log2(1.0 - r.radMax);
	double rMax = pow(2.0, -floor(radExp));
	double rMin = pow(2.0, -ceil(radExp));

	// Handle exact logarithm (max radius on split)
	if (rMax == rMin) {
		rMin = pow(2.0, -(ceil(radExp) + 1.0));
	}

	double radMaxNGP = (1.0 - r.radMax - rMin) / (rMax - rMin);
	double radMinNGP = (1.0 - r.radMin - rMin) / (rMax - rMin);

	// Modifier to account for degenerate subdivision
	int latD = std::min((int)floor(radExp), k);
	r.latMin = latI / (double)(1ll << (k - latD));
	r.latMax = (latI + 1.0) / (double)(1ll << (k - latD));

	// Max and min for mapping technique
	double latExp = -log2(1.0 - r.latMin);
	double lsMin = 1.0 - (pow(2.0, -floor(latExp)));
	double lsMax = 1.0 - (pow(2.0, -ceil(latExp)));

	// Handle exact logarithm (min latitude on split)
	if (lsMax == lsMin) {
		lsMax = 1.0 - (pow(2.0, -(ceil(latExp) + 1.0)));
	}

	double lvMin = 2.0 * lsMin - lsMin * lsMin;
	double lvMax = 2.0 * lsMax - lsMax * lsMax;

	double latMaxNGP = (r.latMax - lsMin) / (lsMax - lsMin);
	double latMinNGP = (r.latMin - lsMin) / (lsMax - lsMin);

	// Modifier to account for degenerate subdivision
	int lngD = std::min(latD + (int)floor(latExp), k);
	r.lngMin = lngI / (double)(1ll << (k - lngD));
	r.lngMax = (lngI + 1.0) / (double)(1ll << (k - lngD));

	// Put bounds into coordinate domain as opposed to parameter
	r.radMin = cbrt(radMinNGP * rMax * rMax * rMax + (1.0 - radMinNGP) * rMin * rMin * rMin) * GRID_RAD;
	if (r.radMin < 0.0) r.radMin = 0.0; // precision issues when min radius is 0
	r.radMax = cbrt(radMaxNGP * rMax * rMax * rMax + (1.0 - radMaxNGP) * rMin * rMin * rMin) * GRID_RAD;
	r.latMin = asin(latMinNGP * lvMax + (1.0 - latMinNGP) * lvMin);

	double temp = latMaxNGP * lvMax + (1.0 - latMaxNGP) * lvMin;
	if (temp > 1.0) temp = 1.0; // handle precision issues that result in asin(1.0 + e)
	r.latMax = asin(temp);

	r.lngMin *= M_PI_2;
	r.lngMax *= M_PI_2;

	return r;
}
