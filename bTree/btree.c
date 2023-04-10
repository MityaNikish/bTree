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

static void* build_node(const void* btree, const void* key) {
    const BTree* tree = btree;
    if ((tree == NULL) || (key == NULL)) {
        return NULL;
    }
    BTreeNode* node = (BTreeNode*)calloc(sizeof(BTreeNode), 1);
    if (node == NULL) {
        return NULL;
    }
    node->Item = (BTreeItem*)malloc(sizeof(BTreeItem));
    if (node->Item == NULL) {
        return NULL;
    }
    node->Item->value = malloc(tree->valueSize);
    node->Item->key = malloc(tree->keySize);
    if ((node->Item->value != NULL) && (node->Item->key != NULL)) {
        memcpy(node->Item->key, key, tree->keySize);
    }
    return node;
}

static void* traversal_tree(void* _node, const void* key, int(*compare)(const void*, const void*), bool* Flag) {
    BTreeNode* node = (BTreeNode*)_node;
    if ((node == NULL) || (key == NULL) || (Flag == NULL) || (compare == NULL)) {
        return NULL;
    }
    int route;
    while (true)
    {
        route = compare(node->Item->key, key);
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

static void delete_node(void* _node, void(*destroy)(void*)) {
    if (_node == NULL) {
        return;
    }
    BTreeNode* node = _node;
    if (destroy != NULL) {
        destroy(node->Item);
    }
    free(node->Item->key);
    free(node->Item->value);
    free(node->Item);
    free(node);
}

static void* leftmostNode(void* _node) {
    if (_node == NULL) {
        return NULL;
    }
    BTreeNode* node = (BTreeNode*)_node;
    while (node->LeftNode != NULL) {
        node = node->LeftNode;
    }
    return node;
}

static void* rightmostNode(void* _node) {
    if (_node == NULL) {
        return NULL;
    }
    BTreeNode* node = (BTreeNode*)_node;
    while(node->RightNode != NULL) {
        node = node->RightNode;
    }
    return node;
}

static void sheet(BTreeNode* node, int(*compare)(const void*, const void*), void(*destroy)(void*)) {
    if ((node->LeftNode == NULL) && (node->RightNode == NULL)) {
        if (node->ParentNode != NULL) {
            int meaning = compare(node->ParentNode->Item->key, node->Item->key);
            if (meaning <= 0) {
                node->ParentNode->RightNode = NULL;
            }
            else {
                node->ParentNode->LeftNode = NULL;
            };
        }
        delete_node(node, destroy);
    }
}

static void decoupling_sheet(BTreeNode* node, int(*compare)(const void*, const void*)) {
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

static void decoupling_node_path_right(BTreeNode* node, int(*compare)(const void*, const void*), BTree* tree) {
    if (node->ParentNode != NULL) {
        int meaning = compare(node->ParentNode->Item->key, node->Item->key);
        if (meaning <= 0) {
            node->ParentNode->RightNode = node->RightNode;
        }
        else {
            node->ParentNode->LeftNode = node->RightNode;
        };
        node->RightNode->ParentNode = node->ParentNode;
    }
    else {
        tree->root = node->RightNode;
        node->RightNode->ParentNode = NULL;
    }
}

static void decoupling_node_path_left(BTreeNode* node, int(*compare)(const void*, const void*), BTree* tree) {
    if (node->ParentNode != NULL) {
        int meaning = compare(node->ParentNode->Item->key, node->Item->key);
        if (meaning <= 0) {
            node->ParentNode->RightNode = node->LeftNode;
        }
        else {
            node->ParentNode->LeftNode = node->LeftNode;
        };
        node->LeftNode->ParentNode = node->ParentNode;
    }
    else {
        tree->root = node->LeftNode;
        node->LeftNode->ParentNode = NULL;
    }
}

static void node_remove(BTreeNode* node, int(*compare)(const void*, const void*), void(*destroy)(void*), BTree* tree) {
    if ((node->LeftNode == NULL) && (node->RightNode == NULL)) {
        decoupling_sheet(node, compare);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode == NULL) && (node->RightNode != NULL)) {
        decoupling_node_path_right(node, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode != NULL) && (node->RightNode == NULL)) {
        decoupling_node_path_left(node, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->LeftNode != NULL) && (node->RightNode != NULL)) {
        BTreeNode* left_root = leftmostNode(node->RightNode);
        if (destroy != NULL) {
            destroy(node->Item);
        }
        memcpy(node->Item->key, left_root->Item->key, tree->keySize);
        memcpy(node->Item->value, left_root->Item->value, tree->valueSize);
        node_remove(left_root, compare, NULL, tree);
    }
}


static void delete_all_nodes(void* _node, void(*destroy)(void*)) {
    if (_node == NULL) {
        return;
    }
    BTreeNode* node = _node;
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
        btree_destroy(btree, destroy);
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
        BTreeNode* sheet = NULL;
        int route = tree->comp(node->Item->key, key);

        if (route > 0) {
            node->LeftNode = build_node(tree, key);
            sheet = (BTreeNode*)node->LeftNode;
        }

        else if (route < 0) {
            node->RightNode = build_node(tree, key);
            sheet = (BTreeNode*)node->RightNode;
        }

        if (sheet == NULL) {
            return NULL;
        }
        tree->size++;
        sheet->ParentNode = node;
        *createFlag = true;
        return sheet->Item->value;
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
    return (size_t)leftmostNode(tree->root);
}

size_t btree_last(const void* btree) {
    if (btree == NULL) {
        return btree_stop(btree);
    }
    BTree* tree = (BTree*)btree;
    if (tree->root == NULL) {
        return btree_stop(btree);
    }
    return (size_t)rightmostNode(tree->root);
}

size_t btree_next(const void* btree, size_t item_id) {
    if ((btree == NULL) || (item_id == 0)) {
        return btree_stop(btree);
    }

    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;

    if (node->RightNode != NULL) {
        return (size_t)leftmostNode(node->RightNode);
    }
    else if (node->ParentNode != NULL) {
        BTreeNode* parent_node = node->ParentNode;
        while (parent_node)
        {
            if (tree->comp(node->Item->key, parent_node->Item->key) < 0) {
                return (size_t)parent_node;
            }
            parent_node = parent_node->ParentNode;
        }
    }
    return btree_stop(btree);
}

size_t btree_prev(const void* btree, size_t item_id) {
    if ((btree == NULL) || (item_id == 0)) {
        return btree_stop(btree);
    }

    BTree* tree = (BTree*)btree;
    BTreeNode* node = (BTreeNode*)item_id;


    if (node->LeftNode != NULL) {
        return (size_t)rightmostNode(node->LeftNode);
    }
    else if (node->ParentNode != NULL) {
        BTreeNode* parent_node = node->ParentNode;
        while (parent_node)
        {
            if (tree->comp(node->Item->key, parent_node->Item->key) > 0) {
                return (size_t)parent_node;
            }
            parent_node = parent_node->ParentNode;
        }
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