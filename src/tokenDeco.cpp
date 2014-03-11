////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au  2006
////////////////////////////////////////////////////////////////////////////////
// Filename: tokenDeco.cpp
////////////////////////////////////////////////////////////////////////////////
// This file contains the TokenDeco implementation.
// TokenDeco is a Decorator class for Token
////////////////////////////////////////////////////////////////////////////////
//
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
#include <map>
#include "tokenDeco.h"
#include "ner.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace AF;

const int CLASSCOUNT=13;

////////////////////////////////////////////////////////////////////////////////
// Constructor: initialises the token pointer.
////////////////////////////////////////////////////////////////////////////////
TokenDeco::TokenDeco(const Token* t):Token(*t),_component(t){
}

////////////////////////////////////////////////////////////////////////////////
// Returns a pointer to the token
////////////////////////////////////////////////////////////////////////////////
const Token* 
TokenDeco::getComponent() const{
  return _component;
}

////////////////////////////////////////////////////////////////////////////////
// 'setInfo(name,value)' sets extra information about a token, name and value.
////////////////////////////////////////////////////////////////////////////////
void
TokenDeco::setInfo(const StringXML name, const double value){
   _info[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
// 'getInfo(name)' returns true if a value that corresponds to the name of some
// information about the token stored, false otherwise. The value is assigned to
// the given 'value' variable.
////////////////////////////////////////////////////////////////////////////////
bool
TokenDeco::getInfo(const StringXML& name, double& value) const{
  bool res = false;
  map<StringXML,double>::const_iterator pos = _info.find(name);
  if (pos!=_info.end()){
    value = pos->second;
    res = true;
  }
  return res;
}

vector<TokenDeco>
AF::convertTokens(const vector<Token>& tokens){
  vector<TokenDeco> res;
  for (vector<Token>::const_iterator ite=tokens.begin();
      ite!=tokens.end();ite++){
    res.push_back(TokenDeco(&(*ite)));
  }
  return res;
}

