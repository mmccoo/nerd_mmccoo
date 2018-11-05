

#ifndef POLY_UTILS_HH
#define POLY_UTILS_HH

#include <gtl_poly_types.h>
#include <geom_poly_types.h>

void gtl_poly2geom_poly(const Polygon_Holes& gtlpoly, polygon& boost_poly);
void gtl_poly2geom_poly(const Polygon_NoHoles& gtlpoly, polygon& boost_poly);


Polygon_Holes gen_capped_line(std::vector<Point> &cur_path, int width);

PolygonSet buffer_poly(const Polygon_Holes &gtlpoly, int distance, bool flat);
PolygonSet buffer_poly(const Polygon_Holes &gtlpoly, std::vector<int> distances, bool flat);

#endif
