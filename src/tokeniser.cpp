////////////////////////////////////////////////////////////////////////////////
// Menno van Zaanen
// menno@ics.mq.edu.au  2005-
////////////////////////////////////////////////////////////////////////////////
// Filename: tokeniser.cpp
////////////////////////////////////////////////////////////////////////////////
// This file is part of the AnswerFinder package.
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the tokeniser functions
// (defined in tokeniser.h).
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
// $Log: tokeniser.cpp,v $
// Revision 1.3  2008/07/09 08:47:08  diego
// Improved the user interface of the executable and added license information
//
// Revision 1.2  2006/10/10 06:44:13  dsmith
// NamedEntity vector changed to set.
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
// Revision 1.6  2005/10/05 07:21:19  menno
// Modified regular expressions.
//
// Revision 1.5  2005/07/26 06:19:49  menno
// Removed s_str(), moved regex's to global and fixed regular expression (word).
//
// Revision 1.4  2005/06/14 00:52:16  menno
// Using regular expressions to find boundaries.
//
// Revision 1.3  2005/06/08 01:14:21  menno
// Fixed bug with ' in first word.
//
// Revision 1.2  2005/06/06 00:54:56  menno
// Modified comments.
//
// Revision 1.1  2005/05/17 01:12:24  menno
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////

#include <boost/regex.hpp>

#include <iostream>
#include <vector>
#include "tokeniser.h"

using namespace std;
using namespace AF;
using boost::smatch;
using boost::regex;
using boost::regex_search;

////////////////////////////////////////////////////////////////////////////////
// 'Token()' constructor, it takes begin and end offset to
// indicated the begin and one past the end of the Token.
////////////////////////////////////////////////////////////////////////////////
Token::Token(StringXML::size_type begin,
      StringXML::size_type end)
      :_begin(begin),_end(end) {
}


////////////////////////////////////////////////////////////////////////////////
// 'setBegin()' sets the begin offset of the token.
////////////////////////////////////////////////////////////////////////////////
void
Token::setBegin(StringXML::size_type begin) {
  _begin=begin;
}


////////////////////////////////////////////////////////////////////////////////
// 'setEnd()' sets the end offset of the token.
////////////////////////////////////////////////////////////////////////////////
void
Token::setEnd(StringXML::size_type end) {
  _end=end;
}


////////////////////////////////////////////////////////////////////////////////
// 'getBegin()' returns the begin offset of the token.
////////////////////////////////////////////////////////////////////////////////
StringXML::size_type
Token::getBegin() const {
  return _begin;
}


////////////////////////////////////////////////////////////////////////////////
// 'getEnd()' returns the end offset of the token.
////////////////////////////////////////////////////////////////////////////////
StringXML::size_type
Token::getEnd() const {
  return _end;
}


////////////////////////////////////////////////////////////////////////////////
// 'getBeginIterator()' returns an iterator to the begin of the token
// based on the origin.
////////////////////////////////////////////////////////////////////////////////
StringXML::const_iterator
Token::getBeginIterator(const StringXML::const_iterator origin) const {
  return origin+_begin;
}

  
////////////////////////////////////////////////////////////////////////////////
// 'getEndIterator()' returns an iterator to the end of the token
// based on the origin.
////////////////////////////////////////////////////////////////////////////////
StringXML::const_iterator
Token::getEndIterator(const StringXML::const_iterator origin) const {
  return origin+_end;
}


////////////////////////////////////////////////////////////////////////////////
// 'getString()' returns a 'StringXML' that is the string the token
// points to.
////////////////////////////////////////////////////////////////////////////////
StringXML
Token::getString(const StringXML& original) const {
  StringXML::const_iterator b=original.begin();
  return StringXML(b+_begin, b+_end);
}


////////////////////////////////////////////////////////////////////////////////
// 'skipWhitespace' skips any whitespace starting from begin.  It returns an
// iterator to the first non-whitespace character.
////////////////////////////////////////////////////////////////////////////////
StringXML::const_iterator
skipWhitespace(const StringXML::const_iterator begin,
    const StringXML::const_iterator end) {
  StringXML::const_iterator i=begin;
  while ((i!=end)&&(isspace(*i)||!isprint(*i))) {

    ++i;
  }
  return i;
}


////////////////////////////////////////////////////////////////////////////////
// 'getToken' returns the first token that can be found starting from
// begin.  Note that the Token that is returned may actually be
// empty (when there is no more token starting from begin and ending
// before end) in that case it returns false.  It returns true if a
// token was found.  The offset indicates the offset of begin in the
// whole StringXML (if any).
////////////////////////////////////////////////////////////////////////////////
bool
AF::getToken(const StringXML::const_iterator begin,
    const StringXML::const_iterator end, Token& token,
    StringXML::size_type offset, bool skipXML) {
  StringXML::const_iterator i=begin;
  i=skipWhitespace(i, end);
  StringXML::const_iterator tokenBegin=i;
  StringXML::const_iterator tokenEnd=end;
  bool foundToken=false;
  bool endOfToken=false;
  bool isWord=true;
  bool isPunct=true;
  while ((i!=end)&&(!endOfToken)) {
    if (isWord&&(isalnum(*i))) { // inside a word
      isWord=true;
      isPunct=false;
      foundToken=true;
    }
    else if (isPunct&&(ispunct(*i))&&(*i!='<')) { // inside punctuation
      isWord=false;
      isPunct=true;
      foundToken=true;
    }
    else if (!foundToken&&(*i=='<')) { // XML tag
      while ((i!=end)&&(*i!='>')) {
        ++i;
      }
      if (i!=end) {
        ++i; // get character following >
      }
      if (skipXML) { // try again
        foundToken=false;
        endOfToken=false;
        isWord=true;
        isPunct=true;
        i=skipWhitespace(i, end);
        tokenBegin=i;
        --i; // undo ++i below
      }
      else {
        endOfToken=true;
        foundToken=true;
        tokenEnd=i;
      }
    }
    else { // end of token found
      endOfToken=true;
      tokenEnd=i;
    }
    ++i;
  }
  if (foundToken) {
    token.setBegin(tokenBegin-begin+offset);
    token.setEnd(tokenEnd-begin+offset);
  }
  return foundToken;
}


////////////////////////////////////////////////////////////////////////////////
// 'tokenise' returns a vector of tokens that can be found between
// begin and end.
////////////////////////////////////////////////////////////////////////////////
vector<Token>
AF::tokenise(const StringXML::const_iterator begin,
    const StringXML::const_iterator end, bool skipXML) {
  vector<Token> tokens;
  Token token;
  StringXML::const_iterator i=begin;
  while (getToken(i, end, token, i-begin, skipXML)) {
    tokens.push_back(token);
    i=begin+token.getEnd();
  }
  return tokens;
}


////////////////////////////////////////////////////////////////////////////////
// 'isAbbreviation' returns true if the string ends on an
// abbreviation, false otherwise.
////////////////////////////////////////////////////////////////////////////////
bool
isAbbreviation(const StringXML& line) {
  const StringXML abbreviations=
  "(\\d+[-\\.:]+(\\d+[-\\.:]+)+$)|"
   "(\\.\\.+$)|"
   "(_\\s*$)|"
   "(\\w+\\.+(\\w+\\.+)+$)|"
   "(A\\.$)|"
   "(Adm\\.$)|"
   "(Ark\\.$)|"
   "(Apr\\.$)|"
   "(Aug\\.$)|"
   "(B\\.$)|"
   "(C\\.$)|"
   "(Calif\\.$)|"
   "(Capt\\.$)|"
   "(Cmdr\\.$)|"
   "(Co\\.$)|"
   "(Conn\\.$)|"
   "(Cpl\\.$)|"
   "(D\\.$)|"
   "(Dec\\.$)|"
   "(Dr\\.$)|"
   "(E\\.$)|"
   "(F\\.$)|"
   "(F\\.W\\.$)|"
   "(Feb\\.$)|"
   "(Fla\\.$)|"
   "(G\\.$)|"
   "(Ga\\.$)|"
   "(Gen\\.$)|"
   "(Gov\\.$)|"
   "(H\\.$)|"
   "(I\\.$)|"
   "(Iraq-U\\.N\\.$)|"
   "(Inc\\.$)|"
   "(J\\.$)|"
   "(Jan\\.$)|"
   "(Jun\\.$)|"
   "(Jul\\.$)|"
   "(Jr\\.$)|"
   "(K\\.$)|"
   "(Ky\\.$)|"
   "(L\\.$)|"
   "(La\\.$)|"
   "(Lt\\.$)|"
   "(Ltd\\.$)|"
   "(m\\.$)|"
   "(M\\.$)|"
   "(Maj\\.$)|"
   "(Mar\\.$)|"
   "(May\\.$)|"
   "(Md\\.$)|"
   "(Mo\\.$)|"
   "(Mr\\.$)|"
   "(Ms\\.$)|"
   "(Mt\\.$)|"
   "(N\\.$)|"
   "(Neb\\.$)|"
   "(Nev\\.$)|"
   "(No\\.$)|"
   "(\\(No\\.$)|"
   "(Nov\\.$)|"
   "(O\\.$)|"
   "(Oct\\.$)|"
   "(Ore\\.$)|"
   "(P\\.$)|"
   "(Pa\\.$)|"
   "(Q\\.$)|"
   "(R\\.$)|"
   "(Rep\\.$)|"
   "(Rev\\.$)|"
   "(R.I\\.$)|"
   "(S\\.$)|"
   "(SA\\.$)|"
   "(Sen\\.$)|"
   "(Sep\\.$)|"
   "(Sgt\\.$)|"
   "(St\\.$)|"
   "(ST\\.$)|"
   "(T\\.$)|"
   "(Tenn\\.$)|"
   "(U\\.$)|"
   "(V\\.$)|"
   "(Va\\.$)|"
   "(vs\\.$)|"
   "(W\\.$)|"
   "(X\\.$)|"
   "(Y\\.$)|"
   "(Z\\.$)";
  static const regex abbrRegex(abbreviations);
  return regex_search(line, abbrRegex);
}


////////////////////////////////////////////////////////////////////////////////
// 'isXML' returns true if the token is an XML tag, false otherwise.
////////////////////////////////////////////////////////////////////////////////
bool
isXML(Token t, const StringXML& text) {
    StringXML::value_type b=t.getString(text)[0];
    return (b=='<');
}

////////////////////////////////////////////////////////////////////////////////
// 'isSentenceEnd' returns true if the token is to be considered the
// end of a sentence, false otherwise.
////////////////////////////////////////////////////////////////////////////////
bool
isSentenceEnd(Token t, const StringXML& text) {
  StringXML::value_type b=t.getString(text)[0];
  if ((b=='.')||(b=='!')||(b=='?')||(b==':')||(b==';')) {
    // dirty!  check if followed by whitespace, XML, or eof
    if ((t.getEnd()==text.size())||(isspace(text[t.getEnd()]))
        ||(text[t.getEnd()]=='<')) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////
// 'getSentence' returns the first sentence that can be found starting
// from begin.  Note that the Sentence that is returned may actually
// be empty (when there is no more sentence starting from begin and
// ending before end) in that case it returns false.  It returns true
// if a sentence was found.  The iterators are pointing into the
// document.
////////////////////////////////////////////////////////////////////////////////
bool
AF::getSentence(const Document* document,
    const StringXML::size_type begin,
    const StringXML::size_type end, Sentence& sentence) {
  if (document==0) {
    return false;
  }
  StringXML text=document->getString(begin, end);
  StringXML::const_iterator tBegin=text.begin();
  StringXML::const_iterator tEnd=text.end();
  StringXML::const_iterator i=text.begin();
  Token t;
  if (!getToken(i, tEnd, t)) { // get first token
    return false;
  }
  i=tBegin+t.getEnd();
  // Skip sentence ends
  while ((isSentenceEnd(t, text))) {
    if (!getToken(i, tEnd, t, i-tBegin)) {
      return false;
    }
    i=tBegin+t.getEnd();
  }
  StringXML::size_type sentenceBegin=t.getBegin();
  StringXML::size_type sentenceEnd=t.getEnd();
  while (getToken(i, tEnd, t, i-tBegin, false)
      &&!(
        (isXML(t, text) // XML is sentence end
        ||isSentenceEnd(t, text)
        &&!isAbbreviation(
          StringXML(tBegin+sentenceBegin, tBegin+t.getEnd()))))) {
    sentenceEnd=t.getEnd();
    i=tBegin+t.getEnd();
  }
  if (isSentenceEnd(t, text)) {
    sentenceEnd=t.getEnd();
  }
  sentence.setBegin(begin+sentenceBegin);
  sentence.setEnd(begin+sentenceEnd);
  return true;
}


////////////////////////////////////////////////////////////////////////////////
// 'sentencise' returns a vector of sentences that can be found
// between begin and end.  The iterators are pointing into the
// document.
////////////////////////////////////////////////////////////////////////////////
vector<Sentence>
AF::sentencise(const Document* document,
    const StringXML::size_type begin,
    const StringXML::size_type end) {
  vector<Sentence> sentences;
  Sentence sentence(document);
  StringXML::size_type i=begin;
  while (getSentence(document, i, end, sentence)) {
    sentences.push_back(sentence);
    i=sentence.getEnd();
  }
  return sentences;
}

// end of file: tokeniser.cpp
