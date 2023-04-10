#pragma once
#include <stdbool.h> // bool
#include <stddef.h>  // size_t

static const size_t INVALID = ~((size_t)0);

typedef
struct BTreeItem
{
    const void* key;
    void* value;
}
BTreeItem;


void* btree_create(size_t keySize, size_t valueSize, int(*compare)(const void*, const void*));
void btree_destroy(void* btree, void(*destroy)(void*));

void* btree_init(
    void* btree,
    size_t keySize,
    size_t valueSize,
    int(*compare)(const void*, const void*),
    void(*destroy)(void*));
void btree_clear(void* btree, void(*destroy)(void*));

size_t btree_count(const void* btree);
void* btree_item(const void* btree, const void* key);
void* btree_insert(void* btree, const void* key, bool* createFlag);
void btree_remove(void* btree, const void* key, void(*destroy)(void*));

size_t btree_first(const void* btree);
size_t btree_last(const void* btree);
size_t btree_next(const void* btree, size_t item_id);
size_t btree_prev(const void* btree, size_t item_id);
size_t btree_stop(const void* btree);
void* btree_current(const void* btree, size_t item_id);
void btree_erase(void* btree, size_t item_id, void(*destroy)(void*));