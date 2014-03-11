////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: entity_tag.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the EntityTag and EntityTagset
// classes. The EntityTag class stores information regarding a tag given to a
// Named Entity.  The EntityTagset class is a collection of tags.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/regex.hpp>
#include "xml_string.h"
#include "entity_tag.h"

using namespace std;
using namespace boost;
using namespace AF;

void parseSubTags(vector<StringXML>& tags,const StringXML& text);

////////////////////////////////////////////////////////////////////////////////
// Helper function to parse a list of subtags in the format 'tag1:tag2:tag3'
// Adds the tags to a given vector
////////////////////////////////////////////////////////////////////////////////
void parseSubTags(vector<StringXML>& tags,const StringXML& text) {
  regex tagex("^(?::)?(.+?)(?:(?:(?::)(.+$))|$)");
  match_results<StringXML::const_iterator> what;
  match_flag_type flags = match_default;
  StringXML::const_iterator begin, end;
  StringXML sub = text;
  // while there is a ':' in the subtags
  while (sub.find(":",0)!=string::npos) {
    begin = sub.begin();
    end = sub.end();
    // extract the first tag
    regex_search(begin,end,what,tagex,flags);
    StringXML tag(what[1].first,what[1].second);
    sub.assign(what[2].first,what[2].second);
    // add the tag
    if (tag.substr(tag.size()-1)==":") {
      tag.erase(tag.size()-1);
    }
    tags.push_back(tag);
    // set the string to the remainder
  }
  // there is no ':' in the subtags at this point
  // add the remaining subtag as the final tag, if not empty
  if (sub.length()>0){
    tags.push_back(sub);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Create an empty tag.
////////////////////////////////////////////////////////////////////////////////
EntityTag::EntityTag() {
  // no need to do anything
}

EntityTag::EntityTag(const EntityTag& tag) {
  _levels = tag._levels;
}

////////////////////////////////////////////////////////////////////////////////
// Accepts a string and parses XML to set tag variables.
////////////////////////////////////////////////////////////////////////////////
EntityTag::EntityTag(const StringXML xml_tag) {
  // parse XML, set _level accordingly
  regex tagex(
      "(?:<\\s*)(.+?)(?:\\s+TYPE=\"|')(.*?)(:?(:?:)(.+?))?(?:\"|'\\s*>)",
      regex::perl|regex::icase);
  match_results<StringXML::const_iterator> what;
  match_flag_type flags = match_default;
  StringXML::const_iterator begin, end;
  begin = xml_tag.begin();
  end = xml_tag.end();
  regex_search(begin,end,what,tagex,flags);
  // what[1] is the root, what[2] is the type, what[5] is subtags
  StringXML root(what[1].first,what[1].second);
  StringXML type(what[2].first,what[2].second);
  StringXML subtypes(what[5].first,what[5].second);
  _levels.push_back(root);
  _levels.push_back(type);
  parseSubTags(_levels,subtypes);
}

////////////////////////////////////////////////////////////////////////////////
// Accepts two strings, the root and sub-tag to set variables.
// Assumes only two levels.
////////////////////////////////////////////////////////////////////////////////
EntityTag::EntityTag(const StringXML root, const StringXML sub) {
  // set the levels of the tag
  _levels[0] = root;
  _levels[1] = sub;
}

////////////////////////////////////////////////////////////////////////////////
// Accepts a vector of strings, assigns the vector values to the tag levels
// according to the index in the vector.
////////////////////////////////////////////////////////////////////////////////
EntityTag::EntityTag (const vector<StringXML> tags) {
  // set the levels of the tag to correspond with those supplied.
  _levels = tags;
}

void
EntityTag::assign(const StringXML& t) {
  _levels.clear();
  // parse XML, set _level accordingly
  regex tagex(
      "(?:<\\s*)(.+?)(?:\\s+TYPE=\"|')(.*?)(:?(:?:)(.+?))?(?:\"|'\\s*>)",
      regex::perl|regex::icase);
  match_results<StringXML::const_iterator> what;
  match_flag_type flags = match_default;
  StringXML::const_iterator begin, end;
  begin = t.begin();
  end = t.end();
  regex_search(begin,end,what,tagex,flags);
  // what[1] is the root, what[2] is the type, what[5] is subtags
  StringXML root(what[1].first,what[1].second);
  StringXML type(what[2].first,what[2].second);
  StringXML subtypes(what[5].first,what[5].second);
  _levels.push_back(root);
  _levels.push_back(type);
  parseSubTags(_levels,subtypes);
}

////////////////////////////////////////////////////////////////////////////////
// Returns a string in XML format with tag details for use as opening tag:
// e.g. <ENAMEX TYPE="ORGANIZATION">
////////////////////////////////////////////////////////////////////////////////
const StringXML
EntityTag::openingTag() const{
  StringXML out = "";
  // if this is not an empty tag
  if (levelCount() > 0) {
    // get the root tag
    out = "<" + _levels[0];
    // get subsequent tags
    if (levelCount() > 1) {
      out += " TYPE=\"" + _levels[1];
      for (int i = 2;i < levelCount();i++) {
        // append sub-tags
        out+= ":" + _levels[i];
      }
      // close Type
      out += "\"";
    }
    out += ">";
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Returns a string in XML format for use as closing tag:
// e.g. </ENAMEX>
////////////////////////////////////////////////////////////////////////////////
const StringXML
EntityTag::closingTag() const {
  StringXML out = "";
  // if this is not an empty tag
  if (levelCount() > 0) {
    // output the root of the tag as close
    out = "</" + _levels[0] + ">";
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the number of levels in the tag:
// e.g. <ENAMEX TYPE="ORGANIZATION"> has 2 levels
////////////////////////////////////////////////////////////////////////////////
const int
EntityTag::levelCount() const {
  return _levels.size();
}

////////////////////////////////////////////////////////////////////////////////
// Returns the detail of the tag given the level.
// e.g. detail(0) of <ENAMEX TYPE="ORGANIZATION"> is "ENAMEX"
////////////////////////////////////////////////////////////////////////////////
const StringXML
EntityTag::detail(const int level) const {
  return _levels[level];
}

bool
EntityTag::operator==(const EntityTag& other) const {
  return (_levels==other._levels);
}

bool
EntityTag::operator<(const EntityTag& other) const {
  return (_levels < other._levels);
}

////////////////////////////////////////////////////////////////////////////////
// EntityTagset Implementation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates an empty tagset, simply initialises varialbles
////////////////////////////////////////////////////////////////////////////////
EntityTagset::EntityTagset() {
  // nothing to do here
}

////////////////////////////////////////////////////////////////////////////////
// Creates a tagset using a tag map in XML format.
////////////////////////////////////////////////////////////////////////////////
EntityTagset::EntityTagset(const StringXML& xml) {
  // for each line
  // read tag, then read index
//  for (afdsaf) {
//    int index;
//    EntityTag tag(t);
//    _tags[tag] = index;
//  }
  
  // read the xml with a parser
  
  // create a TagsetParser - an xml parser
  // feed xml into the tagset
  // for each tag in the parser
    // create a tag object
    // add it to the vector
}

EntityTagset::EntityTagset(istream& in) {
  StringXML str = "";
  while (getline(in,str)) {
    addTag(str);
  }
}
  
////////////////////////////////////////////////////////////////////////////////
// Returns the classification number given a tag and 'isBegin' flag.
// For use in printing training data, determines what classification a 
// tag will be given.
// Assumes that each tag can have two classifications (Begin and In)
// Formula used is (2 * tagIndex) + 1)) + (1 * isBegin)
// 0 is treated as 'Out' or no classification class.
// If tag does not exist, returns 0 as classification.
////////////////////////////////////////////////////////////////////////////////
const int
EntityTagset::getClassification(const EntityTag& tag, 
    const bool isBegin) const {
  // get the index of the given tag
  int c = 0;
  // find the tag
  map<EntityTag,int>::const_iterator ite = _tags.find(tag);
  if (ite != _tags.end()) {
    // calculate the classification
    c = (2 * ite->second) + 1;
    if (!isBegin) {
      c++;
    }
  }
  return c;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the index corresponding to a given tag
////////////////////////////////////////////////////////////////////////////////
const int
EntityTagset::getIndex(const EntityTag& tag) const {
  // get the index of the given tag
  int i = 0;
  // find the tag
  map<EntityTag,int>::const_iterator ite = _tags.find(tag);
  i = ite->second;
  return i;
}

const int
EntityTagset::getIndex(const int c) const {
  int i = 0;
  // if the class is even (in class)
  if (c%2==0) {
    // index is one lower than half class number
    i = (c/2) - 1;
  }
  else {
    // index is one lower than half class num + 1
    i = ((c+1)/2) - 1;
  }
  return i;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the tag given a classification number. 'isBegin' is set to true if
// the given classification is a begin classification.
// For use after classification has occurred: matches classifications
// with tags.
// If classification is invalid, an empty tag is returned.
////////////////////////////////////////////////////////////////////////////////
const EntityTag*
EntityTagset::getTag(const int c, bool& isBegin) const {
  // get the index to find in the map
  // if class is even, must not be begin tag
  // begin tags are odd
  int i = c;
  if (c%2==0) { // if class even
    isBegin=false;
    i--;
  }
  else {
    isBegin=true;
  }
  // reverse the function used to get class from index
  i = (i-1)/2;
  const EntityTag* t;
  bool found = false;
  // search for the index, when found, set tag
  for (map<EntityTag,int>::const_iterator ite=_tags.begin();
      ite!=_tags.end()&&!found;ite++) {
    if (ite->second == i) {
      found = true;
      t = &(ite->first);
    }
  }
  return t;
}

////////////////////////////////////////////////////////////////////////////////
// 'outClass' returns the classification for no tag: 0
////////////////////////////////////////////////////////////////////////////////
  const int 
  EntityTagset::outClass() const {
    return 0;
  }

////////////////////////////////////////////////////////////////////////////////
// 'addTag' adds a tag to the dataset.
////////////////////////////////////////////////////////////////////////////////
// Accepts a string in XML format storing tag details.
////////////////////////////////////////////////////////////////////////////////
void
EntityTagset::addTag(const StringXML& tag) {
  EntityTag t(tag);
  int i = tagCount();
  _tags[t] = i;
}

////////////////////////////////////////////////////////////////////////////////
// Accepts a tag object.
////////////////////////////////////////////////////////////////////////////////
void
EntityTagset::addTag(const EntityTag& tag) {
  //EntityTag t(tag);
  int i = tagCount();
  _tags[tag]=i;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the number of tags in the tagset
////////////////////////////////////////////////////////////////////////////////
int
EntityTagset::tagCount() const {
  return _tags.size();
}

////////////////////////////////////////////////////////////////////////////////
// Returns the number of classes in the tagset
////////////////////////////////////////////////////////////////////////////////
const int
EntityTagset::classCount() const {
  // treat each tag as a class
  // two sub classes for each tag (B and I)
  // one general 'out' class
  return _tags.size() * 2 + 1;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if the given tag exists in the tagset.
////////////////////////////////////////////////////////////////////////////////
const bool
EntityTagset::exists(EntityTag tag) const {
  map<EntityTag,int>::const_iterator ite = _tags.find(tag);
  // return true if pointer not at end, if tag exists
  return (ite!=_tags.end());
}

////////////////////////////////////////////////////////////////////////////////
// Print the tagset to the given output stream.
////////////////////////////////////////////////////////////////////////////////
const void
EntityTagset::printTagMap(ostream& ops) const {
}

const void
EntityTagset::printTags(ostream& ops) const {
  ops << "Index\tClasses\tTag" << endl;
  //cout << _tags.size();
  for (map<EntityTag,int>::const_iterator i=_tags.begin();i!=_tags.end();i++) {
    EntityTag t = i->first;
    ops << i->second << "\t" << getClassification(t) << ", "
        << getClassification(t,false) 
        << "\t" << t.openingTag()<< endl;
  }
}
