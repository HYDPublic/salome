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

import ATOMGEN_ORB
import ATOMGEN_ORB__POA
import SALOME_ComponentPy
import SALOME_DriverPy
import SALOMEDS

#--- init ORB ---
from omniORB import CORBA
myORB = CORBA.ORB_init([''], CORBA.ORB_ID)

def ObjectToString(object):
    return myORB.object_to_string(object)

def StringToObject(string):
    return myORB.string_to_object(string)

#--- init POA ---
from omniORB import PortableServer
myPOA = myORB.resolve_initial_references("RootPOA");

def ObjectToServant(object):
    return myPOA.reference_to_servant(object)

__entry2IOR__ = {}

__pyEngineName__ = "atomgen"

#--- Engine implementation ---
class ATOMGEN( ATOMGEN_ORB__POA.ATOMGEN_Gen,
               SALOME_ComponentPy.SALOME_ComponentPy_i,
               SALOME_DriverPy.SALOME_DriverPy_i ):
    """
    ATOMGEN component engine
    """
    def __init__ ( self, orb, poa, contID, containerName, instanceName,
                   interfaceName ):
        """
        Constructor
        """
        SALOME_ComponentPy.SALOME_ComponentPy_i.__init__( self, orb, poa,
                                                          contID, containerName,
                                                          instanceName, interfaceName, 0 )
        SALOME_DriverPy.SALOME_DriverPy_i.__init__( self, interfaceName )
        
        self._naming_service = SALOME_ComponentPy.SALOME_NamingServicePy_i( self._orb )
        self.study = None
        self.studyData = {}
        self.entry2PyName = {}
        pass

    def getVersion( self ):
        import salome_version
        return salome_version.getVersion("ATOMGEN", True)
    
    def getData( self, studyID ):
        """
        Gets study data
        """
        if self.studyData.has_key( studyID ):
            l = []
            for m in self.studyData[ studyID ]:
                l.append( m._this() )
            return l
        print "ATOMGEN: getData() failed because studyID ", str( studyID ), " was not found"
        return []

    def Save( self, component, URL, isMultiFile ):
        """
        Saves data.
        Nothing to do here because in our case all data
        are stored in the SALOMEDS attributes.
        """
        return ""

    def Load( self, component, stream, URL, isMultiFile ):
        """
        Loads data
        """
        global __entry2IOR__
        __entry2IOR__.clear()
        import StringIO, pickle
        study = component.GetStudy()
        self.setCurrentStudy( study )
        iter = study.NewChildIterator(component)
        data = []
        while iter.More():
            sobject = iter.Value()
            iter.Next()
            found, attr = sobject.FindAttribute("AttributeName")
            if not found: continue
            from ATOMGEN_Data import Molecule, Atom
            mol = Molecule(attr.Value())
            __entry2IOR__[sobject.GetID()] = ObjectToString(mol._this())
            iter1 = study.NewChildIterator(sobject)
            while iter1.More():
                sobject1 = iter1.Value()
                iter1.Next()
                found, attr = sobject1.FindAttribute("AttributeName")
                if not found: continue
                name = attr.Value()
                x = y = z = None
                iter2 = study.NewChildIterator(sobject1)
                while iter2.More():
                    sobject2 = iter2.Value()
                    iter2.Next()
                    found, attr1 = sobject2.FindAttribute("AttributeName")
                    if not found: continue
                    found, attr2 = sobject2.FindAttribute("AttributeReal")
                    if not found: continue
                    if attr1.Value() == "x": x = attr2.Value()
                    if attr1.Value() == "y": y = attr2.Value()
                    if attr1.Value() == "z": z = attr2.Value()
                if None not in [x, y, z]:
                    atom = Atom(name, x, y, z)
                    mol.addAtom(atom)
                    __entry2IOR__[sobject1.GetID()] = ObjectToString(atom._this())
                pass
            data.append(mol)
        self.appendData( data, False )
        return 1

    def IORToLocalPersistentID(self, sobject, IOR, isMultiFile, isASCII):
        """
        Gets persistent ID for the CORBA object.
        It's enough to use study entry.
        """
        return sobject.GetID()

    def LocalPersistentIDToIOR(self, sobject, persistentID, isMultiFile, isASCII):
        "Converts persistent ID of the object to its IOR."
        global __entry2IOR__
        if __entry2IOR__.has_key(persistentID):
            return __entry2IOR__[persistentID]
        return ""

    def Close( self, component ):
        """
        Called when study is closed
        """
        study = component.GetStudy()
        if study and self.studyData.has_key( study._get_StudyId() ):
            del self.studyData[ study._get_StudyId() ]
        if study == self.study:
            self.study = None
        pass

    def CanPublishInStudy( self, IOR ):
        """
        Returns True if the object can be published.
        Nothing to do here. Usually this method should perform
        check for the object type.
        """
        return True

    def PublishInStudy( self, study, sobject, object, name ):
        """
        Publishes the object in the study.
        """
        if study and object and object._narrow(ATOMGEN_ORB.Molecule):
            builder = study.NewBuilder()
            builder.NewCommand()
            # get or create component object
            father = study.FindComponent(self._ComponentDataType)
            if father is None:
                builder
                father = builder.NewComponent(self._ComponentDataType)
                attr = builder.FindOrCreateAttribute(father, "AttributeName")
                attr.SetValue(self._ComponentDataType)
                builder.DefineComponentInstance(father, self._this())
                pass
            # publish molecule
            sobject = builder.NewObject(father)
            attr = builder.FindOrCreateAttribute(sobject, "AttributeName")
            if not name:
                name = object.getName()
            attr.SetValue(name)
            attr = builder.FindOrCreateAttribute(sobject, "AttributeIOR")
            attr.SetValue(ObjectToString(object))
            # publish atoms
            for i in range(object.getNbAtoms()):
                atom = object.getAtom( i )
                sobject1 = builder.NewObject(sobject)
                attr = builder.FindOrCreateAttribute(sobject1, "AttributeName")
                attr.SetValue(atom.getName())
                attr = builder.FindOrCreateAttribute(sobject1, "AttributeIOR")
                attr.SetValue(ObjectToString(atom))
                sobject2 = builder.NewObject(sobject1)
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeName")
                attr.SetValue("x")
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeReal")
                attr.SetValue(atom.getX())
                sobject2 = builder.NewObject(sobject1)
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeName")
                attr.SetValue("y")
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeReal")
                attr.SetValue(atom.getY())
                sobject2 = builder.NewObject(sobject1)
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeName")
                attr.SetValue("z")
                attr = builder.FindOrCreateAttribute(sobject2, "AttributeReal")
                attr.SetValue(atom.getZ())
            builder.CommitCommand()
            return sobject
        return None

    def setCurrentStudy( self, study ):
        """
        Sets the active study
        """
        self.study = study
        if self.study and not self.getData( self.study._get_StudyId() ):
            studyID = self.study._get_StudyId()
            self.studyData[ studyID ] = []
            self.entry2PyName[ studyID ] = {}
            print "ATOMGEN: init new arrays for studyID ", str( studyID ) 
        pass

    def importXmlFile( self, fileName ):
        """
        Imports atomic data from external XML file
        and publishes the data in the active study
        """
        if self.study:
            # import file
            from ATOMGEN_XmlParser import readXmlFile
            new_data = readXmlFile( fileName )
            entries = self.appendData( new_data )
            if len(entries)  > 0 :
                cmd = "[" + ", ".join(entries) + "] = "+__pyEngineName__
                cmd += ".importXmlFile('" + fileName + "')"
                attr = self._getTableAttribute()                
                if attr is not None:
                    attr.PutValue(cmd,attr.GetNbRows()+1,1)
            res = []
            for m in new_data:                
                res.append(m._this())
            return res
        return []

    def exportXmlFile( self, fileName ):
        """
        Exports atomic data from the active study to
        the external XML file
        """
        if self.study:
            from ATOMGEN_XmlParser import writeXmlFile
            studyID = self.study._get_StudyId()
            writeXmlFile( fileName, self.studyData[ studyID ] )
            cmd = __pyEngineName__+ ".exportXmlFile('" + fileName + "')"
            attr = self._getTableAttribute()
            if attr is not None:
                attr.PutValue(cmd,attr.GetNbRows()+1,1)
            return True
        return False

    def processData( self, data ):
        """
        Perform some specific action on the atomic data
        """
        if not self.study: return []
        nb_steps = 5
        new_data = []
        dx = 10.0
        dy = 5.0
        dz = 3.0
        for i in range( nb_steps ):
            for mol in data:
                new_mol = self._translateMolecule( mol, i, dx * (i+1), dy * (i+1), dz * (i+1) )                
                new_data.append( new_mol )
        entries = self.appendData( new_data )
        if len(entries) > 0 :
            lst = []
            for m in data:
                ior = ObjectToString(m)
                so = self.study.FindObjectIOR(ior)
                lst.append(so.GetID())
                
            cmd =  "[" + ", ".join(entries) + "] = "+__pyEngineName__
            cmd += ".processData([" + ", ".join(lst) + "])"
            attr = self._getTableAttribute()
            if attr is not None:
                attr.PutValue(cmd, attr.GetNbRows()+1,1)
        res = []
        for m in new_data:
            res.append(m._this())
        return res

    def appendData( self, new_data, publish = True ):
        """
        Add new molecules to data published under current study
        """
        entries = []
        # activate servants
        if not self.study:
            return entries
        for mol in new_data:
            for i in range(mol.getNbAtoms()):
                mol.atoms[ i ]._this()
            mol_servant = mol._this()
            if publish :
                so = self.PublishInStudy(self.study, None, mol_servant, mol_servant.getName())
                if so is not None:
                    entries.append(so.GetID())
        # store data
        studyID = self.study._get_StudyId()
        if self.studyData.has_key( studyID ):
            data = self.studyData[ studyID ]
            data += new_data
            self.studyData[ studyID ] = data
            return entries
        print "ATOMGEN: could not append new data because studyID ", str( studyID ), " was not found"
        return entries

    def _translateMolecule( self, mol, i, dx, dy, dz ):
        """
        Translates atomic molecule along x, y, z axis
        by given distances dx, dy, dz
        Returns translated molecule
        """
        mol = ObjectToServant( mol )
        from ATOMGEN_Data import Molecule, Atom
        new_mol = Molecule( mol.name + "_translated_" + str( i ) )
        for atom in mol.atoms:
            new_mol.addAtom( Atom( atom.name, atom.x + dx, atom.y + dy, atom.z + dz ) )
        return new_mol


    def DumpPython(self, theStudy, isPublished, isMultiFile):
        script = []
        prefix = ""
        if isMultiFile :
            script.append("import salome")
            script.append("\n")
            prefix = "\t"
        script.append("import ATOMGEN\n")
        script.append(__pyEngineName__ + " = salome.lcc.FindOrLoadComponent(\"FactoryServerPy\", \"ATOMGEN\")")
        
        if isMultiFile :
            script.append("def RebuildData(theStudy):\n")
            script.append(prefix+__pyEngineName__ + ".setCurrentStudy(theStudy)\n")
        else:
            script.append(__pyEngineName__ + ".setCurrentStudy(theStudy)\n")
        
        attr = self._getTableAttribute()
        if attr is not None:
            for idx in range(attr.GetNbRows()):
                s = prefix + attr.GetValue(idx+1,1)
                script.append(s)
        
        if isMultiFile :
    	    script.append(prefix+"pass")
    	else:
    	    script.append("\n")
    	script.append("\0")
        
        all = "\n".join(script)
        self._getPyNames()        
        studyID = self.study._get_StudyId()
        
        for k in self.entry2PyName[studyID].keys() :
            all = all.replace(k,self.entry2PyName[studyID][k])
        
        return (all,1)
    
    def _getTableAttribute(self):
        """
        Create and return AttributeTableOfString attribute
        on the root module object. This attribute used to store
        python script.
        """
        if self.study is not None:            
            father = self.study.FindComponent(self._ComponentDataType)
            if father is not None :
                attribute = None
                hasAttribute, attribute = father.FindAttribute("AttributeTableOfString")
                if not hasAttribute:
                    builder = self.study.NewBuilder()
                    attribute = builder.FindOrCreateAttribute(father,"AttributeTableOfString")
                    attribute.SetNbColumns(1)
                    return attribute
                return attribute
        return None

    def _getPyNames(self):
        """
        Generate python name for the each object published in the study.
        """
        if self.study is None:
            return
        
        studyID = self.study._get_StudyId()
        self.entry2PyName[studyID] = {}
        iter = self.study.NewChildIterator(self.study.FindComponent(self._ComponentDataType))
        while iter.More():
            sobject = iter.Value()
            iter.Next()            
            found, attr = sobject.FindAttribute("AttributeName")
            if not found: continue
            self.entry2PyName[studyID][sobject.GetID()] = self._fixPythonName(attr.Value())            
            
    def _fixPythonName(self, in_str):
        """
        Make a string be a valid python name
        """
        import string
        allowed = string.ascii_lowercase+string.ascii_uppercase + string.digits + "_"
        newstring = ""
        for i in range(len(in_str)):
            ch = in_str[i]
            if ch in allowed:
                newstring += ch
                
        if len(newstring) == 0 :
            newstring = "mol_1"

        if newstring[0] in string.digits:
            newstring = "a"+newstring
            
        studyID = self.study._get_StudyId()
        
        if newstring in self.entry2PyName[studyID].values():
           newstring = self._fixPythonName( newstring + "_1")
        return newstring
