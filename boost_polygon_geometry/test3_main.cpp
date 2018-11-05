

#include <gtl_poly_types.h>
#include <gtl_poly_serialize.h>

// show poly_types
// polygonset is a vector.
// show difference between += and push_back

// affect of noholes shown in show_shapes

// need these for the += type operations to compile
using namespace boost::polygon::operators;
using namespace gtl; //because of operators

int main(int argc, char *argv[]) {

    // if (argc<=1) {
    //     std::cout << "please give a pointer to a serialize file\n";
    //     return(1);
    // }
    Point pts[] = {gtl::construct<Point>(0, 0),
                   gtl::construct<Point>(100, 0),
                   gtl::construct<Point>(100, 100),
                   gtl::construct<Point>(0, 100) };
    Polygon_Holes poly;
    gtl::set_points(poly, pts, pts+4);

    PolygonSet ps1;
    ps1.push_back(poly);

    ps1 += 100;
    ps1.push_back(poly);

    write_polys("inflate100a.out", ps1);

    PolygonSet ps2;
    read_polys("tests/gerber3.out", ps2);

    ps2 += 5000;
    write_polys("inflate100b.out", ps2);


}
