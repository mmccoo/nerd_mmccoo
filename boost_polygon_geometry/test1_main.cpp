

#include <gtl_poly_types.h>
#include <gtl_poly_serialize.h>

// simple quick start polygons.
// show gtl poly_types
// polygonset is a vector.
// show difference between += and push_back

// affect of noholes shown in show_shapes

int main() {

    PolygonSet ps1;
    PolygonSet ps2;

    Point pts[] = {gtl::construct<Point>(0, 0),
                   gtl::construct<Point>(100, 0),
                   gtl::construct<Point>(100, 100),
                   gtl::construct<Point>(0, 100) };

    Polygon_Holes poly;
    gtl::set_points(poly, pts, pts+4);

    // need these for the += type operations to compile
    using namespace boost::polygon::operators;
    using namespace gtl; //because of operators

    ps1 += poly;
    ps2.push_back(poly);

    std::vector<Point> pts2{{gtl::construct<Point>(50, 50),
                             gtl::construct<Point>(150, 50),
                             gtl::construct<Point>(150, 150),
                             gtl::construct<Point>(50, 150) }};

    gtl::set_points(poly, pts2.begin(), pts2.end());

    ps1 += poly;
    ps2.push_back(poly);

    write_polys("test1a.out", ps1);
    write_polys("test1b.out", ps2);

    PolygonSet ps3;
    gtl::assign(ps3, ps2);
    write_polys("test1c.out", ps3);

    std::cout << ps1 << "\n";


    Point pts3[] = {gtl::construct<Point>(10, 10),
                    gtl::construct<Point>(20, 10),
                    gtl::construct<Point>(20, 20),
                    gtl::construct<Point>(10, 20) };

    gtl::set_points(poly, pts3, pts3+4);
    ps1 -= poly;

    write_polys("test1d.out", ps1);

    std::cout << ps1 << "\n";

}
