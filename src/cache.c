#include <stdio.h>
#include <string.h>
#include "list.h"
#include "map.h"

#define BLOCK_SIZE 10240 // 10KB
#define MAX_CACHE_SIZE 104857600 //100MB

/* キャッシュの理想的な要件
 * - 部分的にキャッシュされる
 * - リクエストによって徐々にキャッシュが補完される
 * - 足りない部分だけをもってきてキャッシュと組み合わせる
 * - 全体のキャッシュ制限を超えないようにする
 */

/* キャッシュのミドル要件
 * - リクエスト履歴を保持
 * - 少しでも足りなければ、キャッシュはヒットしていない
 * - 全体のキャッシュ制限を超えないようにする
 */

/* キャッシュの最低要件
 * - リクエストに関わらず一括でメモリに乗せる
 * - 全体のキャッシュ制限を超えないようにする
 */

//ファイルごとにキャッシュ＆統合もできる
typedef struct Cache
{
    char* buffer; //CacheBlock
    int opened;
    int size;
    int mtime;
    int dirty;
} Cache;

typedef struct MemCache
{
    List* cache_queue; //Cache
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

//キャッシュブロックに要求が満たせるか判定
//要求がキャッシュにあるのか


//バッファをキャッシュに書き込む。書き込み場所によっては既存キャッシュを統合・上書きする。

int main()
{
    MemCache* memcache = getMemCache();
    printf("%d files are cached.\n", lenStrMap(memcache->cache_map));
    return 0;
}
