#include <boost/regex.hpp>
#include <iostream>

// this program is to remind myself how to extract the file name components.
// surely there's a standard way to do this. but where?
int main(int argc, char** argv) {

  for(int i=0; i<argc; i++) {
    std::string arg = argv[i];
    std::cout << arg << " ";
    boost::regex expr{"(.*?/?)([^/]+)(\\.kdenlive)"};
    boost::smatch what;
    if (boost::regex_search(arg, what, expr)) {
      std::string dir = what[1];
      std::string basename = what[2];
      std::cout << "whats " << what[0] << ":" << what[1] << ":" << what[2] << std::endl;
      std::cout << "fullpath " << dir << basename << ".kdenlive" << std::endl;
    }
    //std::cout << std::boolalpha << boost::regex_match(arg, expr) << '\n';

  }

  

}
