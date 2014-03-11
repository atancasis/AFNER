///////////////////////////////////////////////////////////////////////////////*
// Menno van Zaanen
// mvzaanen@science.uva.nl   2001-2002
// mvzaanen@uvt.nl           2002-2004
// menno@ics.mq.edu.au       2004-
////////////////////////////////////////////////////////////////////////////////
// Filename: suffixtree.cpp
////////////////////////////////////////////////////////////////////////////////
// This file is part of the suffix tree package.
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the suffixtree and node
// classes. These classes are defined in suffixtree.h.
//////////////////////////////////////////////////////////////////////////////*/
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
#include <string>
#include <vector>
#include "suffixtree.h"

using namespace ns_suffixtree;
using namespace std;

suffixtree::node::node()
    :begin_i(0),
     end_i(0),
     up(0),
     suffix_link(0),
     owner(0),
     number(0) {
// Constructor initialising an empty node.
}

suffixtree::node::node(index b,
                       index e,
                       node *u,
                       suffixtree *o,
                       size_type num)
    :begin_i(b),
     end_i(e),
     up(u),
     suffix_link(0),
     owner(o),
     number(num) {
// Constructor initialising a node with begin_i, end_i, up, owner and
// number. suffix_link and children are still empty.
}

suffixtree::node::~node() {
// Destructor automatically calls delete for each of the children
// recursively.
  for (map<element_type, node *>::iterator i=children.begin();
      i!=children.end(); i++) {
    delete i->second;
  }
}

const suffixtree::index
suffixtree::node::begin() const {
// Return the begin index, stored in begin_i.
  return begin_i;
}

const suffixtree::size_type
suffixtree::node::give_number() const {
// Return the number, stored in number.
  return number;
}

const suffixtree::index
suffixtree::node::end() const {
// Return the end index. This is stored in end_i, unless it is a leaf.
// Then the e value of the owner should be returned, since that
// denotes the longest handled extension so far.
  if (children.size()==0) { // no children
    return owner->e;
  }
  else {
    return end_i;
  }
}

void
suffixtree::node::print_node(ostream &os, size_type depth) const {
// Print the content of the node on os. depth indicates how far the
// current node should be indented.
  if (begin()!=end()) { // print the entry only if it defines a substring
    if (children.size()==0) {
      os << "+"; // start the line with a + if it is a leaf
    } else {
      os << "-"; // start the line with a - if it has branches
    }
    if (depth>0) { // print |'s indicating branch depth
      size_type d=depth;
      while (d>0) {
        os << "|";
        d--;
      }
    }
    os << string(begin(), end()); // print the substring
    if (children.size()==0) { // if it is a leaf, print it's start index
      os << number;
    }
    os << endl;
  }
  for (map<element_type, node *>::const_iterator i=children.begin();
      i!=children.end(); i++) { // print all children
    i->second->print_node(os, depth+1); // depth+1 it is one level deeper
  }
}

suffixtree::position
suffixtree::node::SEA(index b,
                      index e,
                      size_type level,
                      node*& longest,
                      index cur,
                      bool &cont,
                      bool seek) {
// SEA implements the Single Extension Algorithm (SEA) (see
// Gusfield). (b, e) is the string to be inserted.  level denotes
// the start index of the suffix, longest is a pointer to the
// node containing the current longest suffix. cur points to the
// end of the current string, and cont is true to start with (to
// indicate that new extensions should be handled) and set to
// false if handling new extensions in the current phase is not
// needed. seek indicates if sv and gamma need to be found (true, yes,
// false, no).
  node *sv;
  if (seek) {
    sv=find_sv(cur, b, e); // find the sv node starting from cur
    position pos=sv->seek_gamma(b, e-1); // move to the end of gamma from sv
    sv=pos.first;
    cur=pos.second;
  }
  else {
    sv=this;
  }
  // add new character from the end of gamma
  return sv->extend_gamma(b, e, cur, level, longest, cont);
}

vector<suffixtree::element_type> 
suffixtree::node::next_children() const {
// Collect all possible keys of this node and return them in a vector.
  map<element_type, node *>::const_iterator i=children.begin();
  vector<element_type> v;
  while (i!=children.end()) {
    v.push_back(i->first);
    i++;
  }
  return v;
}

const suffixtree::node *
suffixtree::node::find_child(const element_type &e) const {
// Find the child that was indexed with e. It returns 0 if the child
// doesn't exist.
  map<element_type, node *>::const_iterator f_node=children.find(e);
  if (f_node==children.end()) {
    return 0;
  }
  else {
    return f_node->second;
  }
}

void
suffixtree::node::assign_child(const element_type e,
                               node *val) {
// Set val as a child indexed with value e.
  children[e]=val;
}

vector<suffixtree::size_type>
suffixtree::node::find_leaf_positions() const {
  vector<size_type> result;
  if (children.size()==0) {
    result.push_back(number);
  }
  for (map<element_type, node *>::const_iterator c=children.begin();
      c!=children.end(); c++) {
    vector<size_type> res=c->second->find_leaf_positions();
    result.insert(result.end(), res.begin(), res.end());
  }
  return result;
}

const suffixtree::size_type
suffixtree::node::children_size() const {
  return children.size();
}

suffixtree::node::node *
suffixtree::node::find_sv(index cur,
                          index &b,
                          index e) {
// find the sv node. This means going back from e towards b matching
// from cur (cur is the current position, so the string ending with
// cur should match a suffix of (b, e)). Once a suffix link is found,
// it is followed. gamma keeps track of the number of positions that
// have been recognised while going back to find the suffixlink.
// Note that if no suffixlink is found, b is not changed.
  node *sv=this; // seeker node going up and through suffixlink
  size_type gamma=cur-sv->begin(); // current string must be searched
  sv=sv->up; // going up
  while (!((sv->up==0)||(sv->suffix_link!=0))) { // find suffix link or root
    gamma+=sv->end()-sv->begin(); // append gamma
    sv=sv->up; // going up
  }
  if (sv->suffix_link!=0) { // found suffix link
    sv=sv->suffix_link; // follow suffixlink
    b=e-gamma; // new begin
  }
  return sv;
}

suffixtree::position
suffixtree::node::seek_gamma(index &b,
                             index e) {
// seek_gamma follows (b, e) from the end of the current node going
// up.  New branches are selected, b will be adjusted.  seek_begin
// will find the index where gamma ends in the tree. The function
// returns a pointer to the node that contains seek_begin and
// seek_begin itself. It uses the skip count trick.
  suffixtree::node *seek_node=this;
  index seek_begin=seek_node->end();
  size_type g=e-b; // skip count trick
  size_type g_prime=seek_node->end()-seek_begin;
  while (g_prime<g) {
    g-=g_prime; // skip gamma it must match
    b+=g_prime; // skip b it must match
    seek_node=seek_node->children[*b]; // choose correct branch
    seek_begin=seek_node->begin(); // get new seek_begin
    g_prime=seek_node->end()-seek_node->begin(); // compute new g_prime
  }
  seek_begin+=g; // find end of string(b, e); don't change b
  return make_pair(seek_node, seek_begin);
}

suffixtree::position
suffixtree::node::extend_gamma(index b,
                               index e,
                               index cur,
                               size_type level,
                               node *&longest,
                               bool &cont) {
// extend_gamma adds the new letter at the end of the gamma. It
// uses one of the three types of extensions as described in
// (Gusfield). (b, e) is the new part of the branch, cur is the
// current position in the branch of the current node. level
// indicates the starting position in the string of this
// particular suffix. longest denotes the one past the end
// iterator of the longest suffix handled sofar. cont is set to
// false if it is clear that in the current phase, no new
// suffices need to be added.  (observation 1 and trick 2)
  node *seek_node=this; // seek_node points to node that gets added to
  element_type new_el=*(e-1); // get last element (e is one past the end)
  if (cur==end()) {
    if (children.size()==0) { // rule 1
      begin_i=b;
      end_i=e;
      cur=end_i;
    }
    else {
      map<element_type, node *>::iterator place=children.find(new_el);
      if (place==children.end()) { // rule 2
        seek_node=new node(e-1, e, this, owner, level); // new child
        children[new_el]=seek_node; // add a new child
        cur=e;
      }
      else { // rule 3
        seek_node=place->second;
        cur=seek_node->begin()+1; // first one always matches
        cont=false; // rule 3, so don't continue
      }
    }
  } 
  else { // cur!=end()
    if (*cur!=new_el) { // rule 2 break string
      // note that the divisions of nodes is very delicate. The
      // procedure needs to set all pointers correctly, because
      // suffixlinks and return values don't work otherwise.
      node *break_node=new node(begin(), cur, up, owner,
          number); // new begin part
      break_node->up->children[*begin()]=break_node;
      break_node->children[*cur]=this;
      break_node->children[new_el]=new node(e-1, e, break_node,
          owner, level); // new inserted part
      up=break_node; // adjust up to new (break) node
      begin_i=cur; // this becomes end part
      seek_node=break_node->children[new_el];
      cur=seek_node->end();
      if (owner->sw!=0) { // handle sw if present before we store new one
        owner->sw->suffix_link=break_node; // break node is new begin part
        owner->sw=0;
      }
      if(up!=0) { // skip root
        owner->sw=break_node;
      }
      return make_pair(seek_node, cur);
    }
    else { // rule 3
      cur++; // first one always matches
      cont=false; // rule 3, so don't continue
    }
  }
  if (owner->sw!=0) { // store sw's suffix link
    owner->sw->suffix_link=this;
    owner->sw=0;
  }
  return make_pair(seek_node, cur);
}

suffixtree::suffixtree(value_type s)
    :root(sentence.begin(), sentence.end(), 0, this, 0),
     j_i(0),
     init_current(false),
     rule3stopped(false),
     sw(0) {
// suffix tree constructor. s is the sentence.
  longest=&root; // setup longest (which is the entire sentence at the moment
  add_sentence(s); // actually build the suffix tree
}

suffixtree::size_type
suffixtree::find_position(value_type w) const {
// Check if sentence is a substring in the suffix tree. Throw a
// StringNotFound if it is not.
  suffixtree::index i=w.begin(); // search index in w
  const node *current=&root; // start at the root
  if (i==w.end()) { // empty strings are not found
    throw StringNotFound();
  }
  if (current->begin()==current->end()) { // empty substring
    current=current->find_child(*i); // find next branch
    if (current==0) { // couldn't find next branch
      throw StringNotFound();
    }
  }
  suffixtree::index seeker=current->begin(); // begin of substring
  while (*i==*seeker) {
    i++;
    if (i==w.end()) {
      break;
    }
    seeker++;
    if (seeker==current->end()) { // branch
      current=current->find_child(*i); // next branch
      if (current==0) { // couldn't find right branch
        throw StringNotFound(); // and string is not finished yet
      }
      seeker=current->begin(); // adjust seeker pointer
    }
  }
  if (i==w.end()) { // found position
    return current->give_number();
  }
  throw StringNotFound(); // should never be reached
}

vector<suffixtree::size_type>
suffixtree::find_all_positions(value_type w) const {
  suffixtree::index i=w.begin(); // search index in w
  vector<size_type> results;
  const node *current=&root; // start at the root
  if (i==w.end()) { // empty strings are not found
    return results;
  }
  if (current->begin()==current->end()) { // empty substring
    current=current->find_child(*i); // find next branch
    if (current==0) { // couldn't find next branch
      return results;
    }
  }
  suffixtree::index seeker=current->begin(); // begin of substring
  while (*i==*seeker) {
    i++;
    if (i==w.end()) {
      break;
    }
    seeker++;
    if (seeker==current->end()) { // branch
      current=current->find_child(*i); // next branch
      if (current==0) { // couldn't find right branch
        return results; // and string is not finished yet
      }
      seeker=current->begin(); // adjust seeker pointer
    }
  }
  if (i==w.end()) { // found position
    vector<size_type> subres=current->find_leaf_positions();
    results.insert(results.end(), subres.begin(), subres.end());
  }
  return results; // should never be reached
}

bool
suffixtree::self_test(value_type sentence) const {
// Return true if all affixes of sentence can be found in the
// suffixtree and return false otherwise.
  size_type found;
  bool res=true;
  size_type length=sentence.length();
  for (size_type i=0; i<=length; i++) { // begin of substring
    for (size_type j=1; j<=length-i; j++) { // end of substring
      try {
        found=find_position(sentence.substr(i, j)); // check
      }
      catch (StringNotFound) {
        cerr << "Test Results: Fail in string (" << i;
        cerr << "," << j << ")=" << sentence.substr(i, j) << endl;
        res=false;
      }
    }
  }
  if (res) {
    cerr << "Test Results: Success." << endl;
  }
  else {
    cerr << "Test Results: Failed." << endl;
  }
  return res;
}

void
suffixtree::print(ostream &os) const {
// print the suffixtree on os. Start with the root node.
  os << "root" << endl;
  root.print_node(os, 0); // level is 0, it is the root node
}

void
suffixtree::add_sentence(value_type s) {
// Build the suffix tree based on s.
  sentence=s; // remember sentence
  if (sentence.begin()==sentence.end()) { // no sentence
    return; // nothing to do
  }
  // build I_0
  node *first=new node(sentence.begin(), sentence.begin()+1, &root,
      this, 0); // extension starts at index 0
  root.assign_child(*sentence.begin(), first); // first char is a child of root
  e=sentence.begin()+1; // next char to do
  longest=first; // we have a longest substring
  suffixtree::index m=sentence.end();
  suffixtree::index b=sentence.begin();
  for (suffixtree::index i=b+2; i<=m; i++) { // i is `end' pointer
    SPA(i); // do SPA based on Gusfields description of the algorithm
  }
}

void
suffixtree::SPA(suffixtree::index i) {
// Do the SPA algorithm (described in Gusfield). We are currently
// handling phase i (i+1 in Gusfields algorithm).
  e=i; //special case longest string, increase e
  size_type j_index=j_i+1;
  if (!init_current) {
    current=make_pair(longest, longest->end());
    init_current=true;
  }
  bool cont=true;
  if (rule3stopped) {// we already know where to extend
    current=current.first->SEA(i-1, i, j_index, longest,
        current.second, cont, false);
    j_index++;
    rule3stopped=false;
    if (!cont) {
      rule3stopped=true;
      return;
    }
  }
  for (suffixtree::index j=sentence.begin()+j_index; j<i; j++) {
    current=current.first->SEA(j, i, j_index, longest,
        current.second, cont, true);
    if (!cont) { // no need to continue (step 3 has been done)
      rule3stopped=true; // remember that rule 3 stopped us
      break;
    }
    j_index++;
  }
  j_i=j_index-1; // remember j_i
}

// operator<< writes t on os using the print function of the
// suffixtree class.
ostream&
ns_suffixtree::operator<<(ostream &os,
                          const suffixtree &t) {
  t.print(os);
  return os;
}

// end of file: suffixtree.cpp
