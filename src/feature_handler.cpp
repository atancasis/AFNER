////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: feature_handler.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the Feature_Handler class, 
// which facilitates the selection of features for use at run-time
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
#include <string>
#include <boost/regex.hpp>
#include "regex_handler.h"
#include "list_handler.h"
#include "entity_tag.h"
#include "feature_extraction.h"
#include "feature_functions.h"
#include "xml_string.h"
#include "feature_handler.h"

using namespace std;
using namespace AF;
using namespace boost;

////////////////////////////////////////////////////////////////////////////////
// Constructor, initialises the algorithms
////////////////////////////////////////////////////////////////////////////////
FeatureHandler::FeatureHandler(const ListHandler& lh,const RegexHandler& rh,
    const EntityTagset& tset,const StringXML& regex_input,const int context,
    const double default_weight,const StringXML& freqFile,
    const StringXML& prevFreqFile) {
  vector<boost::regex> regex_list;
  // if a filename given, get the regular expressions
  if (regex_input!="") {
    ifstream inFile;
    inFile.open(regex_input.c_str());
    if (inFile) {
      StringXML line="";
      while (getline(inFile,line)) {
        // if line not empty
        if (line.size() > 0) {
          // if line not a comment
          if (line.substr(0,1) != "#") {
            regex r(line,regex::perl|regex::icase);
            regex_list.push_back(r);
          }
        }
      }
    }
    inFile.close();
  }
  _featureAlgorithms = 
      GetFeatureDetectionAlgorithms(lh,rh,tset,regex_list,context,
      default_weight,freqFile,prevFreqFile);
}

////////////////////////////////////////////////////////////////////////////////
// GetFeatureDetectionAlgorithms creates the FeatureValueExtractor objects for
// use by the FeatureVectorValueExtractor.
////////////////////////////////////////////////////////////////////////////////
vector<FeatureValueExtractor*>
FeatureHandler::GetFeatureDetectionAlgorithms(const ListHandler& lh,
    const RegexHandler& rh,const EntityTagset& tset,
    const vector<boost::regex>& regex_list,const int context,
    const double default_weight,const StringXML& freqFile,
    const StringXML& prevFreqFile) const {
  vector<FeatureValueExtractor*> vec;
  // for the contextual range before and after token
  for (int i=(-1*context);i<(context+1);i++) {
    vec.push_back(new InitCaps(i,"IC"));
    vec.push_back(new AllCaps(i,"AC"));
    vec.push_back(new MixedCaps(i,"MC"));
    vec.push_back(new IsSentEnd(i,"ISE"));
    vec.push_back(new InitCapPeriod(i,"ICP"));
    vec.push_back(new OneCap(i,"OC"));
    vec.push_back(new ContainDigit(i,"CD"));
    vec.push_back(new TwoDigits(i,"TD"));
    vec.push_back(new FourDigits(i,"FD"));
    vec.push_back(new MonthName(i,"MN"));
    vec.push_back(new DayOfTheWeek(i,"DW"));
    vec.push_back(new NumberString(i,"NS"));
    vec.push_back(new PrepPreceded(i,"PP"));
    vec.push_back(new AlwaysCapped(i,"AWC"));
    // list features
    for (int j=0;j<lh.ListCount();j++) {
      stringstream ss;
      ss << "list" << j;
      // part of a larger match
      // matching tokens are marked with "list$"
      // where $ is the index corresponding to the matching list
      vec.push_back(new PartMatch(ss.str(),i,"PM"));
      // word matches alone
      vec.push_back(new FoundInList(&lh,j,i,"LS"));
    }

    // for each regex in the feature specific list regex_list
    int j=0;
    for (vector<boost::regex>::const_iterator ite=regex_list.begin();
        ite!=regex_list.end();ite++) {
      stringstream ss;
      ss << j;
      // add a feature for the match
      vec.push_back(new MatchRegex(ss.str(),*ite,i,"MRX"));
      j++;
    }
    
    // for each regex in the RegexHandler
    j = 0;
    for (map<regex,EntityTag>::const_iterator ite=rh.begin();
        ite!=rh.end();ite++) {
      // matching tokens are marked with "regex$"
      // where $ is the index corresponding to the matching regex
      stringstream ss;
      ss << "regex" << j;
      // add as a feature
      vec.push_back(new PartMatch(ss.str(),i,"PM"));
      j++;
    }
    if (freqFile!="") {
      FrequencyHandler* fh = new FrequencyHandler(freqFile);
      // for each class recorded in the frequency file
      for (int cls=0;cls<fh->classCount();cls++) {
        vec.push_back(new TokenFrequency(cls,fh,i,"TF"));
      }
    }
    if (prevFreqFile!="") {
      FrequencyHandler* pfh =  new FrequencyHandler(prevFreqFile);
      // for each class recorded in the frequency file
      for (int cls=0;cls<pfh->classCount();cls++) {
        vec.push_back(new PrevTokenFrequency(cls,pfh,i,"PTF"));
      }
    }
  } // end contextual features
  // for each class
  for (int c=0;c<tset.classCount();c++) {
      vec.push_back(new PrevClass(c,0,"PC"));
      vec.push_back(new ProbClass(c,0,"PRC"));
  }

  // set the default weight of the features
  for (vector<FeatureValueExtractor*>::const_iterator ite=vec.begin();
      ite!=vec.end();ite++) {
    (*ite)->setWeight(default_weight);
  }
  return vec;
}
