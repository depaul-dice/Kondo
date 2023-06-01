clang -I ../Utilities/ -shared -g -ldl -fPIC -lcrypto wrappers.c logLibrary.c ../Utilities/IntervalTree.c ../Utilities/logUtil/logUtilities.c -o auditLib.so
