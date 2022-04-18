#include <stdio.h>
#include "strmap.h"

StrMap* newMap()
{
    StrMap* map;
    map = malloc(sizeof(StrMap));
    memset(map,0,sizeof(StrMap));
    return map;
}

int StrMapKey(char* str)
{
    unsigned int ret = 0;
    for(;(*str)!='\0';str++)
    {
	ret += (int)*str;
    }
    //printf("%s chain is %d\n",str,ret % HASH_SIZE);
    return ret % HASH_SIZE;
}

void* getStrMap(StrMap* map,char* key)
{
    StrMapNode* pNode = map->hash[StrMapKey(key)];
    for(;pNode!=NULL;pNode=pNode->next)
    {
	if(strcmp(pNode->key, key) == 0)
	{
	    return pNode->value;
	}
    }
    return NULL;
}

void insStrMap(StrMap* map, char* key, void* value, int size)
{
    int mapchain = StrMapKey(key);
    StrMapNode* pNode = map->hash[mapchain];
    //新しいノードを予約し、値を埋める。
    int keysize = strlen(key) + 1;
    StrMapNode* newNode = malloc(sizeof(StrMapNode));
    //keyのメモリを予約
    newNode->key = (char*)malloc(keysize);
    //keyに文字列をコピー
    strncpy(newNode->key, key, keysize);
    //valueのメモリを予約
    newNode->value = malloc(size);
    //valueにデータをコピー
    memcpy(newNode->value, value, size);
    //valueのnextにリンクリストの先頭アドレスを登録
    newNode->next = pNode;
    //ハッシュチェーンの先頭に登録
    map->hash[mapchain] = newNode;
    //printf("newNode->key = %s, newNode->value = %s\n",newNode->key,(char*)newNode->value);
}

void delStrMap(StrMap* map, char* key)
{
    int mapchain = StrMapKey(key);
    StrMapNode* pNode = map->hash[mapchain];
    StrMapNode* prev = NULL;
    for(;pNode!=NULL;pNode=pNode->next)
    {
	printf("search map %s\n",key);
	if(strcmp(pNode->key, key) == 0)
	{
	    //前のノードのnextに次のノードのアドレスを譲渡
	    if(prev!=NULL)
	    {
		if(pNode->next!=NULL)
		{
		    prev->next = pNode->next;
		}
	    }
	    else
	    {
		//最初のノードの場合
		map->hash[mapchain] = pNode->next;
	    }
	    //ノードのメモリを解放
	    free(pNode->key);
	    free(pNode->value);
	    free(pNode);
	    break;
	}
	else
	{
	    prev = pNode;
	}
    }
}

int lenStrMap(StrMap* map)
{
    int i,size = 0;
    StrMapNode* pNode;
    for ( i = 0 ; i < HASH_SIZE ; i ++ )
    {
	pNode = map->hash[i];
	while(pNode)
	{
	    size++;
	    pNode = pNode->next;
	}
    }
    return size;
}

void freeStrMap(StrMap* map)
{
    int i;
    StrMapNode* pNode;
    StrMapNode* pNext;
    for ( i = 0 ; i < HASH_SIZE ; i ++ )
    {
	pNode = map->hash[i];
	while(pNode)
	{
	    pNext = pNode->next;
	    free(pNode->key);
	    free(pNode->value);
	    free(pNode);
	    pNode = pNext;
	}	    
    }
}

void printStrMap(StrMap* map)
{
    StrMapNode* pNode;
    int i;
    printf("map contents\n");
    for ( i = 0 ; i < HASH_SIZE ; i ++ )
    {
	pNode = map->hash[i];
	while(pNode)
	{
	    printf("key<%s> = value<%s>\n",pNode->key,(char*)pNode->value);
	    pNode = pNode->next;
	}
    }
    printf("map size is %d\n",lenStrMap(map));
}

