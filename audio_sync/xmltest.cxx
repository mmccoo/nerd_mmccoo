

#include <tinyxml2.h>
#include <iostream>
#include <vector>
#include <map>
#include <boost/foreach.hpp>
#include <cassert>

#include <kdenlive_utils.hxx>

// this is a test program to see what it takes to:
// * read a .kdenlive file
// * find the resource/producer mappings
// * find the tracks used
// * print the existing entries
// * replace one entry with another.
// * save the modified

// some observations on tinyxml2 relative to libxml2 (my experience is limited to the
// perl API of libxml2
// * at first, I liked xpath over xmlvisitor. xpath does still have the most power
//   one annoying thing about libxml2 is that evertying returns a list and I first have
//   to check the length. tinyxml first the first one and it's easy to check yes or now.
// * when adding stuff, the indentation is nice with extra work.
// not sure yet if I will stick with tinyxml.


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "need filename" << std::endl;
    exit(-1);
  }

  std::string filename(argv[1]);
  
  tinyxml2::XMLDocument doc;
  doc.LoadFile(filename.c_str());

  
  kdenlive_data kdata;
  doc.Accept(&kdata);
  BOOST_FOREACH(auto &track, kdata.GetTracks()) {
    std::cout << "track " << track.GetName() << std::endl;

    BOOST_FOREACH(auto &entry, track.GetEntries()) {
      std::cout << " entry " << entry.in;
      std::cout << " out " << entry.out;
      std::cout << " producer " << entry.producer << "(" << entry.resource << ")";
      std::cout << std::endl;
    }
    track.ClearEntriesAndBlanks();
    track.AddEntry(32, 37, "the new one");
    track.AddBlank(100);
    track.AddEntry(32, 37, "the other one");
  }

  doc.SaveFile((filename + std::string(".new")).c_str());
  
}
