////////////////////////////////////////////////////////////////////////////////
// Alex Chilvers
// achilver@ics.mq.edu.au
// April 2006
////////////////////////////////////////////////////////////////////////////////
// Filename: features.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the feature detection functions, implemented as derived
// functions of the FeatureValueExtractor and FeatureVectorValueExtractor 
// classes.
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

#ifndef __feature_classes__
#define __feature_classes__

#include <vector>
#include <string>
#include <boost/regex.hpp>
#include "tokeniser.h"
#include "feature_extraction.h"
#include "list_handler.h"
#include "ner.h"
#include "suffixtree.h"
#include "xml_string.h"

using namespace std;
using namespace AF;

namespace AF {

////////////////////////////////////////////////////////////////////////////////
// The FrequencyHandler class handles the retrieval of frequency of token 
// occurence.
////////////////////////////////////////////////////////////////////////////////
class FrequencyHandler {
public:
  FrequencyHandler(const StringXML filename);

  FrequencyHandler();

  int
  getFrequency(const StringXML s, const int classification) const;

  double
  getProportion(const StringXML s, const int classification) const;

  int
  classCount() const;

private:
  // a vector containing the token frequencies per class, corresponding to index
  vector<map<StringXML,int> > _frequencies;
  // a vector containing the maximum token count per class, 
  // so a percentage can be given
  // e.g. a token freqency alone is arbitrary, but 200/2000 = 2/20
  vector<int> _max_counts;
};


////////////////////////////////////////////////////////////////////////////////
// 'operator()()' computes the FeatureValue.  It works on tokens and
// computes the value of a feature of the particular index. Each class is made
// and the operator function does the work.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Finds whether the first letter is a capital
////////////////////////////////////////////////////////////////////////////////
class InitCaps : public FeatureValueExtractor {
public:
  InitCaps(const int& context=0,const StringXML alias="");
  
  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;  
private:
}; // InitCaps

////////////////////////////////////////////////////////////////////////////////
// Finds whether entire word is capitalised
////////////////////////////////////////////////////////////////////////////////
class AllCaps : public FeatureValueExtractor {
public:
  AllCaps(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // AllCaps

////////////////////////////////////////////////////////////////////////////////
// Finds whether word is a mix of upper and lower case
////////////////////////////////////////////////////////////////////////////////
class MixedCaps : public FeatureValueExtractor {
public:
  MixedCaps(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
}; // MixedCaps

////////////////////////////////////////////////////////////////////////////////
// Finds whether token is an end of sentence character, ie. '.' or '!' or '?'
////////////////////////////////////////////////////////////////////////////////
class IsSentEnd : public FeatureValueExtractor {
public:
  IsSentEnd(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
     const vector<TokenDeco>::iterator index,
     const StringXML& text) const;
private:
}; // IsSentEnd


////////////////////////////////////////////////////////////////////////////////
// Finds whether token starts with cap and is followed by a period, eg. 'Mr''.'
////////////////////////////////////////////////////////////////////////////////
class InitCapPeriod : public FeatureValueExtractor {
public:
  InitCapPeriod(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // InitCapPeriod


////////////////////////////////////////////////////////////////////////////////
// Finds whether the word is one capital letter, eg. 'A' or 'I'
////////////////////////////////////////////////////////////////////////////////
class OneCap : public FeatureValueExtractor {
public:
  OneCap(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // OneCap

////////////////////////////////////////////////////////////////////////////////// Finds whether the word contains a digit
////////////////////////////////////////////////////////////////////////////////
class ContainDigit : public FeatureValueExtractor {
public:
  ContainDigit(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // ContainDigit


////////////////////////////////////////////////////////////////////////////////
// Finds whether the token is 2 digits, eg. '97' or '06'
////////////////////////////////////////////////////////////////////////////////
class TwoDigits : public FeatureValueExtractor {
public:
  TwoDigits(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // TwoDigits

////////////////////////////////////////////////////////////////////////////////
// as above, but 4 digits, eg. '1985'
////////////////////////////////////////////////////////////////////////////////
class FourDigits : public FeatureValueExtractor {
public:
  FourDigits(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // FourDigits

////////////////////////////////////////////////////////////////////////////////// Finds if word is a month name, eg 'November'
////////////////////////////////////////////////////////////////////////////////
class MonthName : public FeatureValueExtractor {
public:
  MonthName(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // MonthName

////////////////////////////////////////////////////////////////////////////////
// Finds if word is a day of the week, eg. 'monday'
////////////////////////////////////////////////////////////////////////////////
class DayOfTheWeek : public FeatureValueExtractor {
public:
  DayOfTheWeek(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // DayOfTheWeek

////////////////////////////////////////////////////////////////////////////////// Finds if word is a number word, eg. 'one', 'thousand' 
////////////////////////////////////////////////////////////////////////////////
class NumberString : public FeatureValueExtractor {
public:
  NumberString(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // NumberString

////////////////////////////////////////////////////////////////////////////////// Finds if word is preceded by a preposition (in a window of 4 tokens)
////////////////////////////////////////////////////////////////////////////////
class PrepPreceded : public FeatureValueExtractor {
public:
  PrepPreceded(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // PrepPreceded

////////////////////////////////////////////////////////////////////////////////// Finds if a token is capitalised every time it appears in the text
////////////////////////////////////////////////////////////////////////////////
class AlwaysCapped : public FeatureValueExtractor {
public:
  AlwaysCapped(const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
}; // AlwaysCapped

class FoundInList : public FeatureValueExtractor {
public:
  FoundInList(const ListHandler* lh, const int list, const int& context=0,
      const StringXML alias="");

  StringXML
  getName() const;

  StringXML
  getAlias() const;

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
  // the ListHandler used to find the token
  const ListHandler* _list_handler;
  // the index of the list to search
  const int _list;
};

class MatchRegex : public FeatureValueExtractor {
public:
  MatchRegex(const StringXML& regex_name,const boost::regex& regex,
      const int& context=0,const StringXML alias="");
  
  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;
private:
  StringXML _regex_name;
  const boost::regex _regex;
};

class PartMatch : public FeatureValueExtractor {
public:

////////////////////////////////////////////////////////////////////////////////
// info_name is the name of the information to look up in the token decorator
////////////////////////////////////////////////////////////////////////////////
  PartMatch(const StringXML& info_name, const int& context=0,
      const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
  const StringXML _info_name;
};

class PrevClass : public FeatureValueExtractor {
public:
  PrevClass(const int checkClass,const int& context=0,
      const StringXML alias="");
  
  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
  const int _checkClass;
};

class ProbClass : public FeatureValueExtractor {
public:
  ProbClass(const int classification,
      const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
  const int _classification;
};

class TokenFrequency : public FeatureValueExtractor {
public:
  TokenFrequency(const int classification,
      const FrequencyHandler* frequencies,      
      const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
  const int _classification;
  const FrequencyHandler* _frequencies;
};

class PrevTokenFrequency : public FeatureValueExtractor {
public:
  PrevTokenFrequency(const int classification,
      const FrequencyHandler* frequencies,
      const int& context=0,const StringXML alias="");

  FeatureValue
  operator()(vector<TokenDeco>& tokens,
      const vector<TokenDeco>::iterator index,
      const StringXML& text) const;

private:
  const int _classification;
  const FrequencyHandler* _frequencies;
};

}

#endif
