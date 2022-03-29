#include <stdio.h>
#include "utils/list.h"

List* newList()
{
    List* plist = malloc(sizeof(List));
    plist->head = NULL;
    return plist;
}

void push_front(List* list, void* Data, int SizeofData)
{   
    //新しいNodeを予約
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = malloc(SizeofData);
    //新しいheadに今までの先頭を登録
    newNode->next = list->head;
    //第２引数からDataをコピー
    memcpy(newNode->data, Data, SizeofData);
    //リストの先頭にnewNodeを登録
    list->head = newNode;  
}

void push_back(List* list, void* Data, int SizeofData)
{
    //新しいNodeを予約
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = malloc(SizeofData);
    newNode->next = NULL;
    //第２引数からDataをコピー
    memcpy(newNode->data, Data, SizeofData);
    //リストが空の場合は先頭に新ノードを登録する。空でなければ最後のノードまで移動する。
    if(list->head == NULL)
    {
	list->head = newNode;
    }
    else
    {
	Node* pNode = list->head;
	while(pNode->next != NULL)
	{
	    pNode = pNode->next;
	}
	//最後のノードのnextにnewNodeを代入
	pNode->next = newNode;
    }
}

void printList(List* list, void (*fptr)(void *))
{
    Node* pNode = list->head;
    while(pNode != NULL)
    {
	fptr(pNode->data);
	pNode = pNode->next;
    }
}

void printInt(void *n)
{
    printf(" %d\n",*(int*)n);
}

void printStr(void *n)
{
    printf(" %s\n",(char*)n);
}

int length(List* list)
{
    int cnt = 0;
    Node* pNode = list->head;
    while(pNode != NULL)
    {
	cnt++;
	pNode = pNode->next;
    }
    return cnt;
}

//int main()
//{
//    List* intlist = newList();
//    int array[] = {0,1,2,3,4,5};
//    for(int i=0;i<6;i++)
//    {
//	push_back(intlist,&array[i],sizeof(int));
//    }
//    printList(intlist,printInt);
//    printf("size of list is %d\n",length(intlist));
//    
//    List* strlist = newList();
//    char* docs[] = {"Apple","Banana","Grape"};
//    for(int i=0;i<3;i++)
//    {
//	push_back(strlist,docs[i],strlen(docs[i])+1);
//    }
//    printList(strlist,printStr);
//    printf("size of list is %d\n",length(strlist));
//    return 0;
//}
//
