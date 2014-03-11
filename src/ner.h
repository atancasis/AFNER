////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
/////////////////////////////////////////////////////////////////////
// Filename: ner.h
/////////////////////////////////////////////////////////////////////
// This file contains the definition of the NamedEntity
// class, and the NEDeco class.  The NamedEntity class
// stores named entities.  It is of type 'TextUnit'.
// The NEDeco class finds named entities and stores them
// in a vector.
/////////////////////////////////////////////////////////////////////
// Log:
// 27/1/2006 - file started, headers written.
/////////////////////////////////////////////////////////////////////
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

#ifndef __ner__
#define __ner__  

#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <boost/regex.hpp>
#include "tokeniser.h"
#include "tokenDeco.h"
#include "suffixtree.h"
#include "iterator.h"
#include "xml_string.h"
#include "list_handler.h"
#include "entity_tag.h"
#include "regex_handler.h"
#include "feature_handler.h"

using namespace std;
using namespace boost;
using namespace ns_suffixtree;
using namespace AF;

namespace AF {

vector<TokenDeco>
tokeniseWithNEInfo(const StringXML::const_iterator begin,
    const StringXML::const_iterator end,const EntityTagset* tset);

class NamedEntity {

public:

/////////////////////////////////////////////////////////////////////
// Constructor:
/////////////////////////////////////////////////////////////////////
  NamedEntity(const StringXML::const_iterator begin,
      const StringXML::const_iterator start, 
      const StringXML::const_iterator end,const EntityTag* t,
      const double prob=0,const StringXML method="classify");
    
/////////////////////////////////////////////////////////////////////
// Offset and size accessors
/////////////////////////////////////////////////////////////////////
  StringXML::size_type leftOffset() const;
  StringXML::size_type rightOffset() const;
// length of entity
  string::size_type length() const;
  

////////////////////////////////////////////////////////////////////////////////
// Returns a pointer to the entity's tag
////////////////////////////////////////////////////////////////////////////////
  const EntityTag* getTag() const;
  
  StringXML getString() const;
  double getProb() const;
  StringXML getMethod() const;
  
/////////////////////////////////////////////////////////////////////
// 'printDetails()' prints the details of the NE to the ostream given
// 'getDetails()' gets the detais of the NE as a string
/////////////////////////////////////////////////////////////////////
  void printDetails(ostream& out) const;
  StringXML getDetails() const;
  StringXML getTRECDetails() const;
  void printTRECDetails(ostream& out) const;
  bool operator<(const NamedEntity& other) const;
  bool operator==(const NamedEntity& other) const;

private:
  StringXML::const_iterator _begin;
  StringXML::const_iterator _start;
  StringXML::const_iterator _end;
  const EntityTag* _tag;
  double _prob;
  StringXML _method;
}; // class NamedEntity


/////////////////////////////////////////////////////////////////////
// Class that recognises and stores named entities
/////////////////////////////////////////////////////////////////////
class NEDeco {

public:
/////////////////////////////////////////////////////////////////////
// Constructor, accepts a StringXML, populates NEList
/////////////////////////////////////////////////////////////////////
  NEDeco(const EntityTagset* t,const RegexHandler* rh,const ListHandler* lh,
      const FeatureHandler* fh,const StringXML& modelFile="",
      const int maxLabels=1,const int context=0,
      const bool singleLabels=false);
/////////////////////////////////////////////////////////////////////
// Iterators pointing to the 'NamedEntity's stored in _entities.
/////////////////////////////////////////////////////////////////////
  set<NamedEntity>::const_iterator begin();
  set<NamedEntity>::const_iterator end();
  
  void printEnts(float threshold=0.0);
  void printEnts(ostream& outfile, float threshold=0.0);
  void printEnts(const set<NamedEntity>& entities, ostream& out,
			float threshold=0.0);
  void printTRECEnts(float threshold=0.0);
  void printTRECEnts(ostream& outfile, float threshold=0.0);
  void printTRECEnts(const set<NamedEntity>& entities, ostream& out,
		     float threshold=0.0);
  
  
/////////////////////////////////////////////////////////////////////
// Static functions for general use in Named Entity recognition.
// Used by each NEDeco instance to populate the private vector 
// '_entities'.
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// 'Decorate' accepts a StringXML, and fills the given 
// vector with NamedEntity objects for those found in the StringXML
/////////////////////////////////////////////////////////////////////
  void Decorate(StringXML* text);

  void PrintTrainingData(StringXML* text,const bool printClasses,
      ostream& out, vector<map<StringXML,int> >& frequencies,
      vector<map<StringXML,int> >& prevFrequencies,
      const bool countFrequencies=true);

  private:
  
  void FindMatches(bool createEntities=true);

  void FindRegexMatches(bool createEntities=true);

  void FindListMatches(bool createEntities=true);

  void FindClassified();
  
/////////////////////////////////////////////////////////////////////
// 'findRegex' accepts a boost::regex pattern, a StringXML
//  a set<NamedEntity>, and a EntityType. Fills the given vector 
// with NamedEntity objects of type given  for those found in the
//  TextUnit that match the given pattern.
/////////////////////////////////////////////////////////////////////
  void FindRegex(const boost::regex& regex,const EntityTag* tag,
      const StringXML& regexName,bool createEntities);

  void  refineEntities(const set<NamedEntity>& old,set<NamedEntity>& ref);
  
/////////////////////////////////////////////////////////////////////
// '_text' is the StringXML given when the NEDeco is created.
/////////////////////////////////////////////////////////////////////
  StringXML* _text;
  vector<TokenDeco> _tokens;
////////////////////////////////////////////////////////////////////////////////
// The list of tags used by the decorator
////////////////////////////////////////////////////////////////////////////////
  const EntityTagset* _tagset;
  const RegexHandler* _regex_handler;
  const ListHandler* _list_handler;
  const FeatureHandler* _feature_handler;
  const StringXML _modelFile;
// The maximum number of labels to allow in classification
  const int _maxLabels;
  const int _context;
  const bool _singleLabels;
/////////////////////////////////////////////////////////////////////
// '_entities' is a vector of NamedEntities that is filled when the
// NEDeco is created.
/////////////////////////////////////////////////////////////////////
  set<NamedEntity> _entities;

}; // class NEDeco

}

#endif
