

#include <gtl_poly_types.h>

std::ostream& operator << (std::ostream& o, const Point& pt)
{
    o << "(" << gtl::x(pt) << ", " << gtl::y(pt) << ")";
    return o;
}

std::ostream& operator << (std::ostream& o, const Polygon_NoHoles& poly)
{
    o << "poly_noholes (";

    for(Point pt : poly) {
        o << pt << ", ";
    }
    o << ")";
    return o;
}

std::ostream& operator << (std::ostream& o, const Polygon_Holes& poly)
{
    o << "poly = ((";

    for(Point pt : poly) {
        o << pt << ", ";
    }
    o << ") (";

    for(auto iter = poly.begin_holes(); iter != poly.end_holes(); ++iter) {
        const gtl::polygon_data<int> h = *iter;
        o << (*iter) << "\n";
    }
    o << "))";
    return o;
}


std::ostream& operator << (std::ostream& o, const PolygonSet& polys)
{
    for (const Polygon_Holes& poly : polys) {
        o << poly << "\n";
    }
    return o;
}
