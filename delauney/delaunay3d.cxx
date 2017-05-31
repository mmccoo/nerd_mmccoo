
#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>
#include <cassert>
#include <list>
#include <vector>


// https://doc.cgal.org/latest/Triangulation_3/
// https://doc.cgal.org/latest/Kernel_23/index.html#Chapter_2D_and_3D_Geometry_Kernel
// http://doc.cgal.org/latest/Triangulation_3/classTriangulationTraits__3.html
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

// https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html
// https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Delaunay__triangulation__3.html
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_data_structure_3.h>

// The doc for Point is a little tricky to find. Delaunay and Triangulation pages
// don't link here:
// https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Point__3.html
// http://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Cartesian.html
// http://www.cgal.org/FAQ.html

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_3<K>                   Triangulation;
typedef Triangulation::Point                                Point;


// these two functions are for my readvtk function.
Triangulation::Vertex_handle
add_vertex(Triangulation &T, std::string& vname, double x, double y, double z)
{
  // vertex name doesn't have any meaning in this context.
  return T.insert(Point(x,y,z));
}

void add_edge(Triangulation &T,
               Triangulation::Vertex_handle s,
               Triangulation::Vertex_handle t)
{
  // no edges to add. CGAL does that.
}

#include <readvtk.hxx>


int main(int argc,char* argv[])
{

    namespace po = boost::program_options;
    po::options_description desc("Usage");

    std::string filename;
    desc.add_options()
        ("help", "produce help message")
        ("filename", po::value<std::string>(&filename)->default_value(""),
         "filename containing input points");
    
    po::variables_map opts;
    po::store(po::parse_command_line(argc, argv, desc), opts);

    try {
        po::notify(opts);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (filename == "") {
      std::cerr << "please provide a vtk file with the --filename <file> option" << std::endl;
      exit(-1);
    }

    std::ifstream input(filename.c_str());
    Triangulation T;
    
    /* if you just have a file with a list of points, you can do this:
       std::list<Point> L;
       Point p ;
       Triangulation T;
       while (input >> p) {
         T.insert(p);
         L.push_front(p);
       }
       // if you already have a list of points:
       // Triangulation T(L.begin(), L.end());
       
    */
    
    readvtk<Triangulation,Triangulation::Vertex_handle>(input, T);


    // at this point, the trianulation is already done. Just have to extract
    // the computed lines.
    Triangulation::size_type n = T.number_of_vertices();

    assert( T.is_valid() ); // checking validity of T



    std::cout << "# vtk DataFile Version 1.0\n";
    std::cout << "3D triangulation data\n";
    std::cout << "ASCII\n";
    std::cout << std::endl;
    std::cout << "DATASET POLYDATA\n";

    // according to http://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html
    // Note that the triangulation data structure has one more
    // vertex than an associated geometric triangulation, if there
    // is one, since the infinite vertex is a standard vertex and
    // is thus also counted.
    // I am omitting this one by using the finite_vertices/edges iterators below
    int numvertices = T.number_of_vertices();
    std::cout << "POINTS " << numvertices << " float\n";

    // the APIs I'm using below are documented here:
    // http://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html

    // as I iterate over the vertices, I assign ids to each vertex. Seems like there'd
    // be an id method, but I haven't found it.
    std::map<Point, int> vertex_ids;
    Triangulation::Finite_vertices_iterator viter;
    for (viter =  T.finite_vertices_begin();
         viter != T.finite_vertices_end();
         viter++) {
      Triangulation::Triangulation_data_structure::Vertex v = *viter;
      // in the line below, looking up the point (the [] part) will add a new
      // element to the map. That's why I subtract 1.
      vertex_ids[v.point()] = vertex_ids.size() - 1;
      std::cout << v.point() << std::endl;
    }

    std::cout << "LINES " << T.number_of_finite_edges() << " "
              << 3*T.number_of_finite_edges() << std::endl;

    Triangulation::Finite_edges_iterator iter;
    for(iter =  T.finite_edges_begin();
        iter != T.finite_edges_end();
        iter++) {
      // edges are not represented as edges in CGAL triangulation graphs.
      // Instead, they are stored in faces/cells.

      Triangulation::Triangulation_data_structure::Edge e = *iter;
      Triangulation::Triangulation_data_structure::Cell_handle c = e.first;
      int i = e.second;
      int j = e.third;
            
      Triangulation::Triangulation_data_structure::Vertex_handle a = c->vertex(i);
      Point pa = a->point();
      Point pb = c->vertex(j)->point();
      int ida, idb;
      if (vertex_ids.find(pa) == vertex_ids.end()) {
        std::cout << "didn't find " << pa << std::endl;
        ida = 0;
      } else {
        ida = vertex_ids[pa];
      }
      if (vertex_ids.find(pb) == vertex_ids.end()) {
        std::cout << "didn't find " << pb << std::endl;
        idb = 0;
      } else {
        idb = vertex_ids[pb];
      }
      std::cout << "2 " << ida << " " << idb << std::endl;
    }
    
    return 0;
}



