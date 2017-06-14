

#include <kdenlive_utils.hxx>

static int trackids = 0;
track_data::track_data(tinyxml2::XMLElement* track_element, kdenlive_data &kdata) :
  element(track_element), kdata(kdata), last_position(0) {
  id = trackids;
  trackids++;
  
  playlist = element->Parent()->ToElement();
  assert(playlist);
  assert(std::string("playlist") == playlist->Name());

  tinyxml2::XMLElement* entry = playlist->FirstChildElement("entry");
  while (entry) {
    //std::cout << "  entry ";
    const char* in = entry->Attribute("in");
    assert(in);
    //std:: cout << " in " << in;
    const char* out = entry->Attribute("out");
    assert(out);
    //std:: cout << " out " << out;
    const char* producer = entry->Attribute("producer");
    assert(producer);
    //std:: cout << " producer " << producer << "(" << kdata.GetResourceForProducer(producer) << ")";        
    //std::cout << std::endl;
    entries.push_back({std::stoi(in), std::stoi(out), producer, kdata.GetResourceForProducer(producer)});
    entry = entry->NextSiblingElement("entry");         
  };
}

tinyxml2::XMLElement* track_data::AddEntry(int in, int out, std::string producer) {
  tinyxml2::XMLDocument* doc = playlist->GetDocument();
  tinyxml2::XMLElement* newentry = doc->NewElement("entry");
  newentry->SetAttribute("in", in);
  newentry->SetAttribute("out", out);
  newentry->SetAttribute("producer", producer.c_str());

  playlist->InsertEndChild(newentry);
  return newentry;    
}

//   <property name="transition.geometry">956/0:960x540:100</property>
//   <property name="transition.geometry">x/y:widhtxheight:100</property> don't know what the 100 is for.
const char* quads[4] = {
  "0/0:960x540:100",
  "960/0:960x540:100",
  "0/540:960x540:100",
  "960/540:960x540:100"
};

void track_data::AddZoomPan(tinyxml2::XMLElement* entry, int quadrant)
{
  tinyxml2::XMLDocument* doc = playlist->GetDocument();
  tinyxml2::XMLElement* newfilter = doc->NewElement("filter");
  newfilter->SetAttribute("id", "pan_zoom");
  newfilter->SetAttribute("out", entry->Attribute("out"));

  std::pair<const char*, const char*> filter_props[] = {
    { "background", "colour:0x00000000"},
    { "mlt_service", "affine"},
    { "kdenlive_id", "pan_zoom"},
    { "tag", "affine"},
    { "kdenlive_ix", "1"},
    { "transition.distort", "0"},
    { "kdenlive:sync_in_out", "1"}
  };
  BOOST_FOREACH(auto fp, filter_props) {
    tinyxml2::XMLElement* newprop = doc->NewElement("property");
    newprop->SetAttribute("name", fp.first);
    newprop->SetText(fp.second);
    newfilter->InsertEndChild(newprop);
  }
  tinyxml2::XMLElement* newprop = doc->NewElement("property");
  newprop->SetAttribute("name", "transition.geometry");
  newprop->SetText(quads[quadrant%(sizeof(quads)/sizeof(char*))]);
  newfilter->InsertEndChild(newprop);

  newprop = doc->NewElement("property");
  newprop->SetAttribute("name", "kdenlike_info");
  newfilter->InsertEndChild(newprop);
  entry->InsertEndChild(newfilter);  
}
