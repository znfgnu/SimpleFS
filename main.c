/* 
 * File:   main.c
 * Author: konrad
 *
 * Created on 18 stycznia 2015, 15:20
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Sfs.h"
#include "Flags.h"

void create_test() {
    Md *md = sfs_open("testdrive", SFS_CREAT);

    
    
    sfs_close(md);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    Md *md = sfs_open(argv[1], SFS_WRITE);
    printf("%d %d %d %d\n", bmpsize(7), bmpsize(8), bmpsize(16), bmpsize(17));
    printf("N: %d\tbs: %d\tfree: %d\n", md->datablocks, md->sizeofblock, md->freeblocks);
    sfs_removeinode(md, 0);

    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    sfs_setdatablock(md, 0);
    
    Inode* inod = sfs_inodemalloc(md);
//    inod->filename = "Las wegas.exe";
    strcpy(inod->filename, "Las wegas.exe");
    inod->filesize = 7;
    inod->datablocks[0] = 0;
    sfs_writeinode(md, inod, 0);
    sfs_setinode(md, 0);
    sfs_inodefree(inod);

    sfs_close(md);
    
//    printf("%d\n", drivesize(8,4096));
    

    return (EXIT_SUCCESS);
}

