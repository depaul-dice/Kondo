#include "../Utilities/IntervalTree.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    Node* root = NULL;
    root = insertNoCombine(root, (Interval) {0,10,0,0});
    root = insertNoCombine(root, (Interval) {30,70,10,0});
    root = insertNoCombine(root, (Interval) {90,100,50,0});
    root = insertNoCombine(root, (Interval) {100,105,60,0});
    root = insertNoCombine(root, (Interval) {10,30,65,0});
    root = chopTree(root, (Interval){5,35,-1,-1});
    print_intervals(root, stdout, 1);
}