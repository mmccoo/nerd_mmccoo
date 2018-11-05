
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

    const int points_per_circle = 36;
    boost::geometry::strategy::buffer::join_round   join_strategy_round(points_per_circle);
    boost::geometry::strategy::buffer::end_round    end_strategy_round(points_per_circle);

    boost::geometry::strategy::buffer::join_miter   join_strategy_miter;
    boost::geometry::strategy::buffer::end_flat     end_strategy_flat;

    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;


    multi_polygon ps1;
    read_polys("tests/buffer_90_0_-3950.out", ps1);

    multi_polygon result1;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_m3950(-3950);
    boost::geometry::buffer(ps1, result1,
                            distance_strategy_m3950, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    write_polys("deflated5a.out", result1);

    multi_polygon ps2;
    for(auto poly : ps1) {
        polygon simplified;
        boost::geometry::simplify(poly, simplified, 10);
        ps2.push_back(simplified);
    }

    multi_polygon result2;

    boost::geometry::buffer(ps2, result2,
                            distance_strategy_m3950, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    write_polys("deflated5b.out", result2);
}
