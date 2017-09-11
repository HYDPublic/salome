// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#ifndef LIGHTAPP_EVENTFILTER_H
#define LIGHTAPP_EVENTFILTER_H

#include "LightApp.h"

#include <QObject>

#if defined WIN32
#pragma warning( disable: 4251 )
#endif


class SALOME_Event;

/*!
  Class provide event filter.
*/
class LIGHTAPP_EXPORT LightApp_EventFilter: public QObject 
{
public:
  static void Init();
  static void Destroy();

protected:
  LightApp_EventFilter();
  virtual ~LightApp_EventFilter();

private:
  /*! global event filter for qapplication */
  virtual bool eventFilter( QObject* o, QEvent* e );
  void processEvent( SALOME_Event* );

private:
  static LightApp_EventFilter* myFilter;
};

#if defined WIN32
#pragma warning( default: 4251 )
#endif

#endif