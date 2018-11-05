

#include <geom_poly_types.h>

std::ostream& operator << (std::ostream& o, const point& pt)
{
    o << "(" << (int) pt.get<0>() << ", " << (int) pt.get<1>() << ")";
    return o;
}

std::ostream& operator << (std::ostream& o, const ring& ring)
{
    o << "ring (";
    for(point pt : ring) {
        o << pt << ", ";
    }
    o << ")";
    return o;
}

std::ostream& operator << (std::ostream& o, const polygon& poly)
{
    o << "poly = ((";
    auto r = poly.outer();

    o << r << ") (";

    for(ring r : poly.inners()) {
        o << r << ", ";

    }
    o << "))";
    return o;
}


std::ostream& operator << (std::ostream& o, const multi_polygon& polys)
{
    for(polygon poly : polys) {
        o << poly << "\n";
    }
    return o;
}
