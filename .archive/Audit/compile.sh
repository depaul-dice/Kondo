clang -I ../Utilities/ -shared -g -ldl -fPIC wrappers.c logLibrary.c ../Utilities/IntervalTree.c ../Utilities/logUtil/logUtilities.c -o auditLib.so
