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

#include "ATOMICGUI_DataModel.h"
#include "ATOMICGUI_DataObject.h"

#include <LightApp_Study.h>
#include <SUIT_Tools.h>
#include <SUIT_DataObjectIterator.h>
#include <CAM_Module.h>
#include <CAM_Application.h>

#include <CAM_Module.h>
#include <CAM_Application.h>

#include <qstring.h>
#include <qfile.h>
#ifndef QT_NO_DOM
#include <qdom.h>
#endif

#include <vector>
#include <string>
#include <fstream>

const QString doc_name     = "Atomic";
const QString doc_tag      = "document";
const QString molecule_tag = "molecule";
const QString atom_tag     = "atom";
const QString name_attr    = "name";
const QString x_attr       = "x";
const QString y_attr       = "y";
const QString z_attr       = "z";

/*! Constructor */
ATOMICGUI_DataModel::ATOMICGUI_DataModel( CAM_Module* module )
  : LightApp_DataModel( module ),
    myStudyURL( "" )
{
}

/*! Destructor */
ATOMICGUI_DataModel::~ATOMICGUI_DataModel()
{
}

/*! Open Data Model.  Build data structure from the given list of files. */
bool ATOMICGUI_DataModel::open( const QString& URL, CAM_Study* study, QStringList listOfFiles )
{
  myMolecules.clear();

  LightApp_Study* aDoc = dynamic_cast<LightApp_Study*>( study );
  if ( !aDoc )
    return false;

  LightApp_DataModel::open( URL, aDoc, listOfFiles );

  // The first list item contains path to a temporary
  // directory, where the persistent files was placed
  if ( listOfFiles.count() > 0 ) {
    QString aTmpDir ( listOfFiles[0] );

    // This module operates with a single persistent file
    if ( listOfFiles.size() == 2 ) {
      myStudyURL = URL;
      QString aFullPath = SUIT_Tools::addSlash( aTmpDir ) + listOfFiles[1];
      return importFile( aFullPath, aDoc );
    }
  }

  return false;
}

/*! Save Data Model.  Export data structure to temprorary files and return the list of files. */
bool ATOMICGUI_DataModel::save( QStringList& listOfFiles )
{
  bool isMultiFile = false; // TODO: decide, how to access this parameter

  LightApp_DataModel::save( listOfFiles );

  LightApp_Study* study = dynamic_cast<LightApp_Study*>( module()->application()->activeStudy() );

  QString aTmpDir(study->GetTmpDir( myStudyURL.toLatin1(), isMultiFile ).c_str());

  QString aFileName = SUIT_Tools::file( myStudyURL, false ) + "_ATOMICGUI.xml";
  QString aFullPath = aTmpDir + aFileName;
  bool ok = exportFile( aFullPath );

  listOfFiles.append( aTmpDir );
  listOfFiles.append( aFileName );

  printf( " \n saved to %s\n ", aFullPath.toLatin1().data() );

  return ok;
}

/*! Save Data Model under given name.  Export data structure to temprorary files and return the list of files. */
bool ATOMICGUI_DataModel::saveAs ( const QString& URL, CAM_Study* study, QStringList& listOfFiles )
{
  myStudyURL = URL;
  return save( listOfFiles );
}

/*! Called on Study closure */
bool ATOMICGUI_DataModel::close()
{
  myStudyURL = "";
  //myMolecules.clear();
  return LightApp_DataModel::close();
}

/*! Called on Study creation */
bool ATOMICGUI_DataModel::create( CAM_Study* study )
{
  myMolecules.clear();
  return true;
}

/*! Default implementation, always returns false so as not to mask study's isModified() */
bool ATOMICGUI_DataModel::isModified() const
{
  return false;
}

/*! Default implementation, always returns true so as not to mask study's isSaved() */
bool ATOMICGUI_DataModel::isSaved() const
{
  return true;
}

/*! Called on update of the structure of Data Objects */
void ATOMICGUI_DataModel::build()
{
  //VSR: module object should be created in a different way for "full" and "light" SALOME
  //ATOMICGUI_ModuleObject* modelRoot = dynamic_cast<ATOMICGUI_ModuleObject*>( root() );
  CAM_ModuleObject* modelRoot = dynamic_cast<CAM_ModuleObject*>( root() );
  if( !modelRoot )  {  // root is not set yet
    //VSR: module object should be created in a different way for "full" and "light" SALOME
    //modelRoot = new ATOMICGUI_ModuleObject( this, 0 );
    LightApp_Study* study = dynamic_cast<LightApp_Study*>( module()->application()->activeStudy() );
    modelRoot = createModuleObject( study->root() );
    setRoot( modelRoot );
  }

  // create 'molecule' objects under model root object and 'atom' objects under 'molecule'-s
  for ( int i = 0; i < myMolecules.count(); i++ ) {
    ATOMICGUI_DataObject* molObj = new ATOMICGUI_DataObject ( modelRoot, &myMolecules[i] );
    for ( int j = 0; j < myMolecules[ i ].count(); j++ ) {
      /*ATOMICGUI_DataObject* atomObj = */new ATOMICGUI_DataObject ( molObj, &myMolecules[i], j );
    }
  }

  //root()->dump();
}

/*! Loads data from the XML file. */
bool ATOMICGUI_DataModel::importFile( const QString& fileName, CAM_Study* study )
{
  bool res = false;

#ifndef QT_NO_DOM

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;

  QDomDocument doc;

  res = doc.setContent( &file );
  file.close();

  if ( !res )
    return false;

  QDomElement root = doc.documentElement();
  if ( root.isNull() || root.tagName() != doc_tag )
    return false;

  QDomNode molNode = root.firstChild();
  while ( res && !molNode.isNull() ) {
    res = molNode.isElement();
    if ( res ) {
      QDomElement molElem = molNode.toElement();
      if ( molElem.tagName() == molecule_tag && molElem.hasAttribute( name_attr ) ) {
	ATOMICGUI_AtomicMolecule aMol( molElem.attribute( name_attr ) );
        QDomNode atomNode = molNode.firstChild();
        while ( res && !atomNode.isNull() ) {
          res = atomNode.isElement();
          if ( res ) {
            QDomElement atomElem = atomNode.toElement();
            if ( atomElem.tagName() == atom_tag &&
                 atomElem.hasAttribute( name_attr ) && 
		 atomElem.hasAttribute( x_attr ) &&
		 atomElem.hasAttribute( y_attr ) &&
		 atomElem.hasAttribute( z_attr ) ) {
	      aMol.addAtom( atomElem.attribute( name_attr ),
			     atomElem.attribute( x_attr ).toDouble(),
			     atomElem.attribute( y_attr ).toDouble(),
			     atomElem.attribute( z_attr ).toDouble() );
            }
            else
              res = false;
          }
	  else
	    res = atomNode.isComment();

          atomNode = atomNode.nextSibling();
        }
	myMolecules.append( aMol );
      }
      else
        res = false;
    }
    else
      res = molNode.isComment();

    molNode = molNode.nextSibling();
  }

#endif

  return res;
}

/*! Saves data to the XML file */
bool ATOMICGUI_DataModel::exportFile( const QString& fileName )
{
  bool res = false;

#ifndef QT_NO_DOM

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;

  QDomDocument doc( doc_name );
  QDomElement root = doc.createElement( doc_tag );
  doc.appendChild( root );

  for ( int i = 0; i < myMolecules.count(); i++ ) {
    QDomElement molecule = doc.createElement( molecule_tag );
    molecule.setAttribute( name_attr, myMolecules[ i ].name() );
    root.appendChild( molecule );
    for ( int j = 0; j < myMolecules[ i ].count(); j++ ) {
      QDomElement atom = doc.createElement( atom_tag );
      atom.setAttribute( name_attr, myMolecules[ i ].atomName( j ) );
      atom.setAttribute( x_attr, myMolecules[ i ].atomX( j ) );
      atom.setAttribute( y_attr, myMolecules[ i ].atomY( j ) );
      atom.setAttribute( z_attr, myMolecules[ i ].atomZ( j ) );
      molecule.appendChild( atom );
    }
  }

  QString docStr = doc.toString();
  res = file.write( docStr.toLatin1(), docStr.length() ) == (int)docStr.length();
  file.close();

#endif

  return res;
}

/*! Adds a new molecule to the data structure */
QString ATOMICGUI_DataModel::createMolecule ()
{
  ATOMICGUI_AtomicMolecule mol;
  
  // temporary code to add a few atoms to a molecule..
  //mol.addAtom( "atom_1", 0, 0, 0 );
  //mol.addAtom( "atom_2", 0, 0, 0 );
  //mol.addAtom( "atom_3", 0, 0, 0 );
  // end of temporary code
  
  myMolecules.append( mol );

  QString id = QString( "ATOMICGUI_%1" ).arg( mol.id() );
 
  update();
  return id;
}

/*! Adds a new atom to the given molecule */
bool ATOMICGUI_DataModel::addAtom( const QString& entry, const QString& atom, 
				   const double x, const double y, const double z )
{
  ATOMICGUI_DataObject* obj = findMolecule( entry );
  if ( obj && obj->isMolecule() ) {
    obj->molecule()->addAtom( atom, x, y, z );
  }
  return true;
}

/*! Rename the given (by entry) object */
bool ATOMICGUI_DataModel::renameObj( const QString& entry, const QString& name )
{
  ATOMICGUI_DataObject* obj = findObject( entry );
  if ( obj ) {
    if ( obj->isMolecule() || obj->isAtom() ) {
      obj->molecule()->setName( name, obj->atomIndex() );
      return true;
    }
  }
  return false;
}

/*! Delete the given objects (list of entries) */
bool ATOMICGUI_DataModel::deleteObjs( const QStringList& entries )
{
  QMap<QString, ATOMICGUI_DataObject*> cmap;
  // first find all molecules
  for ( int i = 0; i < entries.count(); i++ ) {
    ATOMICGUI_DataObject* o = findObject( entries[i] );
    if ( o && o->isMolecule() )
      cmap[ entries[i] ] = o;
  }
  // then find all atoms
  typedef QList<int> IntList;
  QMap<ATOMICGUI_DataObject*, IntList> amap;
  for ( int i = 0; i < entries.count(); i++ ) {
    ATOMICGUI_DataObject* o = findObject( entries[i] );
    if ( o && o->isAtom() ) {
      ATOMICGUI_DataObject* c = dynamic_cast<ATOMICGUI_DataObject*>( o->parent() );
      if ( !c || cmap.find( c->entry() ) == cmap.end() )
	amap[ c ].append( o->atomIndex() );
    }
  }
  // now perform deleting
  int removed = 0;
  QMap<QString, ATOMICGUI_DataObject*>::Iterator it;
  for ( it = cmap.begin(); it != cmap.end(); ++it ) {
    for ( int i = 0; i < myMolecules.count(); i++ ) {
      if ( &myMolecules[ i ] == it.value()->molecule() ) {
	myMolecules.removeAt( i );
	removed++;
	break;
      }
    }
  }
  QMap<ATOMICGUI_DataObject*, IntList>::Iterator it1;
  for ( it1 = amap.begin(); it1 != amap.end(); ++it1 ) {
    IntList indices = it1.value();
    qSort( indices );
    for ( int i = indices.count() - 1; i >= 0; i-- ) {
      it1.key()->molecule()->deleteAtom( indices[i] );
      removed++;
    }
  }
  return removed;
}

/*! Returns the Data Object by entry */
ATOMICGUI_DataObject* ATOMICGUI_DataModel::findObject( const QString& entry )
{
  for ( SUIT_DataObjectIterator it( root(), SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it ) {
    ATOMICGUI_DataObject* obj = dynamic_cast<ATOMICGUI_DataObject*>( it.current() );
    if ( obj && obj->entry() == entry )
      return obj;
  }
  return 0;
}

/*! Returns the Data Object by entry.  If object is an Atom, its parent Molecule is returned. */
ATOMICGUI_DataObject* ATOMICGUI_DataModel::findMolecule( const QString& entry )
{
  for ( SUIT_DataObjectIterator it( root(), SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it ) {
    ATOMICGUI_DataObject* obj = dynamic_cast<ATOMICGUI_DataObject*>( it.current() );
    if ( obj && obj->entry() == entry ) {
      if ( obj->isMolecule() )
	return obj;
      else if ( obj->isAtom() )
	return dynamic_cast<ATOMICGUI_DataObject*>( obj->parent() );
      else
	return 0;
    }
  }
  return 0;
}
/*! Dump the data model in the python script. */
bool ATOMICGUI_DataModel::dumpPython( const QString& theURL,
				      CAM_Study* theStudy,
				      bool isMultiFile,
				      QStringList& theListOfFiles  ) {
  QString aScript = "from AtomicPy import *\n";
  QString aPrefix = "";
  if(isMultiFile) {
    aScript += "def RebuildData(theStudy):\n";
    aPrefix = "\t";
  }
  
  for ( int i = 0; i < myMolecules.count(); i++ ) {
    aScript += aPrefix + QString("mol_%1 = AtomicMolecule('").arg(i) + myMolecules[ i ].name()+"')\n";
    for ( int j = 0; j < myMolecules[ i ].count(); j++ ) {
      aScript += aPrefix + QString("mol_%1.addAtom('").arg(i) + myMolecules[ i ].atomName( j );
      aScript += QString("', %1, %2, %3)\n").arg(myMolecules[ i ].atomX( j )).arg(myMolecules[ i ].atomY( j )).arg(myMolecules[ i ].atomZ( j ));
    }
  }
  
  if(isMultiFile) {
    aScript += aPrefix+"pass\n";
  }

  LightApp_Study* study = dynamic_cast<LightApp_Study*>( theStudy );
  if(!study)
    return false;
  
  std::string aTmpDir = study->GetTmpDir( theURL.toLatin1().constData(), isMultiFile );
  std::string aFile = aTmpDir + "atomic_dump.tmp";

  std::ofstream outfile(aFile.c_str());
  outfile.write (aScript.toLatin1().data(),aScript.size());
  outfile.close();

  theListOfFiles.append(aTmpDir.c_str());
  theListOfFiles.append("atomic_dump.tmp");
  
  return true;
}
