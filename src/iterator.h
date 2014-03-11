///////////////////////////////////////////////////////////////////////////////*
// Menno van Zaanen
// mvzaanen@uvt.nl            2002-2004
// menno@ics.mq.edu.au        2004-
////////////////////////////////////////////////////////////////////////////////
// Filename: iterator.h
////////////////////////////////////////////////////////////////////////////////
// This file is part of the suffix tree package.
////////////////////////////////////////////////////////////////////////////////
// This file contains the implementation of the suffixtree_iterator
// class.  This class can be used to ``walk through'' a suffixtree.
// The iterator is implemented according to the design patterns book
// (not as in the STL standard).
// Iterators are defined in the ns_suffixtree namespace.
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

#ifndef __iterator__
#define __iterator__

#include <vector>
#include "suffixtree.h"

namespace ns_suffixtree {

class OutOfBounds {
}; // Exception

class suffixtree_iterator {
// suffixtree_iterator is a generic iterator class.
public:
  typedef suffixtree::value_type value_type;
  typedef suffixtree::size_type size_type;
  typedef suffixtree::element_type element_type;

  // iterator constructor.
  suffixtree_iterator(const suffixtree& st)
      :tree(st),current(&st.root) {
  }

  // first sets the iterator to the first element of the suffixtree
  void
  first() {
    current=&tree.root;
  }

  // next returns a vector of possible continuations from the current
  // position in the suffixtree. If one of the elements is used in
  // the next function (with argument), it is guaranteed not to throw
  // an exception.
  vector<element_type>
  next() const {
    return current->next_children();
  }

  // next sets the iterator to the next element of the suffixtree
  // (with respect to the current position). It will throw a
  // OutOfBounds exception if the current position cannot be moved
  // towards the element_type of the argument.
  void
  next(const element_type &e) {
    const suffixtree::node *next_node=current->find_child(e);
    if (next_node==0) {
      throw OutOfBounds();
    }
    current=next_node;
  }

  // size returns the number of children that can be reached from the
  // current node. If 0, then the node is a leaf (even though
  // sequential data may still be attached to it).
  size_type size() const {
    return current->children_size();
  }

  // is_done returns true if the current position is on a leaf, false
  // if it points to an internal node.
  bool
  is_done() const {
    return size()==0;
  }

protected:
  // current_begin returns the begin iterator of the current node.
  const suffixtree::index
  current_begin() const {
    return current->begin();
  }

  // current_end returns the end iterator of the current node.
  const suffixtree::index
  current_end() const {
    return current->end();
  }

  // current_number returns the number of the current node.
  const suffixtree::size_type
  current_number() const {
    return current->give_number();
  }

  // current_leaf_positions returns a vector of leaf positions that
  // can be reached from the current node.
  vector<size_type>
  current_leaf_positions() const {
    return current->find_leaf_positions();
  }

private:
  const suffixtree &tree;
  const suffixtree::node *current;
};

}
#endif // __iterator__
// end of file: iterator.h
