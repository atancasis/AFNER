///////////////////////////////////////////////////////////////////////////////
// Alex Chilvers
// achilver@ics.mq.edu.au
// April 2006
////////////////////////////////////////////////////////////////////////////////
// Filename: features.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the operator functions for the 
// feature detection classes.
////////////////////////////////////////////////////////////////////////////////
// Modified: Daniel Smith, June 2006
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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include "tokeniser.h"
#include "feature_extraction.h"
#include "feature_functions.h"
#include "list_handler.h"
#include "entity_tag.h"

using namespace std;
using namespace AF;


////////////////////////////////////////////////////////////////////////////////
// incrementIterator takes a vector<TokenDeco>, an iterator to a position in the
// vector, and an integer to increment the iterator by and increments
// the iterator, returning true if the operation is successful, false
// otherwise.  If unsuccessful, the iterator will point either to the
// end or beginning of the vector.
////////////////////////////////////////////////////////////////////////////////
bool
incrementIterator(const vector<TokenDeco>& vec,
    vector<TokenDeco>::iterator& ite,const int& amount) {
  // if positive amount to increment
  if (amount>=0) {
    // increment required amount
    for (int i=0;i<amount;i++){
      ite++;
      // if not finished and at end, return false
      if (ite==vec.end()&&(amount-i)>1) {
        return false;
      }
    }
  }
  // if negative increment
  else {
    if (ite==vec.begin()) {
      return false;
    }
    for (int i=0;i>amount;i--){
      // decrement iterator required amount
      ite--;
      // if at beginning and still more to go
      if (ite==vec.begin()&&i>amount+1) {
        return false;
      }
    }
  }
  // if reach here, done properly
  return true;
}
////////////////////////////////////////////////////////////////////////////////
// 'operator()()' computes the FeatureValue.  It works on tokens and
// computes the value of a feature of the particular index.
// tokens is a vector of all tokens, index is an iterator to the current for
// which a value is to be computed, and text is the original string.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Finds whether the first letter is a capital
////////////////////////////////////////////////////////////////////////////////

InitCaps::InitCaps(const int& context,
    const StringXML alias) 
    : FeatureValueExtractor("InitCaps",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
InitCaps::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  // the name of the info stored in the token
  StringXML infoName = "initCaps";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute the value if it has not been computed
    if (!checkIndex->getInfo(infoName,value)) {
      //Get the string of the token
      StringXML tokenString = checkIndex->getString(text);
      //Check first character
      if (isupper(tokenString.at(0))) {
        value = 1.0;
      }
      // set the information in the token
      checkIndex->setInfo(infoName,value);
    }
  }
  //Return the name & value pair
  FeatureValue res(_alias,value);
  return res;
}

AllCaps::AllCaps(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("AllCaps",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
AllCaps::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 1.0;
  StringXML name = "allCaps";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute value if not already computed
    if (!checkIndex->getInfo(name,value)){
      StringXML tokenString = checkIndex->getString(text);
      //Check all characters, until lower case is found
      for (unsigned int i = 0; i < tokenString.length(); i++){
        if (islower(tokenString.at(i))){
          value = 0.0;
          break;
        }
      }
      // set the value for future reference
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}


MixedCaps::MixedCaps(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("MixedCaps",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
MixedCaps::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "mixedCaps";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)){
      StringXML tokenString = checkIndex->getString(text);
      //Stores whether each case has been encountered yet
      bool upcnt = 0;
      bool lowcnt = 0;
      for (unsigned int i = 0; i < tokenString.length(); i++) {
        //case is mixed if a lower is encountered, and upper has already been 
        //seen
        if (islower(tokenString.at(i))) {
          if (upcnt) {
            value = 1.0;
            break;
          }
          lowcnt = 1;
        }
        //case is mixed if a upper is encountered, and lower has already been 
        //seen
        else {
          if (lowcnt){
            value = 1.0;
            break;
          }
          upcnt = 1;
        }
      }
      // set for future reference
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

IsSentEnd::IsSentEnd(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("IsSentEnd",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
IsSentEnd::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "isSentEnd";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)){
      StringXML tokenString = checkIndex->getString(text);
      //Check for match
      if (tokenString == "." || tokenString == "!" || tokenString == "?"){
        value = 1.0;
      }
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

InitCapPeriod::InitCapPeriod(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("InitCapPeriod",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
InitCapPeriod::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "initCapPeriod";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      vector<TokenDeco>::iterator next_index = checkIndex;
      next_index++;
      //Check the next token, and the first character of the current token
      if (next_index != tokens.end()) {
        StringXML nextString = next_index->getString(text);
        if (isupper(tokenString.at(0)) && nextString == ".")
          value = 1.0;
      }
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

OneCap::OneCap(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("OneCap",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
OneCap::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "oneCap";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = index->getString(text);
      //Check the 2 conditions  
      if (tokenString.length() == 1 && isupper(tokenString.at(0)))
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

ContainDigit::ContainDigit(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("ContainDigit",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
ContainDigit::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "containDigit";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Search string for a digit   
      for (unsigned int i = 0; i < tokenString.length(); i++)
        if (isdigit(tokenString.at(i))) {
          value = 1.0;
        break;
      }
      index->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

TwoDigits::TwoDigits(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("TwoDigits",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
TwoDigits::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "twoDigits";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Check 3 conditions: length is 2, and both characters are digits   
      if (tokenString.length() == 2 && isdigit(tokenString.at(0))
          && isdigit(tokenString.at(1)))
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

FourDigits::FourDigits(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("FourDigits",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
FourDigits::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "fourDigits";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Check 5 conditions: length is 4, and all characters are digits  
      if (tokenString.length() == 4 && isdigit(tokenString.at(0))
          && isdigit(tokenString.at(1))
          && isdigit(tokenString.at(2)) && isdigit(tokenString.at(3)))
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

MonthName::MonthName(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("MonthName",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
MonthName::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "monthName";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Make case a non-issue
      tokenString.at(0) = toupper(tokenString.at(0));
      //Check for any matches
      if (tokenString == "January" || tokenString == "February"
          || tokenString == "March" || tokenString == "April"
          || tokenString == "May" || tokenString == "June"
          || tokenString == "July" || tokenString == "August"
          || tokenString == "September" || tokenString == "October"
          || tokenString == "November" || tokenString == "December")
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

DayOfTheWeek::DayOfTheWeek(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("DayOfTheWeek",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
DayOfTheWeek::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "dayOfTheWeek";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Make case a non-issue
      tokenString.at(0) = toupper(tokenString.at(0));
      //Check for any matches 
      if (tokenString == "Sunday" || tokenString == "Monday"
          || tokenString == "Tuesday" || tokenString == "Wednesday"
          || tokenString == "Thursday" || tokenString == "Friday"
          || tokenString == "Saturday")
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

NumberString::NumberString(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("NumberString",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
NumberString::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "numberString";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      //Make case a non-issue
      tokenString.at(0) = toupper(tokenString.at(0));
      //Check for any matches   
      if (tokenString == "One" || tokenString == "Two"
          || tokenString == "Three" || tokenString == "Four"
          || tokenString == "Five" || tokenString == "Six"
          || tokenString == "Seven" || tokenString == "Eight"
          || tokenString == "Nine" || tokenString == "Ten"
          || tokenString == "Eleven" || tokenString == "Twelve"
          || tokenString == "Thirteen" || tokenString == "Fourteen"
          || tokenString == "Fifteen" || tokenString == "Sixteen"
          || tokenString == "Seventeen" || tokenString == "Eighteen"
          || tokenString == "Nineteen" || tokenString == "Twenty"
          || tokenString == "Thirty" || tokenString == "Fourty"
          || tokenString == "Fifty" || tokenString == "Sixty"
          || tokenString == "Seventy" || tokenString == "Eighty"
          || tokenString == "Ninety" || tokenString == "Hundred"
          || tokenString == "Thousand" || tokenString == "Million"
          || tokenString == "Billion" || tokenString == "Trillion"
          || tokenString == "Quadrillion" || tokenString == "Quintillion")
        value = 1.0;
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}


PrepPreceded::PrepPreceded(const int& context,
    const StringXML alias)
    : FeatureValueExtractor("PrepPreceded",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
PrepPreceded::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value = 0.0;
  StringXML name = "prepPreceded";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      StringXML tokenString = checkIndex->getString(text);
      vector<TokenDeco>::iterator check_index = checkIndex;
      //Check the previous 4 tokens (if possible)
      for (unsigned int i = 0; i < 4; i++){
        if (check_index == tokens.begin())
          break;
        check_index--;
        StringXML checkString = check_index->getString(text);
        //Make case a non-issue
        checkString.at(0) = toupper(checkString.at(0));
        //Check for matches
        if (checkString == "In" || checkString == "On" || checkString == "At"){
          value = 1.0;
          break;
        }
      }
      checkIndex->setInfo(name,value);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

AlwaysCapped::AlwaysCapped(const int& context,const StringXML alias)
    : FeatureValueExtractor("AlwaysCapped",context,alias) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _context;
  }
  else {
    ss << _feature_name << _context;
  }
  _alias = ss.str();
}

FeatureValue
AlwaysCapped::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value=0.0;
  StringXML name="alwaysCapped";
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment/decrement
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // compute if not already computed
    if (!checkIndex->getInfo(name,value)) {
      // check status of current token, then check sequentially, adding
      // tokens to change to list to modify later
      double cV = 1.0;
      // compute InitCaps if not already done
      if (!checkIndex->getInfo("initCaps",cV)) {
        InitCaps IC;
        FeatureValue fv = IC(tokens,checkIndex,text);
        cV = fv.getValue();
      }
      // initialise to capped status of index
      bool foundSmall = (cV==0.0);
      // a vector to store terators to the same tokens
      vector<vector<TokenDeco>::iterator> sameToks;
      // for each token
      sameToks.push_back(checkIndex);
      for (vector<TokenDeco>::iterator ite=tokens.begin();
            ite!=tokens.end()&&!foundSmall;ite++) {
        // if words are the same
        StringXML a = checkIndex->getString(text);
        StringXML b = ite->getString(text);
        if (a.length()==b.length()){
          for (unsigned int i=0;i<a.length();i++){
            a[i] = tolower(a[i]);
            b[i] = tolower(a[i]);
          }
        }
        if (a==b) {
          // if value has been computed, exit loop store all
          if (ite->getInfo(name,value)) {
            break;
          }
          // otherwise add to list to update
          else{
            sameToks.push_back(ite);
          }
          double checkVal=1.0;
          // compute if value for current has not been computed
          if (!ite->getInfo("initCaps",checkVal)) {
            InitCaps IC;
            FeatureValue fv = IC(tokens,ite,text);
            checkVal = fv.getValue();
          }
          // check if current is capitalised, true if is lower
          foundSmall = (checkVal==0.0);
        }
      }
      if (foundSmall)
        value=0.0;
      for (vector<vector<TokenDeco>::iterator>::const_iterator ite=sameToks.begin();
            ite!=sameToks.end();ite++){
        (**ite).setInfo(name,value);
      }
    }
  }
  // value has been computed at this point
  FeatureValue res(_alias,value);
  return res;
}

FoundInList::FoundInList(const ListHandler* lh, const int list,
    const int& context,const StringXML alias)
    : FeatureValueExtractor("FoundInList",context,alias),_list_handler(lh),
    _list(list) {
  stringstream ss;
  ss << _alias << _list << context;
  _alias = ss.str();
  stringstream ss1;
  ss1 << _feature_name << _list;
  //_feature_name = ss1.str();
}

StringXML
FoundInList::getName() const {
  stringstream ss;
  ss << _feature_name << _list;
  return ss.str();
}

StringXML
FoundInList::getAlias() const {
  stringstream ss;
  ss << _alias << _list;
  return ss.str();
}

FeatureValue
FoundInList::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  double value=0.0;
  stringstream ss;
  ss << "FoundList" << _list;
  StringXML name = ss.str();
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment compute value for token indicated by context
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
    // if search not done
    if (!checkIndex->getInfo(name,value)) {
      // for each list in the ListHandler, set not found
      for (int i=0;i<_list_handler->ListCount();i++) {
        stringstream ss2;
        ss2 << "FoundList" << i;
        checkIndex->setInfo(ss2.str(),0.0);
      }
      // perform the search, update TokenDeco
      vector<int> results = 
          _list_handler->FindString(checkIndex->getString(text));
      for (vector<int>::const_iterator ite=results.begin();
          ite!=results.end();ite++) {
        stringstream ss2;
        ss2 << "FoundList" << *ite;
        checkIndex->setInfo(ss2.str(),1.0);
      } // finished updating decorator
    } // end if search not done
    checkIndex->getInfo(name,value);
  } // end if able to increment
  FeatureValue res(_alias,value);
  return res;
}

MatchRegex::MatchRegex(const StringXML& regex_name,const boost::regex& regex,
    const int& context,const StringXML alias)
    : FeatureValueExtractor("MatchRegex",context,alias),
    _regex_name(regex_name),_regex(regex) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _regex_name << "_" << context;
  }
  else {
    ss << _feature_name << _regex_name << "_" << context;
  }
  _alias = ss.str();
}

FeatureValue
MatchRegex::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  StringXML name = "mRX" + _regex_name;
  double value = 0.0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if can increment and not at end
  if (incrementIterator(tokens,checkIndex,_context)&&
      checkIndex!=tokens.end()) {
    // if not found, compute and add to decorator
    if (!checkIndex->getInfo(name,value)) {
       StringXML tok = checkIndex->getString(text);
       cmatch what;
       // if matches the regular expression
       if (regex_match(tok.c_str(),what,_regex)) {
         // set the value and record in deco
         value = 1.0;
         checkIndex->setInfo(name,value);
       }
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

PartMatch::PartMatch(const StringXML& info_name, const int& context,
    const StringXML alias)
    : FeatureValueExtractor("PartMatch",context,alias)
        ,_info_name(info_name) {
  stringstream ss;
  if (_alias!="") {
    ss << _alias << _info_name << "_" << _context;
  }
  else {
    ss << _feature_name << _info_name << "_" << _context;
  }
  _alias = ss.str();
}

FeatureValue
PartMatch::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  StringXML name = _info_name;
  double value = 0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if can increment and not at end
  if (incrementIterator(tokens,checkIndex,_context)&&
      checkIndex!=tokens.end()) {
    if (checkIndex->getInfo(_info_name,value)) {
      value=1.0;
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

PrevClass::PrevClass(const int checkClass,const int& context,
    const StringXML alias) 
  : FeatureValueExtractor("PrevClass",context,alias),_checkClass(checkClass) {
  stringstream ss1;
  if (_alias!="") {
    ss1 << _alias << _checkClass << "_" << _context;
  }
  else {
    ss1 << _feature_name << _checkClass << "_" << _context;
  }
  _alias = ss1.str();
}

FeatureValue
PrevClass::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  StringXML name;
  stringstream ss2;
  ss2 << "maxProb_" << _context;
  name = ss2.str();

  double value = 0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment compute value for token indicated
  if (incrementIterator(tokens,checkIndex,_context)&&checkIndex!=tokens.end()) {
      // go back one again to token to check
      if (checkIndex != tokens.begin()) {
        checkIndex--;
        // if has been classified
        // get the previous class
        if (checkIndex->getInfo("maxProb",value)) {
          int prevClass = static_cast<int>(value);
          // if the class is what we are looking for, then set value
          if (prevClass==_checkClass) {
            stringstream ss;
            ss << "prob" << prevClass;
            // get the probability
            name = ss.str();
            checkIndex->getInfo(name,value);
          }
          // otherwise, set the value (prob) to 0
          else {
            value = 0;
          }
          stringstream ss1;
	  if (_alias != "")
	    name = _alias;
	  else {
            ss1 << _feature_name << _checkClass << "_" << _context;
	    name = ss1.str();
	  }   
        }
      }
  }
  FeatureValue res(name,value);
  return res;
}

ProbClass::ProbClass(const int classification,
    const int& context,const StringXML alias)
    : FeatureValueExtractor("ProbClass",context,alias),
    _classification(classification) {
  stringstream ss1;
  if (_alias!="") {
    ss1 << _alias << _classification << "_" << _context;
  }
  else {
    ss1 << _feature_name << _classification << "_" << _context;
  }
  _alias = ss1.str();
}

FeatureValue
ProbClass::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  // retrieve the probability
  stringstream ss;
  ss << "prob" << _classification;
  StringXML name = ss.str();
  double value = 0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment compute value for token indicated
  if (incrementIterator(tokens,checkIndex,_context)&&
      checkIndex!=tokens.end()) {
     // get the classification
     if (checkIndex != tokens.begin()) {
       checkIndex--;
       checkIndex->getInfo(name,value);
     }
  }
  FeatureValue res(_alias,value);
  return res; 
}

TokenFrequency::TokenFrequency(const int classification,
    const FrequencyHandler* frequencies,
    const int& context,const StringXML alias)
    : FeatureValueExtractor("TokenFrequency",context,alias),
    _classification(classification),_frequencies(frequencies) {
  stringstream ss1;
  if (_alias!="") {
    ss1 << _alias << _classification << "_" << _context;
  }
  else {
    ss1 << _feature_name << _classification << "_" << _context;
  }
  _alias = ss1.str();
}

FeatureValue
TokenFrequency::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  // retrieve the probability
  stringstream ss;
  ss << "prob" << _classification;
  StringXML name = ss.str();
  double value = 0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment compute value for token indicated
  if (incrementIterator(tokens,checkIndex,_context)&&
      checkIndex!=tokens.end()) {
    // get the text
    if (checkIndex != tokens.begin()) {
      StringXML s = checkIndex->getString(text);
      value = _frequencies->getProportion(s,_classification);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

PrevTokenFrequency::PrevTokenFrequency(const int classification,
    const FrequencyHandler* frequencies,
    const int& context,const StringXML alias)
    : FeatureValueExtractor("PrevTokenFrequency",context,alias),
    _classification(classification),_frequencies(frequencies) {
  stringstream ss1;
  if (_alias!="") {
    ss1 << _alias << _classification << "_" << _context;
  }
  else {
    ss1 << _feature_name << _classification << "_" << _context;
  }
  _alias = ss1.str();
}

FeatureValue
PrevTokenFrequency::operator()(vector<TokenDeco>& tokens,
    const vector<TokenDeco>::iterator index,
    const StringXML& text) const {
  // retrieve the probability
  stringstream ss;
  ss << "prob" << _classification;
  StringXML name = ss.str();
  double value = 0;
  vector<TokenDeco>::iterator checkIndex=index;
  // if able to increment compute value for token indicated
  if (incrementIterator(tokens,checkIndex,_context)&&
      checkIndex!=tokens.end()) {
    // get the text
    if (checkIndex != tokens.begin()) {
      checkIndex--;
      StringXML s = checkIndex->getString(text);
      value = _frequencies->getProportion(s,_classification);
    }
  }
  FeatureValue res(_alias,value);
  return res;
}

////////////////////////////////////////////////////////////////////////////////
// Constructor, accepts a file containing the frequencies of token occurrence
////////////////////////////////////////////////////////////////////////////////
FrequencyHandler::FrequencyHandler(const StringXML filename) {
  vector<map<StringXML,int> > f;
  vector<int> v;
  _max_counts = v;
  _frequencies = f;
  // initialise _frequencies and _max_counts
  ifstream ifs;
  ifs.open(filename.c_str());
  StringXML line="";
  int maxCount=1;
  int currentTag=-1;
  map<StringXML,int>* currFreq = new map<StringXML,int>();
  while(getline(ifs,line)) {
    // if line is not empty
    if (line.size() > 0) {
      istringstream is(line);
      StringXML first="";
      is >> first;
      // if new tag
      if (first=="####") {
        StringXML second="";
        is >> second;
        // create new map, reset max count
        if (currentTag>=0) {
          //_frequencies[currentTag] = *currFreq;
          _frequencies.push_back(*currFreq);
          //_max_counts[currentTag] = maxCount;
          _max_counts.push_back(currentTag);
        }
        // set the current tag
        is >> currentTag;
        // reset count, map of frequencies
        maxCount = 1;
        currFreq = new map<StringXML,int>();
      } // end if new tag
      else {
        int second=0;
        is >> second;
        // if not a new tag
        currFreq->insert(make_pair(first,second));
        if (second>maxCount) {
          maxCount = second;
        }
      } // end not new tag
    } // end if line not empty
  } // end while loop
  if (currentTag>=0) {
    _frequencies.push_back(*currFreq);
    //_frequencies[currentTag] = *currFreq;
    //_max_counts[currentTag] = maxCount;
    _max_counts.push_back(maxCount);
  }
}

FrequencyHandler::FrequencyHandler() {
}

int
FrequencyHandler::getFrequency(const StringXML s, 
    const int classification) const {
  int value = 0;
  map<StringXML,int>::const_iterator iter = 
      _frequencies[classification].find(s);
  if (iter!=_frequencies[classification].end()) {
    value = iter->second;
  }
  return value;
}

double
FrequencyHandler::getProportion(const StringXML s, 
    const int classification) const {
  double value = 0.0;
  map<StringXML,int>::const_iterator iter = 
      _frequencies[classification].find(s);
  if (iter!=_frequencies[classification].end()) {
    value = iter->second;
  }
  if (_max_counts[classification]>0)
    value = (value/_max_counts[classification]);
  return value;
}

int
FrequencyHandler::classCount() const {
  return _max_counts.size();
}
