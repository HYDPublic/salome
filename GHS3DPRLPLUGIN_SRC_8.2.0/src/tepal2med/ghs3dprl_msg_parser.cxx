// Copyright (C) 2007-2016  CEA/DEN, EDF R&D
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

// ---
// File   : ghs3dprl_mesh_parser.cxx
// Author : Christian VAN WAMBEKE (CEA) (from Hexotic plugin Lioka RAZAFINDRAZAKA)
// ---
//
#include <iostream>

#include "ghs3dprl_msg_parser.h"
#include "ghs3dprl_mesh_wrap.h"

//************************************
bool ghs3dprl_msg_parser::startDocument()
{
  indent="";
  begin="distene_msg";
  version="version";
  neighbours="neighbours";
  count="count";
  neighbour="neighbour";
  indice="indice";
  send="send";
  vertices="vertices";
  edges="edges";
  faces="faces";
  elements="elements";
  receive="receive";
  etat=0;
  neighbourscount=0;
  neighbourcourant=0;
  neighbourindice=0;
  nbcount=0;
  verbose=false; //true; //false;
  return true;
}

//************************************
bool ghs3dprl_msg_parser::startElement(const QString &namespaceURI,
                                       const QString &localName,
                                       const QString &qName,
                                       const QXmlAttributes &attrs)
{
  bool ok;
  if (verbose) std::cout<<indent.toLatin1().constData()<<qName.toLatin1().constData()<<std::endl;
  typel=qName; shorttypel="";
  indent += "   ";
  if (qName==version)
  {
    etat=1;
    return true;
  }
  
  if (qName==neighbours)
  {
    etat=2;
    if (attrs.count()>0 && attrs.localName(0)==count)
    {
      neighbourscount=attrs.value(0).toLong(&ok, 10);
      neighbourcourant=-1;
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<neighbourscount<<std::endl;
    }
    return true;
  }
  
  if (qName==neighbour)
  {
    etat=3;
    if (attrs.count()>0 && attrs.localName(0)==indice)
    {
      neighbourindice=attrs.value(0).toLong(&ok, 10);
      neighbourcourant++;
      //mailw.neighbourindices[mailw.neighbourcourant]=neighbourindice;
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<neighbourindice<<std::endl;
    }
    return true;
  }
  
  if (qName==send)
  {
    etat=4;
    sendreceive="s/r=send ";
    shortsendreceive="SE ";
    return true;
  }
  
  if (qName==receive)
  {
    etat=9;
    sendreceive="s/r=receive ";
    shortsendreceive="RE ";
    return true;
  }
  
  if (qName==vertices)
  {
    etat=5;
    shorttypel="VE ";
    if (attrs.count()>0 && attrs.localName(0)==count)
    {
      nbcount=attrs.value(0).toLong(&ok, 10);
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<nbcount<<std::endl;
    }
    return true;
  }
  
  if (qName==edges)
  {
    etat=6;
    shorttypel="ED ";
    if (attrs.count()>0 && attrs.localName(0)==count)
    {
      nbcount=attrs.value(0).toLong(&ok, 10);
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<nbcount<<std::endl;
    }
    return true;
  }
  
  if (qName==faces)
  {
    etat=7;
    shorttypel="FA ";
    if (attrs.count()>0 && attrs.localName(0)==count)
    {
      nbcount=attrs.value(0).toLong(&ok, 10);
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<nbcount<<std::endl;
    }
    return true;
  }
  
  if (qName==elements)
  {
    etat=8;
    shorttypel="EL ";
    if (attrs.count()>0 && attrs.localName(0)==count)
    {
      nbcount=attrs.value(0).toLong(&ok, 10);
      if (verbose) std::cout<<indent.toLatin1().constData()<<attrs.localName(0).toLatin1().constData()<<
		     "="<<nbcount<<std::endl;
    }
    return true;
  }
  
  /*for( int i=0; i<attrs.count(); i++ )
  {
    std::cout<<indent.toLatin1().constData()<<attrs.localName(i).toLatin1().constData()<<"="<<attrs.value(i)<<std::endl;
  }*/
  
  return true;
}

//************************************
bool ghs3dprl_msg_parser::endElement(const QString &namespaceURI,
                                     const QString &localName,
                                     const QString &qName)
{
  indent.remove((uint)0,3);
  if (verbose) std::cout<<indent.toLatin1().constData()<<"\\"<<qName.toLatin1().constData()<<std::endl;
  nbcount=0; //precaution
  if (qName==send || qName==receive)
  {
    sendreceive="";
    shortsendreceive="";
    return true;
  }
  if (qName==begin)
  {
    mailw->nbfiles++;
    return true;
  }
  return true;
}

//************************************
bool ghs3dprl_msg_parser::characters(const QString &strini)
{
  bool ok;
  med_int *tmint=NULL;
  long nb=0;
  //filtre rc,lf,tab et blancs successifs
  QString str=strini.simplified();
  //if ( str.length() == 1 && str=="\n" )
  if (str.length()==0)
  {
    //std::cout<<"EMPTY_LINE_CR"<<std::endl;
    return true;
  }
  else
  {
    if (etat==1)
    {
      double ver=str.toDouble(&ok);
      if (!ok || ver!=1e0)
      {
      std::cout<<"version "<<str.toLatin1().constData()<<" fichier .msg inconnue"<<std::endl;
      return false;
      }
      else return true;
    }
    nb=str.count(' ',Qt::CaseSensitive) + 1; //nb chiffres detectes
    if (nb>1)
    {
      //lecture vecteurs d'entiers separateur blanc
      long i=0;
      tmint=new med_int[nb];
      //printf("%staille attendue=%i taille vue=%i\n",(const char *)indent.toLatin1().constData(),nbcount,nb);
      do
      {
      tmint[i]=str.section(' ',i,i).toLong(&ok);
      //printf("tmint[%i]=%i\n",i,tmint[i]);
      i++;
      } while ((i<nb) && ok );
      if (i<nb)
      {
      std::cout<<"pb conversion "<<i<<" eme entier="<<str.section(' ',i-1,i-1).toLatin1().constData()<<std::endl;
      std::cout<<indent.toLatin1().constData()<<"etat="<<etat<<" nb="<<nb<<std::endl<<str.toLatin1().constData()<<std::endl;
      }
    }
    if (nb==1)
    {
      tmint=new med_int[nb];
      tmint[0]=str.toLong(&ok);
      if (!ok)
      {
      std::cout<<"pb conversion 1er entier="<<str.toLatin1().constData()<<std::endl;
      std::cout<<indent.toLatin1().constData()<<"etat="<<etat<<" nb="<<nb<<std::endl;
      }
    }
    
  }
  //lecture ok stockage de tlong dans mailw.mestab
  CVWtab *montab=new CVWtab(nb,tmint);
  QString tmp;
  /*std::cout<<"InsertKey type="<<typel<<" "<<sendreceive<<
    " neighbour="<<neighbourindice<<
    " file="<<mailw->nofile<<std::endl;*/
  tmp=tmp.sprintf("MS%ld NE%ld ",
  mailw->nofile,neighbourindice)+
  shorttypel+shortsendreceive;
  tmp=tmp.simplified();
  ok=mailw->insert_key(tmp,montab);
  return true;
}

