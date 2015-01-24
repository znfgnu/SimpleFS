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
    printf("Usage: %s DISK\n", argv[0]);
    printf("Shows diagnostic data of DISK.\n");
}

int main(int argc, char** argv) {
    Md *md; // metadata
    int i,j;  // for loop
    int N;
    int realusedinodes = 0;
    int realusedblocks = 0;
    
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help(argv);
        return 0;
    }
    
    if (argc < 2) {
        fprintf(stderr, "Filename not given.\n");
        return 1;
    }
    
    md = sfs_open(argv[1], SFS_READ);
    if (md == NULL) {
        return 1;
    }
    
    N = md->datablocks;
    
    printf("Datablocks: %d\nBlock size: %d\nFree blocks: %d\n", md->datablocks, md->sizeofblock, md->freeblocks);
    printf("Inode size: %d\n", inodsize(N));
    printf("Bitmap size: %d\n", bmpsize(N));

    printf("-- Offsets: ----------\n");
    printf("Inode bitmap offset: %d\n", 3*4);
    printf("Datablock bitmap offset: %d\n", 3*4+bmpsize(N));
    printf("Inodes offset: %d\n", 3*4+2*bmpsize(N));
    printf("Datablocks offset: %d\n", 3*4+2*bmpsize(N)+N*inodsize(N));



    printf("-- Inodes usage: -----\n");
    
    for (i=0; i<bmpsize(md->datablocks); i++) {
        if (md->inodbmp[i]) {
            for (j=0; j<8; j++) {
                if (CHECK_BIT(md->inodbmp[i], j)) {
                   printf("Inode number %d is used. (offset: %d)\n", 8*i+j, sfs_getinodeoffset(md, 8*i+j));
                   realusedinodes++;
                }
            } 
        }
    }

    printf("-- Blocks usage: -----\n");
    
    for (i=0; i<bmpsize(md->datablocks); i++) {
        if (md->blkbmp[i]) {
            for (j=0; j<8; j++) {
                if (CHECK_BIT(md->blkbmp[i], j)) {
                   printf("Datablock number %d is used. (offset: %d)\n", 8*i+j, sfs_getdatablockoffset(md, 8*i+j));
                   realusedblocks++;
                }
            } 
        }
    }
    
    printf("-- Errors: -----------\n");
    if (N-realusedblocks != md->freeblocks) {
        printf("Free blocks variable is incorrect: free %d, given %d.\n", N-realusedblocks, md->freeblocks);
    }
    
    
    return (EXIT_SUCCESS);
}

