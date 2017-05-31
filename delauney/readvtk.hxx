

#ifndef READ_VTK_HH
#define READ_VTK_HH

#include <iostream>
#include <sstream>
#include <string>




template<typename Graph, typename vertex_descriptor>
void readvtk(std::istream& is, Graph &g)
{
  // this is not a generic vtk reader. it only reads ASCII vtk POLYDATA files

  std::string fileline;
  
  int numpoints = 0;
  int numlines = 0;

  std::vector<vertex_descriptor> vertices;
  
  while(std::getline(is,fileline)) {
    const auto firstNonWhite = fileline.find_first_not_of(" \t");
    if (firstNonWhite == std::string::npos) { continue; }

    // trim the initial whitespace
    fileline = fileline.substr(firstNonWhite);
    
    if (fileline[0] == '#') { continue; }

    std::stringstream lineStream(fileline);
    if (numpoints>0) {
      double x,y,z;
      lineStream >> x >> y >> z;
      std::string vname = std::to_string(vertices.size()); // this is just the index in the array
      vertices.push_back(add_vertex(g, vname, x, y, z));
      numpoints--;
      continue;
    }

    if (numlines>0) {
      int numpointstoread;
      lineStream >> numpointstoread;
      int source, target;
      lineStream >> source;
      numpointstoread--;
      for(/*empty*/; numpointstoread; numpointstoread--) {
        lineStream >> target;
        add_edge(g, vertices[source], vertices[target]);
        source = target;
      }
    }
    
    std::string token;
    lineStream >> token;
    if (token == "POINTS") {
      lineStream >> numpoints;
      std::cerr << "Reading " << numpoints << " points" << std::endl;
      continue;
    } else if (token == "LINES") {
      lineStream >> numlines;
      std::cerr << "Reading " << numlines << " lines" << std::endl;
      continue;
    }
  }   
}



#endif
