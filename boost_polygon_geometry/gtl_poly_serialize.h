


#ifndef GTL_POLY_SERIALIZE_HH
#define GTL_POLY_SERIALIZE_HH

#include <gtl_poly_types.h>

void write_polys(std::string filename, const Polygon_Holes &poly);
void write_polys(std::string filename, const PolygonSet &polys);
void read_polys(std::string filename,  PolygonSet &polys);
#endif
