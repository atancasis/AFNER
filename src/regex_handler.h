////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: regex_handler.h
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

#ifndef __regex_handler__
#define __regex_handler__

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

using namespace std;
using namespace boost;
using namespace AF;

namespace AF {

////////////////////////////////////////////////////////////////////////////////
// The RegexHandler class stores mapping between regular expressions and 
// EntityTag objects.
// This allows for custom regular expressions and tags to be read in and used in
// the named entity recogniser.
////////////////////////////////////////////////////////////////////////////////
class RegexHandler {
  public:

////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
    RegexHandler(ifstream& ifs);

    RegexHandler();

////////////////////////////////////////////////////////////////////////////////
// 'begin()' and 'end()' return const_iterators pointing to the beginning and 
// end of the map of regex to tag respectively
////////////////////////////////////////////////////////////////////////////////
    map<regex,EntityTag>::const_iterator
    begin() const;

    map<regex,EntityTag>::const_iterator
    end() const;

    int
    RegexCount() const;

  private:

////////////////////////////////////////////////////////////////////////////////
// Input functions: these functions are used when the RegexHandler is created
// to assist with reading the input file.
////////////////////////////////////////////////////////////////////////////////
    StringXML
    EvaluateExpression(const StringXML& exp,
        const map<StringXML,StringXML>& vars);

    void
    SetVariable(const StringXML& name,const vector<StringXML>& values,
        map<StringXML,StringXML>& vars);
    
    void
    SetRegex(const StringXML regex,const StringXML tag);

    map<regex,EntityTag> _pairs;
    
};

}

#endif
