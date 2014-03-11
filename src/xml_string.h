////////////////////////////////////////////////////////////////////////////////
// Menno van Zaanen
// menno@ics.mq.edu.au  2005-
////////////////////////////////////////////////////////////////////////////////
// Filename: xml_string.h
////////////////////////////////////////////////////////////////////////////////
// This file is part of the AnswerFinder package.
////////////////////////////////////////////////////////////////////////////////
// This file contains the definition of the StringXML class.
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
// $Log: xml_string.h,v $
// Revision 1.3  2008/07/09 08:47:08  diego
// Improved the user interface of the executable and added license information
//
// Revision 1.2  2006/10/10 06:44:13  dsmith
// NamedEntity vector changed to set.
//
// Revision 1.2  2006/08/31 06:19:32  dsmith
// *** empty log message ***
//
// Revision 1.5  2006/03/15 04:43:58  menno
// Xerces support is removed.  StringXML is now a typedef of string.
//
// Revision 1.4  2005/07/26 06:20:28  menno
// Completely redid StringXML class.  It is now a subclass of string (instead of basic_string<XMLCh*>).
//
// Revision 1.3  2005/07/12 05:18:46  menno
// Fixed serialization problems.
//
// Revision 1.2  2005/07/06 03:04:14  menno
// Added serialization code.
//
// Revision 1.1  2005/05/03 04:52:39  menno
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __xml_string__
#define __xml_string__

#include <string>

using std::string;

namespace AF {

// StringXML is a typdef of type string.  If, for example, unicode
// support is needed, this needs to be changed, keeping the string
// interface.
typedef string StringXML;

} // namespace AF

#endif //__xml_string___
// end of file: xml_string.h
