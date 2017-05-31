

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

// the predefined property types are listed here:
// http://www.boost.org/doc/libs/1_64_0/libs/graph/doc/using_adjacency_list.html#sec:adjacency-list-properties

struct VertexData
{
  std::string first_name;
  int num;

  // this is used in predecessor map. The type shouldn't really be
  // hardcoded to int. Instead, it should be set to vertex_descriptor
  // (perhaps via template). I use it directly here, to keep the examples
  // below a little simpler.
  int pred;

  // this is used in distance map
  int dist;
};

struct EdgeData
{
  std::string edge_name;
  double dist;
};


void example1()
{

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::directedS,
                                boost::no_property,
                                boost::property<boost::edge_weight_t, int> > MyGraphType;

  // when instantiating without args, don't make the mistake of writing
  // MyGraphType G();
  // you'll get cryptic/misleading errors if you do.
  MyGraphType G;

  int v0 = add_vertex(G);
  int v1 = add_vertex(G);
  int v2 = add_vertex(G);

  // don't forget the .first, .second tells you if this is a new edge was added
  // (alternately, the edge already existed)
  auto e1 = add_edge(v0, v1, 10, G).first;
  auto e2 = add_edge(v1, v2, 20, G).first;

  // http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/iterator_property_map.html

  typedef typename boost::graph_traits<MyGraphType>::vertex_descriptor vertex_descriptor;

  // while adjacency list is indexed by ints, this doesn't have to be true.
  // a vertex or edge may be a point. In that case, you'd need a mapping from
  // those pointers into int.
  // Because the algorithms try to be generic, you need to give a (trivial mapping)
  // for ints.
  //
  // iterator property takes an iterator and provides a mapping to it indexed by vertex.
  // an array pointer is also an iterator.
  //
  // adjacency list comes with a vertex_index mapping. Still, to be generic, you
  // need to tell iterator_property_map explicitly.
  typedef boost::property_map<MyGraphType, boost::vertex_index_t>::type IdMap;

  
  std::vector<vertex_descriptor> pred(num_vertices(G));
  boost::iterator_property_map<std::vector<vertex_descriptor>::iterator,
                                 IdMap,
                                 vertex_descriptor,
                                 vertex_descriptor&>
    predmap(pred.begin(), get(boost::vertex_index, G));

  // this is property map based on a c style array. The predmap above and distmap_vect use
  // stl vectors
  int* dist = (int*) malloc(num_vertices(G)*sizeof(int));  
  boost::iterator_property_map<int*,
                                 IdMap,
                                 int,
                                 int&>
    distmap_ptr(dist, get(boost::vertex_index, G));

  std::vector<int> distvector(num_vertices(G));
  boost::iterator_property_map<std::vector<int>::iterator,
                                 IdMap,
                                 int,
                                 int&>
    distmap_vect(distvector.begin(), get(boost::vertex_index, G));
    
  // note that the operator between pred map and distance map is a '.'
  // and not a ','. That's the named parameter mechanism.
  // The first of these two gets passed a std::vector. The other is a c array.
  dijkstra_shortest_paths(G, v0,
                          predecessor_map(predmap)
                          .distance_map(distmap_vect));

  // the order of named parameters doesn't matter.
  dijkstra_shortest_paths(G, v0,
                          distance_map(distmap_ptr)
                          .predecessor_map(predmap));

  
  
}


void example2()
{

  // this example uses the bundeled property mechanism. It's probably the
  // nicest way to associate data with the graph.
  // note that bundeled properties are used both for inputs and outputs to
  // dijkstra
  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::directedS,
                                VertexData,
                                EdgeData> MyGraphType;

  // when instantiating without args, don't make the mistake of writing
  // MyGraphType G();
  // you'll get cryptic/misleading errors if you do.
  MyGraphType G;

  int v0 = add_vertex(G);
  int v1 = add_vertex(G);
  int v2 = add_vertex(G);

  // don't forget the .first
  auto e1 = add_edge(v0, v1, {"edge1", 10}, G).first;
  auto e2 = add_edge(v1, v2, {"edge2", 20}, G).first;

  // http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/iterator_property_map.html

 
  // note that the operator between pred map and distance map is a '.'
  // and not a ','. That's the named parameter mechanism.
  // if you omit the weight, you'll get a compile error like this:
  // /usr/include/boost/graph/detail/adjacency_list.hpp:2696:29: error: forming reference to void
  dijkstra_shortest_paths(G, v0,
                          predecessor_map(get(&VertexData::pred, G))
                          .distance_map(get(&VertexData::pred, G))
                          .weight_map(get(&EdgeData::dist, G)));


   
}



typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::no_property,
                              // need edge_index for use in distmap below
                              boost::property<boost::edge_index_t, std::size_t> > MyBareGraphType;


// you don't have the use the helper stuff for property maps.
// you can implement them yourself.
// You just need a get() function if the property is just readable.
// to write, you need a put() and to be in the lvalue class, you need
// operator[]. I don't know when you'd need lvalue beyond read/write,
// however an operator[] makes get/put easier to implement.

// a couple important notes about implementing property maps:
//   don't try to skip the map part by just giving a data structure. The data
//   structure has to be separate. You can refer to it in the map, but the map
//   itself can't own the data. There are two reasons why
//   * The property maps are copied by value in many places in the algorithms. So
//     in the best case or read only, you incur a heavy runtime cost copying them.
//     In the not so good case, writes to properties would be lost.
//   * The alogorithms hold the property maps as consts, so unless you cheat, you
//     won't be able to write to the property.
//
// note that in the get/put functions below, the map is passed as a reference and
// the other one or two arguments are not by reference. If you try to pass a reference
// you'll get confusing compiler errors.
class my_weightmap {
 public:
  typedef boost::readable_property_map_tag category;
  typedef int value_type;
  typedef int reference;
  typedef typename boost::graph_traits<MyBareGraphType>::edge_descriptor key_type;
    
  my_weightmap(std::map<key_type, value_type> &weights) : weights(weights) { /* empty */ }
  value_type getweight(key_type &k) const { return weights[k]; }
  
private:
  std::map<key_type, value_type> &weights;
};

my_weightmap::value_type
get(const my_weightmap& wm, my_weightmap::key_type k)
{
  return wm.getweight(k);
}

class my_distmap {
 public:
  typedef boost::read_write_property_map_tag category;
  typedef int value_type;
  typedef int reference;
  typedef typename boost::graph_traits<MyBareGraphType>::vertex_descriptor key_type;
    
  my_distmap(MyBareGraphType &G, std::vector<value_type> &dists)
    : G(G), dists(dists) { /* empty */ }

  value_type& operator[](key_type vh) const {
    return dists[get(boost::vertex_index, G)[vh]]; 
  }

private:
  std::vector<value_type> &dists;
  MyBareGraphType &G;
};



my_distmap::value_type
get(my_distmap& dm, my_distmap::key_type k)
{
  return dm[k];
}

void
put(const my_distmap& dm, my_distmap::key_type k, my_distmap::value_type v)
{
  dm[k] = v;
}


//http://www.boost.org/doc/libs/1_64_0/libs/property_map/doc/property_map.html
class my_predmap : public boost::put_get_helper<int, my_predmap> {
public:
  typedef boost::read_write_property_map_tag category;
  typedef typename boost::graph_traits<MyBareGraphType>::vertex_descriptor key_type;
  typedef key_type value_type;
  typedef key_type& reference;

  // the put_get_helper, really wants the property map to be const.
  // this makes sense, since a property map is just supposed to be a map and not a container
  my_predmap(std::map<key_type, value_type> &preds) : preds(preds) { /* empty */ }
    
  value_type& operator[](const key_type &vh) const {
    return preds[vh]; 
  }

private:
  std::map<key_type, value_type> &preds;
};


void example3()
{
  
  // when instantiating without args, don't make the mistake of writing
  // MyGraphType G();
  // you'll get cryptic/misleading errors if you do.
  MyBareGraphType G;

  int v0 = add_vertex(G);
  int v1 = add_vertex(G);
  int v2 = add_vertex(G);

  // don't forget the .first
  auto e1 = add_edge(v0, v1, G).first;
  auto e2 = add_edge(v1, v2, G).first;

  std::map<MyBareGraphType::vertex_descriptor, MyBareGraphType::vertex_descriptor> preds;
  my_predmap predmap(preds);
  std::vector<int> dists;
  my_distmap distmap(G, dists);
  std::map<MyBareGraphType::edge_descriptor, int> weights;
  my_weightmap weightmap(weights);
  
  dijkstra_shortest_paths(G, v0,
                          predecessor_map(predmap)
                          .distance_map(distmap)
                          .weight_map(weightmap));
}
int
main(int, char *[])
{

  example1();
  example2();
  example3();
}
