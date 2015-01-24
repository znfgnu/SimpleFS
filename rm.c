/* 
 * File:   sfsdiag.c
 * Author: konrad
 *
 * Created on 19 stycznia 2015, 01:02
 */

#include <stdio.h>
#include <stdlib.h>

#include "Flags.h"
#include "Sfs.h"

/*
 * 
 */

void help(char** argv) {
    printf("Usage: %s DISK FILE1 FILE2...\n", argv[0]);
    printf("Removes files named \"FILE*\" from DISK.\n");
}

int main(int argc, char** argv) {
    Md *md; // metadata
    int i,j,k;  // for loop
    int index;
    int blocks;
    int N;
    int statuscode=0;
    
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help(argv);
        return 0;
    }
    
    if (argc < 2) {
        fprintf(stderr, "Filename not given.\n");
        return 1;
    }
    
    md = sfs_open(argv[1], SFS_WRITE);
    if (md == NULL) {
        return 1;
    }
    
    N = md->datablocks;
    Inode* inod = sfs_inodemalloc(md);
    
    for (i=2; i<argc; i++) {
        index = sfs_getinodeindexbyfilename(md, argv[i]);
        if (index == -1) {
            fprintf(stderr, "File \"%s\" doesn't exist", argv[i]);
            statuscode--;
        }
        else {
            sfs_getinode(md, inod, index);
            blocks = HOWMANYBLOCKS;
            for (j=0; j<blocks; j++) sfs_removedatablock(md, inod->datablocks[j]);
            sfs_removeinode(md, index);
        }
    }

    sfs_inodefree(inod);
    sfs_close(md);
    
    return (statuscode);
}

