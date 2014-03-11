////////////////////////////////////////////////////////////////////////////////
// Filename: feature_extraction.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the feature extraction
// used for named entity recognition.
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

#include <vector>
#include <iostream>
#include "feature_extraction.h"
#include "tokenDeco.h"
#include "regex_handler.h"
#include "list_handler.h"

using namespace std;
using namespace AF;

////////////////////////////////////////////////////////////////////////////////
// 'FeatureValue()' constructor.  It takes the name of a feature and
// its value.
////////////////////////////////////////////////////////////////////////////////
FeatureValue::FeatureValue(const StringXML& feature, double value)
    :_feature(feature),_value(value) {
}


////////////////////////////////////////////////////////////////////////////////
// 'getFeature()' returns the name of the feature.
////////////////////////////////////////////////////////////////////////////////
StringXML
FeatureValue::getFeature() const {
  return _feature;
}

  
////////////////////////////////////////////////////////////////////////////////
// 'getValue()' returns the value of the feature.
////////////////////////////////////////////////////////////////////////////////
double
FeatureValue::getValue() const {
  return _value;
}


////////////////////////////////////////////////////////////////////////////////
// 'FeatureValueExtractor()' constructor.
////////////////////////////////////////////////////////////////////////////////
FeatureValueExtractor::FeatureValueExtractor(const StringXML name,
    const int context,const StringXML alias)
    :_feature_name(name),_context(context),_alias(alias) {
  setWeight(1);
}

////////////////////////////////////////////////////////////////////////////////
// 'FeatureValueExtractor()' destructor.
////////////////////////////////////////////////////////////////////////////////
FeatureValueExtractor::~FeatureValueExtractor() {
}

////////////////////////////////////////////////////////////////////////////////// 'getName' returns the name of the FeatureValueExtractor
////////////////////////////////////////////////////////////////////////////////
StringXML
FeatureValueExtractor::getName() const {
  return _feature_name;
}

////////////////////////////////////////////////////////////////////////////////
//'getAlias' returns the alias (if any) assigned to the FeatureValueExtractor.
// If no alias is assigned, the name is returned
////////////////////////////////////////////////////////////////////////////////
StringXML
FeatureValueExtractor::getAlias() const {
  return _alias;
}
////////////////////////////////////////////////////////////////////////////////
// 'setWeight sets the weight of the FeatureValueExtractor
////////////////////////////////////////////////////////////////////////////////
void
FeatureValueExtractor::setWeight(const double weight) {
  _weight = weight;
}

////////////////////////////////////////////////////////////////////////////////
// 'getWeight' returns the weight of the FeatureValueExtractor
////////////////////////////////////////////////////////////////////////////////
double
FeatureValueExtractor::getWeight() const {
  return _weight;
}

////////////////////////////////////////////////////////////////////////////////// 'getContext' returns the distance away from the target token that the FVE
// calculates a feature value for
////////////////////////////////////////////////////////////////////////////////
int
FeatureValueExtractor::getContext() const{
    return _context;
}

////////////////////////////////////////////////////////////////////////////////
// Default constructor
////////////////////////////////////////////////////////////////////////////////
FeatureVectorValueExtractor::FeatureVectorValueExtractor() {
}

////////////////////////////////////////////////////////////////////////////////
// 'FeatureVectorValueExtractor()' destructor.
////////////////////////////////////////////////////////////////////////////////
FeatureVectorValueExtractor::~FeatureVectorValueExtractor() {
   // delete all the featureValueExtractor objects
   for (vector<FeatureValueExtractor*>::const_iterator
       i=_featureAlgorithms.begin(); i!=_featureAlgorithms.end();
       ++i) {
     delete *i;
   }
}

////////////////////////////////////////////////////////////////////////////////
// 'printFeatures' prints a list of the features available in the
// FeatureValueExtractor, their alias, and weight to the given output stream.
////////////////////////////////////////////////////////////////////////////////
void
FeatureVectorValueExtractor::printFeatures(ostream& os) const {
  os << "Feature Name\t\tAlias\t\tContext\t\tWeight" << endl;
  for (vector<FeatureValueExtractor*>::const_iterator
      i=_featureAlgorithms.begin(); i!=_featureAlgorithms.end();
      ++i) {
    os << (*i)->getName() << "\t\t" << (*i)->getAlias() << "\t\t" 
        << (*i)->getContext() << "\t\t" 
        << (*i)->getWeight() << endl;
  }
}

bool
FeatureVectorValueExtractor::setFeatureWeight(const StringXML& feature_name, 
    const double weight) {
  bool set=false;
  for (vector<FeatureValueExtractor*>::const_iterator
      i=_featureAlgorithms.begin(); i!=_featureAlgorithms.end();
      ++i) {
    // if name matches, set the weight
    if (((*i)->getName()==feature_name)||
        ((*i)->getAlias()==feature_name)) {
      (*i)->setWeight(weight);
      set = true;
    }
  }
  return set;
}

void
FeatureVectorValueExtractor::setDefaultFeatureWeight(const double weight) {
  for (vector<FeatureValueExtractor*>::const_iterator
      i=_featureAlgorithms.begin(); i!=_featureAlgorithms.end();
      ++i) {
    (*i)->setWeight(weight);
  }
}

////////////////////////////////////////////////////////////////////////////////
// 'operator()()' applies the FeatureValueExtractor algorithms to
// the index in vector<TokenDeco> and returns a FeatureVector for that
// TokenDeco.
////////////////////////////////////////////////////////////////////////////////
FeatureVector
FeatureVectorValueExtractor::operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text,const bool ignoreWeights) const {
  FeatureVector result;
  for (vector<FeatureValueExtractor*>::const_iterator
      i=_featureAlgorithms.begin(); i!=_featureAlgorithms.end();
      ++i) {
    // Run only if the weight of the feature is >0 or if ignore
    if (((*i)->getWeight()>0)||(ignoreWeights)) {
      FeatureValue fv=(*(*i))(tokens, index,text);
      result.push_back(fv);
    }
  }
  return result;
}


////////////////////////////////////////////////////////////////////////////////
// 'operator()()' applies the FeatureValueExtractor algorithms to
// the vector<TokenDeco> and returns a vector<FeatureVector> that has the
// same length of the vector of tokens.  For each token, a feature
// vector will be computed and returned in the same order.
////////////////////////////////////////////////////////////////////////////////
vector<FeatureVector>
FeatureVectorValueExtractor::operator()(vector<TokenDeco>& tokens,
    const StringXML& text,const bool ignoreWeights) const {
  vector<FeatureVector> results;
  int j=0;
  for (vector<TokenDeco>::iterator i=tokens.begin(); i!=tokens.end();
      ++i) {
    results.push_back(this->operator()(tokens,i,text,ignoreWeights));
    j++;
  }
  return results;
}

// end of file: feature_extraction.cpp

