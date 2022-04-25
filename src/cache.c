#include <stdio.h>
#include <string.h>
#include "list.h"
#include "map.h"

#define BLOCK_SIZE 10240 // 10KB
#define MAX_CACHE_SIZE 104857600 //100MB

//ファイルの部分ごとにキャッシュ＆統合もできる
//blocksはCacheBlockの配列である。
typedef struct Cache
{
    char** buffers;
    int opened;
    int block_num;
    int size;
    int mtime;
} Cache;

typedef struct MemCache
{
    List* cache_queue;
    StrMap* cache_map;
} MemCache;

//シングルトンMemCacheを取得する。
MemCache* getMemCache()
{
    static MemCache* memcache = NULL;
    if(memcache == NULL)
    {
	memcache = malloc(sizeof(MemCache));
	memcache->cache_queue = newList();
	memcache->cache_map = newStrMap();
    }
    return memcache;
}

/*総キャッシュサイズを調べるためのカウンタ*/
void measureCache(void* cache, void* size)
{
    *(int*)size += ((Cache*)cache)->size;
}

//総キャッシュサイズを調べる
int sizeofMemCache(MemCache* memcache)
{
    int size;
    mapStrMap(memcache->cache_map, &size, measureCache);
    return size;
}

//MemCacheからキャッシュを検索する。
Cache* lookupCache(char* path)
{
    Cache* cache;
    MemCache* memcache = getMemCache();

    if(memcache == NULL)
    {
	return NULL;
    }
    cache = getStrMap(memcache->cache_map, path);
    return cache;
}

int slimCache(MemCache* memcache, int size);
//MemCacheにcacheを登録する。キャッシュサイズが大きければ古いキャッシュを削除する。
int registerCache(char* path, Cache* cache)
{
    MemCache* memcache = getMemCache();

    if(memcache == NULL)
    {
	return -1;
    }
    slimCache(memcache, cache->size);
    insStrMap(memcache->cache_map, path, cache, sizeof(Cache)); 
    push_back(memcache->cache_queue, path, strlen(path) + 1);
    return 0;
}

//キャッシュが最新の状態か修正日時を用いてチェックする。
int validateCache(Cache* cache, int mtime)
{
    if(cache == NULL)
    {
	return -1;
    }
    if(cache->mtime > mtime)
    {
	return 1;
    }
    return 0;
}

void freeCache(void* cache)
{
    Cache* pcache = cache;
    for(int i = 0; i < pcache->block_num; i++)
    {
	if(pcache->buffers[i] != NULL)
	{
	    free(pcache->buffers[i]);
	}
    }
    free(pcache->buffers);
    free(pcache);
}

//キャッシュを削除
void delCache(MemCache* memcache, char* path)
{
    delStrMap(memcache->cache_map, path, freeCache);
}

//古いキャッシュをサイズ分削除
int slimCache(MemCache* memcache, int size)
{
    int cache_size, room;
    char* path;
    List* queue;
    Cache* cache;
    
    queue = memcache->cache_queue;
    cache_size = sizeofMemCache(memcache);
    room = MAX_CACHE_SIZE - cache_size;
    //余地があれば終了
    if(room > size)
    {
	return 0;
    }
    //余地がサイズ分を超えるまでキューの前から削除
    while(room < size)
    {
	path = queue->head->data;
	cache = lookupCache(path);
	room += cache->size;
	delCache(memcache, path);
	pop_front(queue, free);
    }
    return 0;
}

//バッファにキャッシュがヒットしたものをコピーする。ヒットしなかった箇所のリストを返す。
List* ReadCache(Cache* cache, char* buf, int offset, int size)
{
    cache->buffers ;
}

//バッファをキャッシュに書き込む。書き込み場所によっては既存キャッシュを統合・上書きする。
int WriteCache(Cache* cache, char* buf, int offset, int size)
{
    ;
}

int main()
{
    MemCache* memcache = getMemCache();
    printf("%d files are cached.\n", lenStrMap(memcache->cache_map));
    return 0;
}
