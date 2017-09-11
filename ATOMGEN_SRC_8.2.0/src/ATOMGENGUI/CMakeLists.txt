# Copyright (C) 2013-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(UseQtExt)
INCLUDE(UsePyQt)

# --- scripts ---

# scripts / static
SET(_bin_SCRIPTS
  ATOMGENGUI.py
)

# --- resources ---

# uic files / to be processed by pyuic
SET(_pyuic_files
  rundlg.ui
)

# scripts / pyuic wrappings
PYQT_WRAP_UIC(_pyuic_SCRIPTS ${_pyuic_files})

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  resources/ATOMGEN_images.ts
  resources/ATOMGEN_msg_en.ts
  resources/ATOMGEN_msg_ja.ts
)

SET(_all_SCRIPTS ${_bin_SCRIPTS} ${_pyuic_SCRIPTS})

# --- rules ---

SALOME_INSTALL_SCRIPTS("${_all_SCRIPTS}" ${SALOME_INSTALL_SCRIPT_PYTHON})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_ATOMGEN_INSTALL_RES_DATA}")
