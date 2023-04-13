#include "btree.h"
#include <stdlib.h>
#include <string.h>

typedef struct BTreeNode BTreeNode;
typedef struct BTree BTree;


struct BTreeNode {
    BTreeItem* Item;
    BTreeNode* RightNode;
    BTreeNode* LeftNode;
    BTreeNode* ParentNode;
};

struct BTree {
    size_t keySize;
    size_t valueSize;
    size_t size;
    BTreeNode* root;
    int(*comp)(const void*, const void*);
};

static void* build_node(const BTree* tree, const void* key) {
    if ((tree == NULL) || (key == NULL)) {
        return NULL;
    }
    BTreeNode* node = (BTreeNode*)calloc(sizeof(BTreeNode), 1);
    if (node == NULL) {
        return NULL;
    }
    node->Item = (BTreeItem*)malloc(sizeof(BTreeItem));
    if (node->Item == NULL) {
        free(node);
        return NULL;
    }
    node->Item->value = malloc(tree->valueSize);
    node->Item->key = malloc(tree->keySize);
    if ((node->Item->value == NULL) || (node->Item->key == NULL)) {
        free(node->Item);
        free(node);
        return NULL;
    }
    memcpy((void*)node->Item->key, key, tree->keySize);
    return node;
}

static void* traversal_tree(BTreeNode* node, const void* key, int(*compare)(const void*, const void*), bool* Flag) {
    if ((node == NULL) || (key == NULL) || (Flag == NULL) || (compare == NULL)) {
        return NULL;
    }

    while (true)
    {
        int route = compare(node->Item->key, key);
        if (route == 0) {
            *Flag = true;
            return node;
        }
        else if (route > 0) {
            if (node->LeftNode == NULL) {
                return node;
            }
            node = node->LeftNode;
        }
        else if (route < 0) {
            if (node->RightNode == NULL) {
                return node;
            }
            node = node->RightNode;
        }
    }
}

static void delete_node(BTreeNode* node, void(*destroy)(void*)) {
    if (node == NULL) return;
    if (destroy != NULL) {
        destroy(node->Item);
    }
    free((void*)node->Item->key);
    free(node->Item->value);
    free(node->Item);
    free(node);
}

static void* leftmost_node(BTreeNode*  node) {
    if (node == NULL) return NULL;
    while (node->LeftNode != NULL) {
        node = node->LeftNode;
    }
    return node;
}

static void* rightmost_node(BTreeNode* node) {
    if (node == NULL) return NULL;
    while(node->RightNode != NULL) {
        node = node->RightNode;
    }
    return node;
}

static void decoupling_leaf(BTreeNode* node, int(*compare)(const void*, const void*)) {
    if (node->ParentNode != NULL) {
        int meaning = compare(node->ParentNode->Item->key, node->Item->key);
        if (meaning <= 0) {
            node->ParentNode->RightNode = NULL;
        }
        else {
            node->ParentNode->LeftNode = NULL;
        };
    }
}

static void decoupling_node_path(BTreeNode* node, int(*compare)(const void*, const void*), BTree* tree) {
    if (node->ParentNode->ParentNode != NULL) {
        int meaning = compare(node->ParentNode->ParentNode->Item->key, node->Item->key);
        if (meaning <= 0) {
            node->ParentNode->ParentNode->RightNode = node;
        }
        else {
            node->ParentNode->ParentNode->LeftNode = node;
        };
        node->ParentNode = node->ParentNode->ParentNode;
    }
    else {
        tree->root = node;
        node->ParentNode = NULL;
    }
}

static void node_remove(BTreeNode* node, int(*compare)(const void*, const void*), void(*destroy)(void*), BTree* tree) {
    if ((node->LeftNode == NULL) && (node->RightNode == NULL)) {
        decoupling_leaf(node, compare);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode == NULL) && (node->RightNode != NULL)) {
        decoupling_node_path(node->RightNode, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode != NULL) && (node->RightNode == NULL)) {
        decoupling_node_path(node->LeftNode, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode != NULL) && (node->RightNode != NULL)) {
        BTreeNode* left_root = leftmost_node(node->RightNode);
        if (destroy != NULL) {
            destroy(node->Item);
        }
        memcpy((void*)node->Item->key, left_root->Item->key, tree->keySize);
        memcpy(node->Item->value, left_root->Item->value, tree->valueSize);
        node_remove(left_root, compare, NULL, tree);
    }
}

static void delete_all_nodes(BTreeNode* node, void(*destroy)(void*)) {
    if (node == NULL) return;
    if (node->LeftNode != NULL) {
        BTreeNode* left_node = (BTreeNode*)node->LeftNode;
        left_node->ParentNode = NULL;
        node->LeftNode = NULL;
        delete_all_nodes(left_node, destroy);
    }
    if (node->RightNode != NULL) {
        BTreeNode* right_node = node->RightNode;
        right_node->ParentNode = NULL;
        node->RightNode = NULL;
        delete_all_nodes(right_node, destroy);
    }
    delete_node(node, destroy);
}

static size_t traversal_by_perents(BTreeNode* node, BTree* tree, bool inversion_enabled) {
    BTreeNode* parent_node = node->ParentNode;
    while (parent_node)
    {
        if ((tree->comp(node->Item->key, parent_node->Item->key) < 0) ^ inversion_enabled) {
            return (size_t)parent_node;
        }
        parent_node = parent_node->ParentNode;
    }
    return (size_t)NULL;
}


void* btree_create(size_t keySize, size_t valueSize, int(*compare)(const void*, const void*)){
    if ((keySize == 0) || (valueSize == 0) || (compare == NULL)) {
        return NULL;
    }

    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (tree == NULL) {
        return NULL;
    }

    tree->size = 0;
    tree->keySize = keySize;
    tree->valueSize = valueSize;
    tree->comp = compare;
    tree->root = NULL;

    return tree;
}

void btree_destroy(void* btree, void(*destroy)(void*)) {
    if (btree == NULL) {
        return;
    }
    btree_clear(btree, destroy);
    free(btree);
}

void* btree_init(void* btree, size_t keySize, size_t valueSize, int(*compare)(const void*, const void*), void(*destroy)(void*)) {
    if ((keySize == 0) || (valueSize == 0) || (compare == NULL) || (btree == NULL)) {
        return NULL;
    }
    BTree* tree = (BTree*)btree;
    btree_clear(tree, destroy);
    tree->keySize = keySize;
    tree->valueSize = valueSize;
    tree->comp = compare;
    return tree;
}

void btree_clear(void* btree, void(*destroy)(void*)) {
    if (btree == NULL) {
        return;
    }
    BTree* tree = (BTree*)btree;
    if (tree->root != NULL) {
        delete_all_nodes(tree->root, destroy);
    }
    tree->root = NULL;
    tree->size = 0;
}


size_t btree_count(const void* btree){
    if (btree == NULL) {
        return INVALID;
    }
    BTree* tree = (BTree*)btree;
    return tree->size;
}

void* btree_item(const void* btree, const void* key) {
    BTree* tree = (BTree*)btree;
    if ((tree == NULL) || (key == NULL)) {
        return NULL;
    }
    bool node_found = false;
    BTreeNode* node = NULL;
    node = (BTreeNode*)traversal_tree(tree->root, key, tree->comp, &node_found);
    if (!node_found) {
        return NULL;
    }
    return node->Item->value;
}

void* btree_insert(void* btree, const void* key, bool* createFlag){
    BTree* tree = (BTree*)btree;
    if ((tree == NULL) || (key == NULL) || (createFlag == NULL)) {
        return NULL;
    }

    if (tree->root == NULL) {
        tree->root = build_node(tree, key);
        if (tree->root == NULL) {
            return NULL;
        }
        *createFlag = true;
        tree->size++;
        return tree->root->Item->value;
    }

    bool node_found = false;
    BTreeNode* node = NULL;
    node = (BTreeNode*)traversal_tree(tree->root, key, tree->comp, &node_found);

    if (node == NULL) {
        return NULL;
    }

    if (node_found) {
        *createFlag = false;
        return node->Item->value;
    }
    else {
        BTreeNode* leaf = NULL;
        int route = tree->comp(node->Item->key, key);

        if (route > 0) {
            node->LeftNode = build_node(tree, key);
            leaf = (BTreeNode*)node->LeftNode;
        }

        else if (route < 0) {
            node->RightNode = build_node(tree, key);
            leaf = (BTreeNode*)node->RightNode;
        }

        if (leaf == NULL) {
            return NULL;
        }
        tree->size++;
        leaf->ParentNode = node;
        *createFlag = true;
        return leaf->Item->value;
    }
}

void btree_remove(void* btree, const void* key, void(*destroy)(void*)) {
    if ((btree == NULL) || (key == NULL)) {
        return;
    }
    BTree* tree = (BTree*)btree;
    if (tree->root == NULL) {
        return;
    }
    bool Flag = false;
    BTreeNode* node = NULL;
    node = traversal_tree(tree->root, key, tree->comp, &Flag);
    if (Flag) {
        if (node != NULL) {
            node_remove(node, tree->comp, destroy, tree);
            tree->size -= 1;
            if (tree->size == 0) {
                tree->root = NULL;
            }
        }
    }
}


size_t btree_first(const void* btree) {
    if (btree == NULL) {
        return btree_stop(btree);
    }
    BTree* tree = (BTree*)btree;
    if (tree->root == NULL) {
        return btree_stop(btree);
    }
    return (size_t)leftmost_node(tree->root);
}

size_t btree_last(const void* btree) {
    if (btree == NULL) {
        return btree_stop(btree);
    }
    BTree* tree = (BTree*)btree;
    if (tree->root == NULL) {
        return btree_stop(btree);
    }
    return (size_t)rightmost_node(tree->root);
}

size_t btree_next(const void* btree, size_t item_id) {
    if ((btree == NULL) || (item_id == 0)) {
        return btree_stop(btree);
    }
    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;
    if (node->RightNode != NULL) {
        return (size_t)leftmost_node(node->RightNode);
    }
    else if (node->ParentNode != NULL) {
        size_t temp = traversal_by_perents(node, tree, false);
        if (temp != (size_t)NULL) return temp;
    }
    return btree_stop(tree);
}

size_t btree_prev(const void* btree, size_t item_id) {
    if ((btree == NULL) || (item_id == 0)) {
        return btree_stop(btree);
    }

    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;


    if (node->LeftNode != NULL) {
        return (size_t)rightmost_node(node->LeftNode);
    }
    else if (node->ParentNode != NULL) {
        size_t temp = traversal_by_perents(node, tree, true);
        if (temp != (size_t)NULL) return temp;
    }
    return btree_stop(btree);
}

size_t btree_stop(const void* btree) {
    return (size_t)NULL;
}

void* btree_current(const void* btree, size_t item_id)
{
    if ((item_id == 0) || (btree == NULL)) {
        return NULL;
    }
    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;
    if (btree_item(btree, node->Item->key) != NULL) {
        return node->Item;
    }
    return NULL;
}

void btree_erase(void* btree, size_t item_id, void(*destroy)(void*)) {
    if ((btree == NULL) || (item_id == 0)) {
        return;
    }

    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;
    btree_remove(btree, node->Item->key, destroy);
}