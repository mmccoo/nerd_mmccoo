
#include <iostream>
#include <geom_poly_types.h>
#include <geom_poly_serialize.h>
#include <gtl_poly_serialize.h>
#include <poly_utils.h>

// show poly_types
// polygonset is a vector.
// show difference between += and push_back

// affect of noholes shown in show_shapes

// poly_utils helps convert gtl2geom and back.

// simplify can avoid some bugs.

int main(int argc, char *argv[]) {

    bool do_correction = true;
    if ((argc==2) && (std::string(argv[1]) == "nocorrect")) {
        do_correction = false;
    }

    polygon polygon1 = {{{0, 0},
                         {100, 0},
                         {100, 100},
                         {0, 100}}
    };
    if (do_correction) { boost::geometry::correct(polygon1); }

    const int points_per_circle = 36;
    boost::geometry::strategy::buffer::join_round   join_strategy_round(points_per_circle);
    boost::geometry::strategy::buffer::end_round    end_strategy_round(points_per_circle);

    boost::geometry::strategy::buffer::join_miter   join_strategy_miter;
    boost::geometry::strategy::buffer::end_flat     end_strategy_flat;

    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;

    multi_polygon result1;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_10(10);
    boost::geometry::buffer(polygon1, result1,
                            distance_strategy_10, side_strategy,
                            join_strategy_round, end_strategy_round, circle_strategy);

    multi_polygon result2;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_20(20);
    boost::geometry::buffer(polygon1, result2,
                            distance_strategy_20, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    result1.insert(result1.end(), result2.begin(), result2.end());
    write_polys("inflate4a.out", result1);


    // my reference files are mostly gtl based.
    PolygonSet ps1;
    read_polys("tests/merged.out", ps1);

    multi_polygon ps1_mp;
    for (auto gtlpoly : ps1) {
        polygon geopoly;
        gtl_poly2geom_poly(gtlpoly, geopoly);
        ps1_mp.push_back(geopoly);
    }

    multi_polygon result3;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_m1000(-3900);
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_1000(3900);
    boost::geometry::buffer(ps1_mp, result3,
                            distance_strategy_m1000, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    multi_polygon result4;
    boost::geometry::buffer(result3, result4,
                            distance_strategy_1000, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    write_polys("inflate4b.out", result4);
    write_polys("inflate4b_in.out", ps1_mp);

}
