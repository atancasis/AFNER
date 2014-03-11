////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Filename: ner.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the NamedEntity and NEDeco classes 
// as well as functions for printing of training data, preparation of lists etc
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
  
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include <dirent.h>
#include <boost/regex.hpp>
//#include <mcheck.h>
#include "feature_extraction.h"
#include "feature_functions.h"
#include "feature_handler.h"
#include "maxent.h"
#include "ner.h"
#include "iterator.h"
#include "tokenDeco.h"
#include "list_handler.h"
#include "entity_tag.h"
#include "regex_handler.h"

using namespace std;
using namespace boost;
using namespace ns_suffixtree;
using namespace AF;

void markTokens(vector<vector<TokenDeco>::iterator>& toks,
    const StringXML& info,const StringXML& text);

////////////////////////////////////////////////////////////////////////////////
// 'extractType' Extracts the type as a string from an XML tag.
////////////////////////////////////////////////////////////////////////////////
StringXML extractType(StringXML& token) {
  regex typex("(=\"|')(.+?)(\"|'>)");
  match_results<StringXML::const_iterator> what;
  match_flag_type flags = match_default;
  StringXML::const_iterator begin, end;
  begin = token.begin();
  end = token.end();
  regex_search(begin,end,what,typex,flags);   // extract the type
  StringXML type(what[2].first,what[2].second);        // type is group 2
  return type;
}

////////////////////////////////////////////////////////////////////////////////
//'tokeniseWithNEInfo' returns a vector of tokens that can be found between
// begin and end, taking into account XML tags, but skipping them
////////////////////////////////////////////////////////////////////////////////
vector<TokenDeco>
AF::tokeniseWithNEInfo(const StringXML::const_iterator begin,
    const StringXML::const_iterator end,const EntityTagset* tset) {
  vector<TokenDeco> tokens;
  // Create a token to add to the vector
  StringXML::const_iterator i = begin;
  Token token;
  // To store the class of the next token
  int currentClass = tset->outClass();
  // To store whether currently at the first token in a tag
  bool beginTag = true;
  // While there are still tokens to get, get the token, not skipping tags
  StringXML tagText = "";
  EntityTag t;
  while (getToken(i,end,token,i-begin,false)) {
    StringXML tokenText(token.getBeginIterator(begin),
        token.getEndIterator(begin));
    regex startex("(<\\w.+?>)");     // matches start tag
    regex endex("(</\\w.+?>)");    // matches end tag
    cmatch what;
    // if token is a start tag
    if(regex_match(tokenText.c_str(), what, startex)) {
      // work out what entity type it is
      StringXML orig(begin,end);
      StringXML tokStr = token.getString(orig);
      tagText = extractType(tokStr);
      beginTag = true;
      t.assign(tokenText);
      // set class of next token
      currentClass = tset->getClassification(t,beginTag);
    }
    else if (regex_match(tokenText.c_str(), what, endex)) {
      // reset class of next token if end token
      currentClass = tset->outClass();
      tagText = "";
    }
    else {      // token is not a tag
      TokenDeco neToken(&token);
      // set the probabilities - max class
      neToken.setInfo("maxProb",currentClass);
      // for each class, set the probability to either 1 or 0
      for (int i=0;i<tset->classCount();i++) {
        stringstream ss;
        ss << "prob" << i;
        // if not at current class, set to 0, else 1
        if (i!=currentClass) {
          neToken.setInfo(ss.str(),0);
        }
        else {
          neToken.setInfo(ss.str(),1);
        }
      }   
      tokens.push_back(neToken);
      beginTag = false;
      // set class of next token, if still in a tag
      if (currentClass != tset->outClass()) {
        currentClass = tset->getClassification(t,beginTag);
      }
    }
    i=begin+token.getEnd();
  }
  return tokens;
}


////////////////////////////////////////////////////////////////////////////////
// NamedEntity Member functions
////////////////////////////////////////////////////////////////////////////////
// Type/String conversion functions
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
NamedEntity::NamedEntity(const StringXML::const_iterator begin,
    const StringXML::const_iterator start, 
    const StringXML::const_iterator end,const EntityTag* t, const double prob,
    const StringXML method)
    :_begin(begin),_start(start),_end(end),_tag(t),_prob(prob),_method(method){
  }

StringXML::size_type NamedEntity::leftOffset() const{
  return _start - _begin -1;
}

StringXML::size_type NamedEntity::rightOffset() const{
  return _end - _begin - 1;
}

StringXML::size_type NamedEntity::length() const{
  return rightOffset() - leftOffset();
}

////////////////////////////////////////////////////////////////////////////////
// Returns a pointer to the entity's tag
////////////////////////////////////////////////////////////////////////////////
const EntityTag*
NamedEntity::getTag() const {
  return _tag;
}

double NamedEntity::getProb() const {
  return _prob;
}

StringXML NamedEntity::getMethod() const {
  return _method;
}

StringXML NamedEntity::getString() const{
  StringXML s(_start,_end);
  return s;
}

void NamedEntity::printDetails(ostream& out) const {
  StringXML t = getDetails();
  out << t << endl;
}

void NamedEntity::printTRECDetails(ostream& out) const {
  StringXML t = getTRECDetails();
  out << t << endl;
}

StringXML NamedEntity::getDetails() const{
  StringXML::size_type l = this->leftOffset();
  StringXML::size_type r = this->rightOffset();
  stringstream outstream;
  outstream << "Offset: "<< l << "-" << r
     << "; Word: " << getString() << "; Entity Type: "
     << _tag->openingTag() << "; Probability: "
     << getProb() << " Method: " << getMethod();
  return StringXML(outstream.str());
}

StringXML NamedEntity::getTRECDetails() const {
  StringXML::size_type l = this->leftOffset();
  StringXML::size_type r = this->rightOffset();
  stringstream outstream;
  outstream << _tag->openingTag() << "  " << l << "  " << r;
  return StringXML(outstream.str());
}

bool
NamedEntity::operator<(const NamedEntity& other) const {
  // if left offsets are the same, return lower right
  if (leftOffset()==other.leftOffset()) {
    // if right offsets are the same as well, return lower type
    if (rightOffset()==other.rightOffset()) {
      return (*_tag<*other._tag);
    }
    else {
      return (rightOffset()<other.rightOffset());
    }
  }
  else {
    return (leftOffset()<other.leftOffset());
  }
}

bool
NamedEntity::operator==(const NamedEntity& other) const {
     return ((leftOffset()==other.leftOffset()) &&
            (rightOffset()==other.rightOffset()) &&
            (*_tag==*other._tag));
}

////////////////////////////////////////////////////////////////////////////////
// NEDeco Implementation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Constructor: 
////////////////////////////////////////////////////////////////////////////////
NEDeco::NEDeco(const EntityTagset* t,const RegexHandler* rh,
    const ListHandler* lh,const FeatureHandler* fh,
    const StringXML& modelFile,
    const int maxLabels,const int context,const bool singleLabels)
    :_tagset(t),_regex_handler(rh),_list_handler(lh),
    _feature_handler(fh),_modelFile(modelFile),_maxLabels(maxLabels),
    _context(context),_singleLabels(singleLabels) {
}

set<NamedEntity>::const_iterator
NEDeco::begin() {
  return _entities.begin();
}

set<NamedEntity>::const_iterator
NEDeco::end() {
  return _entities.end();
}

////////////////////////////////////////////////////////////////////////////////
// Administrative functions
////////////////////////////////////////////////////////////////////////////////
// Printing
////////////////////////////////////////////////////////////////////////////////
// Prints the entities to a ostream given
void NEDeco::printEnts(ostream& out, float threshold) {
  printEnts(_entities,out,threshold);
}

void NEDeco::printTRECEnts(ostream& out, float threshold) {
  printTRECEnts(_entities,out,threshold);
}

////////////////////////////////////////////////////////////////////////////////
// Prints the entities stored in the NEDeco this function is called from
// to cout
////////////////////////////////////////////////////////////////////////////////
void NEDeco::printEnts(float threshold) {
  printEnts(_entities,cout,threshold);
}

void NEDeco::printTRECEnts(float threshold) {
  printTRECEnts(_entities,cout,threshold);
}

////////////////////////////////////////////////////////////////////////////////
// Prints the entities to out -- static
////////////////////////////////////////////////////////////////////////////////
void NEDeco::printEnts(const set<NamedEntity>& entities, ostream& out, 
		       float threshold) {
  set<NamedEntity>::const_iterator ite;
  for (ite = entities.begin(); ite != entities.end(); ite++){
    if (ite->getProb() >= threshold)
      out << ite->getDetails() << "\n";
  }
}

void NEDeco::printTRECEnts(const set<NamedEntity>& entities, ostream& out,
			   float threshold) {
  set<NamedEntity>::const_iterator ite;
  for (ite = entities.begin(); ite != entities.end(); ite++){
    if (ite->getProb() >= threshold)
      out << ite->getTRECDetails() << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Interface functions -- These functions interface with worker functions to 
// find the entities and provide a clean interface.  These functions are static.
////////////////////////////////////////////////////////////////////////////////
// Decorate contains the list of other function calls to find ALL entities
////////////////////////////////////////////////////////////////////////////////
void NEDeco::Decorate(StringXML* text) {
  // tokenise
  _text = text;
  vector<Token> tokens = tokenise(_text->begin(),_text->end());
  // convert tokens to TokenDecos, set variable
  _tokens = convertTokens(tokens);
  // match lists & regular expressions
  // create entities and decorate tokens with matches
  FindMatches(true);
  // classify tokens, create entities
  FindClassified();
  set<NamedEntity> ref;
  if (_singleLabels) {
    refineEntities(_entities,ref);
    _entities = ref;
  }
}

////////////////////////////////////////////////////////////////////////////////
// 'printTrainingData' prints training data for the given string to the given
// output stream. 'printClasses' is an optional argument to specify whether the
// first line of the training data (the number of classes) should be printed.
// printClasses is set to true by default.
////////////////////////////////////////////////////////////////////////////////
void NEDeco::PrintTrainingData(StringXML* text,const bool printClasses,
    ostream& out, vector<map<StringXML,int> >& frequencies,
    vector<map<StringXML,int> >& prevFrequencies,
    const bool countFrequencies) {
  _text = text;
  // tokenise the text
  _tokens = tokeniseWithNEInfo(_text->begin(),
      _text->end(),_tagset);
  ////////////////////////////////////////////////////////////
  // Count frequencies
  ////////////////////////////////////////////////////////////
  if (countFrequencies) {
    for (vector<TokenDeco>::const_iterator token = _tokens.begin();
	 token != _tokens.end(); token++) {
      double d = 0;
      token->getInfo("maxProb",d);
      int c = static_cast<int>(d);      
      if (c>0) {
	int index = _tagset->getIndex(c);
	StringXML s(token->getBeginIterator(_text->begin()),
		    token->getEndIterator(_text->begin()));
	map<StringXML,int>::iterator iter = frequencies[index].find(s);
	// if token found, increment, otherwise set to 1
	if (iter!=frequencies[index].end()) {
	  iter->second += 1;
	}
	else {
	  frequencies[index][s] = 1;
	}
	// update previousToken frequencies
	if ((token)!=_tokens.begin()) {
	  vector<TokenDeco>::const_iterator pToken = token;
	  pToken--;
	  StringXML ps(pToken->getBeginIterator(_text->begin()),
		       pToken->getEndIterator(_text->begin()));
	  map<StringXML,int>::iterator pIter = 
	    prevFrequencies[index].find(ps);
	  if (pIter!=prevFrequencies[index].end()) {
	    pIter->second += 1;
	  }
	  else {
	    prevFrequencies[index][ps] = 1;
	  }
	}
      }
    }
  }
  ////////////////////////////////////////////////////////////
  // End count frequencies
  ////////////////////////////////////////////////////////////
  else {
    if (printClasses)
      out << _tagset->classCount() << endl;
    set<NamedEntity> ents; 
    FindMatches();
    // obtain feature vectors for each token
    vector<FeatureVector> fvectors = (*_feature_handler)(_tokens,*_text,false);
    // for each token
    vector<TokenDeco>::const_iterator token = _tokens.begin();
    // and for each feature vector
    for (vector<FeatureVector>::const_iterator fvec=fvectors.begin();
	 fvec!=fvectors.end();fvec++) {
      double d = 0;
      token->getInfo("maxProb",d);
      int c = static_cast<int>(d);
      unsigned int weight = 0;
      // for each category (class)
      // print the class of the current token
      StringXML s(token->getBeginIterator(_text->begin()),
		  token->getEndIterator(_text->begin()));
      out << c << " @ ";
      for (int i = 0; i<_tagset->classCount();i++) {
	// determine the weight of the current category
	i==c ? weight=1 : weight=0;
	// print weight
	out << "@ " << weight << " ";
	// for each feature in the feature vector
	for (FeatureVector::const_iterator feat=fvec->begin();
	     feat!=fvec->end();feat++) {
	  /////////////// DEBUG 18/1/08 Diego ///////
	  //{
	  //  string f = feat->getFeature();
	  //  if (f[0]=='T' && f[1]=='F' ||
	  //      f[0]=='P' && f[1]=='T' && f[2]=='F')
	  //    cerr << f << ":" << feat->getValue() << endl;
	  //}
	  /////////////// END DEBUG ////////////////
	  // generate name of current feature - category dependent
	  if (feat->getValue()!=0) {
	    stringstream ss;
	    ss << "cat" << i << "_" << feat->getFeature();
	    // print feature and value
	    out << ss.str() << " " << feat->getValue() << " ";
	  }
	} // finished for a feature
	out << "# ";
      } // finished for a token - last category done
      out << endl;
      token++;
    } // end looping over tokens
  } 
}

void
NEDeco::FindMatches(const bool createEntities) {
  FindRegexMatches(createEntities);
  FindListMatches(createEntities);
}


////////////////////////////////////////////////////////////////////////////////
// Finds entities matched by regular expressions in the regex handler
////////////////////////////////////////////////////////////////////////////////
void
NEDeco::FindRegexMatches(const bool createEntities) {
  // for each regex in the regex handler
  int j = 0;
  for (map<regex,EntityTag>::const_iterator i=_regex_handler->begin();
      i!=_regex_handler->end();i++) {
    stringstream ss;
    // the name of the current regex
    ss << "regex" << j;
    StringXML rxname = ss.str();
    FindRegex(i->first,&(i->second),rxname,createEntities);
    j++;
  }
}

////////////////////////////////////////////////////////////////////////////////
// refines the entity list to allow a single entity labels with no
// overlap
// Menno van Zaanen
////////////////////////////////////////////////////////////////////////////////
void NEDeco::refineEntities(const set<NamedEntity>& old,set<NamedEntity>& ref) {
  // greedy search
  set<NamedEntity>::const_iterator i=old.begin();
  set<NamedEntity>::const_iterator last=ref.end();
  while (i!=old.end()) {
    if (ref.size()==0) {
      set<NamedEntity>::const_iterator k=i;
      ++k;
      while ((k!=old.end()) && (k->leftOffset()==i->leftOffset())) {
         if (k->rightOffset()>i->rightOffset()) {
			  i=k;
         }
         else if ((k->rightOffset()==i->rightOffset())
             &&(k->getProb()>=i->getProb())) {
			  i=k;
         }
			k++;
      }
      pair<set<NamedEntity>::iterator, bool> res=ref.insert(*i);
      last=res.first;
    }
    else if (i->leftOffset()>=last->rightOffset()) {
      set<NamedEntity>::const_iterator k=i;
      ++k;
      while ((k!=old.end()) && (k->leftOffset()==i->leftOffset())) {
         if (k->rightOffset()>i->rightOffset()) {
			  i=k;
         }
         else if ((k->rightOffset()==i->rightOffset())
             &&(k->getProb()>=i->getProb())) {
			  i=k;
         }
			++k;
      }
      pair<set<NamedEntity>::iterator, bool> res=ref.insert(*i);
      last=res.first;
    }
    ++i;
  }
}

////////////////////////////////////////////////////////////////////////////////
// addTokenProbs takes an iterator to a TokenDeco and a vector of
// probabilities and modifies the info map of the token to contain the
// probability information.
////////////////////////////////////////////////////////////////////////////////
void addTokenProbs(const vector<TokenDeco>::iterator& token,
    const vector<double>& probs){
  int i = 0;          // current probablitity
  // for each classification
  for (vector<double>::const_iterator ite=probs.begin();
      ite!=probs.end();ite++){
    stringstream ss;
    ss << "prob" << i;              // generate info name
    StringXML name = ss.str();
    token->setInfo(name,*ite);      // add to info map
    i++;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Used to store possible entities before they are added to the set.
// Allows for multiple tags to be assigned to the same tokens.
////////////////////////////////////////////////////////////////////////////////
struct EntBuffer {
  StringXML::const_iterator begin;
  StringXML::const_iterator end;
  const EntityTag* tag;
  vector<double> probs;
  int tokenCount;
  EntBuffer(const TokenDeco& tok,const StringXML& text,
      const EntityTag* t,double p) {
    begin=tok.getBeginIterator(text.begin());
    end=tok.getEndIterator(text.begin());
    tokenCount=1;
    tag=t;
    probs.push_back(p);
  }
  StringXML getString() const {
    StringXML r(begin,end);
    return r;
  }
  void addToken(const StringXML::const_iterator tokEnd,const double prob) {
    StringXML bef(begin,end);
    StringXML aft(begin,tokEnd);
    end=tokEnd;
    probs.push_back(prob);
    tokenCount++;
  }
  double getMean() const {
    double gm=0;
    for (vector<double>::const_iterator p=probs.begin();
        p!=probs.end();p++) {
      gm+=log(*p);
    }
    gm = gm/probs.size();
    gm = exp(gm);
    return gm;
  }
  bool
  operator<(const EntBuffer& other) const {
    return (begin < other.begin);
  }
};

vector<pair<int,double> >
getCandidateClasses(const vector<double>& results, const int num=1) {
  vector<pair<int,double> > out;
  vector<double> res = results;
  double prev=0;
  for (int i=0;i<num;i++) {
    vector<double>::const_iterator max = max_element(res.begin(),
        res.end());
    // set relative threshold
    if (*max>(0.5*prev)&&*max>0.1) {
    //if (*max>0.15) {
      pair<int,double> result;
      result.first =
          static_cast<int>(max-res.begin());
      prev=*max;
      result.second = *max;
      // set previous max to 0, so not found again 
      res[(max-res.begin())]=0.0;
      out.push_back(result);
    }
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// 'FindClassified' accepts a list of tokens, classifies each token
// and adds each found named entity to the vector of named entities provided.
// Modifies token decorators to contain probablities of previous
// token. In a series of Named Entities, chooses the most likely
// candidate
////////////////////////////////////////////////////////////////////////////////
void NEDeco::FindClassified() {
  // base classifier so no need to re-read modelfile
  MaxEnt base_classifier(_tagset->classCount(),_modelFile);
  set<EntBuffer> previous;
  set<EntBuffer> current;
  // for each token
  for (vector<TokenDeco>::iterator token=_tokens.begin();
        token!=_tokens.end();token++) {
    // get a classifier
    MaxEnt classifier=base_classifier;
    FeatureVector featVec = (*_feature_handler)(_tokens,token,*_text);
    vector<double> results=classifier.classify(featVec);
    addTokenProbs(token,results);
    //vector<double>::const_iterator o = 
    //    max_element(results.begin(),results.end());
    vector<pair<int,double> > poss =
        getCandidateClasses(results,_maxLabels);
    // if no entities
    if (poss.size()==0) {
      for (set<EntBuffer>::const_iterator buf=previous.begin();
          buf!=previous.end();buf++) {
        NamedEntity ent(_text->begin(),buf->begin,buf->end,buf->tag,
            buf->getMean());
        _entities.insert(ent);
      }
    }
    // for each candidate class
    for (vector<pair<int,double> >::const_iterator
        ite=poss.begin();ite!=poss.end();ite++) {
      int c=ite->first;
      double prob=ite->second;
      bool isBeginClass=true;
      const EntityTag* t=_tagset->getTag(c,isBeginClass);
      // if current candidate is a begin class
      if (c==_tagset->outClass()) {
        // add all previous buffers
        for (set<EntBuffer>::const_iterator buf=previous.begin();
            buf!=previous.end();buf++) {
          NamedEntity ent(_text->begin(),buf->begin,buf->end,buf->tag,
              buf->getMean());
          _entities.insert(ent);
        }
      } // end out
      else {
        if (isBeginClass) {
          // add as new buffer
          EntBuffer e(*token,*_text,t,prob);
          current.insert(e);
          // if buffer of same type exists, append token
          for (set<EntBuffer>::const_iterator buf=previous.begin();
              buf!=previous.end();buf++) {
            // if of same type
            if (buf->tag==t) {
              // create entity
              NamedEntity ent(_text->begin(),buf->begin,buf->end,buf->tag,
                  buf->getMean());
              _entities.insert(ent);
              // append token and add to current buffers
              EntBuffer d=*buf;            // duplicate
              d.addToken(token->getEndIterator(_text->begin()),prob);
              current.insert(d);
            }
          }
        } // end if begin class
        else { // if not begin class
          bool found=false;
          for (set<EntBuffer>::const_iterator buf=previous.begin();
              buf!=previous.end();buf++) {
            // if of same type
            if (buf->tag==t) {
              found=true;
              EntBuffer d=*buf;
              d.addToken(token->getEndIterator(_text->begin()),prob);
              current.insert(d);
            }
          }
          // if no buffer exists
          if (!found) {
            EntBuffer e(*token,*_text,t,prob);
            current.insert(e);
          }
        } // end if not begin class
      } // end not out
    }// end loop over classifications
    previous.clear();
    for (set<EntBuffer>::const_iterator buf=current.begin();
        buf!=current.end();buf++) {
      previous.insert(*buf);
    }
    current.clear();
  } // end looping over tokens
  // add remaining buffers as entities
  for (set<EntBuffer>::const_iterator buf=previous.begin();
      buf!=previous.end();buf++) {
    NamedEntity ent(_text->begin(),buf->begin,buf->end,buf->tag,
      buf->getMean());
    _entities.insert(ent);
  }
}

////////////////////////////////////////////////////////////////////////////////
// 'FindRegex' finds any entity that matches a regular expression, using the 
// type provided.
// Marks the tokens with the regular expression matched
////////////////////////////////////////////////////////////////////////////////
void NEDeco::FindRegex(const regex& regx, const EntityTag* tag,
    const StringXML& regexName,bool createEntities) {
  // create iterators
  StringXML::const_iterator start, end;
  start = _text->begin();
  end = _text->end();
  // create result list, expression, flags
  match_results<StringXML::const_iterator> what;
  match_flag_type flags = match_default;
  vector<pair<int,int> > matches;
  // find the matches
  while (regex_search(start,end,what,regx,flags)){
    if (createEntities) {
      NamedEntity ent(_text->begin(),what[0].first,
        what[5].first,tag,1.0,"regex");
      _entities.insert(ent);
    }
    pair<int,int> match;
    match.first = what[0].first - _text->begin();
    match.second = what[5].first - _text->begin();
    matches.push_back(match);
    start = what[0].second;
    flags |= match_prev_avail;
    flags |= match_not_bob;
  }
  // for each token, check if a match
  for (vector<TokenDeco>::iterator token=_tokens.begin();
      token!=_tokens.end();token++) {
    // iterate through matches
    for (vector<pair<int,int> >::const_iterator match=matches.begin();
        match!=matches.end();match++) {
      int b = token->getBegin();
      int e = token->getEnd();
      // if token offsets lie within match offsets
      if (((b>=match->first)&&(b<=match->second))||
           (e<=match->second)&&(e>match->first)) {
        // set the token field
        token->setInfo(regexName,1.0);
      }
    }
  }
}

// marks tokens pointed to by the iterators in the given vector
void markTokens(vector<vector<TokenDeco>::iterator>& toks,
    const StringXML& info,const StringXML& text) {
  for (vector<vector<TokenDeco>::iterator>::iterator i = toks.begin();
      i!=toks.end();i++) {
    (*i)->setInfo(info,1.0);
  }
}

void
NEDeco::FindListMatches(bool createEntities) {
  // for each token
  for(vector<TokenDeco>::iterator token = _tokens.begin();
      token!=_tokens.end();token++) {
    // a vector of iterators to store which tokens to mark
    vector<vector<TokenDeco>::iterator> mark;
    mark.push_back(token);
    StringXML txt(token->getBeginIterator(_text->begin()),
        token->getEndIterator(_text->begin()));
    // get incomplete matches
    vector<int> locs=_list_handler->FindString(txt,false);
    bool go=(locs.size()>0);
    vector<TokenDeco>::iterator latest=token;
    while(go) {
      go=false;
      // get complete matches
      locs=_list_handler->FindString(txt);
      // for each complete match, set info of tokens, create entity
      for (unsigned int i=0;i<locs.size();i++) {
        stringstream ss;
        ss << "list" << locs[i];
        markTokens(mark,ss.str(),*_text);
        if (createEntities) {
          // create entity here, from all tokens that form ent
          NamedEntity ent(_text->begin(),
              mark[0]->getBeginIterator(_text->begin()),
              mark[mark.size()-1]->getEndIterator(_text->begin()),
              _list_handler->GetTag(locs[i]),1.0,"list");
          _entities.insert(ent);
        }
      }
      // next token
      vector<TokenDeco>::iterator next_token=latest;
      next_token++;
      // if on last token, stop, else check next match
      if (next_token!=_tokens.end()) {
        // set the text to be from the start token to end
        txt.assign(token->getBeginIterator(_text->begin()),
            next_token->getEndIterator(_text->begin()));
        locs=_list_handler->FindString(txt,false);
        // if there are matches with the next token, continue
        if (locs.size()>0) {
          // add next to list to mark
          mark.push_back(next_token);
          go=true;
          // increment token
          latest = next_token;
        }
      } // end if not at end token
    } // end while
  } // end tokens
}

