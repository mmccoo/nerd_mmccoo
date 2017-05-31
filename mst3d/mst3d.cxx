

#include <iostream>
#include <fstream>
#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/program_options.hpp>

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>


struct VertexData {
  std::string name;
  double x,y,z;
};

struct EdgeData {
  double distance;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              VertexData,
                              boost::property<boost::edge_weight_t, double, EdgeData>
                              > MyGraphType;



// these are needed by readvtk
typedef typename boost::graph_traits<MyGraphType>::vertex_descriptor vertex_descriptor;
typedef typename boost::graph_traits<MyGraphType>::edge_descriptor   edge_descriptor;
typename boost::graph_traits<MyGraphType>::vertex_descriptor
add_vertex(MyGraphType &G, std::string& vname, double x, double y, double z)
{
  typedef typename boost::graph_traits<MyGraphType>::vertex_descriptor vertex_descriptor;
  vertex_descriptor v = add_vertex(G);
  G[v].x = x;
  G[v].y = y;
  G[v].z = z;
  return v;
}

inline
double distance(MyGraphType &G,
                typename boost::graph_traits<MyGraphType>::vertex_descriptor v1,
                typename boost::graph_traits<MyGraphType>::vertex_descriptor v2)
{
  return sqrt((G[v1].x - G[v2].x)*(G[v1].x - G[v2].x) +
              (G[v1].y - G[v2].y)*(G[v1].y - G[v2].y) +
              (G[v1].z - G[v2].z)*(G[v1].z - G[v2].z));
}


typename boost::graph_traits<MyGraphType>::edge_descriptor
add_edge(MyGraphType &G,
         typename boost::graph_traits<MyGraphType>::vertex_descriptor v1,
         typename boost::graph_traits<MyGraphType>::vertex_descriptor v2)
{
  typedef typename boost::graph_traits<MyGraphType>::edge_descriptor edge_descriptor;
  edge_descriptor e = add_edge(v1, v2, G).first;
  boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);
  weightmap[e] = distance(G, v1, v2);
  return e;
}



#include <readvtk.hxx>


enum MSTAlgorithm { PRIM, KRUSKAL };
std::istream& operator>>(std::istream& in, MSTAlgorithm &format)
{
    std::string token;
    in >> token;
    if (token == "prim")
        format = PRIM;
    else if (token == "kruskal")
        format = KRUSKAL;
    else 
        in.setstate(std::ios_base::failbit);
    return in;
}


int
main(int argc,char* argv[])
{

  namespace po = boost::program_options;
  po::options_description desc("Usage");

  std::string filename;
  desc.add_options()
    ("help", "produce help message")
    ("filename", po::value<std::string>(&filename)->default_value(""),
     "filename containing input points");

  MSTAlgorithm mst_algorithm;
    desc.add_options()
        ("algorithm", po::value<MSTAlgorithm>(&mst_algorithm)->default_value(KRUSKAL),
         "which output format");    

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


  
  MyGraphType G;

  std::ifstream input(filename);
  readvtk<MyGraphType,vertex_descriptor>(input, G);

  
  std::cerr << "running prim" << std::endl;
  std::vector<vertex_descriptor> mst_prim(num_vertices(G));

  std::cout << "# vtk DataFile Version 1.0\n";
  std::cout << "3D triangulation data\n";
  std::cout << "ASCII\n";
  std::cout << std::endl;
  std::cout << "DATASET POLYDATA\n";

  std::cout << "POINTS " << num_vertices(G) << " float\n";
  for(int i=0; i<num_vertices(G); i++) {
    std::cout << G[i].x  << " " << G[i].y << " " << G[i].z << std::endl;
  }

  std::cout << "LINES " << (num_vertices(G)-1) << " " << (num_vertices(G)-1)*3 << std::endl;
  if (mst_algorithm == PRIM) {
  
    // the not particularly helpful doc for iterator_property_map:
    // http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/iterator_property_map.html
    // iterator_property_map<RandomAccessIterator, OffsetMap, T, R>
    //
    typedef boost::property_map<MyGraphType, boost::vertex_index_t>::type IdMap;
    boost::iterator_property_map<std::vector<vertex_descriptor>::iterator,
                                 IdMap,
                                 vertex_descriptor,
                                 vertex_descriptor&>
      predmap(mst_prim.begin(), get(boost::vertex_index, G));
                                       
    boost::prim_minimum_spanning_tree(G, predmap);

    for(int i=0; i<num_vertices(G); i++) {
      if (i == mst_prim[i]) {
        std::cerr << "skipping " << i << std::endl;
        continue;
      }
      std::cout << "2 " << i << " " << mst_prim[i] << std::endl;
    }
  }

  if (mst_algorithm == KRUSKAL) {
    std::cerr << "running kruskal" << std::endl;
    std::list<boost::graph_traits<MyGraphType>::edge_descriptor> mst_kruskal;
    boost::kruskal_minimum_spanning_tree(G, std::back_inserter(mst_kruskal));

    for(auto iter=mst_kruskal.begin();
        iter != mst_kruskal.end();
        iter++) {
      std::cout << "2 " << source(*iter, G) << " " << target(*iter, G) <<std::endl;
    }
  }
  
}
