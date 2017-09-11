// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

#if !defined(ATOMICGUI_DATA_H)
#define ATOMICGUI_DATA_H

#include "ATOMICGUI_ATOMICGUI.hxx"

#include <qlist.h>
#include <qstring.h>

/*!
 * Class       : ATOMICGUI_AtomicMolecule, ATOMICGUI_AtomicMolecule::Atom
 * Description : Core classes for data of ATOMIC component
 */
class ATOMICGUI_EXPORT ATOMICGUI_AtomicMolecule
{
private:

  class Atom
  {
  public:
    Atom();
    Atom( const QString& name, const double x, const double y, const double z );
    QString name() const { return myName; }
    double  x()    const { return myX;    }
    double  y()    const { return myY;    }
    double  z()    const { return myZ;    }
    
    int     id ()  const { return myId;   }
    
  private:
    QString    myName;
    double     myX;
    double     myY;
    double     myZ;
    int        myId;
    static int myMaxId;

    friend class ATOMICGUI_AtomicMolecule;
  };

public:
  ATOMICGUI_AtomicMolecule( const QString& name = QString::null );
  virtual ~ATOMICGUI_AtomicMolecule();

  void addAtom( const QString& atom, const double x, const double y, const double z );
  void deleteAtom( const int index );
  
  int         id     () const { return myId; }
  QString     name   () const { return myName; }
  int         count  () const { return myAtoms.count(); }

  int         atomId   ( const int index ) const;
  QString     atomName ( const int index ) const;
  double      atomX    ( const int index ) const;
  double      atomY    ( const int index ) const;
  double      atomZ    ( const int index ) const;

  void        setName( const QString& name, const int index = -1 );

private:
  QString          myName;
  QList<Atom> myAtoms;
  int              myId;
  static int       myMaxId;
};

#endif // ATOMICGUI_DATA_H
