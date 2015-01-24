/* 
 * File:   mksfs.c
 * Author: konrad
 *
 * Created on 18 stycznia 2015, 16:33
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "Sfs.h"
#include "Flags.h"

/*
 * 
 */

void help(char** argv) {
    printf("Usage: %s -n NUMBEROFBLOCKS [-b BLOCKSIZE] FILENAME\n", argv[0]);
    printf("Arguments:\n");
    printf("  -n N\tself-explainable\n");
    printf("  -b BLOCKSIZE\tset size of each datablock to BLOCKSIZE (in bytes).\n");
}

int main(int argc, char** argv) {
    char *charerr = NULL;
    
    int dblocks = -1;
    int blocksize = 4096;
    
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help(argv);
        return 0;
    }
    
    opterr = 0;
    int c;
    while ((c = getopt(argc, argv, "hb:n:")) != -1) {
        switch(c) {
            case 'h':
                help(argv);
                return 0;
            case 'b':
                // block size
                blocksize = strtol(optarg, &charerr, 10);
                if (blocksize <= 0) charerr=1;
                if (*charerr != '\0') {
                    fprintf (stderr, "Invalid size of datablock (should be an integer).\n");
                    return 1;
                }
                break;
            case 'n':
                // number of blocks
                dblocks = strtol(optarg, &charerr, 10);
                if (dblocks <= 0) charerr=1;
                if (*charerr != '\0') {
                    fprintf (stderr, "Invalid number of blocks (should be an integer).\n");
                    return 1;
                }
                break;
            case '?':
                if (optopt == 'b' || optopt == 'n')
                  fprintf (stderr, "Option -%c requires an argument. See %s -h for more information.\n", optopt, argv[0]);
                else if (isprint (optopt))
                  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                  fprintf (stderr,
                           "Unknown option character `\\x%x'.\n",
                           optopt);
                // no break!
            default:
                return 1;
                
        }
    }  
    
    if (dblocks <= 0) {
        fprintf (stderr, "Number of datablocks not set.\n");
        return 1;
    }
    
    if (optind == argc) {
        fprintf (stderr, "Filename not set.\n");
        return 1;
    }
    
    for (; optind < argc; optind++) {
        Md *md = malloc(sizeof(Md));
        md->datablocks = dblocks;
        md->sizeofblock = blocksize;
        md->freeblocks = dblocks;
        sfs_init(md, argv[optind]);
        sfs_close(md);
    }
    
    return (EXIT_SUCCESS);
}

