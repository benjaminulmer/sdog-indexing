#include "IndexOperations.h"

#include <libmorton/morton.h>

#include <algorithm>
#include <cmath>


double logB(double arg, double base) {
	return log(arg) / log(base);
}

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

	auto mid = [=](double max, double min, SdogCellType type) {
		return (max + min) / 2.0;
	};
	radSplit = mid;
	latSplit = mid;
}


SimpleOperations::SimpleOperations(bool volume) {

	if (volume) {
		auto r = [=](double max, double min, SdogCellType type) {
			if (type == SdogCellType::NG || type == SdogCellType::LG) {
				return cbrt((max * max * max + min * min * min) / 2.0);
			}
			else {
				return (max + min) / 2.0;
			}
		};
		radSplit = r;

		auto l = [=](double max, double min, SdogCellType type) {
			if (type == SdogCellType::SG || type == SdogCellType::LG) {
				return asin(0.75 * sin(max) + 0.25 * sin(min));
			}
			else {
				return asin((sin(max) + sin(min)) / 2.0);
			}
		};
		latSplit = l;
	}
	else {
		SimpleOperations();
	}
}


SimpleOperations::SimpleOperations(double radPower, double latScale) {

	auto r = [=](double max, double min, SdogCellType type) {
		if (type == SdogCellType::NG || type == SdogCellType::LG) {
			return pow((pow(max, radPower) + pow(min, radPower)) / 2.0, 1.0 / radPower);
		}
		else {
			return (max + min) / 2.0;
		}
	};
	radSplit = r;

	auto l = [=](double max, double min, SdogCellType type) {
		if (type == SdogCellType::SG || type == SdogCellType::LG) {
			return asin(0.75 * sin(max) + 0.25 * sin(min));
		}
		else {
			return latScale * asin(0.5 * sin(max / latScale) + 0.5 * sin(min / latScale));
		}
	};
	latSplit = l;
}


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
		double radMid = radSplit(r.radMax, r.radMin, curType);
		double latMid = latSplit(r.latMax, r.latMin, curType);
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

		double radMid = radSplit(r.radMax, r.radMin, type);
		double latMid = latSplit(r.latMax, r.latMin, type);
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


ModifiedEfficient::ModifiedEfficient() {

	radInterpFunc = [=](double max, double min, double d) {
		return cbrt(d * max * max * max + (1.0 - d) * min * min * min);
	};
	radPercFunc = [=](double max, double min, double value) {
		return (value * value * value - min * min * min) / (max * max * max - min * min * min);
	};

	latInterpFunc = [=](double max, double min, double d) {
		return asin(d * max + (1.0 - d) * min);
	};
	latPercFunc = [=](double max, double min, double value) {
		return (value - min) / (max - min);
	};
}


ModifiedEfficient::ModifiedEfficient(double radPower, double latScale) {

	radInterpFunc = [=](double max, double min, double d) {
		return pow(d * pow(max, radPower) + (1.0 - d) * pow(min, radPower), 1 / radPower);
	};
	radPercFunc = [=](double max, double min, double value) {
		return (pow(value, radPower) - pow(min, radPower)) / (pow(max, radPower) - pow(min, radPower));
	};

	latInterpFunc = [=](double max, double min, double d) {
		double maxD = asin(max);
		double minD = asin(min);

		return latScale * asin(d * sin(maxD / latScale) + (1.0 - d) * sin(minD / latScale));
	};
	latPercFunc = [=](double max, double min, double value) {
		double maxD = asin(max);
		double minD = asin(min);
		double valueD = asin(value);
		
		return (sin(valueD / latScale) - sin(minD / latScale)) / (sin(maxD / latScale) - sin(minD / latScale));
	};
}


Index EfficientOperations::pointToIndex(const Point& p, int k) const {

	// Percentage distance in each coordinate
	double radPerc = p.rad / GRID_RAD;
	double latPerc = p.lat / M_PI_2;
	double lngPerc = p.lng / M_PI_2;

	int shell = floor(logB(radPerc, 0.5));
	int zone = floor(logB(1.0 - latPerc, 0.5));

	// Modifiers to account for semiregular degenerate refinement
	int latMod = std::min(shell, k);
	int lngMod = std::min(latMod + zone, k);

	// Index in each coordinate
	// 1ll << k == 2^k
	DimIndex radI = (DimIndex)floor((1ll << k) * (1.0 - radPerc));
	DimIndex latI = (DimIndex)floor((1ll << (k - latMod)) * latPerc);
	DimIndex lngI = (DimIndex)floor((1ll << (k - lngMod)) * lngPerc);

	// Interleave Morton Code and set 1 bit at beginning to mark start of index
	return libmorton::morton3D_64_encode(lngI, latI, radI) + (1ll << (k * 3));
}


Index ModifiedEfficient::pointToIndex(const Point& p, int k) const {

	// Percentage distance in each coordinate
	double radPerc = p.rad / GRID_RAD;
	double latPerc = sin(p.lat);
	double lngPerc = p.lng / M_PI_2;

	int shell = floor(logB(radPerc, 0.5));
	int zone = floor(logB(1.0 - latPerc, 0.25));

	// Modifiers to account for semiregular degenerate refinement
	int latMod = std::min(shell, k);
	int lngMod = std::min(latMod + zone, k);

	// Uppers and lowers for mapping technique
	double radUpp = pow(0.5, shell);
	double radLow = pow(0.5, shell + 1);

	double latLowG = 1.0 - (pow(0.5, zone));
	double latUppG = 1.0 - (pow(0.5, zone + 1));

	double latLowP = 1.0 - (pow(0.25, zone));
	double latUppP = 1.0 - (pow(0.25, zone + 1));


	// Map from physical to grid domain
	double latD = latPercFunc(latUppP, latLowP, latPerc);
	latPerc = latD * latUppG + (1.0 - latD) * latLowG;

	double radD = radPercFunc(radUpp, radLow, radPerc);
	radPerc = radD * radUpp + (1.0 - radD) * radLow;


	// Index in each coordinate
	// 1ll << k == 2^k
	DimIndex radI = (DimIndex)floor((1ll << k) * (1.0 - radPerc));
	DimIndex latI = (DimIndex)floor((1ll << (k - latMod)) * latPerc);
	DimIndex lngI = (DimIndex)floor((1ll << (k - lngMod)) * lngPerc);

	// Interleave Morton Code and set 1 bit at beginning to mark start of index
	return libmorton::morton3D_64_encode(lngI, latI, radI) + (1ll << (k * 3));
}


Range EfficientOperations::indexToRange(Index index) const {

	Range r;

	// Find width of index
	Index copy = index;
	int width = 0;
	while (copy >>= 1) {
		width++;
	}

	// Refinement level is one third width
	int k = width / 3;

	// Remove leading bit
	index ^= 1ll << width;

	// Unweave Morton Code 
	DimIndex radI, latI, lngI;
	libmorton::morton3D_64_decode(index, lngI, latI, radI);

	// Calculate radius
	r.radMax = 1.0 - (radI / (double)(1ll << k));
	r.radMin = 1.0 - ((radI + 1.0) / (double)(1ll << k));

	// Calculate latitude
	int shell = floor(logB(r.radMax, 0.5));
	int latMod = std::min(shell, k); // Modifier to account for semiregular degenerate refinement

	r.latMin = latI / (double)(1ll << (k - latMod));
	r.latMax = (latI + 1.0) / (double)(1ll << (k - latMod));

	// Calculate longitude
	int zone = floor(logB(1.0 - r.latMin, 0.5));
	int lngMod = std::min(latMod + zone, k); // Modifier to account for semiregular degenerate refinement

	r.lngMin = lngI / (double)(1ll << (k - lngMod));
	r.lngMax = (lngI + 1.0) / (double)(1ll << (k - lngMod));

	// Put bounds into coordinate domain as opposed to parameter
	r.radMin *= GRID_RAD;
	r.radMax *= GRID_RAD;
	r.latMin *= M_PI_2;
	r.latMax *= M_PI_2;
	r.lngMin *= M_PI_2;
	r.lngMax *= M_PI_2;

	return r;
}


Range ModifiedEfficient::indexToRange(Index index) const {

	Range r;

	// Find width of index
	Index copy = index;
	int width = 0;
	while (copy >>= 1) {
		width++;
	}

	// Refinement level is one third width
	int k = width / 3;

	// Remove leading bit
	index ^= 1ll << width;

	// Unweave Morton Code 
	DimIndex radI, latI, lngI;
	libmorton::morton3D_64_decode(index, lngI, latI, radI);

	// Calculate radius
	r.radMax = 1.0 - (radI / (double)(1ll << k));
	r.radMin = 1.0 - ((radI + 1.0) / (double)(1ll << k));


	int shell = floor(logB(r.radMax, 0.5));

	// Uppers and lowers for mapping technique
	double radUpp = pow(0.5, shell);
	double radLow = pow(0.5, shell + 1);

	double radMaxD = (r.radMax - radLow) / (radUpp - radLow);
	double radMinD = (r.radMin - radLow) / (radUpp - radLow);

	// Modifier to account for semiregular degenerate refinement
	int latD = std::min((int)floor(shell), k);
	r.latMin = latI / (double)(1ll << (k - latD));
	r.latMax = (latI + 1.0) / (double)(1ll << (k - latD));

	// Uppers and lowers for mapping technique
	int zone = floor(logB(1.0 - r.latMin, 0.5));
	double latLowG = 1.0 - (pow(0.5, zone));
	double latUppG = 1.0 - (pow(0.5, zone + 1));

	double latLowP = 1.0 - (pow(0.25, zone));
	double latUppP = 1.0 - (pow(0.25, zone + 1));

	double latMaxD = (r.latMax - latLowG) / (latUppG - latLowG);
	double latMinD = (r.latMin - latLowG) / (latUppG - latLowG);

	// Modifier to account for degenerate subdivision
	int lngD = std::min(latD + (int)floor(zone), k);
	r.lngMin = lngI / (double)(1ll << (k - lngD));
	r.lngMax = (lngI + 1.0) / (double)(1ll << (k - lngD));

	// Put bounds into coordinate domain as opposed to parameter
	r.radMin = GRID_RAD * radInterpFunc(radUpp, radLow, radMinD);
	r.radMax = GRID_RAD * radInterpFunc(radUpp, radLow, radMaxD);
	if (r.radMin < 0.0 || isnan(r.radMin)) r.radMin = 0.0; // precision issues when min radius is 0

	r.latMin = latInterpFunc(latUppP, latLowP, latMinD);
	r.latMax = latInterpFunc(latUppP, latLowP, latMaxD);
	if (isnan(r.latMax)) r.latMax = M_PI_2;

	r.lngMin *= M_PI_2;
	r.lngMax *= M_PI_2;

	return r;
}
