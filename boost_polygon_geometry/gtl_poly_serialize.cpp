
#include <gtl_poly_serialize.h>

#include <fstream>
#include <iostream>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/tracking.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


// it seems that the serialize stuff doesn't like having both types of polygon declared.
// because of this, it's especially important to keep the boost::geometry and boost::polygon stuff separate.
namespace boost{
    namespace serialization{


        // serialization for Point
        template<class Archive>
        void save(Archive &ar, const Point &p, unsigned int version)
        {
            ar & gtl::x(p);
            ar & gtl::y(p);
        }

        template<class Archive>
        void load(Archive & ar, Point &p, unsigned int version)
        {
            int x,y;

            ar & x;
            ar & y;

            p = Point(x,y);
        }

        // Serialization for Polygon_NoHoles
        template<class Archive>
        void save(Archive & ar, const Polygon_NoHoles &poly, unsigned int version)
        {
            ar & poly.size();
            for(const Point &pt : poly) {
                ar & pt;
            }
        }

        template<class Archive>
        void load(Archive & ar, Polygon_NoHoles &poly, unsigned int version)
        {
            int numpts;
            ar & numpts;

            std::vector<Point> pts;
            for(int i=0; i<numpts; i++) {
                Point pt;
                ar & pt;
                pts.push_back(pt);
            }
            gtl::set_points(poly, pts.begin(), pts.end());
        }

        // Serialization for Polygon_Holes
        template<class Archive>
        void save(Archive & ar, const Polygon_Holes &poly, unsigned int version)
        {
            ar & poly.size();
            for(Point pt : poly) {
                ar & pt;
            }

            ar & poly.size_holes();
            for(auto i=poly.begin_holes(); i!=poly.end_holes(); ++i) {
                ar & (*i);
            }
        }

        template<class Archive>
        void load(Archive & ar, Polygon_Holes &poly, unsigned int version)
        {
            int numpts;
            ar & numpts;

            std::vector<Point> pts;
            for(int i=0; i<numpts; i++) {
                Point pt;
                ar & pt;
                pts.push_back(pt);
            }
            gtl::set_points(poly, pts.begin(), pts.end());

            int numholes;
            ar & numholes;

            std::vector<Polygon_NoHoles> holes;
            for(int i=0; i<numholes; i++) {
                Polygon_NoHoles hole;
                ar & hole;
                holes.push_back(hole);
            }
            gtl::set_holes(poly, holes.begin(), holes.end());
        }

// not actually necessary since polyset is a vector.
#if 0
        template<class Archive>
        void save(Archive & ar, const PolygonSet &ps, unsigned int version)
        {
            ar & ps.size();
            for (const Polygon_Holes &poly : ps) {
                ar & poly;
            }
        }

        template<class Archive>
        void load(Archive & ar, PolygonSet &ps, unsigned int version)
        {
            int numpoly;
            ar & numpoly;

            ps.clear();
            for(int i=0; i<numpoly; i++) {
                Polygon_Holes poly;
                ar & poly;
                ps.push_back(poly);
            }
        }
#endif
        /*
        template<class Archive>
        void save(Archive & ar, const Point &p, unsigned int version)
        {
            // ar & ...
        }

        template<class Archive>
        void load(Archive & ar, Point &t, unsigned int version)
        {
            // ar & ...
        }
        */

    }
}



BOOST_SERIALIZATION_SPLIT_FREE(Point)
BOOST_SERIALIZATION_SPLIT_FREE(Polygon_NoHoles)
BOOST_SERIALIZATION_SPLIT_FREE(Polygon_Holes)

// polyset is a vector, this isn't needed.
// BOOST_SERIALIZATION_SPLIT_FREE(PolygonSet)


void write_polys(std::string filename, const Polygon_Holes &poly)
{
    const PolygonSet polys{poly};

    write_polys(filename, polys);
}



void write_polys(std::string filename, const PolygonSet &polys)
{
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);

    oa << polys;
}

void read_polys(std::string filename, PolygonSet &polys)
{
    polys.clear();

    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> polys;
}
