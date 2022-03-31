#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE   13

typedef struct StrMapNode{
    struct StrMapNode* next;
    char* key;
    void* value;
} StrMapNode;

typedef struct StrMap{
    StrMapNode* hash[HASH_SIZE];
    int size;
} StrMap;

StrMap* newMap();
void* getStrMap(StrMap* map,char* key);
void insStrMap(StrMap* map, char* key, void* value, int size);
void delStrMap(StrMap* map, char* key);
int lenStrMap(StrMap* map);
void freeStrMap(StrMap* map);
void printStrMap(StrMap* map);

