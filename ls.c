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
    printf("Lists files containde in DISK.\n");
}

int main(int argc, char** argv) {
    Md *md; // metadata
    int i,j,k;  // for loop
    int N;
    int blocks;
    
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
    Inode* inod = sfs_inodemalloc(md);
    
    for (i=0; i<bmpsize(md->datablocks); i++) {
        if (md->inodbmp[i]) {
            for (j=0; j<8; j++) {
                if (CHECK_BIT(md->inodbmp[i], j)) {
                   //printf("Inode number %d, offset: %d\n", 8*i+j, sfs_getinodeoffset(md, 8*i+j));
                   
                   sfs_getinode(md, inod, 8*i+j);
                   printf("%s ", inod->filename);
                   printf("(%dB)\t", inod->filesize);
                   printf("inodes: %d ", inod->datablocks[0]);
                   blocks = (inod->filesize+md->sizeofblock-1)/(md->sizeofblock);
                   for (k=1; k<blocks; k++) {
                       printf(", %d ", inod->datablocks[k]);
                   }
                   printf("\n");
                }
            } 
        }
    }
    
    sfs_inodefree(inod);
    sfs_close(md);
    
    return (EXIT_SUCCESS);
}

