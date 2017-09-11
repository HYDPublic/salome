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

from qtsalome import *

from omniORB import CORBA
from SALOME_NamingServicePy import *
from LifeCycleCORBA import *
import SALOMEDS
import SALOMEDS_Attributes_idl
import ATOMGEN_ORB

################################################
# Global definitions
#

# module name
__MODULE_NAME__ = "ATOMGEN"

# action IDs
__CMD_IMPORT_XML__ = 4000
__CMD_EXPORT_XML__ = 4001
__CMD_RUN_ALGO__   = 4002
__CMD_RUN_ALGO1__  = 4010

# study data
__study_data_map__ = {}

################################################
# Init GUI wrappers

# get SALOME PyQt interface
import SalomePyQt
sgPyQt = SalomePyQt.SalomePyQt()

# get SALOME Swig interface
import libSALOME_Swig
sg = libSALOME_Swig.SALOMEGUI_Swig()

################################################
# Global intializations

# init ORB
orb = CORBA.ORB_init( [''], CORBA.ORB_ID )

# create naming service instance
naming_service = SALOME_NamingServicePy_i( orb )

# create life cycle CORBA instance
lcc = LifeCycleCORBA( orb )

# get study manager
obj = naming_service.Resolve( '/myStudyManager' )
studyManager = obj._narrow( SALOMEDS.StudyManager )

################################################
# Internal methods

def tr( s ):
    """
    Translate the message
    """
    return qApp.translate( "ATOMGENGUI", s )

def processException( e ):
    """
    Prints exception info
    """
    print "Exception has been caught:", e
    pass

def warning( message, title = None ):
    """
    Show Warning message box
    """
    if not title: title = tr( "WARNING" )
    QMessageBox.warning( sgPyQt.getDesktop(), title, message )
    pass

# --- get ATOMGEN engine ---
engine = None
def _getEngine():
    """
    Gets an engine
    """
    global engine
    if not engine:
        engine = lcc.FindOrLoadComponent( "FactoryServerPy", __MODULE_NAME__ )
    return engine

# --- get active study ---
def _getStudy():
    """
    Gets actuve study
    """
    studyId = sgPyQt.getStudyId()
    study = studyManager.GetStudyByID( studyId )
    return study

myStudy = None

################################################
# Call back GUI methods
# 

def initialize():
    """
    This method is called when GUI module is being created
    and initialized.
    Creates menus, toolbars and performs other internal
    initialization
    """
    print "ATOMGENGUI::initialize"
    global __study_data_map__
    # get study id
    studyId = sgPyQt.getStudyId()
    if not __study_data_map__.has_key( studyId ):
        __study_data_map__[ studyId ] = {}
    # get selection object
    selection = sgPyQt.getSelection()
    selection.ClearIObjects()
    __study_data_map__[ studyId ][ "selection" ] = selection
    print "ATOMGENGUI::initialize done"
    pass

def windows():
    """
    This method is called when GUI module is being created
    and initialized.
    Should return a map of the SALOME dockable windows id's
    needed to be opened when module is activated.
    """
    print "ATOMGENGUI::windows"
    winMap = {}
    winMap[ SalomePyQt.WT_ObjectBrowser ] = Qt.LeftDockWidgetArea
    winMap[ SalomePyQt.WT_PyConsole ]     = Qt.BottomDockWidgetArea
    return winMap

def views():
    """
    This method is called when GUI module is being created
    and initialized.
    Should return a list of the SALOME view window types
    needed to be opened when module is activated.
    """
    print "ATOMGENGUI::views"
    return None

def activate():
    """
    This method is called when GUI module is being activated.
    """
    print "ATOMGENGUI::activate"
    # set current study
    global myStudy
    global __study_data_map__
    myStudy = _getStudy()
    _getEngine().setCurrentStudy( myStudy )
    studyId = myStudy._get_StudyId()

    # create actions
    __study_data_map__[ studyId ][ "actions" ] = {}
    a = sgPyQt.createAction( __CMD_IMPORT_XML__,
                             tr( "MEN_IMPORT_XML" ),
                             tr( "TOP_IMPORT_XML" ),
                             tr( "STB_IMPORT_XML" ) )
    __study_data_map__[ studyId ][ "actions" ][ __CMD_IMPORT_XML__ ] = a
    a = sgPyQt.createAction( __CMD_EXPORT_XML__,
                             tr( "MEN_EXPORT_XML" ),
                             tr( "TOP_EXPORT_XML" ),
                             tr( "STB_EXPORT_XML" ) )
    __study_data_map__[ studyId ][ "actions" ][ __CMD_EXPORT_XML__ ] = a

    # create menus
    fileMnu = sgPyQt.createMenu( QApplication.translate( "ATOMGENGUI", "MEN_FILE" ), -1, -1 )
    sgPyQt.createMenu( sgPyQt.createSeparator(), fileMnu, -1, 20 )
    sgPyQt.createMenu( __CMD_IMPORT_XML__, fileMnu, 20 )
    sgPyQt.createMenu( __CMD_EXPORT_XML__, fileMnu, 20 )
    sgPyQt.createMenu( sgPyQt.createSeparator(), fileMnu, -1, 20 )

    # connect selection
    selection = __study_data_map__[ studyId ][ "selection" ]
    selection.ClearIObjects()
    selection.currentSelectionChanged.connect( selectionChanged )
    global myRunDlg
    if myRunDlg:
        myRunDlg.close()
        myRunDlg = None
    return True

def deactivate():
    """
    This method is called when GUI module is being deactivated.
    """
    print "ATOMGENGUI::deactivate"
    # connect selection
    global myStudy
    studyId = myStudy._get_StudyId()
    selection = __study_data_map__[ studyId ][ "selection" ]
    selection.ClearIObjects()
    selection.currentSelectionChanged.disconnect( selectionChanged )
    global myRunDlg
    if myRunDlg:
        myRunDlg.close()
        myRunDlg = None
    myStudy = None
    pass

def activeStudyChanged( studyId ):
    """
    This method is called when active study is chaghed
    (user switches between studies desktops).
    <studyId> is an id of study being activated.
    """
    print "ATOMGENGUI::activeStudyChanged: study Id =", studyId
    global myStudy
    if myStudy and myStudy._get_StudyId() == studyId:
        return
    global myRunDlg
    if myRunDlg:
        myRunDlg.close()
        myRunDlg = None
    pass

def createPopupMenu( popup, context ):
    """
    This method is called when popup menu is requested
    by the user.
    Should analyze the selection and fill in the popup menu
    with the corresponding actions
    """
    print "ATOMGENGUI::createPopupMenu: popup =", popup, "; context =", context
    selected = selectedItems()
    isOk = False
    for entry in selected:
        sobject = myStudy.FindObjectID( entry )
        if sobject and sobject.GetObject() and sobject.GetObject()._narrow( ATOMGEN_ORB.Molecule ):
            isOk = True
            break
        pass
    a = sgPyQt.action( __CMD_RUN_ALGO__ )
    if isOk and a and context == "ObjectBrowser":
        popup.addAction(a)
        #a.addTo( popup )
    pass

def OnGUIEvent( commandId ):
    """
    This method is called when user activates some GUI action
    <commandId> is an ID of the GUI action.
    """
    print "ATOMGENGUI::OnGUIEvent: commandId =", commandId
    if dict_command.has_key( commandId ):
        try:
            dict_command[ commandId ]()
        except Exception, e:
            processException( e )
    else:
       print "ATOMGENGUI::OnGUIEvent: Action is not implemented: ", commandId
    pass

################################################
# GUI actions implementation

from rundlg_ui import Ui_RunDlg

class RunDlg(QDialog, Ui_RunDlg):
    """
    Run Algo simple dialog box
    """
    def __init__(self):
        """
        Constructor
        """
        QDialog.__init__(self, sgPyQt.getDesktop())
	self.setupUi(self)
        self.onCheckAll()
        self.selected = []
        self.selectionChanged()
        pass
        
    def onCheckAll(self):
        """
        Called when user switches <Process all> check box
        """
        self.acLab.setEnabled( not self.allCheck.isChecked() )
        self.acName.setEnabled( not self.allCheck.isChecked() )

        selection = __study_data_map__[ myStudy._get_StudyId() ][ "selection" ]
        if not self.allCheck.isChecked():
            selection.currentSelectionChanged.connect(self.selectionChanged)
        else:
            selection.currentSelectionChanged.connect(self.selectionChanged)
        pass

    def selectionChanged(self):
        """
        Called when selection is changed
        """
        self.selected = []
        selected = selectedItems()
        for entry in selected:
            sobject = myStudy.FindObjectID(entry)
            if sobject:
                obj = sobject.GetObject()
                if obj and obj._narrow( ATOMGEN_ORB.Molecule ):
                    self.selected.append( obj._narrow( ATOMGEN_ORB.Molecule ) )
        if len( self.selected ) == 1:
            self.acName.setText( self.selected[0].getName() )
        elif len( self.selected ) > 1:
            self.acName.setText(" %d objects selected"%len( self.selected ) )
        else:
            self.acName.setText( "" )
        pass
    
    def run(self):
        """
        Starts algo
        """
        data = [] # all data to be processed
        if not self.allCheck.isChecked():
            data = self.selected
        else:
            component = myStudy.FindComponent( "ATOMGEN" )
            if not component: return
            iter = myStudy.NewChildIterator( component )
            while iter.More():
                sobject = iter.Value()
                if sobject and sobject.GetObject() and sobject.GetObject()._narrow ( ATOMGEN_ORB.Molecule ):
                    data.append( sobject.GetObject()._narrow( ATOMGEN_ORB.Molecule ) )
                iter.Next()
            pass
        if not len( data ): return
        if not len(_getEngine().processData( data )):
            warning( "ALGO_ERROR" )
        else:
            sgPyQt.updateObjBrowser()
        pass

    def close(self):
        """
        Closes dialog box
        """
        global myRunDlg
        myRunDlg = None
        QDialog.close(self)
        pass
        
myRunDlg = None

def selectedItems():
    """
    Gets list of entries of selected objects
    """
    nbSel = sg.SelectedCount()
    selected = []
    for i in range(nbSel):
        selected.append(sg.getSelected(i))
    return selected

def selectionChanged():
    """
    Global selection changed slot
    """
    selected = selectedItems()
    print "--> Selected objects: %d"%len(selected)
    pass

def onImportXml():
    """
    Import XML file action slot
    """
    print "--> onImportXml() is started"
    filters = []
    filters.append( tr( "XML_FILES" ) )
    fileName = sgPyQt.getFileName( sgPyQt.getDesktop(),
                                   "",
                                   filters,
                                   tr( "IMPORT_XML" ),
                                   True )
    fileName = unicode(fileName)

    if len(fileName) > 0 :
        if not _getEngine().importXmlFile( str( fileName ) ):
            warning( "IMPORT_ERROR" )
        else:
            sgPyQt.updateObjBrowser()
    print "--> onImportXml() is finished"
    pass

def onExportXml():
    """
    Export XML file action slot
    """
    print "--> onExportXml() is started"
    filters = []
    filters.append( tr( "XML_FILES" ) )
    fileName = sgPyQt.getFileName( sgPyQt.getDesktop(),
                                   "",
                                   filters,
                                   tr( "EXPORT_XML" ),
                                   False )
    fileName = unicode(fileName)

    if len(fileName) > 0 :
        if not _getEngine().exportXmlFile( str( fileName ) ):
            warning( "EXPORT_ERROR" )
    print "--> onExportXml() is finished"
    pass

def onRunAlgo():
    print "--> onRunAlgo() is started !!!"
    global myRunDlg
    if not myRunDlg:
        myRunDlg = RunDlg()
    myRunDlg.show()
    myRunDlg.activateWindow()
    myRunDlg.setFocus()
    print "--> onRunAlgol() is finished"
    pass

################################################
# action-to-function map

dict_command = {
    __CMD_IMPORT_XML__ : onImportXml,
    __CMD_EXPORT_XML__ : onExportXml,
    __CMD_RUN_ALGO__   : onRunAlgo,
    __CMD_RUN_ALGO1__  : onRunAlgo,
    }
