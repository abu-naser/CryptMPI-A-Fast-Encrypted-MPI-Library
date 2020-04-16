#!/bin/bash
# Thomas Naughton <naughtont@ornl.gov>
# Quick script to grab GNU Autotools and build them.

#
# Base directory for everything.
# The source code will live in:
#     $basedir/<autotool-version>
#
# The installation will live in:
#     $basedir/{bin,include,lib,share,src}
#
basedir=$HOME/automake

m4_version=1.4.17
autoconf_version=2.69
automake_version=1.15.1
libtool_version=2.4.6

WGET=wget
#WGET='curl -O '
TAR=tar

#======

$WGET http://ftp.gnu.org/gnu/m4/m4-$m4_version.tar.gz
$WGET http://ftp.gnu.org/gnu/autoconf/autoconf-$autoconf_version.tar.gz
$WGET http://ftp.gnu.org/gnu/automake/automake-$automake_version.tar.gz
$WGET http://ftp.gnu.org/gnu/libtool/libtool-$libtool_version.tar.gz

mkdir -p $basedir/src/
$TAR -zxf m4-$m4_version.tar.gz               -C $basedir/src/
$TAR -zxf autoconf-$autoconf_version.tar.gz   -C $basedir/src/
$TAR -zxf automake-$automake_version.tar.gz   -C $basedir/src/
$TAR -zxf libtool-$libtool_version.tar.gz     -C $basedir/src/


# Pre-pend install path for new Autotools
export PATH=$basedir/bin:$PATH
export LD_LIBRARY_PATH=$basedir/lib:$LD_LIBRARY_PATH

# 1) M4
(cd $basedir/src/m4-$m4_version/ ; ./configure --prefix=$basedir && make && make install) || exit 1

# 2) Autoconf
(cd $basedir/src/autoconf-$autoconf_version/ ; ./configure --prefix=$basedir && make && make install) || exit 1

# 3) Automake
(cd $basedir/src/automake-$automake_version/ ; ./configure --prefix=$basedir && make && make install) || exit 1

# 4) Libtool
(cd $basedir/src/libtool-$libtool_version/ ; ./configure --prefix=$basedir && make && make install) || exit 1



echo ""
echo "#####################################"
for tool in m4 autoconf automake libtool ; do
    $tool --version | head -1
    which $tool
    echo ""
done

echo ""
echo "TODO: Manually add following to PATH EnvVar:"
echo "  $basedir/bin"
echo ""


echo "TODO: Manuallly add following to LD_LIBRARY_PATH EnvVar:"
echo "  $basedir/lib"

echo ""
echo " === FOR EXAMPLE WITH BASH ==="
echo ""
echo " export PATH=$basedir/bin:\$PATH"
echo " export LD_LIBRARY_PATH=$basedir/lib:\$LD_LIBRARY_PATH"
echo ""
echo " ============================="

exit 0
