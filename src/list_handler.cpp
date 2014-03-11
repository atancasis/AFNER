////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: list_handler.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the ListHandler class.
// The ListHandler class performs functions associated with reading,
// searching and managing lists. Lists are stored in one SuffixTree.
// Search operations return an integer referring to the list in which
// the search string is found.
////////////////////////////////////////////////////////////////////////////////
// Log:
// 20/10/06 - file started.
////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2008  Diego Molla-Aliod <diego@ics.mq.edu.au>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <vector>
#include <sstream>
#include <map>
#include "list_handler.h"
#include "suffixtree.h"
#include "iterator.h"
#include "xml_string.h"
#include "entity_tag.h"

using namespace std;
using namespace boost;
using namespace ns_suffixtree;
using namespace AF;

ListHandler::ListHandler() {
  _num_lists=0;
}

ListHandler::ListHandler(const vector<StringXML>& filenames) {
  int i = 0;
  StringXML total;
  _num_lists = filenames.size();
  for (vector<StringXML>::const_iterator file=filenames.begin();
      file!=filenames.end();file++) {
    StringXML listString = ReadList(*file);
    ListBoundary l(ListBoundary(total.length(),
                            total.length()+listString.length(),i));
    _boundaries.push_back(l);
    total += listString;
    i++;
  }
  ofstream of;
  of.open("data/total.list");
  of << total << endl;
  of.close();
  _tree = new suffixtree(total);
}

ListHandler::ListHandler(const map<StringXML,EntityTag> lists) {
  int i=0;
  StringXML total;
  _num_lists = lists.size();
  for (map<StringXML,EntityTag>::const_iterator list=lists.begin();
      list!=lists.end();list++) {
    StringXML listString = ReadList(list->first);
    ListBoundary l(ListBoundary(total.length(),
                        total.length()+listString.length(),i));
    _boundaries.push_back(l);
    total += listString;
    // match the tag with the list
    _tags[i] = list->second;
    i++;
  }
  _tree = new suffixtree(total);
}

ListHandler::~ListHandler() {
  delete _tree;
}

////////////////////////////////////////////////////////////////////////////////
// Returns a vector of pairs of integers and EntityTags that correspond to the 
// lists in the map passed in the constructor
////////////////////////////////////////////////////////////////////////////////
vector<int>
ListHandler::FindString(const StringXML& input, const bool complete,
      const bool extra) const {
  vector<int> res;
  StringXML search("^" + input);
  if (complete) {
    search=search + "|";
  }
  else {
    if (extra) {
      search=search + " ";
    }
  }
  // find the occurrences, add start and end markers
  vector<StringXML::size_type> locs =
      _tree->find_all_positions(search);
  // for each found occurrence
  for (vector<StringXML::size_type>::const_iterator i=locs.begin();
      i!=locs.end();i++) {
    // get the integer corresponding to the list
    int list=GetListInt(*i);
    // check if the list is already in the list being returned
    bool found=false;
    for (unsigned int j=0;j<res.size()&&!found;j++) {
      found=(res[j]==list);
    }
    if (!found) {
      res.push_back(list);
    }
  }
  return res;
}

const EntityTag*
ListHandler::GetTag(const int list) const {
  map<int,EntityTag>::const_iterator f = _tags.find(list);
  const EntityTag* e = &(f->second);
  return e;
}

StringXML removeTags(const StringXML& input){
  StringXML out="";
  istringstream is(input);
  StringXML c = "";
  bool space = false;
  while (is >> c) {
    if (!(c=="LOC"||c=="PER"||c=="MISC"||c=="ORG"||c=="LANG")){
      if (space) {
        out += " " + c;
      }
      else {
        out +=c;
        space = true;
      }
    }
  }
  return out;
}

StringXML
ListHandler::ReadList(const StringXML& filename) {
  ifstream inFile;
  StringXML out = "";
  inFile.open(filename.c_str());   // open file
  StringXML in = "";
  if (!inFile) {    // if problem opening, report to cout
     cout << "Unable to open file: " << filename << endl;
  }
  else {
    while (! inFile.eof() ) {
      StringXML line="";
      getline(inFile,line);
      line = "^" + removeTags(line) + "|";
      out += line;
    }
  }
  inFile.close();   // close file
  return out;
}

int
ListHandler::ListCount() const {
  return _num_lists;
}

int
ListHandler::GetListInt(const StringXML::size_type location) const {
  for (vector<ListBoundary>::const_iterator i=_boundaries.begin();
      i!=_boundaries.end();i++) {
    if (i->WithinBounds(location)) {
      return i-_boundaries.begin();;
    }
  }
  return -1;
}

