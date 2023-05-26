#include <stdio.h>
#include <stdlib.h>

#include "IntervalTree.h"

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int height(Node *node)
{
    if (node == NULL)
        return 0;
    return node->height;
}

int get_max(Node *node)
{
    if (node == NULL)
        return 0;
    return node->max;
}

int get_balance(Node *node)
{
    if (node == NULL)
        return 0;
    return height(node->left) - height(node->right);
}

Node *new_node(Interval interval)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->interval = interval;
    node->max = interval.high;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

Node *right_rotate(Node *y)
{
    Node *x = y->left;
    Node *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    y->max = max(max(get_max(y->left), get_max(y->right)), y->interval.high);
    x->max = max(max(get_max(x->left), get_max(x->right)), x->interval.high);

    return x;
}

Node *left_rotate(Node *x)
{
    Node *y = x->right;
    Node *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    x->max = max(max(get_max(x->left), get_max(x->right)), x->interval.high);
    y->max = max(max(get_max(y->left), get_max(y->right)), y->interval.high);

    return y;
}

Node *insertNoCombine(Node *node, Interval interval)
{
    // printf("Insert called inserting [%d,%d]\n", interval.low, interval.high);
    if (node == NULL)
    {
        // printf("IS NULL\n");
        return new_node(interval);
    }
    if (interval.low < node->interval.low)
    {
        // printf("If 1\n");
        node->left = insertNoCombine(node->left, interval);
    }
    else
    {
        // printf("Else 1");
        node->right = insertNoCombine(node->right, interval);
    }

    node->height = max(height(node->left), height(node->right)) + 1;

    node->max = max(max(get_max(node->left), get_max(node->right)), node->interval.high);

    int balance = get_balance(node);

    if (balance > 1 && interval.low < node->left->interval.low)
        return right_rotate(node);

    if (balance < -1 && interval.low > node->right->interval.low)
        return left_rotate(node);

    if (balance > 1 && interval.low > node->left->interval.low)
    {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && interval.low < node->right->interval.low)
    {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}
Node *insert(Node *node, Interval interval)
{
    // printf("Insert called inserting [%d,%d]\n", interval.low, interval.high);
    if (node == NULL)
    {
        // printf("IS NULL\n");
        return new_node(interval);
    }
    if (interval.low <= node->interval.high && interval.high >= node->interval.low)
    {
        // printf("THEY OVERLAP\n [%d,%d] [%d,%d]\n",interval.low, interval.high, node->interval.low, node->interval.high);
        if (interval.low < node->interval.low)
        {
            node->interval.low = interval.low;
        }
        if (interval.high > node->interval.high)
        {
            node->interval.high = interval.high;
        }
        // printf("COMBINED TO [%d,%d]\n",node->interval.low, node->interval.high);
    }
    else if (interval.low < node->interval.low)
    {
        // printf("If 1\n");
        node->left = insert(node->left, interval);
    }
    else
    {
        // printf("Else 1");
        node->right = insert(node->right, interval);
    }

    node->height = max(height(node->left), height(node->right)) + 1;

    node->max = max(max(get_max(node->left), get_max(node->right)), node->interval.high);

    int balance = get_balance(node);

    if (balance > 1 && interval.low < node->left->interval.low)
        return right_rotate(node);

    if (balance < -1 && interval.low > node->right->interval.low)
        return left_rotate(node);

    if (balance > 1 && interval.low > node->left->interval.low)
    {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && interval.low < node->right->interval.low)
    {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

Node *min_node(Node *node)
{
    Node *current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

Node *delete(Node *node, Interval interval)
{
    if (node == NULL)
    {
        return node;
    }
    if (interval.low < node->interval.low)
        node->left = delete (node->left, interval);
    else if (interval.low > node->interval.low)
        node->right = delete (node->right, interval);
    else
    {
        if (node->left == NULL || node->right == NULL)
        {
            Node *temp = node->left ? node->left : node->right;

            if (temp == NULL)
            {
                temp = node;
                node = NULL;
            }
            else
            {
                *node = *temp;
            }
            free(temp);
        }
        else
        {
            Node *temp = min_node(node->right);
            node->interval = temp->interval;
            node->right = delete (node->right, temp->interval);
        }
    }

    if (node == NULL)
        return node;

    node->height = max(height(node->left), height(node->right)) + 1;

    node->max = max(max(get_max(node->left), get_max(node->right)), node->interval.high);

    int balance = get_balance(node);

    if (balance > 1 && get_balance(node->left) >= 0)
        return right_rotate(node);

    if (balance > 1 && get_balance(node->left) < 0)
    {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && get_balance(node->right) <= 0)
        return left_rotate(node);

    if (balance < -1 && get_balance(node->right) > 0)
    {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

Node *search(Node *node, Interval interval)
{
    // Base Condition
    if(node == NULL)
        return NULL;
    Node* res = NULL;
    // check left hand side if viable
    if (node->left != NULL && node->left->max >= interval.low)
    {
        res = search(node->left, interval);
        if(res!=NULL)
            return res;
    }
    // not on left so check cur
    if ( (node->interval.low < interval.high && interval.low < node->interval.high))
        return node;

    // if not on left or cur then check right and return that irrespectively
    return search(node->right, interval);
}
void combineIntervals(Interval *Interval, int low, int high)
{
    Interval->low = Interval->low < low ? Interval->low : low;
    Interval->high = Interval->high > high ? Interval->high : high;
}

Node *insertInterval(Node *root, Interval interval)
{
    Node *search_result = search(root, interval);
    while (search_result != NULL)
    {
        // combine them
        combineIntervals(&interval, search_result->interval.low, search_result->interval.high);
        // delete the searched element
        root = delete (root, search_result->interval);
        // search again
        search_result = search(root, interval);
    }
    // add new node in
    root = insert(root, interval);
    return root;
}
Node *chopTree(Node *root, Interval interval)
{
    Node *search_result = search(root, interval);
    while (search_result != NULL)
    {
        int oldLow = search_result->interval.low;
        int oldHigh = search_result->interval.high;
        int off = search_result->interval.off;
        int flg = search_result->interval.fileFlag;
        root = delete (root, search_result->interval);
        // 4 cases
        // s2 s1 e1 e2 full delete
        if (interval.low <= oldLow && interval.high >= oldHigh)
        {
        }
        // s2 s1 e2 e1 change start
        else if (interval.low < oldLow && interval.high < oldHigh)
        {
            root = insert(root, (Interval){interval.high, oldHigh, interval.high-oldLow+off, flg});
        }
        // s1 s2 e1 e2 change end
        else if (oldLow < interval.low && oldHigh < interval.high)
        {
            root = insert(root, (Interval){oldLow, interval.low, off, flg});
        }
        // s1 s2 e2 e1 split to 2
        else
        {
            root = insert(root, (Interval){oldLow, interval.low, off, flg});
            root = insert(root, (Interval){interval.high, oldHigh, interval.high-oldLow+off, flg});
        }

        search_result = search(root, interval);
    }
    return root;
}

void print_intervals(Node *node, FILE *stream, int complete)
{
    if (node == NULL)
        return;
    print_intervals(node->left, stream, complete);
    if(complete != 1)
    fprintf(stream, "[%d, %d]\n", node->interval.low, node->interval.high);
    else
    fprintf(stream, "[%d, %d]: %d %d\n", node->interval.low, node->interval.high, node->interval.off, node->interval.fileFlag);

    print_intervals(node->right, stream, complete);
}
void addNode(int low, int high, int off, int fileFlag, NodeList **pHead)
{
    NodeList *pNewNode = malloc(sizeof(NodeList));
    pNewNode->pInterval = malloc(sizeof(Interval));
    pNewNode->pInterval->low = low;
    pNewNode->pInterval->high = high;
    pNewNode->pInterval->off = off;
    pNewNode->pInterval->fileFlag = fileFlag;
    if (*pHead == NULL)
    {
        *pHead = pNewNode;
        pNewNode->pPrev = NULL;
        pNewNode->pNext = NULL;
    }
    else
    {
        pNewNode->pNext = *pHead;
        pNewNode->pPrev = NULL;
        (*pHead)->pPrev = pNewNode;
        *pHead = pNewNode;
    }
}

Node *chopAndReturn(Node *root, Interval interval, NodeList **pPtr)
{
    Node *search_result = search(root, interval);
    while (search_result != NULL)
    {
        int oldLow = search_result->interval.low;
        int oldHigh = search_result->interval.high;
        int off = search_result->interval.off;
        int flg = search_result->interval.fileFlag;
        root = delete (root, search_result->interval);
        // 4 cases
        // s2 s1 e1 e2 full delete
        if (interval.low <= oldLow && interval.high >= oldHigh)
        {
            // add s1 e1 to nodeList
            addNode(oldLow, oldHigh, off, flg, pPtr);
        }
        // s2 s1 e2 e1 change start
        else if (interval.low < oldLow && interval.high < oldHigh)
        {
            root = insert(root, (Interval){interval.high, oldHigh, interval.high-oldLow+off, flg});
            // add s2 s1 to nodeList

            addNode(oldLow, interval.high, off, flg, pPtr);
        }
        // s1 s2 e1 e2 change end
        else if (oldLow < interval.low && oldHigh < interval.high)
        {
            root = insert(root, (Interval){oldLow, interval.low, off, flg});
            // add s2 e1 to nodeList

            addNode(interval.low, oldHigh,  interval.low-oldLow+off, flg, pPtr);
        }
        // s1 s2 e2 e1 split to 2
        else
        {
            root = insert(root, (Interval){oldLow, interval.low, off, flg});
            root = insert(root, (Interval){interval.high, oldHigh,  interval.high-oldLow+off, flg});
            // add s2 e2 to nodeList

            addNode(interval.low, interval.high,  interval.low-oldLow+off, flg, pPtr);
        }

        search_result = search(root, interval);
    }
    return root;
}

NodeList *removeHead(NodeList *toRemove)
{
    if (toRemove == NULL)
    {
        return NULL;
    }
    NodeList *toRet;
    if (toRemove->pNext != NULL)
    {
        toRet = toRemove->pNext;
        toRemove->pNext->pPrev = NULL;
        toRemove->pNext = NULL;
        toRemove->pPrev = NULL;
        return toRet;
    }
    return NULL;
}

void getIntersectionsAndChopInterval(Node *tree, Interval *pInterval, NodeList **pIntersection, NodeList **pLeftOver)
{
    if (search(tree, *pInterval) == NULL)
    {
        *pIntersection = NULL;
        *pLeftOver = malloc(sizeof(NodeList));
        (*pLeftOver)->pInterval = pInterval;
        (*pLeftOver)->pNext = NULL;
        (*pLeftOver)->pPrev = NULL;
        return;
    }

    NodeList *toProcess = malloc(sizeof(NodeList));
    toProcess->pNext = NULL;
    toProcess->pPrev = NULL;
    toProcess->pInterval = malloc(sizeof(Interval));
    toProcess->pInterval->low = pInterval->low;
    toProcess->pInterval->high = pInterval->high;

    NodeList *pCur = toProcess;
    toProcess = removeHead(toProcess);

    Node *search_result = search(tree, *pCur->pInterval);

    while (search_result != NULL)
    {
        int treeLow = search_result->interval.low;
        int treeHigh = search_result->interval.high;
        int off = search_result->interval.off;
        int flg = search_result->interval.fileFlag;
        Interval *interval = pCur->pInterval;
        // 4 cases
        // s2 s1 e1 e2
        if (interval->low <= treeLow && interval->high >= treeHigh)
        {
            addNode(treeLow, treeHigh, off, flg, pIntersection);
            if(interval->low!=treeLow)
            addNode(interval->low, treeLow,  interval->low-treeLow+off, flg, &toProcess);
            if(interval->high != treeHigh)
            addNode(treeHigh, interval->high, treeHigh-treeLow+off, flg,&toProcess);
        }
        // s2 s1 e2 e1 change start
        else if (interval->low < treeLow && interval->high < treeHigh)
        {
            addNode(treeLow, interval->high, off, flg,pIntersection);
            addNode(interval->low, treeLow, interval->low-treeLow+off, flg,&toProcess);
        }
        // s1 s2 e1 e2 change end
        else if (treeLow < interval->low && treeHigh < interval->high)
        {
            addNode(interval->low, treeHigh, interval->low-treeLow+off, flg,pIntersection);
            addNode(treeHigh, interval->high, treeHigh-treeLow+off, flg,&toProcess);
        }
        // s1 s2 e2 e1 split to 2
        else
        {
            addNode(interval->low, interval->high, interval->low-treeLow+off, flg,pIntersection);
        }
        if (toProcess == NULL)
        {
            return;
        }
        while (toProcess != NULL)
        {
            pCur = toProcess;
            toProcess = removeHead(toProcess);

            search_result = search(tree, *pCur->pInterval);

            if (search_result != NULL)
                break;

            addNode(pCur->pInterval->low, pCur->pInterval->high, pCur->pInterval->off, flg, pLeftOver);
        }
    }
}
