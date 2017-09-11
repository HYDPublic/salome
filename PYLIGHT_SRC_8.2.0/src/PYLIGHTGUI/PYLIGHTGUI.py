# Copyright (C) 2009-2016  OPEN CASCADE
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

#  Author : Roman NIKOLAEV, Open CASCADE S.A.S. (roman.nikolaev@opencascade.com)
#  Date   : 13/04/2009
#
import traceback
from SalomePyQt import *
import PYLIGHT_DataModel
from qtsalome import *
import libSALOME_Swig

import os
import libSalomePy
import vtk

# Get SALOME PyQt interface
sgPyQt = SalomePyQt()
# Get SALOME Swig interface
sg = libSALOME_Swig.SALOMEGUI_Swig()

################################################
# GUI context class
# Used to store actions, menus, toolbars, etc...
################################################

class GUIcontext:

    # constructor
    def __init__( self ):
        # Load File action
        sgPyQt.createAction(dict_actions["loadfile"], "Load text File", "Load text file")
        # Save File action
        sgPyQt.createAction(dict_actions["savefile"], "Save text File", "Save text file")
        # Insert Line action
        sgPyQt.createAction(dict_actions["insertLine"], "Insert Line", "Insert new text line")
        # Insert new line action
        sgPyQt.createAction(dict_actions["insertLine"], "Insert Line", "Insert new line")
        # Edit selected line action
        sgPyQt.createAction(dict_actions["editLine"], "Edit Line", "Edit selected line")
        # Remove selected line action
        sgPyQt.createAction(dict_actions["removeLine"], "Remove Lines", "Remove selected lines")
        # Clear paragraph
        sgPyQt.createAction(dict_actions["clearParagraph"], "Clear Paragraph", "Clear selected paragraph")
        # Clear all paragraphs
        sgPyQt.createAction(dict_actions["clearAll"], "Clear All", "Clear all paragraphs")
        # Display line
        sgPyQt.createAction(dict_actions["displayLine"], "Display Line", "Display selected line")
        # Erase line
        sgPyQt.createAction(dict_actions["eraseLine"], "Erase Line", "Erase selected line")
        # Separator
        separator = sgPyQt.createSeparator()

        # Get Menu 'File'
        menuFile = sgPyQt.createMenu( "File", -1, -1 )
        # Add actions in the menu 'File'
        sgPyQt.createMenu( separator,                menuFile, -1, 10)
        sgPyQt.createMenu( dict_actions["loadfile"], menuFile, 10 );
        sgPyQt.createMenu( dict_actions["savefile"], menuFile, 10 );
        sgPyQt.createMenu( separator,                menuFile, -1, 10)
        # Create 'PyLight' menu 
        menuPyLight = sgPyQt.createMenu( "PyLight", -1, -1, 50)
        # Add actions in the menu 'PyLight'
        sgPyQt.createMenu( dict_actions["insertLine"],  menuPyLight, 10 );
        sgPyQt.createMenu( dict_actions["editLine"],    menuPyLight, 10 );
        sgPyQt.createMenu( dict_actions["removeLine"],  menuPyLight, 10 );
        sgPyQt.createMenu( separator,                   menuPyLight, -1, 10);
        sgPyQt.createMenu( dict_actions["clearAll"],    menuPyLight, 10 );
        sgPyQt.createMenu( separator,                   menuPyLight, -1, 10);
        sgPyQt.createMenu( dict_actions["displayLine"], menuPyLight, 10 );
        sgPyQt.createMenu( dict_actions["eraseLine"],   menuPyLight, 10 );

        # Create DataModel
        self.DM = PYLIGHT_DataModel.PYLIGHT_DataModel()

        pass # def __init__( self )

    pass # class GUIcontext

################################################
# Global variables and functions
################################################

###
# get active study ID
###
def _getStudyId():
    return sgPyQt.getStudyId()

# verbosity level
__verbose__ = None

###
# Get verbose level
###
def verbose():
    global __verbose__
    if __verbose__ is None:
        try:
            __verbose__ = int( os.getenv( 'SALOME_VERBOSE', 0 ) )
        except:
            __verbose__ = 0
            pass
        pass
    return __verbose__

# study-to-context map
__study2context__   = {}
# current context
__current_context__ = None

###
# get current GUI context
###
def _getContext():
    global __current_context__
    return __current_context__

###
# set and return current GUI context
# study ID is passed as parameter
###
def _setContext( studyID ):
    global __study2context__, __current_context__
    if not __study2context__.has_key(studyID):
        __study2context__[studyID] = GUIcontext()
        pass
    __current_context__ = __study2context__[studyID]
    return __current_context__

################################################

# Create actions and menus
def initialize():
    if verbose(): print "PYLIGHTGUI::initialize()"
    return

# called when module is activated
# returns True if activating is successfull and False otherwise
def activate():
    if verbose() : print "PYLIGHTGUI.activate() : study : %d" % _getStudyId()
    ctx = _setContext( _getStudyId() )
    return True

# called when module is deactivated
def deactivate():
    if verbose() : print "PYLIGHTGUI.deactivate() : study : %d" % _getStudyId()
    pass

# called when active study is changed
# active study ID is passed as parameter
def activeStudyChanged( studyID ):
    if verbose() : print "PYLIGHTGUI.activeStudyChanged(): study : %d" % studyID
    ctx = _setContext( _getStudyId() )
    pass

# Process GUI action
def OnGUIEvent(commandID):
    if verbose() : print "PYLIGHTGUI::OnGUIEvent : commandID = %d" % commandID
    if dict_command.has_key( commandID ):
        try:
            dict_command[commandID]()
        except:
            traceback.print_exc()
    else:
       if verbose() : print "The command is not implemented: %d" % commandID
    pass

# Customize popup menu
def createPopupMenu(popup, context):
    if verbose() : print "PYLIGHTGUI.createPopupMenu(): context = %s" % context

    if context != 'ObjectBrowser':
        return
    
    studyId = _getStudyId()
    ctx = _setContext( studyId )
    selcount = sg.SelectedCount()
    if selcount == 1:
        entry = sg.getSelected( 0 )
        obj = ctx.DM.getObject(entry)
        if obj is not None:
            if obj.getText() != "\n":
                # Line is selected
                popup.addAction(sgPyQt.action(dict_actions["editLine"]))
                popup.addAction(sgPyQt.action(dict_actions["removeLine"]))
                popup.addSeparator()
                popup.addAction(sgPyQt.action(dict_actions["displayLine"]))
                popup.addAction(sgPyQt.action(dict_actions["displayLine"]))
                popup.addAction(sgPyQt.action(dict_actions["eraseLine"]))
                pass
            else:
                # Paragraph is selected
                popup.addAction(sgPyQt.action(dict_actions["insertLine"]))
                popup.addAction(sgPyQt.action(dict_actions["clearParagraph"]))
                pass
            pass
        else:
            onlyLines = True
            pass
        pass # if selcount == 1
    pass

# For saving data in the study
def saveFiles(prefix):
    if verbose(): print "PYLIGHTGUI::saveFile()"
    ctx = _setContext( _getStudyId() )
    postfix = "PYLIGHT.txt"
    filename = prefix+postfix
    ctx.DM.saveFile(filename)
    return postfix

# For restore data from the study
def openFiles(filelist):
    if verbose(): print "PYLIGHTGUI::openFile()"
    ctx = _setContext( _getStudyId() )
    filename =  filelist[0]
    filename.append(filelist[1])
    ctx.DM.loadFile(filename)
    return True

# Loading a text file
def loadfile():
    ctx = _setContext( _getStudyId() )
    aFilter = "Text files (*.txt)"
    filename = QFileDialog.getOpenFileName(sgPyQt.getDesktop(), "Open text file", "", aFilter, "Choose a text file to open")

    if isinstance(filename,tuple) and len(filename) >=2:
       filename = filename[0]

    filename = unicode(filename)

    if len(filename) == 0:
        return
    
    if os.access(str(filename),os.R_OK):
        ctx.DM.loadFile(filename)
    else:
        QMessageBox.warning(sgPyQt.getDesktop(),
                            "Error!",
                            "Can not read file:\n%s"%(filename))
        pass
    sg.updateObjBrowser(True)
    pass

# Saving a text file
def savefile():
    ctx = _setContext( _getStudyId() )
    aFilter = "Text files (*.txt)"
    filename = QFileDialog.getSaveFileName(sgPyQt.getDesktop(),"Save text file", "", aFilter, "Choose a text file to save")

    if isinstance(filename,tuple) and len(filename) >=2:
        filename = filename[0]

    filename = unicode(filename)

    if filename.endswith(".txt") == 0:
        filename+=".txt"
        pass

    fn = filename
    # Get directory name and check access
    if os.access(str(fn[:fn.rindex(os.path.sep)]), os.W_OK):
        ctx.DM.saveFile(filename)
    else:
        QMessageBox.warning(sgPyQt.getDesktop(),
                            "Error!",
                            "Can not save file:\n%s"%(filename))
        pass
    pass

def insertLine():
    '''
    Insert new line in the selected paragraph.
    '''
    ctx = _setContext( _getStudyId() )
    #Get result
    res = QInputDialog.getText(sgPyQt.getDesktop(),
                               "Add new line",
                               "Enter the text",
                               QLineEdit.Normal)
    if not res[1]: ### user click cancel button
        return
    
    text = res[0]
    # Nb selected objects
    selcount = sg.SelectedCount()
    # Nb object in the Data Model
    paragrCount = len(ctx.DM.getParagraphs())

    # Create first paragraph
    if paragrCount == 0:
        ctx.DM.createObject()
        # If line not empty create first line
        if text != "\n":
            ctx.DM.createObject(text,ctx.DM.getParagraphs()[0])
        sg.updateObjBrowser(True)
        return
    # Create paragraph
    if text == "\n":
        ctx.DM.createObject()
        sg.updateObjBrowser(True)
        return
    else:
        if selcount == 0:
            QMessageBox.warning(sgPyQt.getDesktop(),
                                'Error!',
                                'Please, select paragraph!')
            return
        if selcount == 1:
            entry = sg.getSelected( 0 )
            obj = ctx.DM.getObject(entry)
            if obj is not None:
                # Create line
                if(obj.getText() == "\n"):
                    ctx.DM.createObject(text,entry)
                    sg.updateObjBrowser(True);
                    return
                else:
                    QMessageBox.warning(sgPyQt.getDesktop(),
                                        'Error!',
                                        'Please, select paragraph!')
            elif selcount > 1:
                QMessageBox.warning(sgPyQt.getDesktop(),
                                    'Error!',
                                    'Please, select only one paragraph!')
    pass
        
        
# Edit selected line
def editLine():
    ctx = _setContext( _getStudyId() )
    if sg.SelectedCount() == 1:
        entry = sg.getSelected( 0 )
        obj = ctx.DM.getObject(entry)
        if(obj.getText() != "\n"):
            #Get text line
            res = QInputDialog.getText(sgPyQt.getDesktop(),
                                       "Edit line",
                                       "Enter the text",
                                       QLineEdit.Normal,
                                       PYLIGHT_DataModel.processText(obj.getText()))
            if not res[1]: ### user click cancel button
                return
            text = res[0]
            
            obj.setText(text)
        else:
            QMessageBox.information(sgPyQt.getDesktop(),
                                    'Info',
                                    'Please, select line!') 
    else:
        QMessageBox.information(sgPyQt.getDesktop(),
                                'Info',
                                'Please, select one line!')
    sg.updateObjBrowser(True);
    pass

# Remove selected lines
def removeLine():
    ctx = _setContext( _getStudyId() )
    selcount = sg.SelectedCount()
    onlyLines = True
    lines = []
    while selcount != 0:
        entry = sg.getSelected( selcount - 1)
        #Check what only lines selected
        if ctx.DM.getObject(entry).getText() == "\n":
            onlyLines = False
            break
        lines.append(entry)
        selcount = selcount-1
        pass
    if not onlyLines:
        return
    else:
        renderer=libSalomePy.getRenderer()
        for ln in lines:
            actor = getActor(ln)
            if actor is not None:
                renderer.RemoveActor(actor)
                pass
            pass
        ctx.DM.removeObjects(lines)
        sg.updateObjBrowser(True)
        pass
    pass

# Remove all lines from all paragraphs
def clearAll():
    ctx = _setContext( _getStudyId() )
    paragraphs = ctx.DM.getParagraphs()
    for paragr in paragraphs:
        lines = sgPyQt.getChildren(paragr)
        ctx.DM.removeObjects(lines)
        renderer=libSalomePy.getRenderer()
        for l in lines:
            actor = getActor(l)
            if actor is not None:
                renderer.RemoveActor(actor)
                pass
            pass
    sg.updateObjBrowser(True)
    pass

# Display the selected line
def displayLine():
    ctx = _setContext( _getStudyId() )
    if sg.SelectedCount() != 1:
        return
    entry = sg.getSelected(0)
    text = ctx.DM.getObject(entry).getText()
    if text == "\n":
        return
    renderer=libSalomePy.getRenderer()
    actor = getActor(entry)
    if actor is None:
        actor = vtk.vtkTextActor()
        dict_actors[entry] = actor
        pass
    center = renderer.GetCenter()
    actor.SetInput(str(text))
    actor.SetPosition(center[0],center[1])
    txtPr = vtk.vtkTextProperty()
    txtPr.SetFontSize(30)
    actor.SetTextProperty(txtPr)
    for act in  dict_actors.values():
        renderer.RemoveActor(act)
    renderer.AddActor(actor)
    pass

# Clear remove all lines under selected paragraph
def clearParagraph():
    ctx = _setContext( _getStudyId() )
    lines = sgPyQt.getChildren(sg.getSelected(0))
    ctx.DM.removeObjects(lines)
    sg.updateObjBrowser(True)
    pass

# Erase the selected line
def eraseLine():
    ctx = _setContext( _getStudyId() )
    if sg.SelectedCount() != 1:
        return
    entry = sg.getSelected(0)
    text = ctx.DM.getObject(entry).getText()
    if text == "\n":
        return
    renderer=libSalomePy.getRenderer()
    actor = getActor(entry)
    if actor is not None:
        renderer.RemoveActor(actor)
        pass
    pass

# Return vtkActor by entry
def getActor(entry):
    entry = str(entry)
    if dict_actors.has_key(entry):
        return dict_actors[entry]
    return None

# Define commands
dict_command = {
    951 : loadfile,
    952 : savefile,
    961 : insertLine,
    962 : editLine,
    963 : removeLine,
    964 : clearAll,
    971 : displayLine,
    972 : eraseLine,
    973 : clearParagraph,
    }

# Define actions
dict_actions = {
    "loadfile"   :    951,
    "savefile"   :    952,
    "insertLine" :    961,
    "editLine"   :    962,
    "removeLine" :    963,
    "clearAll"   :    964,
    "displayLine":    971,
    "eraseLine"  :    972,
    "clearParagraph": 973,
    }

# Define Actors
dict_actors = {}
