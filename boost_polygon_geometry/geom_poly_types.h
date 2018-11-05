#ifndef BOOST_POLY_TYPES_HH
#define BOOST_POLY_TYPES_HH


#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>


// coordinate_type is used somewhere I think. Get a compile error if I use that name.
typedef boost::geometry::model::d2::point_xy<double>     point;

// false indicates counter clockwise
typedef boost::geometry::model::polygon<point,false>     polygon;
typedef boost::geometry::ring_type<polygon>::type        ring;
typedef boost::geometry::model::multi_polygon<polygon>   multi_polygon;
typedef boost::geometry::model::box<point>               box;


std::ostream& operator << (std::ostream& o, const point& pt);
std::ostream& operator << (std::ostream& o, const ring& ring);
std::ostream& operator << (std::ostream& o, const polygon& poly);
std::ostream& operator << (std::ostream& o, const multi_polygon& polys);


#endif
