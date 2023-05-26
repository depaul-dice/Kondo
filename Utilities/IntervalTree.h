#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H
#include <stdio.h>

typedef struct Interval
{
    int low;
    int high;
    int off;
    int fileFlag;
} Interval;

typedef struct Node
{
    Interval interval;
    int max;
    struct Node *left;
    struct Node *right;
    int height;
} Node;

typedef struct NodeList
{
    Interval *pInterval;
    struct NodeList *pNext;
    struct NodeList *pPrev;
} NodeList;

/// @brief Given the root node to a tree add the given interval
/// @param root root node of a avl interval tree to add intrerval to
/// @param interval interval to add to the tree
/// @return new root node pointer
Node *insertInterval(Node *root, Interval interval);

/// @brief Given an itnerval remove the intersecting parts of the tree with the interval
/// @param root root node of the tree to remove the interval from
/// @param interval interval to remove from the tree
/// @return new root node pointer
Node *chopTree(Node *root, Interval interval);

/// @brief Given an itnerval remove the intersecting parts of the tree with the interval and return a LL of the intersecting regions
/// @param root root node of the tree to remove the interval from
/// @param interval interval to remove from the tree
/// @param pIntersection Double pointer to head of LL of intersecting regions
/// @return new root node pointer
Node *chopAndReturn(Node *root, Interval interval, NodeList **pIntersection);

/// @brief Print the given itnerval tree
/// @param node root node of the tree to print
/// @param stream stream to print to
/// @param complete 1 if you want offset and file flag to be printed too else 0
void print_intervals(Node *node, FILE *stream, int complete);

/// @brief Given an interval and a tree return the first node of the tree which
/// intersects with the given interval
/// @param node root node of tree to search in
/// @param interval interval to search for
/// @return pointer to interse3cting node in the tree
Node *search(Node *node, Interval interval);

/// @brief Given a tree and an interval return a LL of itnersecting regiins
/// as well as a LL of leftover aprts of the interval
/// @param tree tree to find the itnersections with
/// @param pInterval interval to find the intersections for
/// @param pIntersection double pointer to LL of intersections
/// @param pLeftOver double pointer to LL of leftover parts of the Intersection
void getIntersectionsAndChopInterval(Node *tree, Interval *pInterval, NodeList **pIntersection, NodeList **pLeftOver);

Node *insertNoCombine(Node *node, Interval interval);
#endif