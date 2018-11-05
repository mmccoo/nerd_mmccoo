
#include <geom_poly_serialize.h>

#include <fstream>
#include <iostream>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/tracking.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


// it seems that the serialize stuff doesn't like having both types of polygon declared.
namespace boost{
    namespace serialization{

        template<class Archive>
        inline void serialize(Archive & ar, point &point, const unsigned int file_version)
        {
            ar & const_cast<double &>(point.x());
            ar & const_cast<double &>(point.y());
        }

        template<class Archive>
        inline void serialize(Archive & ar, ring &ring, const unsigned int file_version)
        {
            //std::cout << "Ring: Serializing ring" << std::endl;
            ar & static_cast<std::vector<point>& >(ring);
        }

        template<class Archive>
        inline void serialize(Archive & ar, polygon &t, const unsigned int file_version)
        {
            //std::cout << "Polygon: Serializing outer ring" << std::endl;
            ar & t.outer();

            //std::cout << "Polygon: Serializing inner rings" << std::endl;
            ar & t.inners();
        }


        template<class Archive>
        inline void serialize(Archive & ar, multi_polygon &polys, const unsigned int file_version)
        {
            //std::cout << "Multi: Serializing polys" << std::endl;
            ar & static_cast<std::vector<polygon>& >(polys);
        }

    }
}



void write_polys(std::string filename, const multi_polygon &polys)
{
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);

    oa << polys;
    // for (polygon poly : polys) {
    //     oa << poly;
    // }

}

void read_polys(std::string filename, multi_polygon &polys)
{
    polys.clear();

    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> polys;
}
