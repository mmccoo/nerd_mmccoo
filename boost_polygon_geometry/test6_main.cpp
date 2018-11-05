
#include <iostream>
#include <geom_poly_types.h>
#include <geom_poly_serialize.h>
#include <gtl_poly_serialize.h>
#include <poly_utils.h>

// this file demonstrates converting a sequence of points (a line string) into a thicker line.
int main(int argc, char *argv[]) {

    const int points_per_circle = 36;
    boost::geometry::strategy::buffer::join_round   join_strategy_round(points_per_circle);
    boost::geometry::strategy::buffer::end_round    end_strategy_round(points_per_circle);

    boost::geometry::strategy::buffer::join_miter   join_strategy_miter;
    boost::geometry::strategy::buffer::end_flat     end_strategy_flat;

    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;


    std::vector<point> path{{0,0}, {100,0}, {200,100}, {200,200}};

    boost::geometry::model::linestring<point> ls;
    for(auto pt : path) {
        boost::geometry::append(ls, pt);
    }

    multi_polygon result1;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_10(10);
    boost::geometry::buffer(ls, result1,
                            distance_strategy_10, side_strategy,
                            join_strategy_round, end_strategy_round, circle_strategy);

    multi_polygon result2;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy_20(20);
    boost::geometry::buffer(ls, result2,
                            distance_strategy_20, side_strategy,
                            join_strategy_miter, end_strategy_flat, circle_strategy);

    write_polys("inflate6a.out", result1);
    write_polys("inflate6b.out", result2);

}
