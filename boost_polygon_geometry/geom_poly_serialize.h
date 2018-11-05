


#ifndef GEOM_POLY_SERIALIZE_HH
#define GEOM_POLY_SERIALIZE_HH

#include <geom_poly_types.h>

void write_polys(std::string filename, const multi_polygon &polys);
void read_polys(std::string filename,  multi_polygon &polys);
#endif
