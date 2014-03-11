////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: list_handler.h
////////////////////////////////////////////////////////////////////////////////
// This file contains the definition of the ListHandler class.
// The ListHandler class performs functions associated with reading,
// searching and managing lists. Lists are stored in one SuffixTree.
// Search operations return an integer referring to the list in which
// the search string is found.
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
// Log:
// 20/10/06 - file started.
////////////////////////////////////////////////////////////////////////////////

#ifndef __list_handler__
#define __list_handler__

#include <cstdlib>
#include <string>
#include <iostream>
#include <boost/regex.hpp>
#include <vector>
#include <map>
#include "suffixtree.h"
#include "iterator.h"
#include "xml_string.h"
#include "entity_tag.h"

using namespace std;
using namespace boost;
using namespace ns_suffixtree;

namespace AF {

  class ListHandler {
    public:
      struct ListBoundary {
        int begin;
        int end;
        int index;
        ListBoundary(int b, int e, int i) {
          begin = b; end = e; index = i;
        }
        bool WithinBounds(const int in) const {
           return (begin <= in && in < end);
        }
      };
///////////////////////////////////////////////////////////////////////////////
// ListHandler constructor: accepts a vector of filenames, initialises
// SuffixTree and list boundary indexes.
///////////////////////////////////////////////////////////////////////////////
      ListHandler();
      ListHandler(const vector<StringXML>& filenames);
      ListHandler(const map<StringXML,EntityTag> lists);
///////////////////////////////////////////////////////////////////////////////
//  Destructor
///////////////////////////////////////////////////////////////////////////////
      ~ListHandler();
///////////////////////////////////////////////////////////////////////////////
// 'FindString' searches for a particular string in the lists and
// returns a vector of EntityTags corresponding to the lists in which
// the search string was found. If the string is not found, an empty
// vector is returned.
// 'complete' indicates whether the entire string is to be found. the
// default value is true.
// 'extra' indicates whether the string should match with a space
// following, default false.
///////////////////////////////////////////////////////////////////////////////
      vector<int>
      FindString(const StringXML& input, const bool complete=true,
          const bool extra=false) const;
      
      int
      ListCount() const;

      const EntityTag*
      GetTag(const int list) const;
      
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    private:
      StringXML ReadList(const StringXML& filename);
///////////////////////////////////////////////////////////////////////////////
// 'GetListint' returns the list that a given location in the tree
// falls under.
///////////////////////////////////////////////////////////////////////////////
      int GetListInt(const StringXML::size_type location) const;
      int _num_lists;
      suffixtree* _tree;
      vector<ListBoundary> _boundaries;
      map<int,EntityTag> _tags;
  };
   
}

#endif // __list_handler__
// end of file: list_handler.h

