# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import ATOMGEN_ORB__POA

class Atom(ATOMGEN_ORB__POA.Atom):
    """
    Atom interface implementation
    """
    def __init__(self, name, x, y, z):
        """
        Constructor
        """
        self.name = name
        self.x = x
        self.y = y
        self.z = z

    def getName(self):
        """
        Gets the name of the atom
        """
        return self.name

    def getX(self):
        """
        Gets x coordinate of the atom
        """
        return self.x

    def getY(self):
        """
        Gets y coordinate of the atom
        """
        return self.y

    def getZ(self):
        """
        Gets z coordinate of the atom
        """
        return self.z

class Molecule(ATOMGEN_ORB__POA.Molecule):
    """
    Atomic configuration interface implementatio
    """
    def __init__(self, name):
        """
        Constructor
        """
        self.name = name
        self.atoms = []
        pass

    def getName(self):
        """
        Gets the name of the configuration
        """
        return self.name

    def getNbAtoms(self):
        """
        GEts the number of the atoms
        """
        return len(self.atoms)

    def addAtom( self, atom ):
        """
        Adds an atom
        """
        self.atoms.append( atom )
        pass
        
    def getAtom(self, index):
        """
        Gets the atom by index
        """
        if index < 0 or index > len(self.atoms)-1:
            raise Exception( "Bad index" )
        return self.atoms[ index ]._this()
