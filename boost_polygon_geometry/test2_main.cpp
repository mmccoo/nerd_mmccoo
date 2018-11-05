
#include <iostream>
#include <geom_poly_types.h>
#include <geom_poly_serialize.h>

// things that this file demonstrates
// static initialization of polygon
// multi_polygon is basically a std::vector: /usr/include/boost/geometry/geometries/multi_polygon.hpp
// union assumes args 1 and 2 are already merged.

// point ordering is clockwise or counterclockwise in polygon typedef
// correct can fix
// two polys unioned is wrong without correction.

int main(int argc, char *argv[]) {

    bool docorrection = true;
    if ((argc>1) && (std::string(argv[1]) == "nocorrect")) {
        docorrection = false;
        std::cout << "not doing correction\n";
    }

    multi_polygon ps1;

    polygon polygon1 = {{{0, 0},
                         {100, 0},
                         {100, 100},
                         {0, 100}}
    };
    if (docorrection) { boost::geometry::correct(polygon1); }
    std::cout << "polygon1 is valid: " << boost::geometry::is_valid(polygon1) << "\n";


    ps1.push_back(polygon1);

    ring r{{50, 50},
           {150, 50},
           {150, 150},
           {50, 150} };

    polygon polygon2;
    polygon2.outer() = r;
    if (docorrection) { boost::geometry::correct(polygon2); }
    std::cout << "polygon2 is valid: " << boost::geometry::is_valid(polygon2) << "\n";

    ps1.push_back(polygon2);

    write_polys("test2a.out", ps1);
    std::cout << "printing the pushed polygons\n";
    std::cout << ps1 << "\n";


    multi_polygon ps2;

    multi_polygon empty_ps;
    boost::geometry::union_(ps1, empty_ps, ps2);
    write_polys("test2b.out", ps2);
    std::cout << "printing the pushed polygons unioned with an empty list\n";
    std::cout << ps2 << "\n";

    // union appends to the last argument.s
    ps2.clear();
    boost::geometry::union_(polygon1, polygon2, ps2);

    write_polys("test2c.out", ps2);
    std::cout << "this time it's the two polygons unioned together\n";
    std::cout << ps2 << "\n";


    std::vector<point> pts3{{10, 10},
                            {20, 10},
                            {20, 20},
                            {10, 20} };

    polygon polygon3;
    polygon3.outer().assign(pts3.begin(), pts3.end());


    std::list<polygon> ps3;
    boost::geometry::difference(ps2, polygon3, ps3);

    // my write_polys functions just take a multi_polygon.
    multi_polygon ps3_mp;
    ps3_mp.assign(ps3.begin(), ps3.end());
    write_polys("test2d.out", ps3_mp);

    // I could also write a version for list/vector of polygon
    std::cout << "and now the difference\n";
    std::cout << ps3_mp << "\n";

}
