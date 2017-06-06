

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/named_function_params.hpp>

// the predefined property types are listed here:
// http://www.boost.org/doc/libs/1_64_0/libs/graph/doc/using_adjacency_list.html#sec:adjacency-list-properties
// http://www.boost.org/doc/libs/1_64_0/libs/graph/doc/bundles.html

struct VertexData
{
  std::string first_name;
  int num;
};

struct EdgeData
{
  std::string edge_name;
  double dist;
};

void example0()
{
  // a simple adjaceny list with no extra properties.
  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::directedS,
                                boost::no_property,
                                boost::no_property
                                > MyGraphType;

  MyGraphType G;
  auto v1 = add_vertex(G);
  auto v2 = add_vertex(G);
  auto e = add_edge(v1, v2, G);


  auto vpair = vertices(G);
  for(auto iter=vpair.first; iter!=vpair.second; iter++) {
    std::cout << "vertex " << *iter << std::endl;
  }

  auto epair = edges(G);
  for(auto iter=epair.first; iter!=epair.second; iter++) {
    std::cout << "edge " << source(*iter, G) << " - " << target(*iter, G) << std::endl;
  }
  
}

void example0a()
{

  // a simple graph with bundled properties for vertex and a predefined property
  // for the edge.
  // for a list of predefined properties, you can look here:
  // http://www.boost.org/doc/libs/1_64_0/libs/graph/doc/using_adjacency_list.html#sec:adjacency-list-properties
  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::undirectedS,
                                VertexData,
                                boost::property<boost::edge_weight_t, double>
                                > MyGraphType;
  MyGraphType G(5);
  G[0].first_name = "Jeremy";

  // first is the edge. second is a bool telling you whether this is a new edge
  // or an existing one.
  auto e = add_edge(1,2,G).first;

  // the weight can be assigned using a weight map.
  boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap =
    get(boost::edge_weight, G);
  weightmap[e] = 10.1;

  std::cout << "vertex name " << G[0].first_name << std::endl;
  std::cout << "name getter " << get(&VertexData::first_name, G)[0] << std::endl;


  std::cout << "saved weight " << weightmap[e] << std::endl;  
  put(weightmap, e, 20);
  std::cout << "saved weight " << get(weightmap, e) << std::endl;
}


void example1()
{

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::directedS,
                                VertexData,
                                boost::property<boost::edge_weight_t, double, EdgeData>
                                > MyGraphType;
  MyGraphType G(5);
  G[0].first_name = "Jeremy";

  // first is the edge. second is a bool telling you whether this is a new edge
  // or an existing one.
  auto e = add_edge(1,2,G).first;
  G[e].edge_name = "the edge";

  boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap =
    get(boost::edge_weight, G);
  weightmap[e] = 10.1;

  std::cout << "vertex name " << G[0].first_name << std::endl;
  std::cout << "name getter " << get(&VertexData::first_name, G)[0] << std::endl;

  std::cout << "saved edge name " << G[e].edge_name << std::endl;
  std::cout << "edge name getter " << get(&EdgeData::edge_name, G)[e] << std::endl;
  // add example using get to get the property value
  std::cout << "saved weight " << weightmap[e] << std::endl;



}


// /usr/include/boost/graph/named_function_params.hpp
void example2()
{

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::undirectedS,
                                boost::property<boost::vertex_distance_t, int,
                                                boost::property<boost::vertex_distance2_t, int> >,
                                boost::property<boost::edge_weight_t, double, EdgeData>
                                > MyGraphType;
  MyGraphType G(5);

  boost::property_map<MyGraphType, boost::vertex_distance_t>::type distance_map =
    get(boost::vertex_distance, G);
  boost::property_map<MyGraphType, boost::vertex_distance2_t>::type distance_map2 =
    get(boost::vertex_distance2, G);

  auto v = add_vertex(G);
  distance_map[v] = 10;
  distance_map2[v] = 20;
  
  std::cout << "distance "  << get(boost::vertex_distance, G)[v] << std::endl;
  std::cout << "distance2 " << get(boost::vertex_distance2, G)[v] << std::endl; 
  
}

void example3()
{

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::directedS,
                                VertexData,
                                EdgeData> MyGraphType;

  // when instantiating without args, don't make the mistake of writing
  // MyGraphType G();
  // you'll get cryptic/misleading errors if you do.
  MyGraphType G;

  VertexData vd = { "my name", 42 };
  add_vertex(vd, G);

  vd = { "my second name", 43 };
  int v1 = add_vertex(G);
  G[v1] = vd;

  int v2 = add_vertex({ "my last name", 44 }, G);

  // don't forget the .first
  auto e = add_edge(v1, v2, {"the edge", 77.3}, G).first;

  std::cout << "name getter0 " << get(&VertexData::first_name, G)[0] << std::endl;
  std::cout << "name getter1 " << get(&VertexData::first_name, G)[1] << std::endl;

  std::cout << "dist getter " << get(&EdgeData::dist, G)[e] << std::endl;
  
}



int
main(int, char *[])
{
  example0();
  example0a();
  example1();
  example2();
  example3();
}
