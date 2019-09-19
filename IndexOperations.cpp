#include "IndexOperations.h"

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

enum SdogCellType {
	SG,
	LG,
	NG
};


Index SimpleOperations::pointToIndex(const Point& p, int k) const {

	Range r;
	r.radMin = 0.0;
	r.radMax = GRID_RAD;
	r.latMin = 0.0;
	r.latMax = M_PI_2;
	r.lngMin = 0.0;
	r.lngMax = M_PI_2;

	Index index;
	index[0] = 1;

	// Loop for desired number of levels and determine which child point is in for each itteration
	SdogCellType curType = SdogCellType::SG;
	for (unsigned int i = 0; i < k; i++) {

		unsigned int childCode = 0;
		double radMid = 0.5 * r.radMin + 0.5 * r.radMax;
		double latMid = 0.5 * r.latMin + 0.5 * r.latMax;
		double lngMid = 0.5 * r.lngMin + 0.5 * r.lngMax;

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

	Range r;
	r.radMin = 0.0;
	r.radMax = GRID_RAD;
	r.latMin = 0.0;
	r.latMax = M_PI_2;
	r.lngMin = 0.0;
	r.lngMax = M_PI_2;

	//// Loop for each char in code and determine properties based on code
	//SdogCellType type = SdogCellType::SG;
	//for (unsigned int i = 1; i < code.length(); i++) {

	//	double midLat = 0.5 * minLat + 0.5 * maxLat;
	//	double midLong = 0.5 * minLong + 0.5 * maxLong;
	//	double midRad = 0.5 * minRad + 0.5 * maxRad;

	//	if (type == SdogCellType::NG) {

	//		if (code[i] == '0') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			maxLong = midLong;
	//		}
	//		else if (code[i] == '1') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			minLong = midLong;
	//		}
	//		else if (code[i] == '2') {
	//			minRad = midRad;
	//			minLat = midLat;
	//			maxLong = midLong;
	//		}
	//		else if (code[i] == '3') {
	//			minRad = midRad;
	//			minLat = midLat;
	//			minLong = midLong;
	//		}
	//		else if (code[i] == '4') {
	//			maxRad = midRad;
	//			maxLat = midLat;
	//			maxLong = midLong;
	//		}
	//		else if (code[i] == '5') {
	//			maxRad = midRad;
	//			maxLat = midLat;
	//			minLong = midLong;
	//		}
	//		else if (code[i] == '6') {
	//			maxRad = midRad;
	//			minLat = midLat;
	//			maxLong = midLong;
	//		}
	//		else if (code[i] == '7') {
	//			maxRad = midRad;
	//			minLat = midLat;
	//			minLong = midLong;
	//		}
	//		else {
	//			type = SdogCellType::INVALID;
	//			break;
	//		}
	//		// type doesn't change
	//	}
	//	else if (type == SdogCellType::LG) {

	//		if (code[i] == '0') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			maxLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '1') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			minLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '2') {
	//			minRad = midRad;
	//			minLat = midLat;
	//			// type doesn't change
	//		}
	//		else if (code[i] == '3') {
	//			maxRad = midRad;
	//			maxLat = midLat;
	//			maxLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '4') {
	//			maxRad = midRad;
	//			maxLat = midLat;
	//			minLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '5') {
	//			maxRad = midRad;
	//			minLat = midLat;
	//			// type doesn't change
	//		}
	//		else {
	//			type = SdogCellType::INVALID;
	//			break;
	//		}
	//	}
	//	else {// type == CellType::SG

	//		if (code[i] == '0') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			maxLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '1') {
	//			minRad = midRad;
	//			maxLat = midLat;
	//			minLong = midLong;
	//			type = SdogCellType::NG;
	//		}
	//		else if (code[i] == '2') {
	//			minRad = midRad;
	//			minLat = midLat;
	//			type = SdogCellType::LG;
	//		}
	//		else if (code[i] == '3') {
	//			maxRad = midRad;
	//			// type doesn't change
	//		}
	//		else {
	//			type = SdogCellType::INVALID;
	//			break;
	//		}
	//	}
	//}
	return r;
}



Index EfficientOperations::pointToIndex(const Point& p, int k) const {

	// Find percentage distance in each coordinate
	double radP = 1 - (p.rad / GRID_RAD);
	double latP = p.lat / M_PI_2;
	double lngP = p.lng / M_PI_2;

	// Modifiers to account for degenerate subdivision
	int latD = std::min(floor(-log2(1 - radP)), (double)k);
	int lngD = std::min(latD + floor(-log2(1 - latP)), (double)k);

	// Find index in each coordinate
	// 1ll << k == 2^k
	Index radI = floor((1ll << k) * radP);
	Index latI = floor((1ll << (k - latD)) * latP);
	Index lngI = floor((1ll << (k - lngD)) * lngP);

	// Set 1 to mark start of index
	Index index;
	index[0] = 1;

	// Interleave component bits (Morton coding)
	for (int i = k - 1; i >= 0; i--) {
		index <<= 1;
		index[0] = radI[i];

		index <<= 1;
		index[0] = latI[i];

		index <<= 1;
		index[0] = lngI[i];
	}

	return index;
}


Range EfficientOperations::indexToRange(Index index) const {

	// Find width of index
	int width;
	for (width = INDEX_WIDTH - 1; width >= 0; width--) {
		if (index[width] == 1) {
			break;
		}
	}

	// Unweave index into components
	Index radI, latI, lngI;
	for (int i = width - 1; i >= 0; i -= 3) {
		radI <<= 1;
		radI[0] = index[i];

		latI <<= 1;
		latI[0] = index[i - 1];

		lngI <<= 1;
		lngI[0] = index[i - 2];
	}

	Range r;
	int k = width / 3;

	r.radMax = radI.to_ullong() / (double)(1ll << k);
	r.radMin = (radI.to_ullong() + 1) / (double)(1ll << k);

	// Modifier to account for degenerate subdivision
	int latD = std::min(floor(-log2(1 - r.radMax)), (double)k);

	r.latMin = latI.to_ullong() / (double)(1ll << (k - latD));
	r.latMax = (latI.to_ullong() + 1) / (double)(1ll << (k - latD));

	// Modifier to account for degenerate subdivision
	int lngD = std::min(latD + floor(-log2(1 - r.latMin)), (double)k);

	r.lngMin = lngI.to_ullong() / (double)(1ll << (k - lngD));
	r.lngMax = (lngI.to_ullong() + 1) / (double)(1ll << (k - lngD));

	// Put bounds into coordinate domain as opposed to parameter
	r.radMin = (1.0 - r.radMin) * GRID_RAD;
	r.radMax = (1.0 - r.radMax) * GRID_RAD;
	r.latMin *= M_PI_2;
	r.latMax *= M_PI_2;
	r.lngMin *= M_PI_2;
	r.lngMax *= M_PI_2;

	return r;
}

//
//MappedOperations::MappedOperations() {
//
//	auto r = [&](double max, double min, double d) {
//		return GRID_RAD * cbrt(d * max*max*max + (1.0 - d) * min*min*min);
//	};
//	rad = r;
//
//	auto l = [&](double max, double min, double d) {
//		return asin(d * sin(max) + (1.0 - d) * sin(min));
//	};
//	lat = l;
//}
//
//
//MappedOperations::MappedOperations(double radPower, double latScale) {
//
//	auto r = [&](double max, double min, double d) {
//		return GRID_RAD * pow(d * pow(max, radPower) + (1.0 - d) * pow(min, radPower), 1 / radPower);
//	};
//	rad = r;
//
//	auto l = [&](double max, double min, double d) {
//		return latScale * asin(d * (1.0 / latScale) * sin(max) + (1.0 - d) * (1.0 / latScale) * sin(min));
//	};
//	lat = l;
//}
//
//
//MappedOperations::MappedOperations(InterpFunc rad, InterpFunc lat) :
//	rad(rad),
//	lat(lat)
//{}
