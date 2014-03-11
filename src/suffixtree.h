///////////////////////////////////////////////////////////////////////////////*
// Menno van Zaanen
// mvzaanen@science.uva.nl   2001-2002
// mvzaanen@uvt.nl           2002-2004
// menno@ics.mq.edu.au       2004-
////////////////////////////////////////////////////////////////////////////////
// Filename: suffixtree.h
////////////////////////////////////////////////////////////////////////////////
// This file is part of the suffix tree package.
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the classes used in the
// suffix tree package. The class suffixtree contains the
// functionality of a suffix tree, while the class node is only used
// by the suffixtree class. Both classes are defined in the
// ns_suffixtree namespace.
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

#ifndef __suffixtree__
#define __suffixtree__

#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace ns_suffixtree {

class StringNotFound {
}; // Exception

class suffixtree {
  friend class suffixtree_iterator;

public:
  typedef string value_type;
  typedef value_type::size_type size_type;
  typedef value_type::value_type element_type;
  typedef value_type::const_iterator index;

protected:
  class node; // forward reference

  typedef pair<node *, index> position;

  class node {
  // node is a support class for the suffixtree class. Each
  // instance of node describes a part of the actual suffix tree. 
  // Actually, node describes the (sub)string that is between two
  // nodes. The node's suffixlink is attached to the node before
  // the begin of the (sub)string.
  public:
    // node creates an empty node
    node();

    // node creates a node with substring (b, e). u denotes the node
    // one level up, o is the owner of the node (the suffixtree
    // instance that it belongs to) and num is the offset in the
    // string where the suffix starts.
    node(index b,
         index e,
         node *u,
         suffixtree *o,
         size_type num);

    // ~node destructor.
    ~node();
  
    // begin returns the index denoting the begin of the (sub)string
    // in the node.
    const index
    begin() const;

    // end returns the index denoting the end of the (sub)string
    // in the node. (end actually returns the one past the end
    // pointer.)
    const index
    end() const;

    // give_number returns the number stored in the node.
    const size_type
    give_number() const;

    // print_node writes the contents of the node on os in a
    // pleasing format. depth indicates how deep the node is
    // expected to be in the tree. This is used to print offsets
    // denoting tree depth.
    void
    print_node(ostream &os, size_type depth) const;

    // SEA implements the Single Extension Algorithm (SEA) (see
    // Gusfield). (b, e) is the string to be inserted. level denotes
    // the start index of the suffix, longest is a pointer to the
    // node containing the current longest suffix. cur points to the
    // end of the current string, and cont is true to start with (to
    // indicate that new extensions should be handled) and set to
    // false if handling new extensions in the current phase is not
    // needed. seek indicates if sv and gamma need to be found.
    position
    SEA(index b,
        index e,
        size_type level,
        node *&longest,
        index cur,
        bool &cont,
        bool seek);

    // find_child finds the child node whose string begins with e.
    // A pointer to that node is returned. It returns 0 if the child
    // doesn't exist.
    vector<element_type>
    next_children() const;

    // find_child finds the child node whose string begins with e.
    // A pointer to that node is returned. It returns 0 if the child
    // doesn't exist.
    const node *
    find_child(const element_type &e) const;

    // assign_child sets val as a child of this node indexed with
    // value e. It is expected that e is the first value of the
    // string contained in val.
    void
    assign_child(const element_type e,
                 node *val);

    // find_leaf_positions finds all leaves reachable from this. It
    // returns all numbers that are found in these leaves. The
    // vector is unordered.
    vector<size_type>
    find_leaf_positions() const;

    // children_size returns the number of direct children at this
    // node.
    const size_type
    children_size() const;

  protected:
  private:
    // find_sv finds the sv node starting from cur. Once the v node
    // is found, the suffix_link is followed and b is adjusted. It
    // finds the new gamma (b, e) with b adjusted if needed. A
    // pointer to the sv node is returned.
    node *
    find_sv(index cur,
            index &b,
            index e);

    // seek_gamma follows (b, e) from the end of the current node.
    // New branches are selected, b will be adjusted. seek_begin
    // will find the index where gamma ends in the tree. The
    // function returns a position containing a pointer to a node
    // and an index indicating where gamma ends.
    position
    seek_gamma(index &b,
               index e);

    // extend_gamma adds the new letter at the end of the gamma. It
    // uses one of the three types of extensions as described in
    // (Gusfield). (b, e) is the new part of the branch, cur is the
    // current position in the branch of the current node. level
    // indicates the starting position in the string of this
    // particular suffix. longest denotes the one past the end
    // iterator of the longest suffix handled sofar. cont is set to
    // false if it is clear that in the current phase, no new
    // suffices need to be added. (observation 1 and trick 2)
    position
    extend_gamma(index b,
                 index e,
                 index cur,
                 size_type level,
                 node *&longest,
                 bool &cont);
  
    // children relates the children of the node by their first
    // element of the string.
    map<element_type, node *> children;

    // begin_i points to the begin of the substring of the current
    // node.
    index begin_i;

    // end_i points to the one past the end of the substring of the
    // current node.
    index end_i;
  
    // up points to the node one level up. If it is the root node,
    // it points to 0.
    node *up;

    // suffix_link points to another node in the tree. It's
    // semantics is that of a suffix_link (see (Gusfield)).
    node *suffix_link;

    // owner points to a suffixtree class that has this node as
    // a part of the actual tree.
    suffixtree *owner;

    // number is the offset in the string indicating where the
    // suffix has started.
    size_type number;
  };

public:
  // suffixtree creates a suffixtree based on s.
  suffixtree(value_type s);

  // find_position returns a position if w is present in the
  // suffixtree and otherwise it raises a StringNotFound exception. w need
  // not be a suffix.
  size_type
  find_position(value_type w) const;

  // find_all_positions finds all positions of substring w occurring
  // in the suffix tree. The vector need not be ordered.
  vector<size_type>
  find_all_positions(value_type w) const;

  // self_test returns true if all affixes of w can be found in the
  // suffixtree and it returns false if it doesn't. It dumps some
  // debug information on cerr.
  bool
  self_test(value_type w) const;

  // print writes the suffixtree on os.
  void
  print(ostream &os) const;

protected:
  // add_sentence actually builds the suffixtree based on s.
  void
  add_sentence(value_type s);

  // SPA implements the Single Pass Algorithm (SPA). i is what
  // Gusfield calls i+1.
  void
  SPA(index i);

  // sentence contains the sentence contained in the suffixtree
  value_type sentence;

  // root is the root node of the suffix tree
  node root;

  // longest is a pointer to the node containing the longest suffix
  node *longest;

  // e indexes into sentence denoting the longest extension handled
  // so far. It shifts one character further each phase.
  index e;

private:
  // static variables that need to be stored in a tree (to allow for
  // multiple suffixtrees).

  // j_i is used in the SPA function
  size_type j_i;

  // init_current is used in the SPA function and indicates whether
  // current has been initialised
  bool init_current;

  // current is used in the SPA function
  position current;

  // rule3stopped is used in the SPA function
  bool rule3stopped;

  // sw will be used as a suffix link 
  node *sw;
};

// operator<< writes t on os using the print function of the
// suffixtree class.
ostream &
operator<<(ostream &os,
           const suffixtree &t);

}
#endif // __suffixtree__
// end of file: suffixtree.h
