# Copyright (C) 2005-2016  OPEN CASCADE
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

###############################################################
# File    : RANDOMIZER.py
# Author  : Vadim SANDLER (OCN)
# Created : 13/07/05
###############################################################
#
import RANDOMIZER_ORB__POA
import SALOME_ComponentPy

class RANDOMIZER( RANDOMIZER_ORB__POA.RANDOMIZER,
                  SALOME_ComponentPy.SALOME_ComponentPy_i ):
    """
    The implementation of the RANDOMIZER interface
    """
    def __init__ ( self, orb, poa, contID, containerName, instanceName, interfaceName ):
        """
        Constructor
        """
        SALOME_ComponentPy.SALOME_ComponentPy_i.__init__( self, orb, poa, contID, containerName,instanceName, interfaceName, 0 )

    def getVersion( self ):
        """
        Get version information.
        """
        import salome_version
        return salome_version.getVersion("RANDOMIZER", True)

    def InitPoint( self ):
        """
        Generate random 2d-point:  x, y = [0,1)
        """
        self.beginService( "RANDOMIZER::InitPoint" )

        import random
        x = random.random()
        y = random.random()

        self.endService( "RANDOMIZER::InitPoint" )

        return x, y
    
    def NextIteration( self ):
        """
        Get next random iteration step: ret = {1,2,3}
        """
        self.beginService( "RANDOMIZER::NextIteration" )

        import random
        iter = random.randint( 1,3 )

        self.endService( "RANDOMIZER::NextIteration" )
        
        return iter
