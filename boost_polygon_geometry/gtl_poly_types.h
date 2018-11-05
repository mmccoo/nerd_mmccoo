#ifndef GTL_POLY_TYPES_HH
#define GTL_POLY_TYPES_HH

#include <boost/polygon/polygon.hpp>
namespace gtl = boost::polygon;


// doing it this way, will prevent holes. The holes are still represented, but
// by slicing into the poly
//typedef gtl::polygon_data<int> Polygon;
typedef gtl::polygon_with_holes_data<int>              Polygon_Holes;
typedef gtl::polygon_data<int>                         Polygon_NoHoles;
typedef gtl::rectangle_data<int>                       Rect;
typedef gtl::polygon_traits<Polygon_Holes>::point_type Point;
typedef std::vector<Polygon_Holes>                     PolygonSet;
typedef std::vector<Polygon_NoHoles>                   PolygonSet_NoHoles;

std::ostream& operator << (std::ostream& o, const Point& pt);
std::ostream& operator << (std::ostream& o, const Polygon_NoHoles& poly);
std::ostream& operator << (std::ostream& o, const Polygon_Holes& poly);
std::ostream& operator << (std::ostream& o, const PolygonSet& polys);



#endif
