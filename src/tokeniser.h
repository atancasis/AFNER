////////////////////////////////////////////////////////////////////////////////
// Menno van Zaanen
// menno@ics.mq.edu.au  2005-
////////////////////////////////////////////////////////////////////////////////
// Filename: tokeniser.h
////////////////////////////////////////////////////////////////////////////////
// This file is part of the AnswerFinder package.
////////////////////////////////////////////////////////////////////////////////
// This file contains several tokeniser functions.
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
// $Log: tokeniser.h,v $
// Revision 1.3  2008/07/09 08:47:08  diego
// Improved the user interface of the executable and added license information
//
// Revision 1.2  2006/10/10 06:44:13  dsmith
// NamedEntity vector changed to set.
//
// Revision 1.5  2006/09/26 02:13:43  dsmith
//
// Removed tokeniser_ne.h and tokeniser_ne.cpp
//
// Revision 1.4  2006/08/10 22:50:01  dsmith
//
// Machine learner only:
// 	context included
// 	token decorator
// 	regular expression features
//
// Revision 1.3  2006/06/16 02:43:21  dsmith
// *** empty log message ***
//
// Revision 1.2  2006/04/27 00:35:00  dsmith
// Feature extraction classes added in feature_functions.h and
// feature_functions.cpp.  Integration of tokenisation, feature detection
// classes complete.  Feature extraction logic needs to be implemented in more
// classes.
//
// Revision 1.1  2006/04/20 06:23:52  dsmith
// Tokenisation files added
//
// Revision 1.1  2006/04/04 06:04:13  menno
// Renamed tokenizer.* to tokeniser.*.
//
// Revision 1.2  2005/06/06 00:54:56  menno
// Modified comments.
//
// Revision 1.1  2005/05/17 01:12:24  menno
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __tokeniser__
#define __tokeniser__

#include <vector>
#include <string>
#include "xml_string.h"

using namespace std;
using namespace AF;

namespace AF {

class Document {
public:
  Document(){
  }
  Document(StringXML in){
    str = in;
  }
  StringXML getString(const StringXML::size_type begin,
      const StringXML::size_type end) const {
    return str;
  }
  StringXML str;
};

class Sentence {
public:
  Sentence(){
  }
  Sentence(const Document* doc){
  }
  void
  setBegin(StringXML::size_type begin){
  }
  void
  setEnd(StringXML::size_type begin){
  }
  StringXML::size_type
  getEnd(){
    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////
// 'Token' denotes a word in the text.
////////////////////////////////////////////////////////////////////////////////
class Token {
////////////////////////////////////////////////////////////////////////////////
public:
////////////////////////////////////////////////////////////////////////////////
// 'Token()' constructor, it takes begin and end offsets to
// indicated the begin and one past the end of the Token.
////////////////////////////////////////////////////////////////////////////////
  Token(StringXML::size_type begin=0,
      StringXML::size_type end=0);

////////////////////////////////////////////////////////////////////////////////
// 'setBegin()' sets the begin offset of the token.
////////////////////////////////////////////////////////////////////////////////
  void
  setBegin(StringXML::size_type);
  
////////////////////////////////////////////////////////////////////////////////
// 'setEnd()' sets the end offset of the token.
////////////////////////////////////////////////////////////////////////////////
  void
  setEnd(StringXML::size_type);
  
////////////////////////////////////////////////////////////////////////////////
// 'getBegin()' returns the begin offset of the token.
////////////////////////////////////////////////////////////////////////////////
  StringXML::size_type
  getBegin() const;
  
////////////////////////////////////////////////////////////////////////////////
// 'getEnd()' returns the end offset of the token.
////////////////////////////////////////////////////////////////////////////////
  StringXML::size_type
  getEnd() const;
  
////////////////////////////////////////////////////////////////////////////////
// 'getBeginIterator()' returns an iterator to the begin of the token
// based on the origin.
////////////////////////////////////////////////////////////////////////////////
  StringXML::const_iterator
  getBeginIterator(const StringXML::const_iterator origin) const;
  
////////////////////////////////////////////////////////////////////////////////
// 'getEndIterator()' returns an iterator to the end of the token
// based on the origin.
////////////////////////////////////////////////////////////////////////////////
  StringXML::const_iterator
  getEndIterator(const StringXML::const_iterator origin) const;
  
////////////////////////////////////////////////////////////////////////////////
// 'getString()' returns a 'StringXML' that is the string the token
// points to with respect to 'original'.  (Token only stores offsets.)
////////////////////////////////////////////////////////////////////////////////
  StringXML
  getString(const StringXML& original) const;
  
protected:
private:
////////////////////////////////////////////////////////////////////////////////
// '_begin' stores the offset to the begin of the string of the
// Token.
////////////////////////////////////////////////////////////////////////////////
  StringXML::size_type _begin;

////////////////////////////////////////////////////////////////////////////////
// '_end' stores the offset to the one past the end of the string of
// the Token.
////////////////////////////////////////////////////////////////////////////////
  StringXML::size_type _end;

};

////////////////////////////////////////////////////////////////////////////////
// 'getToken' returns the first token that can be found starting from
// begin.  Note that the Token that is returned may actually be
// empty (when there is no more token starting from begin and ending
// before end) in that case it returns false.  It returns true if a
// token was found.  The offset indicates the offset of begin in the
// whole StringXML (if any).
////////////////////////////////////////////////////////////////////////////////
bool
getToken(const StringXML::const_iterator begin,
    const StringXML::const_iterator end, Token& token,
    StringXML::size_type offset=0, bool skipXML=true);

////////////////////////////////////////////////////////////////////////////////
// 'tokenise' returns a vector of tokens that can be found between
// begin and end.
////////////////////////////////////////////////////////////////////////////////
vector<Token>
tokenise(const StringXML::const_iterator begin,
    const StringXML::const_iterator end, bool skipXML=true);


    
////////////////////////////////////////////////////////////////////////////////
// 'getSentence' returns the first sentence that can be found starting
// from begin.  Note that the Sentence that is returned may actually
// be empty (when there is no more sentence starting from begin and
// ending before end) in that case it returns false.  It returns true
// if a sentence was found.  The iterators are pointing into the
// document.
////////////////////////////////////////////////////////////////////////////////
bool
getSentence(const Document* document,
    const StringXML::size_type begin,
    const StringXML::size_type end, Sentence& sentence);

////////////////////////////////////////////////////////////////////////////////
// 'sentencise' returns a vector of sentences that can be found
// between begin and end.  The iterators are pointing into the
// document. 
////////////////////////////////////////////////////////////////////////////////
vector<Sentence>
sentencise(const Document* document,
    const StringXML::size_type begin,
    const StringXML::size_type end);

} // namespace AF

#endif //__tokeniser__
// end of file: tokeniser.h
