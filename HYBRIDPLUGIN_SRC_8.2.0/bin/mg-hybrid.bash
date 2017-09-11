#!/bin/bash

#bash script mg-hybrid.bash
#we have renamed binary executable mg-hybrid.exe V1.3.0 as mg-hybrid.exe_Linux_64_juillet2014
#to assume call of other beta-versions of distene mg-hybrid.exe code in standalone plugin HYBRIDPLUGIN sources
#and also assume licence file set to overriding licence file of other distene products ( HEXOTIC, GHS3D, etc... )
#all that for for only one call of mg-hybrid.bash from salome plugin HYBRIDPLUGIN

function findOutFile {
#find --out parameter result in $resFindOutFile
while test $# -gt 0; do
  case "$1" in
    -o|--out)
      if [ -z "$2" ]    # $String is null
      then
        resFindOutFile="None"
      else
        resFindOutFile=$2
      fi
      return
      ;;
    *)
      shift
      ;;
  esac
done
resFindOutFile="None"
}

#echo "mg-hybrid.bash initial parameters are:" $1 $2 $3 $4 
echo "mg-hybrid.bash initial parameters are:" $*
#$0 is ignored

echo "mg-hybrid.bash assume licence file set:"
env | grep DLIM

findOutFile $*
echo "result output File is:" $resFindOutFile

$MESHGEMS_ROOT_DIR/bin/run_mg-hybrid.sh $*

#$MESHGEMS_ROOT_DIR/bin/Linux_64/mg-hybrid.exe $*

#to EZ and direct debug visualization (with gui salome load script createGroupsOnEntitiesFromGMFFile.py)
#cp -f $resFindOutFile /tmp/tmp.mesh

