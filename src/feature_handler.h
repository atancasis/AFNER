////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: feature_handler.h
////////////////////////////////////////////////////////////////////////////////
// This file contains the definition of the Feature_Handler class, which allows
// for features to be selected for use at run-time
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

#ifndef __feature_handler__
#define __feature_handler__

#include <vector>
#include <string>
#include <boost/regex.hpp>
#include "regex_handler.h"
#include "list_handler.h"
#include "entity_tag.h"
#include "feature_extraction.h"
#include "feature_functions.h"
#include "xml_string.h"

using namespace std;
using namespace boost;

namespace AF {

class FeatureHandler : public FeatureVectorValueExtractor{
  public:
    FeatureHandler(const ListHandler& lh,const RegexHandler& rh,
      const EntityTagset& tset,const StringXML& regex_input,const int context,
      const double default_weight,const StringXML& freqFile="",
      const StringXML& prevFreqFile="");
  protected:
  private:
    virtual vector<FeatureValueExtractor*>
    GetFeatureDetectionAlgorithms(const ListHandler& lh,
      const RegexHandler& rh, const EntityTagset& tset,
      const vector<boost::regex>& regex_list,
      const int context,const double default_weight,
      const StringXML& freqFile,const StringXML& prevFreqFile) const;
};

}

#endif

