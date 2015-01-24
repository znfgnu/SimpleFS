/* 
 * File:   sfsdiag.c
 * Author: konrad
 *
 * Created on 19 stycznia 2015, 01:02
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "Flags.h"
#include "Sfs.h"

/*
 * 
 */

void help(char** argv) {
    printf("Usage: %s DISK FILE1 FILE2...\n", argv[0]);
    printf("Copies files named \"FILE*\" to DISK from actual directory.\n");
}

int main(int argc, char** argv) {
    Md *md; // metadata
    int i,j,k;  // for loop
    int index;
    int blocks;
    int N;
    int statuscode=0;
    char* fn;
    
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help(argv);
        return 0;
    }
    
    if (argc < 3) {
        fprintf(stderr, "Filename not given.\n");
        return 1;
    }
    
    md = sfs_open(argv[1], SFS_WRITE);
    if (md == NULL) {
        return 1;
    }
    
    N = md->datablocks;
    Inode* inod = sfs_inodemalloc(md);
    index = -1;
    
    // looking for existing file
    for (i=2; i<argc; i++) {
        fn = basename(argv[i]);
        index = sfs_getinodeindexbyfilename(md, fn);
        if (index == -1) {
            sfs_copytodisk(md, argv[i], fn);
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

