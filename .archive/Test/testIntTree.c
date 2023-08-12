#include "../Utilities/IntervalTree.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    Node* root = NULL;
    root = insertInterval(root, (Interval) {0,10});
    root = insertInterval(root, (Interval) {30,70});
    root = insertInterval(root, (Interval) {90,100});
    NodeList* pHead;
    print_intervals(root, stdout);
    NodeList* pIntersection;
    NodeList* pLeftOver;
     getIntersectionsAndChopInterval(root, &(Interval){5,95}, &pIntersection, &pLeftOver);
    NodeList* pCur = pIntersection;
    fprintf(stdout, "***********\n");
    while(pCur != NULL)
    {
        fprintf(stdout, "%d %d\n", pCur->pInterval->low, pCur->pInterval->high);
        pCur = pCur->pNext;
    }
    fprintf(stdout, "***********\n");
    pCur = pLeftOver;
    fprintf(stdout, "***********\n");
    while(pCur != NULL)
    {
        fprintf(stdout, "%d %d\n", pCur->pInterval->low, pCur->pInterval->high);
        pCur = pCur->pNext;
    }
    fprintf(stdout, "***********\n");
    print_intervals(root, stdout);
}