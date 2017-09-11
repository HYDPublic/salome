#!/bin/bash

#bash script mg-tetra_hpc.bash
#could assume overriding set licence file of user/other new version distene products
#this script is used for only one call of mg-tetra_hpc.bash from salome plugin GHS3DPRLPLUGIN

#$0 have to be ignored: name of script
echo "mg-tetra_hpc.bash initial parameters are:\n  "$@"\n"

#example mg-tetra_hpc_mpi v2.1.11
#mpirun -n 6 mg-tetra_hpc_mpi.exe --gradation 1.05 --max_size 0 --min_size 0 --verbose 3 --in /tmp/GHS3DPRL.mesh --out /tmp/GHS3DPRL_out.mesh


#this is for example, sometimes DISTENE_LICENSE_FILE have to be overriden
#if [ $HOST == "hpcspot" ]
# then
#  export DISTENE_LICENSE_FILE="Use global envvar: DLIM8VAR"
#  export DLIM8VAR="dlim8 1:1:29030@10.27.51.1/002590c96d98::8fbdc02cde090ca0369ad028e839065b97709e3c33e640eb6a3c2c7e40fe3985"
# else
#   export DISTENE_LICENSE_FILE="Use global envvar: DLIM8VAR"
#   export DLIM8VAR="dlim8 1:1:29030@132.166.151.49/84c419b8::87af196ab2a936ab31363624539bff8096fbe1f3c83028c8f6b399b0a904ef85"
#fi


#this is for example, sometimes need to compile stubs/libmeshgems_mpi.so for mg-tetra_hpc.exe, depends of openmpi system or not#CURRENTDIR=`pwd`
CURRENTDIR=`pwd`
COMPILDIR=$MESHGEMSHOME/stubs           #/export/home/prerequisites_SALOME_780/MeshGems-2.1-11/FROM_nothing/stubs
MPI_EXE=`which mg-tetra_hpc_mpi.exe`
TARGETDIR=$MESHGEMSHOME/lib/Linux_64    #/export/home/prerequisites_SALOME_780/MeshGems-2.1-11/FROM_nothing/lib/Linux_64
echo "CURRENTDIR=" $CURRENTDIR
#rm $TARGETDIR/libmeshgems_mpi.so #compile every times
#if [ ! -f $TARGETDIR/libmeshgems_mpi.so ]
#then
  echo "Compile libmeshgems_mpi.so with current openmpi:"
  echo `which mpicc`
  echo "COMPILDIR=" $COMPILDIR
  echo "TARGETDIR=" $TARGETDIR
  cd $COMPILDIR
  mpicc  meshgems_mpi.c -DMESHGEMS_LINUX_BUILD -I../include -shared -fPIC -o $TARGETDIR/libmeshgems_mpi.so
  find $MESHGEMSHOME -name "libmeshgems_mpi.so"
  cd $CURRENTDIR
#fi

#example version mpi a mano
#cd /export/home/prerequisites_SALOME_780/MeshGems-2.1-11/FROM_nothing/stubs
#mpicc meshgems_mpi.c -DMESHGEMS_LINUX_BUILD -I../include -shared -fPIC -o libmeshgems_mpi.so
#cp -f libmeshgems_mpi.so /export/home/prerequisites_SALOME_780/MeshGems-2.1-11/FROM_nothing/lib/Linux_64/.

echo "mg-tetra_hpc.bash assume licence file set:"
echo DLIM8VAR=$DLIM8VAR
echo DISTENE_LICENSE_FILE=$DISTENE_LICENSE_FILE

#version multithread: why not use plugin/hypothesis mg-tetra.exe instead?
#which mg-tetra_hpc.exe
#echo mg-tetra_hpc.exe --max_number_of_threads $2 --number_of_subdomains $2 --in ... --out /tmp/GHS3DPRL_out.mesh --verbose 3
#for multithread only one /tmp/GHS3DPRL_out.mesh created...
#TODO create one med file from /tmp/GHS3DPRL_out.mesh if utile later


which mg-tetra_hpc_mpi.exe
#echo mpirun -n $2 mg-tetra_hpc_mpi.exe ${@:3:30}
echo mpirun -n $2 mg-tetra_hpc_mpi.exe --in /tmp/GHS3DPRL.mesh --out /tmp/GHS3DPRL_out.mesh
mpirun -n $2 mg-tetra_hpc_mpi.exe --in /tmp/GHS3DPRL.mesh --out /tmp/GHS3DPRL_out.mesh

#typically write files /tmp/GHS3DPRL_out.00000?.mesh  & /tmp/GHS3DPRL_out.00000?.global


