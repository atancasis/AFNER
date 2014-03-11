////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: regex_handler.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the RegexHandler class.
// The RegexHandler class is used to handle custom regular expressions for named
// entity creation.
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
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <boost/regex.hpp>
#include "xml_string.h"
#include "entity_tag.h"
#include "regex_handler.h"

using namespace std;
using namespace boost;
using namespace AF;

////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
RegexHandler::RegexHandler(ifstream& ifs) {
  StringXML line = "";
  map<StringXML,StringXML> vars;
  // for each line
  while (getline(ifs,line)) {
    // if line not empty
    if (line.size() > 0) {
      // if line not a comment
      if (line.substr(0,1) != "#") {
        istringstream is(line);
        StringXML key = "";
        StringXML name = "";
        StringXML t = "";
        vector<StringXML> vals;
        // get the keyword and variable name
        is >> key;
        is >> name;
        // for each remaining token
        while (is >> t) {
          // add the token to the vector
          vals.push_back(t);
        }
        // if set keyword is first
        if (key=="SET") {
          SetVariable(name,vals,vars);
        }
        // if map keyword is first
        else if (key=="MAP") {
          // evaluate the regex from variable name if necessary
          StringXML re = EvaluateExpression(name,vars);
          // recombine the tokens in vector to form the tag text
          StringXML tag = "";
          for (vector<StringXML>::const_iterator i = vals.begin();
              i!=vals.end();i++) {
            tag += *i + " ";
          }
          // set the tag
          SetRegex(re,tag);
        }
      }
    }
  }
}

RegexHandler::RegexHandler() {
}

////////////////////////////////////////////////////////////////////////////////
// 'begin()' and 'end()' return const_iterators pointing to the beginning and
// end of the map of regex to tag respectively
////////////////////////////////////////////////////////////////////////////////
map<regex,EntityTag>::const_iterator
RegexHandler::begin() const {
  return _pairs.begin();
}

map<regex,EntityTag>::const_iterator
RegexHandler::end() const {
  return _pairs.end();
}

int
RegexHandler::RegexCount() const {
  return _pairs.size();
}

////////////////////////////////////////////////////////////////////////////////
// Input functions: these functions are used when the RegexHandler is created
// to assist with reading the input file.
////////////////////////////////////////////////////////////////////////////////
StringXML
RegexHandler::EvaluateExpression(const StringXML& exp,
    const map<StringXML,StringXML>& vars) {
  if (exp.size() > 1) {
    // if size is sufficient, and first character is $
    //StringXML c(exp.at(0));
    if (exp.substr(0,1) == "$") {
      // extract substring from 1 to end of exp to get varname
      StringXML sub = exp.substr(1,exp.size());
      // if variable exists, return value
      map<StringXML,StringXML>::const_iterator i = vars.find(sub);
      if (i!=vars.end()) {
        return i->second;
      }
      else {
        cerr << "Error in reading regex file: variable " 
            << sub << " not found." << endl;
      }
    }
    return exp;
  }
  return exp;
}

void
RegexHandler::SetVariable(const StringXML& name,const vector<StringXML>& values,
    map<StringXML,StringXML>& vars) {
  StringXML res = "";
  // for each value in the list to set
  for (vector<StringXML>::const_iterator i=values.begin();i!=values.end();i++) {
    // add the expression to the result
    res += EvaluateExpression(*i,vars);
  }
  vars[name] = res;
}

void
RegexHandler::SetRegex(const StringXML re,const StringXML tag) {
  EntityTag t(tag);
  regex r(re,regex::perl|regex::icase);
  _pairs[r] = t;
}


