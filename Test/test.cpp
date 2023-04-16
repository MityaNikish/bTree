#include "pch.h"
#include <random>

//#include <assert.h>
//#include <stdlib.h>

extern "C"
{
#include "btree.h"
}

//----------------------------------------------------------//

typedef struct {
    int array[8];
    float d_variable;
} Value;

typedef struct {
    char name[10];
} Key;


static int compare(const void* lhsp, const void* rhsp) {
    const auto lhs = static_cast<const Key*>(lhsp);
    const auto rhs = static_cast<const Key*>(rhsp);
    return strcmp(lhs->name, rhs->name);
}

//----------------------------------------------------------//

static int compare_int(const void* a, const void* b) {
    const auto arg1 = static_cast<const int*>(a);
    const auto arg2 = static_cast<const int*>(b);
    return *arg1 > *arg2 ? 1 : *arg1 == *arg2 ? 0 : -1;
}

//----------------------------------------------------------//

typedef struct _KeyMy
{
    char* name;
} KeyMy;

typedef struct _ValueMy
{
    int* array;
} ValueMy;


void build_key(KeyMy* key) {
    key->name = static_cast<char*>(malloc(sizeof(char) * 10));
}

void init_key(KeyMy* key, const int index) {
    build_key(key);
    std::string str;
    str.append("Key" + std::to_string(index));

    for (int i = 0; i < 10; i++) {
        if (i < str.size()) {
            key->name[i] = str[i];
        }
        else {
            key->name[i] = '\0';
        }
    }
}

void free_key(const KeyMy* key) {
    free(key->name);
}

void build_value(ValueMy* value) {
    value->array = static_cast<int*>(malloc(sizeof(int) * 10));
}

void init_value(ValueMy* value) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> rnd_value(0, 100);
    build_value(value);
    for (int i = 0; i < 10; i++) {
        value->array[i] = static_cast<int>(rnd_value(rng));
    }
}

void free_value(const ValueMy* value) {
    free(value->array);
}

void destroy(void* _item) {
    const BTreeItem* item = static_cast<BTreeItem*>(_item);
    free_key(static_cast<const KeyMy*>(item->key));
    free_value(static_cast<const ValueMy*>(item->value));
}

static int compare_(const void* lhsp, const void* rhsp) {
    const auto lhs = static_cast<const KeyMy*>(lhsp);
    const auto rhs = static_cast<const KeyMy*>(rhsp);
    return strcmp(lhs->name, rhs->name);
}

//----------------------------------------------------------//

TEST(TestExample, Test) {

    //Создаем бинарное дерево: ключ - тип Key; значение - тип Value;
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    EXPECT_TRUE(0 == btree_count(btree));
    EXPECT_TRUE(btree_stop(btree) == btree_first(btree));

    //Создаем ключ и значение 
    Key key = { "key1" };
    Value value = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };

    bool isCreated = false;

    //Добавляем по ключу значение
    Value* insertedValue = (Value*)btree_insert(btree, &key, &isCreated);
    EXPECT_TRUE(true == isCreated);
    *insertedValue = value;

    //Проверяем, что по добавленному ключу получим соответствующее значение
    Value* item = (Value*)btree_item(btree, &key);

    for (size_t i = 0; 8 > i; ++i) {
        EXPECT_TRUE(item->array[i] == value.array[i]);
    }

    EXPECT_TRUE(fabsf(item->d_variable - value.d_variable) < 1e-10f);

    //EXPECT_TRUE(btree_last(btree) == btree_first(btree));
    //EXPECT_TRUE(btree_next(btree, btree_first(btree)) == btree_stop(btree));

    btree_destroy(btree, NULL);

    //EXPECT_EQ(1, 1);
    //EXPECT_TRUE(true);
}

TEST(EmergencySituation_btree_create, Test_1) {
    //incorrect KEY size values
    void* btree = btree_create(0, sizeof(Value), compare);
    EXPECT_TRUE(btree == NULL);
}

TEST(EmergencySituation_btree_create, Test_2) {
    //incorrect VALUE size values
    void*  btree = btree_create(sizeof(Key), 0, compare);
    EXPECT_TRUE(btree == NULL);
}

TEST(EmergencySituation_btree_create, Test_3) {
    //incorrect DATA size values
    void*  btree = btree_create(0, 0, compare);
    EXPECT_TRUE(btree == NULL);
}

TEST(EmergencySituation_btree_create, Test_4) {
    //incorrect DATA size values
    void* btree = btree_create(sizeof(int), sizeof(int), compare);
    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_destroy, Test_1) {
    //btree == NULL
    void* btree = NULL;
    btree_destroy(btree, NULL);

    //проверка на утечку
    EXPECT_TRUE(true);
}

TEST(EmergencySituation_btree_destroy, Test_2) {
    //(btree != NULL) and (root == NULL)
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    btree_destroy(btree, NULL);

    //проверка на утечку
    EXPECT_TRUE(true);
}

TEST(EmergencySituation_btree_destroy, Test_3) {
    //(btree != NULL) and (root != NULL)
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key = { "key1" };
    Value value = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };
    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key, &isCreated);
    *insertedValue = value;

    btree_destroy(btree, NULL);

    //проверка на утечку
    EXPECT_TRUE(true);
}


TEST(EmergencySituation_btree_init, Test_1) {
    //btree with old parametres
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key_old = { "key1" };
    Value value_old = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };
    bool isCreated = false;

    Value* insertedValue_old = (Value*)btree_insert(btree, &key_old, &isCreated);
    if (isCreated) {
        *insertedValue_old = value_old;
    }

    //btree with new parametres
    EXPECT_TRUE(btree_init(btree, sizeof(int), sizeof(int), compare_int, NULL) != NULL);

    int key_new = 5;
    int value_new = 12;
    isCreated = false;

    int* insertedValue_new = (int*)btree_insert(btree, &key_new, &isCreated);
    if (isCreated) {
        *insertedValue_new = value_new;
    }
    
    EXPECT_TRUE(btree_stop(btree) != btree_first(btree));

    btree_destroy(btree, NULL);

    //проверка на утечку
}

TEST(EmergencySituation_btree_init, Test_2) {
    //btree with old parametres
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    //have not been changes
    EXPECT_TRUE(NULL == btree_init(btree, NULL, sizeof(int), compare_int, NULL));
    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_init, Test_3) {
    //btree with old parametres
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    //have not been changes
    EXPECT_TRUE(NULL == btree_init(btree, sizeof(int), NULL, compare_int, NULL));
    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_init, Test_4) {
    //btree with old parametres
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    //have not been changes
    EXPECT_TRUE(NULL == btree_init(btree, sizeof(int), sizeof(int), NULL, NULL));
    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_clear, Test_1) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Value value = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };

    bool isCreated = false;

    //Добавляем по ключу значение
    Value* insertedValue1 = (Value*)btree_insert(btree, &key1, &isCreated);
    Value* insertedValue2 = (Value*)btree_insert(btree, &key2, &isCreated);
    *insertedValue1 = value;
    *insertedValue2 = value;

    EXPECT_TRUE(btree_stop(btree) != btree_first(btree));
    btree_clear(btree, NULL);
    EXPECT_TRUE(btree_stop(btree) == btree_first(btree));
    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_count, Test_1) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Value value1 = { {1}, 0.f };
    Value value2 = { {2}, 0.f };
    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key1, &isCreated);
    *insertedValue = value1;
    insertedValue = (Value*)btree_insert(btree, &key2, &isCreated);
    *insertedValue = value2;

    insertedValue = (Value*)btree_insert(btree, &key1, &isCreated);

    EXPECT_TRUE(insertedValue->array[0] == value1.array[0]);
    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_count, Test_2) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int key;

    for (int i = 1; i < 10; i++) {
        key = rand() % 20 + 5;
        int* val = (int*)btree_insert(btree, &key, &isCreated);
        *val = key;
        val = (int*)btree_item(btree, &key);
        EXPECT_TRUE(*val = key);
    }

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_count, Test_3) {
    //haven't tree
    void* btree = NULL;
    EXPECT_TRUE(btree_count(btree) == INVALID);
}

TEST(EmergencySituation_btree_insert, Test_1) {
    //haven't tree
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int key;
    int count = 0;

    for (int i = 0; i < 20; i = i + 2) {
        key = i;
        int* val = (int*)btree_insert(btree, &key, &isCreated);
        if (isCreated) {
            *val = key;
            val = (int*)btree_item(btree, &key);
            EXPECT_TRUE(*val == key);
            count++;
        }
    }

    for (int i = 21; i > 0; i = i - 2) {
        key = i;
        int* val = (int*)btree_insert(btree, &key, &isCreated);
        if (isCreated) {
            *val = key;
            val = (int*)btree_item(btree, &key);
            EXPECT_TRUE(*val == key);
            count++;
        }
    }

    for (int i = 0; i < 10; i++) {
        key = 10;
        int* val = (int*)btree_insert(btree, &key, &isCreated);
        EXPECT_TRUE(*val == key);
    }
    EXPECT_TRUE(btree_count(btree) == count);

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_first, Test_1) {
    //haven't tree
    void* btree = NULL;
    EXPECT_TRUE(btree_first(btree) == btree_stop(btree));
}

TEST(EmergencySituation_btree_first, Test_2) {
    //haven't root
    void*  btree = btree_create(sizeof(Key), sizeof(Value), compare);
    EXPECT_TRUE(btree_first(btree) == btree_stop(btree));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_first, Test_3) {
    //ok
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key = { "key" };
    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key, &isCreated);
    EXPECT_TRUE(true == isCreated);
    if (true == isCreated)
    {
        EXPECT_TRUE(btree_first(btree) != btree_stop(btree));
    }

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_last, Test_1) {
    //haven't tree
    void* btree = NULL;
    EXPECT_TRUE(btree_last(btree) == btree_stop(btree));
}

TEST(EmergencySituation_btree_last, Test_2) {
    //haven't root
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    EXPECT_TRUE(btree_last(btree) == btree_stop(btree));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_last, Test_3) {
    //ok
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key = { "key" };
    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key, &isCreated);
    EXPECT_TRUE(true == isCreated);
    if (true == isCreated)
    {
        EXPECT_TRUE(btree_last(btree) != btree_stop(btree));
    }

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_next, Test_1) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    bool isCreated = false;

    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);

    //node from the left from the root
    EXPECT_TRUE(btree_last(btree) == btree_next(btree, btree_first(btree)));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_next, Test_2) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);
    btree_insert(btree, &key3, &isCreated);

    //the node is the root
    size_t node = btree_next(btree, btree_first(btree));
    node = btree_next(btree, node);
    EXPECT_TRUE(btree_last(btree) == node);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_next, Test_3) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);
    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);


    //node from the right from the root
    size_t node = btree_next(btree, btree_first(btree));
    node = btree_next(btree, node);
    node = btree_next(btree, node);
    EXPECT_TRUE(btree_last(btree) == node);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_next, Test_4) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);
    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);


    //last node
    size_t node = btree_next(btree, btree_first(btree));
    node = btree_next(btree, node);
    node = btree_next(btree, node);
    node = btree_next(btree, node);
    EXPECT_TRUE(btree_stop(btree) == node);

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_next, Test_5) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int key[15] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 };
    int* val;

    for (int i = 0; i < 15; i++) {
        val = (int*)btree_insert(btree, &key[i], &isCreated);
        *val = key[i];
    }

    size_t node_id = btree_first(btree);
    size_t count = 0;
    BTreeItem* item_prev = NULL;
    while (node_id != btree_stop(btree))
    {
        BTreeItem* item = (BTreeItem*)btree_current(btree, node_id);
        if (item_prev != NULL) {
            EXPECT_TRUE(compare_int(item_prev->key, item->key) < 0);
        }
        item_prev = item;
        count++;
        node_id = btree_next(btree, node_id);
    }

    EXPECT_EQ(node_id, btree_stop(btree));
    EXPECT_EQ(btree_count(btree), count);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_prev, Test_1) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key4 = { "key4" };
    Key key3 = { "key3" };
    bool isCreated = false;

    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);

    //node from the left from the root
    EXPECT_TRUE(btree_first(btree) == btree_prev(btree, btree_last(btree)));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_prev, Test_2) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);
    btree_insert(btree, &key2, &isCreated);

    //the node is the root
    size_t node = btree_prev(btree, btree_last(btree));
    node = btree_prev(btree, node);
    EXPECT_TRUE(btree_first(btree) == node);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_prev, Test_3) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);
    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);


    //node from the right from the root
    size_t node = btree_prev(btree, btree_last(btree));
    node = btree_prev(btree, node);
    node = btree_prev(btree, node);
    EXPECT_TRUE(btree_first(btree) == node);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_prev, Test_4) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Key key3 = { "key3" };
    Key key4 = { "key4" };
    bool isCreated = false;

    btree_insert(btree, &key3, &isCreated);
    btree_insert(btree, &key4, &isCreated);
    btree_insert(btree, &key2, &isCreated);
    btree_insert(btree, &key1, &isCreated);


    //last node
    size_t node = btree_prev(btree, btree_last(btree));
    node = btree_prev(btree, node);
    node = btree_prev(btree, node);
    node = btree_prev(btree, node);
    EXPECT_TRUE(btree_stop(btree) == node);

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_current, Test_1) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key = { "key1" };
    Value value = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };

    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key, &isCreated);
    *insertedValue = value;

    BTreeItem* item = (BTreeItem*)btree_current(btree, btree_first(btree));
    EXPECT_TRUE(compare(item->key, &key) == 0);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_current, Test_2) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key1 = { "key1" };
    Key key2 = { "key2" };
    Value value = { {1, 2, 3, 4, 5, 6, 7, 8}, 0.f };

    bool isCreated = false;

    Value* insertedValue = (Value*)btree_insert(btree, &key1, &isCreated);
    *insertedValue = value;
    insertedValue = (Value*)btree_insert(btree, &key2, &isCreated);
    *insertedValue = value;

    BTreeItem* item = (BTreeItem*)btree_current(btree, btree_last(btree));
    EXPECT_TRUE(compare(item->key, &key2) == 0);

    btree_destroy(btree, NULL);
}


TEST(EmergencySituation_btree_remove, Test_1) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int key;
    int* val;

    for (int i = 0; i < 10; i++) {
        key = rand() % 20 + 5;
        val = (int*)btree_insert(btree, &key, &isCreated);
        *val = key;
    }

    int key_test = 16;
    val = (int*)btree_insert(btree, &key_test, &isCreated);
    *val = key_test;

    for (int i = 0; i < 10; i++) {
        key = rand() % 20 + 5;
        val = (int*)btree_insert(btree, &key, &isCreated);
        *val = key;
    }

    //EXPECT_TRUE(*(int*)btree_item(btree, &key) == key);
    EXPECT_TRUE(btree_item(btree, &key_test) != NULL);
    btree_remove(btree, &key_test, NULL);
    EXPECT_TRUE(btree_item(btree, &key_test) == NULL);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_2) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int* val;

    for (int i = 0; i < 10; i++) {
        val = (int*)btree_insert(btree, &i, &isCreated);
        *val = i;
    }

    int key_test = 10;
    val = (int*)btree_insert(btree, &key_test, &isCreated);
    *val = key_test;

    for (int i = 11; i < 20; i++) {
        val = (int*)btree_insert(btree, &i, &isCreated);
        *val = i;
    }

    //EXPECT_TRUE(*(int*)btree_item(btree, &key) == key);
    EXPECT_TRUE(btree_item(btree, &key_test) != NULL);
    btree_remove(btree, &key_test, NULL);
    EXPECT_TRUE(btree_item(btree, &key_test) == NULL);

    EXPECT_TRUE(btree_last(btree) != btree_stop(btree));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_3) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int* val;

    for (int i = 20; i > 10; i--) {
        val = (int*)btree_insert(btree, &i, &isCreated);
        *val = i;
    }

    int key_test = 10;
    val = (int*)btree_insert(btree, &key_test, &isCreated);
    *val = key_test;

    for (int i = 9; i >= 0; i--) {
        val = (int*)btree_insert(btree, &i, &isCreated);
        *val = i;
    }

    //EXPECT_TRUE(*(int*)btree_item(btree, &key) == key);
    EXPECT_TRUE(btree_item(btree, &key_test) != NULL);
    btree_remove(btree, &key_test, NULL);
    EXPECT_TRUE(btree_item(btree, &key_test) == NULL);

    EXPECT_TRUE(btree_first(btree) != btree_stop(btree));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_4) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);
    Key key = { "key1" };

    bool isCreated = false;

    btree_insert(btree, &key, &isCreated);

    btree_remove(btree, &key, NULL);

    EXPECT_TRUE(btree_first(btree) == btree_stop(btree));

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_5) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int* val;

    for (int i = 0; i < 10; i++) {
        val = (int*)btree_insert(btree, &i, &isCreated);
        *val = i;
    }

    for (int i = 0; i < 10; i++) {
        btree_remove(btree, &i, NULL);
    }

    EXPECT_TRUE(btree_count(btree) == 0);
        EXPECT_TRUE(btree_first(btree) == btree_stop(btree));


    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_6)
{
    void* btree = btree_create(sizeof(size_t), sizeof(Value), compare_int);
    size_t size = 100;

    size_t count = 0;
    size_t key;
    bool isCreated;

    for (size_t i = 0; i < size; ++i)
    {
        key = i * i;
        isCreated = false;
        btree_insert(btree, &key, &isCreated);

        if (!isCreated)
        {
            EXPECT_EQ(btree_count(btree), count);
            continue;
        }
        ++count;
        EXPECT_EQ(btree_count(btree), count);
    }

    EXPECT_EQ(btree_count(btree), count);

    for (size_t i = 0; i < size; ++i)
    {
        key = i * i;
        EXPECT_TRUE(btree_item(btree, &key) != NULL);
    }

    for (size_t i = 0; i < size / 2; i++)
    {
        key = i * i;
        bool flag_complite = (btree_item(btree, &key) != NULL);
        btree_remove(btree, &key, NULL);
        count -= flag_complite ? 1 : 0;
        EXPECT_EQ(btree_count(btree), count);
    }

    EXPECT_EQ(btree_count(btree), count);
    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove, Test_7) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int* val;
    int key = 0;

    key = 2;
    val = (int*)btree_insert(btree, &key, &isCreated);
    *val = key;

    key = 1;
    val = (int*)btree_insert(btree, &key, &isCreated);
    *val = key;

    key = 3;
    val = (int*)btree_insert(btree, &key, &isCreated);
    *val = key;

    key = 2;
    btree_remove(btree, &key, NULL);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_erase, Test_1) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int key;
    int* val;

    for (int i = 0; i < 10; i++) {
        key = rand() % 20 + 5;
        val = (int*)btree_insert(btree, &key, &isCreated);
        *val = key;
    }

    //EXPECT_TRUE(*(int*)btree_item(btree, &key) == key);
    size_t first_node = btree_first(btree);

    btree_erase(btree, first_node, NULL);

    size_t first_node_new = btree_first(btree);

    EXPECT_TRUE(first_node != first_node_new);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_erase, Test_2) {
    void* btree = btree_create(sizeof(Key), sizeof(Value), compare);

    Key key1 = { "key1" };
    Key key2 = { "key2" };
    bool isCreated = false;

    btree_insert(btree, &key1, &isCreated);
    btree_insert(btree, &key2, &isCreated);

    //EXPECT_TRUE(*(int*)btree_item(btree, &key) == key);
    size_t first_node = btree_first(btree);

    btree_erase(btree, first_node, NULL);

    size_t first_node_new = btree_first(btree);

    EXPECT_TRUE(first_node != first_node_new);

    btree_destroy(btree, NULL);
}

TEST(EmergencySituation_btree_remove_and_insert, Test_1) {
    void* btree = btree_create(sizeof(int), sizeof(int), compare_int);
    bool isCreated = false;
    int* val;

    int count = 0;

    int key = 0;
    int count_elem = 100;
    int* data_set = (int*)calloc(count_elem, sizeof(int));
    if (data_set == NULL) assert(false);


    for (int i = 0; i < count_elem; i++) {
        key = rand() % 1000 + 1;
        val = (int*)btree_insert(btree, &key, &isCreated);
        if (!isCreated) {
            BTreeItem *item = (BTreeItem*)btree_item(btree, &key);
            EXPECT_TRUE(item != NULL);
        }
        else{
            *val = key;
            *(data_set + i) = key;
            count++;
        }
        isCreated = false;
    }

    EXPECT_TRUE(btree_count(btree) == count);

    for (int i = 0; i < count_elem; i++) {
        key = *(data_set + i);
        if (key != 0) {
            btree_remove(btree, &key, NULL);
            count--;
        }
    }

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(btree_count(btree) == 0);

    EXPECT_TRUE(btree_first(btree) == btree_stop(btree));

    btree_destroy(btree, NULL);
    free(data_set);
}

TEST(EmergencySituation_btree_remove_and_insert, Test_2) {
    void* btree = btree_create(sizeof(ValueMy), sizeof(KeyMy), compare_);
    bool isCreated = false;
    int index;
    int count = 0;
    int count_elem = 10;
    ValueMy* insertedValue;
    KeyMy* data_set = (KeyMy*)calloc(count_elem, sizeof(KeyMy));
    if (data_set == NULL) assert(false);

    for (int i = 0; i < count_elem; i++) {
        KeyMy key;
        index = rand() % 1000 + 1;
        init_key(&key, index);
        insertedValue = (ValueMy*)btree_insert(btree, &key, &isCreated);
        if (!isCreated) {
            BTreeItem* item = (BTreeItem*)btree_item(btree, &key);
            EXPECT_TRUE(item != NULL);
            free_key(&key);
        }
        else {
            init_value(insertedValue);
            init_key((data_set + i), index);
            count++;
        }
        isCreated = false;
    }

    EXPECT_TRUE(btree_count(btree) == count);

    for (int i = 0; i < count_elem; i++) {
        if ((data_set + i)->name != NULL) {
            btree_remove(btree, (data_set + i), destroy);
            count--;
        }
    }

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(btree_count(btree) == 0);

    EXPECT_TRUE(btree_first(btree) == btree_stop(btree));

    btree_destroy(btree, destroy);

    for (int i = 0; i < count_elem; i++) {
        free_key(data_set + i);
    }
    free(data_set);
}

//проверка на работоспособность тестов на утечку
//TEST(TestMem, Test) { 
//    BTreeItem* item = (BTreeItem*)malloc(sizeof(BTreeItem));
//
//    item->value = malloc(sizeof(Value));
//    item->key = malloc(sizeof(Key));
//    
//    free(item);
//}
