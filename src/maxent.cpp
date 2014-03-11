////////////////////////////////////////////////////////////////////////////////
// Modified by Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Much of this code is adapted from YASMET by Franz Josef Och
// This file contains the implementation of the class MaxEnt
// MaxEnt is a class for a maximum entropy classifier
////////////////////////////////////////////////////////////////////////////////
// Filename: maxent.cpp
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

#include <fstream>
#include <cmath> 
#include <string>
#include <utility>
#include <iostream>
#include <numeric>
#include <ext/hash_map>
#include <sstream>
#include "maxent.h"

using namespace std; 

typedef pair<int,double> fea;
typedef vector<fea> vfea;

////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// MaxEnt Constructor
// initialises variables, reads in model
// Takes number of classes (integer) and model file (StringXML)
////////////////////////////////////////////////////////////////////////////////
MaxEnt::MaxEnt(const unsigned int numberClasses,StringXML modelFile){
  C=numberClasses;
  F=0.0;
  lN=false; // length normalisation (arg)
  dSmoothN=0.0;
  read_model(modelFile);
}

////////////////////////////////////////////////////////////////////////////////
// MaxEnt classify
// given a token and a function to read the data with
// returns the classification
////////////////////////////////////////////////////////////////////////////////
// function has been modified to check probabilities
////////////////////////////////////////////////////////////////////////////////
//int
vector<double>
MaxEnt::classify(const FeatureVector& features) {
  processFeatureVector(features);
  vector<double> results = computeProbabilities();
  // loop for debugging
  for (vector<double>::const_iterator ite=results.begin();ite!=results.end();ite++){
  }
  vector<double>::const_iterator max=max_element(results.begin(),results.end());
  return results;
}
////////////////////////////////////////////////////////////////////////////////
// 'printVector' prints a vector out for use as training data to the output
// stream given
////////////////////////////////////////////////////////////////////////////////
void
MaxEnt::printVector(const FeatureVector& features,ostream out){
    // fill this function
    // problem - the classification needs to be known
    // needs to be in the format:
    // class @ @ weight feature1 value1 feature2 value2 # @ weight feature1 etc
    // perhaps function should accept a vector of tokens (or at least classes
    // and a vector of FeatureVectors ??
  }
////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// 'processFeatureVector' reads a feature vector and modifies the classifier's
// data members accordingly
////////////////////////////////////////////////////////////////////////////////
void
MaxEnt::processFeatureVector(const FeatureVector& features){  
//  Class=2; // what does this do? Need to be there?
  en.f.resize(C);
  en.fs.resize(C);
  en.Ny.resize(C);
  size_t c=0; // class
  en.Ny[c]=0.0;         // strength of class (first num after @)
  // for each feature
  for (FeatureVector::const_iterator feat = features.begin();
      feat!=features.end();feat++)
  {
    // these loops may have to be reversed !!
    // the classes may need to be looped over first, and features within
    // instead of classes within feature loop
    StringXML feature;
    double value = feat->getValue();
    // for each class
    for (unsigned int i=0;i<C;i++){
      stringstream ss;
      ss << "cat" << i << "_" << feat->getFeature();
      feature = ss.str();
      if (value!=0) {
      // set values
      if (f2s.count(feature)!=0) { // check whether feature exists
        (s2f)[f2s[feature]].second+=(en.Ny[i]!=0);
        en.f[i].push_back(make_pair(f2s[feature], value));
        en.fs[i]+=value;
        F=max(F, en.fs[i]);
      }
      }
    }
  }
}
  
////////////////////////////////////////////////////////////////////////////////
// Reads a model given the data members and model file
// Taken from YASMET
// This function is private and is called by the MaxEnt constructor.  Its inner 
// workings do not need to be known
////////////////////////////////////////////////////////////////////////////////
void
MaxEnt::read_model(const StringXML& model_file) {
  // initialise
  s2f.push_back(pair<string,double>("@@@CORRECTIVE-FEATURE@@@",0.0));
  (f2s)["@@@CORRECTIVE-FEATURE@@@"]=0;
  z.push_back(Z());

  // read in model
  ifstream muf(model_file.c_str()); // input stream
  string s;
  double d;
  while(muf>>s>>d) {
    size_t p=(!f2s.count(s))?((f2s)[s]=s2f.size()):((f2s)[s]);
    Z k(0,d,0);
    pair<string,double> sd(s,0.0);

    if (p<s2f.size()) {
      (s2f)[p]=sd;
    }
    else {
      s2f.push_back(sd);
    }

    if (p<z.size()) {
      (z)[p]=k;
    }
    else {
      z.push_back(k);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// MaxEnt computeProbabilities - private
// Private function to classify a token after the data has been read in
////////////////////////////////////////////////////////////////////////////////
vector<double>
MaxEnt::computeProbabilities() {
  for (size_t c=0; c<C; ++c) {
    if (lN) {
      for (size_t k=0; k<en.f[c].size(); ++k) {
        en.f[c][k].second/=en.fs[c];
      }
    }
    else {
      en.f[c].push_back(make_pair(0,F-en.fs[c]));
    }
  }
  if (lN) {
    F=1.0;
  }
  const event& ei=en;
  size_t I=s2f.size();
  z.resize(I);
  for (size_t y=0; y<ei.Ny.size(); ++y) {
    for (size_t j=0; j<ei.f[y].size(); ++j) {
      z[ei.f[y][j].first].k+=1.0*ei.Ny[y]*ei.f[y][j].second;
    }
  }
  for (size_t i=0; i<I; ++i) {
    z[i].q=0;
  }
  vector<double> p(C);
  ei.computeProb(z,p);
  return p;
}
