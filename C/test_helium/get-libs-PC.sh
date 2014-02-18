NETMAN=$(readlink -f "$0")
NETMAN_DIR=$(dirname "$NETMAN")
mkdir -p bin
mkdir -p lib/include

echo '\n Building shakespeare...\n'
cd ../space-lib/shakespeare
echo "cd: \c"
pwd
cp inc/shakespeare.h $NETMAN_DIR/lib/include
sh x86-compile-lib-static.sh
cp lib/libshakespeare.a $NETMAN_DIR/lib

# Builds libhe100.a 
echo '\n Building HE-100 Library...\n'

cd $NETMAN_DIR
cd ../HE100-lib/C
echo "cd: \c" 
pwd
sh x86-compile-lib-static-cpp.sh
cp lib/libhe100-cpp.a $NETMAN_DIR/lib
cp src/SC_he100.h $NETMAN_DIR/lib/include
cd $NETMAN_DIR/lib 
mv libhe100-cpp.a libhe100.a

# Timer library
echo '\n Building timer Library...\n'
cd $NETMAN_DIR
cd ../space-timer-lib 
echo "cd: \c" 
pwd
sh x86-compile-lib-static-cpp.sh
cp lib/libtimer.a $NETMAN_DIR/lib
cp inc/timer.h $NETMAN_DIR/lib/include

# namedpipe & commander

echo '\n Building Namedpipes and commander...\n'
cd ../space-commander
echo "cd: \c" 
pwd
cp include/Net2Com.h lib/include
cp include/NamedPipe.h lib/include
make buildBin
cp bin/space-commander $NETMAN_DIR/bin
make staticlibs.tar
cp staticlibs.tar $NETMAN_DIR/lib
cd $NETMAN_DIR/lib
tar -xf staticlibs.tar
rm staticlibs.tar


cd $NETMAN_DIR/lib
ls -al
