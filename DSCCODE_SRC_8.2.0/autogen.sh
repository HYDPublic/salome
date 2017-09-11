#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 adm_local/ltmain.sh

########################################################################
# Test if the KERNEL_ROOT_DIR is set correctly

if test ! -d "${KERNEL_ROOT_DIR}"; then
    echo "failed : KERNEL_ROOT_DIR variable is not correct !"
    exit 1
fi

echo "Running aclocal..."    ; 
aclocal --force -I ${KERNEL_ROOT_DIR}/salome_adm/unix/config_files || exit 1
echo "Running autoheader..." ; autoheader --force                  || exit 1
echo "Running autoconf..."   ; autoconf --force                    || exit 1
echo "Running libtoolize..." ; libtoolize --copy --force           || exit 1
echo "Running automake..."   ; automake --add-missing --copy --gnu || exit 1

