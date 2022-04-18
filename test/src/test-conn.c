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

    /* getConnector */
    Connector* connector = getConnector(configpath);
    if(connector==NULL){
	exit(EXIT_FAILURE);
    }
    Connector* con2 = getConnector(NULL);
    if(con2 != NULL)
    {
	printf("static conn succeed\n");
    }

    /* connReaddir */
    list = connReaddir("/home/yonde/Documents/RemoteFS");
    for(node = list->head; node != NULL; node = node->next)
    {
	attr = (Attribute*)node->data;
	printf("%s %ld %ld\n",attr->path,attr->st.st_size,attr->st.st_mtime);		
    }
    freeList(list, freeAttr);
    printf("\n\n");

    /* connStat */
    printf("connStat\n");
    attr = connStat("/etc/ssh/sshd_config");
    if(attr != NULL)
    {
	printf("%s %ld %ld\n",attr->path,attr->st.st_size,attr->st.st_mtime);		
    }
    printf("\n\n");

    return 0;
}
