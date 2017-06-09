

#include <tinyxml2.h>
#include <iostream>
#include <vector>
#include <map>
#include <boost/foreach.hpp>
#include <cassert>


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
class kdenlive_data : public tinyxml2::XMLVisitor {
public:
  bool VisitEnter(const tinyxml2::XMLElement& element,
                  const tinyxml2::XMLAttribute* firstAttribute) {

    // we want these:
    // <property name="kdenlive:track_name">Audio 2</property>

    // is it a property?
    if (element.Name() != std::string("property")) { 
      return true;
    }
    
    // is it a track_name?
    if (element.Attribute("name", "kdenlive:track_name")) {
      //std::cout << "got track " << element.Name() << " " << element.GetText() << std::endl;
      track_names.push_back(const_cast<tinyxml2::XMLElement*>(&element));
      return true; // returning true means recurse
    }

    if (element.Attribute("name", "resource")) {
      const tinyxml2::XMLElement* producer = element.Parent()->ToElement();
      assert(producer);
      const char* id = producer->Attribute("id");
      if (!id) { return true; }
      producer2resource[id] = element.GetText();
      return true;
    }
    
    return true;
  }
    
  std::vector<tinyxml2::XMLElement*> GetTrackNames() { return track_names; };

  std::string GetResourceForProducer(const std::string &producer) {
    auto prod = producer2resource.find(producer);
    if (prod == producer2resource.end()) { return ""; }
    return (*prod).second;
  }
private:
  std::vector<tinyxml2::XMLElement*> track_names;
  std::map<std::string, std::string> producer2resource;
};


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
  BOOST_FOREACH(auto tn, kdata.GetTrackNames()) {
    std::cout << "track " << tn->GetText() << std::endl;
    tinyxml2::XMLElement* playlist = tn->Parent()->ToElement();
    assert(playlist);
    assert(std::string("playlist") == playlist->Name());
    tinyxml2::XMLElement* entry = playlist->FirstChildElement("entry");
    while (entry) {
      std::cout << "  entry ";
      if (const char* in = entry->Attribute("in")) {
        std:: cout << " in " << in;
      }
      if (const char* out = entry->Attribute("out")) {
        std:: cout << " out " << out;
      }
      if (const char* producer = entry->Attribute("producer")) {
        std:: cout << " producer " << producer << "(" << kdata.GetResourceForProducer(producer) << ")";
      }
      std::cout << std::endl;
      tinyxml2::XMLElement* todelete = entry;
      entry = entry->NextSiblingElement("entry");
      playlist->DeleteChild(todelete);
    }

    tinyxml2::XMLElement* newentry = doc.NewElement("entry");
    newentry->SetAttribute("in", 32);
    newentry->SetAttribute("out", 37);
    newentry->SetAttribute("producer", "the new one");
    playlist->InsertEndChild(newentry);
  }

  doc.SaveFile((filename + std::string(".new")).c_str());
  
}
