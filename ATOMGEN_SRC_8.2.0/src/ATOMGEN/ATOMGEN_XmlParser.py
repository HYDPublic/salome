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

import xml.dom.minidom
from ATOMGEN_Data import *

__doc_name__  = "Atomic"
__doc_tag__   = "document"
__mol_tag__   = "molecule"
__atom_tag__  = "atom"
__name_attr__ = "name"
__x_attr__    = "x"
__y_attr__    = "y"
__z_attr__    = "z"

def readXmlFile( fileName ):
    """
    Reads the atomic data from the XML file
    """
    data = []
    doc = xml.dom.minidom.parse( fileName )
    if str( doc.doctype.name ) != __doc_name__:
        return # wrong file format
    docnode = doc.getElementsByTagName( __doc_tag__ )
    if not docnode:
        return # wrong file format
    mols = docnode[0].getElementsByTagName( __mol_tag__ )
    for mol in mols:
        c_name = str( mol.getAttribute( __name_attr__ ) )
        ac =  Molecule( c_name )
        atoms = mol.getElementsByTagName( __atom_tag__ )
        for atom in atoms:
            a_name = str( atom.getAttribute( __name_attr__ ) )
            x = float( atom.getAttribute( __x_attr__ ) )
            y = float( atom.getAttribute( __y_attr__ ) )
            z = float( atom.getAttribute( __z_attr__ ) )
            ac.addAtom( Atom( a_name, x, y, z ) )
        data.append( ac )
    return data

def writeXmlFile( filename, data ):
    """
    Writes the atomic data to the XML file
    """
    if not data:
        raise Exception( "Bad data" )
    doc = xml.dom.minidom.Document()
    doc.appendChild(xml.dom.minidom.DocumentType( __doc_name__ ) )
    docnode = doc.appendChild( doc.createElement( __doc_tag__ ) )
    for mol in data:
        molnode = doc.createElement( __mol_tag__ )
        molnode.setAttribute( __name_attr__, mol.name )
        for atom in mol.atoms:
            atomnode = doc.createElement( __atom_tag__ )
            atomnode.setAttribute( __name_attr__, atom.name )
            atomnode.setAttribute( __x_attr__, str( atom.x ) )
            atomnode.setAttribute( __y_attr__, str( atom.y ) )
            atomnode.setAttribute( __z_attr__, str( atom.z ) )
            molnode.appendChild( atomnode )
        docnode.appendChild( molnode )
    file = open( filename, "w" )
    file.write( doc.toprettyxml() )
    file.close()
    pass
