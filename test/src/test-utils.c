#include "list.h"
#include "strmap.h"
#include <stdio.h>

int test_list()
{
    List* intlist = newList();
    int array[] = {0,1,2,3,4,5};
    char* docs[] = {"Apple","Banana","Grape"};
    for(int i=0;i<6;i++)
    {
	push_back(intlist,&array[i],sizeof(int));
    }
    printList(intlist,printInt);
    printf("size of list is %d\n",length(intlist));
    freeList(intlist,NULL);
    
    List* strlist = newList();
    for(int i=0;i<3;i++)
    {
	push_back(strlist,docs[i],strlen(docs[i])+1);
    }
    printList(strlist,printStr);
    printf("size of list is %d\n",length(strlist));
    freeList(strlist,NULL);
    return 0;
}

int test_strmap()
{
    StrMap* map = newMap();
    char* keys[] = {
	"red","green","yellow"
    };
    char* values[] = {
	"apple","kiwii","banana"
    };
    char* query[] = {"red","purple"};
    for(int i=0;i<3;i++)
    {
	insStrMap(map,keys[i],values[i],strlen(values[i])+1);
    }
    printStrMap(map);
    delStrMap(map,query[0]);
    printStrMap(map);
    printf("key<%s> = value<%s>\n",query[0],(char*)getStrMap(map, query[0]));

    freeStrMap(map);
    return 0;
}

int main()
{
    test_list();
    test_strmap();
    return 0;
}
