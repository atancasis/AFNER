////////////////////////////////////////////////////////////////////////////////
// Modified by Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Much of this code is adapted from YASMET by Franz Josef Och
// This file contains the interface for the class MaxEnt
// MaxEnt is a class for a maximum entropy classifier
////////////////////////////////////////////////////////////////////////////////
// Filename: maxent.h
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

#ifndef __maxent__
#define __maxent__

#include <fstream>
#include <cmath>  
#include <string> 
#include <utility>
#include <iostream>
#include <numeric>
#include <ext/hash_map>
#include "feature_extraction.h"
#include "xml_string.h"

using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
using namespace std; 
using namespace AF;

typedef pair<int,double> fea;
typedef vector<fea> vfea;

template<class T> ostream &
operator<<(ostream&out,const vector<T>&x) {
  copy(x.begin(),x.end(),ostream_iterator<T>(out," "));
  return out << endl;
}

struct Z {
  double k,q,l;
  Z(double a=0,double b=-1,double c=0):k(a),q(c),l((b<0)?1:log(b)) {};
};

struct expclass {
  double
  operator()(double x) {
    return exp(x);
  }
};

struct hash_str {
  size_t
  operator()(const string&s)const {
    return __gnu_cxx::hash<const char*>()(s.c_str());
  }
};

struct event {
  vector<double> Ny;
  size_t y;
  vector<vfea> f;
  vector<double> fs;

  vector<double>&
  computeProb(const vector<Z>&z,vector<double>&pr) const {
    vector<double>::iterator p=pr.begin(),pb=pr.begin(),pe=pr.end();
    for (vector<vfea>::const_iterator fi=f.begin(); fi!=f.end();
        ++fi, ++p) {
      *p=0.0;
      for (vfea::const_iterator j=fi->begin(); j!=fi->end(); ++j) {
        *p+=z[j->first].l*j->second;
      }
    }
    transform(pb,pe,pb,bind2nd(plus<double>(),-*max_element(pb,pe)));
    transform(pb,pe,pb,expclass());
    transform(pb,pe,pb,bind2nd(divides<double>(),accumulate(pb,pe,0.0)));
    return pr;
  }
};

////////////////////////////////////////////////////////////////////////////////
// MaxEnt Class
// The MaxEnt class is a machine learning classifier using Maximum Entropy
// The code was adapted from YASMET by Franz Josef Och
////////////////////////////////////////////////////////////////////////////////
class MaxEnt {
////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
public:
////////////////////////////////////////////////////////////////////////////////
// Constructor
// initialises variables, reads model
////////////////////////////////////////////////////////////////////////////////
  MaxEnt(const unsigned int numberClasses,StringXML modelFile);
////////////////////////////////////////////////////////////////////////////////
// classify accepts a FeatureVector and returns the category with the highest
// probability, i.e. the classification
////////////////////////////////////////////////////////////////////////////////
// function has been modified to check probabilites
////////////////////////////////////////////////////////////////////////////////
  //int
  vector <double>
  classify(const FeatureVector& features);     //should this be const?
////////////////////////////////////////////////////////////////////////////////
// 'printVector' prints a vector out for use as training data to the output
// stream given
////////////////////////////////////////////////////////////////////////////////
  void printVector(const FeatureVector& features,ostream out);
////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES AND MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
private:
////////////////////////////////////////////////////////////////////////////////
// 'read_model' reads a model from a given model file and inserts the values
// into the classifier's data members
////////////////////////////////////////////////////////////////////////////////
  void
  read_model(const StringXML& model_file);
////////////////////////////////////////////////////////////////////////////////
// 'processFeatureVector' reads a feature vector and modifies the classifier's
// data members accordingly
////////////////////////////////////////////////////////////////////////////////
  void
  processFeatureVector(const FeatureVector& features);
////////////////////////////////////////////////////////////////////////////////
// 'computeProbabilities' does the classification work.
////////////////////////////////////////////////////////////////////////////////
  vector<double>
  computeProbabilities();
////////////////////////////////////////////////////////////////////////////////
// Private Data Members - unsure of what they do exactly
////////////////////////////////////////////////////////////////////////////////
  vector<Z> z;
  vector<pair<string,double> > s2f;
  hash_map<string,int,hash_str> f2s;
  size_t C;
  event en;
  double F;
  bool lN; // length normalisation (arg)
  double dSmoothN;
};

#endif
