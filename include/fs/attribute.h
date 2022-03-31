#include <sys/stat.h>
#include <stdlib.h>

typedef struct Attribute
{
    struct stat st;
    char* path;
} Attribute;

void attrFree(Attribute* attr)
{
    free(attr->path);
    free(attr);
}
