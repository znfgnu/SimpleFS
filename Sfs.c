#include "Sfs.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <math.h>

#include "Flags.h"

#define ITERINOD md->lastinode
#define ITERDTBL md->lastdatablock

Md* sfs_open(const char *filename, int flags) {
    Md *md;
    md = malloc(sizeof(Md));
    
    md->fd = open(filename, flags);
    if (md->fd == -1) {
        perror("Error while opening file");
        return NULL;
    }
    
    read(md->fd, md, 3*4);
    md->inodbmp = malloc(bmpsize(md->datablocks));
    md->blkbmp = malloc(bmpsize(md->datablocks));
    read(md->fd, md->inodbmp, bmpsize(md->datablocks));
    read(md->fd, md->blkbmp, bmpsize(md->datablocks));
    // read metastuff
    md->lastinode = 0;
    md->lastdatablock = 0;
    
    return md;
}

int sfs_close(Md *md) {
    // write stuff into file
    int fd = md->fd;
    int N = md->datablocks;
    
    lseek(fd, 0, SEEK_SET);
    //header
    write(fd, md, 3*4);
    //bitmapy
    write(fd, md->inodbmp, bmpsize(N));
    write(fd, md->blkbmp, bmpsize(N));
    //i tyle, bo inody i databloki sa updateopwane na biezaco
    
    close(md->fd);
    free(md->blkbmp);
    free(md->inodbmp);
    free(md);
    
    // free bitmaps and other md stuff
    return 0;
}

int sfs_init(Md *md, const char* filename) {
    md->fd = open(filename, SFS_CREAT, (mode_t)0660);
    if (md->fd == -1) {
        perror("Error while opening file");
        return 1;
    }
    int fd = md->fd;
    write(fd, md, 3*4);
    ftruncate(fd, (off_t)drivesize(md->datablocks, md->sizeofblock));
    md->lastinode = 0;
    md->lastdatablock = 0;
}

int sfs_getinodeoffset(Md* md, int n) {
    return 3*4+2*bmpsize(md->datablocks)+n*inodsize(md->datablocks);
}

int sfs_getdatablockoffset(Md* md, int n) {
    return 3*4+2*bmpsize(md->datablocks)+(md->datablocks)*inodsize(md->datablocks)+n*(md->sizeofblock);
}

int sfs_removeinode(Md* md, int n) {
    // only changes bitmap
    int bit = n&7;
    int byte = (n>>3);
    md->inodbmp[byte] &= ~((char)1<<bit);
    return 0;
}

int sfs_setinode(Md* md, int n) {
    // only changes bitmap
    int bit = n&7;
    int byte = (n>>3);
    md->inodbmp[byte] |= ((char)1<<bit);
    return 0;
}

int sfs_removedatablock(Md* md, int n) {
    // only changes bitmap
    int bit = n&7;
    int byte = (n>>3);
    if (md->blkbmp[byte] & ((char)1<<bit)) md->freeblocks++;    // if bit was set
    md->blkbmp[byte] &= ~((char)1<<bit);
    return 0;
}

int sfs_setdatablock(Md* md, int n) {
    // only changes bitmap
    int bit = n&7;
    int byte = (n>>3);
    if (!(md->blkbmp[byte] & ((char)1<<bit))) md->freeblocks--;    // if bit was not set
    md->blkbmp[byte] |= ((char)1<<bit);
    return 0;
}

int sfs_getinode(Md* md, Inode* inod, int n) {
    lseek(md->fd, sfs_getinodeoffset(md, n), SEEK_SET);
    read(md->fd, inod, 260);
    read(md->fd, inod->datablocks, 4*(md->datablocks));
    return 0;
}

int sfs_writeinode(Md* md, Inode* inod, int n) {
    sfs_setinode(md, n);
    lseek(md->fd, sfs_getinodeoffset(md, n), SEEK_SET);
    write(md->fd, inod, 260);
    write(md->fd, inod->datablocks, 4*(md->datablocks));
    return 0;
}

int sfs_getinodeindexbyfilename(Md* md, const char *filename) {
    // remember to free memory after function is used!
    int i,j;
    
    Inode* inod = sfs_inodemalloc(md);
    for (i=0; i<bmpsize(md->datablocks); i++) {
        if (md->inodbmp[i]) {
            for (j=0; j<8; j++) {
                if (CHECK_BIT(md->inodbmp[i], j)) {
                    sfs_getinode(md, inod, 8*i+j);
                    if(!strcmp(inod->filename, filename)){
                        sfs_inodefree(inod);
                        return 8*i+j;
                    }
                }
            } 
        }
    }
    sfs_inodefree(inod);
    return -1;  // not found
}

int sfs_copytodisk(Md* md, const char* srcpath, const char* destfilename) {
    int i,j, index, nowblock, blocks, copied, offset, bytes, nbytes;
    int finished = 0;
    char buf[md->sizeofblock];
    
    char* srcfilename = basename(srcpath);
    int srcfd = open(srcpath, O_RDONLY);
    
    struct stat info;
    fstat(srcfd, &info);
    nbytes = info.st_size;
    
    if (md->freeblocks * md->sizeofblock < nbytes) {
        fprintf(stderr, "Not enough space (%dB free, needed %dB).\n", md->freeblocks*md->sizeofblock, nbytes);
        return -1;
    }
    
    // make new inode in memory and insert filename and filesize
    Inode* inod = sfs_inodemalloc(md);
    strcpy(inod->filename, destfilename);
    inod->filesize = nbytes;
    blocks = HOWMANYBLOCKS;
    copied = 0;
    nowblock = 0;
    
    // find datablocks and copy data into them
    for (i=ITERDTBL; i<bmpsize(md->datablocks) && !finished; i++) {
        if (md->blkbmp[i] != (char)0xFF) {
            for (j=0; j<8 && !finished; j++) {
                if (!CHECK_BIT(md->blkbmp[i], j)) { // if bit not set
                    index = 8*i+j;
                    
                    printf("index: %d\n", index);
                    
                    if (index > md->datablocks) break;
                    sfs_setdatablock(md, index);    // is used now
                    inod->datablocks[nowblock++] = index;
                    offset = sfs_getdatablockoffset(md, index);
                    lseek(md->fd, offset, SEEK_SET);
                    
                    if (md->sizeofblock < nbytes-copied) bytes = read(srcfd, buf, md->sizeofblock);
                    else bytes = read(srcfd, buf, nbytes-copied);
                    printf("Writing %d bytes...\n", bytes);
                    write(md->fd, buf, bytes);
                    copied += bytes;
                    printf("Copied %d of %d bytes.\n", copied, nbytes);
                    
                    if (copied == nbytes) {
                        ITERDTBL = i;
                        finished = 1;
                        break;
                    }
                }
            }
        }
    }
    
    // data copied into file, inode still in memory
    
    // find free inode and copy one from the memory
    printf("Looking for inode.\n");
    
    i=ITERINOD;
    while(md->inodbmp[i] == 0xFF) i++;
    for (j=0; j<8; j++) {
        if (!CHECK_BIT(md->inodbmp[i], j)) {
            index = i*8+j;
            printf("Found free inode %d.\n", index);
            if (index > md->datablocks) break;
            sfs_setinode(md, index);
            sfs_writeinode(md, inod, index);
            break;
        }
    }
    
    sfs_inodefree(inod);
    
    // voila.
    return 0;
}

int sfs_copyfromdisk(Md* md, const char* srcfilename, const char* destpath) {
    int blocks, nowblock, bytes;
    int index = sfs_getinodeindexbyfilename(md, srcfilename);
    char buf[md->datablocks];
    
    if (index == -1) {
        fprintf(stderr, "File \"%s\" doesn't exist.\n", srcfilename);
        return -1;
    }
    
    Inode *inod = sfs_inodemalloc(md);
    sfs_getinode(md, inod, index);
    
    int destfd = open(destpath, O_WRONLY|O_CREAT, (mode_t)0660);
    if (destfd == -1) {
        fprintf(stderr, "Error while accessing \"%s\".\n", destpath);
        return -1;
    }
    int32_t bytesremaining = inod->filesize;
    index = 0;
    while (bytesremaining > 0) {
        nowblock = inod->datablocks[index++];
        lseek(md->fd, sfs_getdatablockoffset(md,nowblock), SEEK_SET);
        
        if (bytesremaining < md->sizeofblock) bytes = read(md->fd, buf, bytesremaining);
        else bytes = read(md->fd, buf, md->sizeofblock);
        
        write(destfd, buf, bytes);
        
        bytesremaining -= bytes;
    }
    
    close(destfd);
    
    sfs_inodefree(inod);
    
    return 0;
}

// ---- Inode utils -----

Inode* sfs_inodemalloc(Md* md) {
    Inode* inod = (Inode*) malloc(sizeof(Inode));
    inod->datablocks = malloc((md->datablocks) * 4);
    return inod;
}

void sfs_inodefree(Inode* inod) {
    free(inod->datablocks);
    free(inod);
}
