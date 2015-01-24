/* 
 * File:   sfs.h
 * Author: konrad
 *
 * Created on 18 stycznia 2015, 15:03
 */

#ifndef SFS_H
#define	SFS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <fcntl.h>
#include"Md.h"
    
#define CHECK_BIT(x,pos) ((x) & (1<<(pos)))
#define HOWMANYBLOCKS ( inod->filesize!=0 ?((inod->filesize+md->sizeofblock-1)/(md->sizeofblock)) : 1 )
    
Md* sfs_open(const char* filename, int flags);
int sfs_close(Md* md);
int sfs_init(Md *md, const char* filename);
// ...
Inode* sfs_inodemalloc(Md* md);
void sfs_inodefree(Inode* inod);

// [TODO] DOpisac reszte deklaracji

#ifdef	__cplusplus
}
#endif

#endif	/* SFS_H */

