clang -I ../Utilities/ -shared -g -ldl -fPIC -lcrypto wrappers.c repeatLibrary.c ../Utilities/IntervalTree.c ../Utilities/repeatUtil/repeatUtilities.c -o repeatLib.so
