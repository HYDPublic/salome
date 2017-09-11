# -*- coding: utf-8 -*-
# Copyright (C) 2011-2016  CEA/DEN, EDF R&D
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
"""
Python script for HOMARD
Utilitaires pour les tests
"""
__revision__ = "V1.4"

import os
#========================================================================
#========================================================================
def remove_dir(directory) :
  """
Empties, then removes a directory.
Copyright EDF-R&D 2013
  """
#
  l_aux = os.listdir(directory)
  for fic in l_aux :
    fic_a = os.path.join(directory, fic)
    if os.path.isdir(fic_a) :
      remove_dir(fic_a)
    else :
      os.remove(fic_a)
  os.rmdir(directory)
#
  return
#
#========================================================================
#========================================================================
def test_results(rep_test, test_name, dircase, n_iter_test_file, n_rep_test_file, destroy_dir = True) :
  """
Test of the result
rep_test: repertoire des tests
test_name: nom du test
dircase: repertoire des resultats du test
n_iter_test_file: numero de l'iteration a tester
n_rep_test_file: numero du repertoire de l'iteration a tester
destroy_dir: destruction du repertoire de calcul
Copyright EDF-R&D 2014
  """
  #
  test_file_suff = "apad.%02d.bilan" % n_iter_test_file
  rep_test_file = "I%02d" % n_rep_test_file
  #
  test_file = os.path.join(rep_test, test_name + "." + test_file_suff)
  mess_error_ref = "\nReference file: " + test_file
#
# Existence du fichier de référence
#
  try :
    file = open (test_file, "r")
    les_lignes_ref = file.readlines()
    file.close()
  except :
    mess_error = mess_error_ref + "\nThis file does not exist.\n"
    destroy_dir = False
    raise Exception(mess_error)
#
# Existence du fichier de l'exécution courante
#
  test_file = os.path.join(dircase, rep_test_file, test_file_suff)
  if os.path.isfile (test_file) :
    file = open (test_file, "r")
    les_lignes = file.readlines()
    file.close()
  else :
    mess_error  = "\nResult file: " + test_file
    mess_error += "\nThis file does not exist.\n"
    destroy_dir = False
    raise Exception(mess_error)
#
# Nombre de lignes identiques
#
  nblign = len(les_lignes_ref)
  if ( len(les_lignes) != nblign ):
    mess_error = mess_error_ref +  "\nResult file: " + test_file
    mess_error += "\nThe number of lines of the files are not the same.\n"
    destroy_dir = False
    raise Exception(mess_error)
#
# Comparaison des lignes, à l'esception de la date
#
  for num in range(nblign) :
    if ( "creation" not in les_lignes_ref[num] ) :
      if ( les_lignes_ref[num] != les_lignes[num] ) :
        message_erreur = "\nRefe : " + les_lignes_ref[num]
        message_erreur += "Test : " + les_lignes[num][:-1]
        message_erreur += "\nThe test is different from the reference."
        destroy_dir = False
        raise Exception(message_erreur)
#
# Destruction éventuelle du répertoire du calcul
#
  if destroy_dir:
    remove_dir(dircase)
#
  return
#
#========================================================================
#========================================================================
