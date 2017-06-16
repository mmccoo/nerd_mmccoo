

#ifndef KDENLIVE_UTILS_HH
#define KDENLIVE_UTILS_HH

#include <tinyxml2.h>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <boost/foreach.hpp>

class kdenlive_data;


struct entry {
  int in;
  int out;
  std::string producer;
  std::string resource;
};
  
class track_data {
public:
  track_data(tinyxml2::XMLElement* track_element, kdenlive_data &kdata);

  std::string GetName() { return element->GetText(); }
  std::vector<entry> &GetEntries() { return entries; }

  int  GetLastPosition() { return last_position; }
  void SetLastPosition(int pos) { last_position = pos; }
  
  tinyxml2::XMLElement* AddEntry(int in, int out, std::string producer);
  void AddZoomPan(tinyxml2::XMLElement* entry, int quadrant);
  
  void AddBlank(int length) {
    tinyxml2::XMLElement* newblank = playlist->GetDocument()->NewElement("blank");
    newblank->SetAttribute("length", length);
    playlist->InsertEndChild(newblank);
  }

  int GetId() { return id; }
  void ClearEntriesAndBlanks() {
    tinyxml2::XMLElement* entry = playlist->FirstChildElement("entry");
    while (entry) {
      tinyxml2::XMLElement* todelete = entry;
      entry = entry->NextSiblingElement("entry");
      playlist->DeleteChild(todelete);
    };
    tinyxml2::XMLElement* blank =playlist->FirstChildElement("blank");
    while (blank) {
      tinyxml2::XMLElement* todelete = blank;
      blank = blank->NextSiblingElement("blank");
      playlist->DeleteChild(todelete);
    };
  }

  void Print() {
    std::cout << GetName() << std::endl;
    tinyxml2::XMLPrinter printer;
    playlist->Accept(&printer);
    std::cout << printer.CStr();
    
  }
  
private:
  int                   last_position;
  tinyxml2::XMLElement* element;
  tinyxml2::XMLElement* playlist;
  std::vector<entry>    entries;
  kdenlive_data        &kdata;
  int                   id;
};
  

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
      tracks.push_back(track_data(const_cast<tinyxml2::XMLElement*>(&element), *this));
      return true; // returning true means recurse
    }

    if (element.Attribute("name", "resource")) {
      const tinyxml2::XMLElement* producer = element.Parent()->ToElement();
      assert(producer);
      const char* id = producer->Attribute("id");
      if (!id) { return true; }
      producer2resource[id] = element.GetText();
      resource2producer[element.GetText()] = id;
      return true;
    }
    
    return true;
  }
    
  std::vector<tinyxml2::XMLElement*> GetTrackNames() { return track_names; };
  std::vector<track_data> &GetTracks() { return tracks; }
  
  std::string GetResourceForProducer(const std::string &producer) {
    auto prod = producer2resource.find(producer);
    if (prod == producer2resource.end()) { return ""; }
    return (*prod).second;
  }

  std::string GetProducerForResource(const std::string &resource) {
    auto res = resource2producer.find(resource);
    if (res == resource2producer.end()) { return ""; }
    return (*res).second;
  }
private:
  std::vector<tinyxml2::XMLElement*> track_names;
  std::vector<track_data> tracks;
  std::map<std::string, std::string> producer2resource;
  std::map<std::string, std::string> resource2producer;
};


#endif
