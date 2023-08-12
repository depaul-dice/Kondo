# Set aflgo-instrumenter
PROGRAM=$1
export SUBJECT="$PWD"
export TMP_DIR="$PWD/temp"
export AFLGO="/root/build/llvm_tools/build-llvm/msan/aflgo"
export CC=$AFLGO/afl-clang-fast
export CXX=$AFLGO/afl-clang-fast++
export COPY_CFLAGS=$CFLAGS
export COPY_CXXFLAGS=$CXXFLAGS
export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"
export CFLAGS="$CFLAGS $ADDITIONAL"
export CXXFLAGS="$CXXFLAGS $ADDITIONAL"

echo $SUBJECT
echo $TMP_DIR
echo $AFLGO

rm -rf $TMP_DIR
mkdir -p $TMP_DIR
mkdir -p "$PWD/output"


sudo $CC offset_check.c "${PROGRAM}.c" $CFLAGS -o "${PROGRAM}_ll"

echo "Function targets"
cat $TMP_DIR/Ftargets.txt

# Clean up
cat $TMP_DIR/BBnames.txt | grep -v "^$"| rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
cat $TMP_DIR/BBcalls.txt | grep -Ev "^[^,]*$|^([^,]*,){2,}[^,]*$"| sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt

# Generate distance ☕️
# $AFLGO/scripts/genDistance.sh is the original, but significantly slower, version
sudo $AFLGO/scripts/gen_distance_fast.py $SUBJECT $TMP_DIR "${PROGRAM}_ll"

# Check distance file
echo "Distance values:"
head -n5 $TMP_DIR/distance.cfg.txt
echo "..."
tail -n5 $TMP_DIR/distance.cfg.txt


echo "Compiling again"
export CFLAGS="$COPY_CFLAGS -distance=$TMP_DIR/distance.cfg.txt"
export CXXFLAGS="$COPY_CXXFLAGS -distance=$TMP_DIR/distance.cfg.txt"
sudo $CC offset_check.c "${PROGRAM}.c" $CFLAGS  -o "${PROGRAM}_ll"

rm -rf output/*
rm output.txt
touch output.txt
sudo $AFLGO/afl-fuzz -i testcases -o output/ "./${PROGRAM}_ll"