#include <stdio.h>
#include "conn.h"
#include "list.h"

int main(int argc, char**argv)
{
    List* list;
    Node* node;
    Attribute* attr;
    if(argc < 2)
    {
	printf("conn.o [SSHCONFIG]\n");
	exit(EXIT_FAILURE);
    }
    char* configpath = argv[1];
    Connector* connector = getConnector(configpath);
    if(connector==NULL){
	exit(EXIT_FAILURE);
    }
    Connector* con2 = getConnector(NULL);
    if(con2 != NULL)
    {
	printf("static conn succeed\n");
    }
    list = connReaddir("/home/yonde/Documents/RemoteFS");
    for(node = list->head; node != NULL; node = node->next)
    {
	attr = (Attribute*)node->data;
	printf("%s %ld %ld\n",attr->path,attr->st.st_size,attr->st.st_mtime);		
    }
    printf("\n");
    return 0;
}
