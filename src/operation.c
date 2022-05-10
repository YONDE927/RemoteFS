#include "conn.h"
#include "map.h"

typedef struct FileHandler 
{
    off_t offset;
    FileSession* session;
} FileHandler;

int oarRead(FileHandler* fh, char *buffer, size_t size, off_t offset, int flag)
{
    int rc;

    //オフセットの設定 
    fh->offset = offset;

    if(flag==1)
    {
        //通信呼び出し
        rc = connRead(fh->session, offset, buffer, size);
        if(rc < 0)
        {
            return -1;
        }
        //オフセットの設定 
        return rc;
    }
    else
    {
        return -1;
    }
    fh->offset += rc;
    return rc;
}
