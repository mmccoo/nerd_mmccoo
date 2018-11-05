

#include <poly_utils.h>

#include <geom_poly_types.h>
#include <gtl_poly_types.h>

#include <gtl_poly_serialize.h>
#include <geom_poly_serialize.h>



const int points_per_circle = 36;
boost::geometry::strategy::buffer::join_round   join_strategy_round(points_per_circle);
boost::geometry::strategy::buffer::end_round    end_strategy_round(points_per_circle);

boost::geometry::strategy::buffer::join_miter   join_strategy_miter;
boost::geometry::strategy::buffer::end_flat     end_strategy_flat;

boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
boost::geometry::strategy::buffer::point_square point_strategy;
boost::geometry::strategy::buffer::side_straight side_strategy;


void gtl_poly2geom_poly(const Polygon_Holes& gtlpoly, polygon& boost_poly)
{
    boost_poly.clear();

    for(Point pt : gtlpoly) {
        boost_poly.outer().push_back(point(gtl::x(pt),gtl::y(pt)));
    }

    int num_holes =0;
    for(auto iter = gtlpoly.begin_holes(); iter != gtlpoly.end_holes(); ++iter) {
        num_holes++;
        const gtl::polygon_data<int> h = *iter;

        ring r;
        for(Point pt : h) {
            r.push_back(point(gtl::x(pt),gtl::y(pt)));
        }
        boost_poly.inners().push_back(r);
    }

    // gtl goes in the wrong direction. clockwise vs c-clockwise.
    boost::geometry::correct(boost_poly);
    //std::cout << "gtl2boost " << gtlpoly.size() << " holes " << num_holes << "\n";
}

void gtl_poly2geom_poly(const Polygon_NoHoles& gtlpoly, polygon& boost_poly)
{
    boost_poly.clear();

    for(Point pt : gtlpoly) {
        boost_poly.outer().push_back(point(gtl::x(pt),gtl::y(pt)));
    }

    boost::geometry::correct(boost_poly);
    //std::cout << "gtl2boost " << gtlpoly.size() << " holes " << num_holes << "\n";
}

void geom_poly2gtl_poly(const polygon& boost_poly, Polygon_Holes& gtlpoly)
{

    //std::cout << "boost2gtl " << boost_poly.outer().size() << " holes " << boost_poly.inners().size() << "\n";
    std::vector<Point> pts;
    for (point pt : boost_poly.outer()) {
        pts.push_back(gtl::construct<Point>(pt.get<0>(),
                                            pt.get<1>()));
    }
    gtl::set_points(gtlpoly, pts.begin(), pts.end());

    std::vector<Polygon_NoHoles> holes;
    for (ring r: boost_poly.inners()) {
        std::vector<Point> pts;
        for (point pt : r) {
            pts.push_back(gtl::construct<Point>(pt.get<0>(),
                                                pt.get<1>()));
        }
        Polygon_NoHoles hole;
        gtl::set_points(hole, pts.begin(), pts.end());
        holes.push_back(hole);
    }
    gtl::set_holes(gtlpoly, holes.begin(), holes.end());

}



Polygon_Holes gen_capped_line(std::vector<Point> &path, int width)
{

    boost::geometry::model::linestring<point> ls;
    for(auto pt : path) {
        boost::geometry::append(ls, point(gtl::x(pt), gtl::y(pt)));
    }

    multi_polygon result;

    // Declare strategies
    const int buffer_distance = width/2.0;

    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(buffer_distance);

    // Create the buffer of a linestring
    // buffer doesn't work with int coordinate type.
    boost::geometry::buffer(ls, result,
                            distance_strategy, side_strategy,
                            join_strategy_round, end_strategy_round, circle_strategy);

    // we're just buffering a line. it should always be one polygon.
    assert(result.size() == 1);

    Polygon_Holes gtlpoly;
    geom_poly2gtl_poly(result.front(), gtlpoly);
    return gtlpoly;
}

PolygonSet buffer_poly(const Polygon_Holes &gtlpoly, std::vector<int> distances, bool flat)
{
    polygon boost_poly;
    gtl_poly2geom_poly(gtlpoly, boost_poly);

    multi_polygon curpolys{boost_poly};
    for (int distance : distances) {
        boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(distance);

        multi_polygon newpolys;
        for (polygon poly : curpolys) {
            multi_polygon result;

            polygon simplified;
            // with a scale factor of inches*1000000, 1 mil is 1000.
            // so simplifying by 10 eliminates 1/100 mil.
            // why do I have to do this? some buffering operations yield some polys that confuse subsequent
            // buffer operations.
            boost::geometry::simplify(poly, simplified, 10);

            // buffer doesn't work with int coordinate type.
            if (flat) {
                boost::geometry::buffer(simplified, result,
                                        distance_strategy, side_strategy,
                                        join_strategy_miter, end_strategy_flat, point_strategy);
            } else {
                boost::geometry::buffer(simplified, result,
                                        distance_strategy, side_strategy,
                                        join_strategy_round, end_strategy_round, circle_strategy);
            }
            newpolys.insert(newpolys.end(), result.begin(), result.end());
        }

        curpolys = newpolys;

    }

    PolygonSet retval;
    for (auto poly : curpolys) {
        Polygon_Holes newgtlpoly;
        geom_poly2gtl_poly(poly, newgtlpoly);
        retval.push_back(newgtlpoly);
    }

    return retval;


}


PolygonSet buffer_poly(const Polygon_Holes &gtlpoly, int distance, bool flat)
{
    std::vector<int> distances{distance};
    return buffer_poly(gtlpoly, distances, flat);
}
