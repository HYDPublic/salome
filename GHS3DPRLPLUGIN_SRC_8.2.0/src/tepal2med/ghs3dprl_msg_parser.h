// Copyright (C) 2007-2016  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// ---
// File   : ghs3dprl_mesh_parser.h
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#ifndef GHS3DPRL_MSG_PARSER_H
#define GHS3DPRL_MSG_PARSER_H

#include "ghs3dprl_mesh_wrap.h"
#include <QXmlDefaultHandler>

class ghs3dprl_msg_parser : public QXmlDefaultHandler
{
 public:
   bool startDocument();
   bool startElement(const QString&,
                     const QString&,
                     const QString& ,
                     const QXmlAttributes&);
   bool endElement(const QString&,
                   const QString&,
                   const QString&);
   bool characters(const QString&);

   ghs3dprl_mesh_wrap *mailw; //results maillages wrap
   bool verbose;

private:
   QString indent,
           typel,shorttypel,
           sendreceive,shortsendreceive,
           begin,
           version,
           neighbours,
           count,
           neighbour,
           indice,
           send,
           vertices,
           edges,
           faces,
           elements,
           receive;

   long etat,
        neighbourscount,neighbourcourant,neighbourindice,
        nbcount; //nb faces vertices edges elements expected
};

#endif
