#include <fcntl.h>
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

    /* Read Write*/
    char buf[256] = {0};
    int nbytes = 0;
    int read_size = 100;
    int write_size = 64;
    char write_buf[64] = "Hello connWrite test.\n";
    char path[64] = "/tmp/connWrite.txt";
    FILE* file;

    /* connRead */
    printf("\nconnRead\n");
    nbytes = connRead("/tmp/connRead.txt", buf, 0, read_size);
    if(nbytes > 0)
    {
	buf[read_size] = '\0';
	printf("%s\n",buf);
    }
    else
    {
	puts("connRead error");
    }
    
    /* connWrite */
    printf("connWrite\n");
    memset(buf, 0, 256);
    creat(path, S_IRWXU);
    nbytes = connWrite(path, write_buf, 0, write_size);
    if(nbytes > 0)
    {
	file = fopen(path, "r");
	nbytes = fread(buf, sizeof(char), write_size, file);
	buf[write_size] = '\0';
	printf("%s\n", buf);
    }

    return 0;
}
