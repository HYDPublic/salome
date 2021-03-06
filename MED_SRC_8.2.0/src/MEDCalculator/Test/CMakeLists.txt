# Copyright (C) 2012-2016  CEA/DEN, EDF R&D
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
# Author : Anthony Geay (CEA/DEN)

ADD_DEFINITIONS(${CPPUNIT_DEFINITIONS})

INCLUDE_DIRECTORIES(
  ${PYTHON_INCLUDE_DIRS}
  ${CPPUNIT_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../Swig
  )

SET(TestMEDCalculator_SOURCES
  TestMEDCalculator.cxx
  MEDCalculatorBasicsTest.cxx
  )

ADD_EXECUTABLE(TestMEDCalculator ${TestMEDCalculator_SOURCES})
TARGET_LINK_LIBRARIES(TestMEDCalculator medcalculatorspython medcalculator ${CPPUNIT_LIBRARIES} ${PLATFORM_LIBRARIES})
ADD_TEST(TestMEDCalculator TestMEDCalculator)
SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)
SET_TESTS_PROPERTIES(TestMEDCalculator PROPERTIES ENVIRONMENT "${tests_env}")

INSTALL(TARGETS TestMEDCalculator DESTINATION ${SALOME_INSTALL_BINS})

# Application tests

SET(TEST_INSTALL_DIRECTORY ${SALOME_MED_INSTALL_TEST}/MEDCalculator)
INSTALL(TARGETS TestMEDCalculator DESTINATION ${TEST_INSTALL_DIRECTORY})

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
