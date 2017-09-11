// Copyright (C) 2013-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : ATOMICGUI_ATOMICGUI.hxx
// Author : Roman NIKOLAEV, Open CASCADE S.A.S. (roman.nikolaev@opencascade.com)
//
#ifndef ATOMICGUI_ATOMICGUI_HXX
#define ATOMICGUI_ATOMICGUI_HXX

#if defined WIN32
#  if defined ATOMICGUI_EXPORTS || defined ATOMIC_EXPORTS
#    define ATOMICGUI_EXPORT __declspec( dllexport )
#  else
#    define ATOMICGUI_EXPORT __declspec( dllimport )
#  endif
#else
#  define ATOMICGUI_EXPORT
#endif

#endif // ATOMICGUI_ATOMICGUI_HXX
