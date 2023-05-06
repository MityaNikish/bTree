#include "btree.h"
#include <stdlib.h>
#include <string.h>

typedef struct BTreeNode BTreeNode;
typedef struct BTree BTree;


struct BTreeNode {
    BTreeItem* item;
    BTreeNode* right_node;
    BTreeNode* left_node;
    BTreeNode* parent_node;
};

struct BTree {
    size_t key_size;
    size_t value_size;
    size_t size;
    BTreeNode* root;
    int(*comp)(const void*, const void*);
};

static BTreeNode* build_node(const BTree* tree, const void* key) {
    if ((tree == NULL) || (key == NULL)) {
        return NULL;
    }
    BTreeNode* node = calloc(sizeof(BTreeNode), 1);
    if (node == NULL) {
        return NULL;
    }
    node->item = (BTreeItem*)malloc(sizeof(BTreeItem));
    if (node->item == NULL) {
        free(node);
        return NULL;
    }
    node->item->value = malloc(tree->value_size);
    node->item->key = malloc(tree->key_size);
    if ((node->item->value == NULL) || (node->item->key == NULL)) {
        free(node->item->value);
        free((void*)node->item->key);
    	free(node->item);
        free(node);
        return NULL;
    }
    memcpy((void*)node->item->key, key, tree->key_size);
    return node;
}

static BTreeNode* traversal_tree(BTreeNode* node, const void* key, int(*compare)(const void*, const void*), bool* flag) {
    if ((node == NULL) || (key == NULL) || (flag == NULL) || (compare == NULL)) {
        return NULL;
    }

    while (true)
    {
        const int route = compare(node->item->key, key);
        if (route == 0) {
            *flag = true;
            return node;
        }
        if (route > 0) {
            if (node->left_node == NULL) return node;
            node = node->left_node;
        }
        if (route < 0) {
            if (node->right_node == NULL) return node;
            node = node->right_node;
        }
    }
}

static void delete_node(BTreeNode* node, void(*destroy)(void*)) {
    if (node == NULL) {
        return;
    }
    if (destroy != NULL) {
        destroy(node->item);
    }
    free((void*)node->item->key);
    free(node->item->value);
    free(node->item);
    free(node);
}

static BTreeNode* leftmost_node(BTreeNode*  node) {
    if (node == NULL) {
        return NULL;
    }
    while (node->left_node != NULL) {
        node = node->left_node;
    }
    return node;
}

static BTreeNode* rightmost_node(BTreeNode* node) {
    if (node == NULL) {
        return NULL;
    }
    while(node->right_node != NULL) {
        node = node->right_node;
    }
    return node;
}

static void decoupling_leaf(const BTreeNode* node, int(*compare)(const void*, const void*)) {
    if (node->parent_node == NULL) {
        return;
    }
    const int meaning = compare(node->parent_node->item->key, node->item->key);
    if (meaning <= 0) {
        node->parent_node->right_node = NULL;
    }
    else {
        node->parent_node->left_node = NULL;
    }
}

static void decoupling_node_path(BTreeNode* node, int(*compare)(const void*, const void*), BTree* tree) {
    if (node->parent_node->parent_node == NULL) {
        tree->root = node;
        node->parent_node = NULL;
        return;
    }
    const int meaning = compare(node->parent_node->parent_node->item->key, node->item->key);
    if (meaning <= 0) {
        node->parent_node->parent_node->right_node = node;
    }
    else {
        node->parent_node->parent_node->left_node = node;
    }
    node->parent_node = node->parent_node->parent_node;
}

static void node_remove(BTreeNode* node, int(*compare)(const void*, const void*), void(*destroy)(void*), BTree* tree) {
    if ((node->left_node == NULL) && (node->right_node == NULL)) {
        decoupling_leaf(node, compare);
        delete_node(node, destroy);
    }
    else if ((node->left_node == NULL) && (node->right_node != NULL)) {
        decoupling_node_path(node->right_node, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->left_node != NULL) && (node->right_node == NULL)) {
        decoupling_node_path(node->left_node, compare, tree);
        delete_node(node, destroy);
    }
    else if ((node->left_node != NULL) && (node->right_node != NULL)) {
        BTreeNode* left_root = leftmost_node(node->right_node);
        if (destroy != NULL) {
            destroy(node->item);
        }
        memcpy((void*)node->item->key, left_root->item->key, tree->key_size);
        memcpy(node->item->value, left_root->item->value, tree->value_size);
        node_remove(left_root, compare, NULL, tree);
    }
}

static void delete_all_nodes(BTreeNode* node, void(*destroy)(void*)) {
    if (node == NULL) {
        return;
    }
    if (node->left_node != NULL) {
        BTreeNode* left_node = node->left_node;
        left_node->parent_node = NULL;
        node->left_node = NULL;
        delete_all_nodes(left_node, destroy);
    }
    if (node->right_node != NULL) {
        BTreeNode* right_node = node->right_node;
        right_node->parent_node = NULL;
        node->right_node = NULL;
        delete_all_nodes(right_node, destroy);
    }
    delete_node(node, destroy);
}

static bool functor_next(size_t* item_id) {
    const BTreeNode* node = (const BTreeNode*)*item_id;
    if (node->right_node != NULL) {
        *item_id = (size_t)leftmost_node(node->right_node);
        return true;
    }
    return false;
}

static bool functor_prev(size_t* item_id) {
    const BTreeNode* node = (const BTreeNode*)*item_id;
    if (node->left_node != NULL) {
        *item_id = (size_t)rightmost_node(node->left_node);
        return true;
    }
    return false;
}

static bool key_less(const void* first_key, const void* second_key, int(*compare)(const void*, const void*)) {
    return compare(first_key, second_key) < 0;
}

static bool key_more(const void* first_key, const void* second_key, int(*compare)(const void*, const void*)) {
    return compare(first_key, second_key) > 0;
}

static size_t manager_neighbors(const BTree* tree, size_t item_id, bool(*functor)(size_t*), bool (*rule)(const void*, const void*, int(*)(const void*, const void*))) {
    if ((tree == NULL) || (item_id == 0)) {
        return btree_stop(tree);
    }

    if (functor(&item_id)) {
        return item_id;
    }

    const BTreeNode* node = (const BTreeNode*)item_id;

    if (node->parent_node != NULL) {
        BTreeNode* parent_node = node->parent_node;
        while (parent_node)
        {
            if (rule(node->item->key, parent_node->item->key, tree->comp)) {
                return (size_t)parent_node;
            }
            parent_node = parent_node->parent_node;
        }
    }
    return btree_stop(tree);
}


void* btree_create(size_t keySize, size_t valueSize, int(*compare)(const void*, const void*)){
    if ((keySize == 0) || (valueSize == 0) || (compare == NULL)) {
        return NULL;
    }

    BTree* tree = malloc(sizeof(BTree));
    if (tree == NULL) {
        return NULL;
    }

    tree->size = 0;
    tree->key_size = keySize;
    tree->value_size = valueSize;
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
    BTree* tree = btree;
    btree_clear(tree, destroy);
    tree->key_size = keySize;
    tree->value_size = valueSize;
    tree->comp = compare;
    return tree;
}

void btree_clear(void* btree, void(*destroy)(void*)) {
    if (btree == NULL) {
        return;
    }
    BTree* tree = btree;
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
    const BTree* tree = btree;
    return tree->size;
}

void* btree_item(const void* btree, const void* key) {
    const BTree* tree = btree;
    if ((tree == NULL) || (key == NULL)) {
        return NULL;
    }
    bool node_found = false;
    const BTreeNode* node = traversal_tree(tree->root, key, tree->comp, &node_found);
    if (!node_found) {
        return NULL;
    }
    return node->item->value;
}

void* btree_insert(void* btree, const void* key, bool* createFlag){
    BTree* tree = btree;
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
        return tree->root->item->value;
    }

    bool node_found = false;
    BTreeNode* node = traversal_tree(tree->root, key, tree->comp, &node_found);

    if (node == NULL) {
        return NULL;
    }

    if (node_found) {
        *createFlag = false;
        return node->item->value;
    }

    BTreeNode* leaf = NULL;
    const int route = tree->comp(node->item->key, key);

    if (route > 0) {
        node->left_node = build_node(tree, key);
        leaf = node->left_node;
    }
    else if (route < 0) {
        node->right_node = build_node(tree, key);
        leaf = node->right_node;
    }
    if (leaf == NULL) {
        return NULL;
    }
    tree->size++;
    leaf->parent_node = node;
    *createFlag = true;
    return leaf->item->value;

}

void btree_remove(void* btree, const void* key, void(*destroy)(void*)) {
    if ((btree == NULL) || (key == NULL)) {
        return;
    }
    BTree* tree = btree;
    if (tree->root == NULL) {
        return;
    }
    bool flag = false;
    BTreeNode* node = traversal_tree(tree->root, key, tree->comp, &flag);
    if (flag && (node != NULL)) {
        node_remove(node, tree->comp, destroy, tree);
        tree->size -= 1;
        if (tree->size == 0) tree->root = NULL;
    }
}


size_t btree_first(const void* btree) {
    if (btree == NULL) {
        return btree_stop(btree);
    }
    const BTree* tree = btree;
    if (tree->root == NULL) {
        return btree_stop(btree);
    }
    return (size_t)leftmost_node(tree->root);
}

size_t btree_last(const void* btree) {
    if (btree == NULL) {
        return btree_stop(btree);
    }
    const BTree* tree = btree;
    if (tree->root == NULL) {
        return btree_stop(btree);
    }
    return (size_t)rightmost_node(tree->root);
}

size_t btree_next(const void* btree, size_t item_id) {
    return manager_neighbors(btree, item_id, functor_next, key_less);
}

size_t btree_prev(const void* btree, size_t item_id) {
    return manager_neighbors(btree, item_id, functor_prev, key_more);
}

size_t btree_stop(const void* btree) {
    return (size_t)NULL;
}

void* btree_current(const void* btree, size_t item_id)
{
    if ((item_id == 0) || (btree == NULL)) {
        return NULL;
    }
    const BTreeNode* node = (const BTreeNode*)item_id;
    if (btree_item(btree, node->item->key) != NULL) {
        return node->item;
    }
    return NULL;
}

void btree_erase(void* btree, size_t item_id, void(*destroy)(void*)) {
    if ((btree == NULL) || (item_id == 0)) {
        return;
    }
    const BTreeNode* node = (const BTreeNode*)item_id;
    btree_remove(btree, node->item->key, destroy);
}