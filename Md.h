/* 
 * File:   md.h
 * Author: konrad
 * 
 * Metadata structure
 *
 * Created on 18 stycznia 2015, 15:04
 */

#ifndef MD_H
#define	MD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include<stdint.h>
    
#define bmpsize(N) ((N+7)/8)
#define inodsize(N) (256+4+N*4)   // nazwa, rozmiar, bitmapy, bloki z danymi
#define drivesize(N,bs) (3*4 + 2*bmpsize(N) + N*inodsize(N) + N*bs)
    
#define FN_LEN 256
    
typedef struct {
    char filename[FN_LEN];
    int32_t filesize;
    int32_t *datablocks;
} Inode;
    
typedef struct {
    int32_t datablocks;
    int32_t sizeofblock;
    int32_t freeblocks;
    char *inodbmp;
    char *blkbmp;
    int fd;
    int lastinode;  // artificial
    int lastdatablock;  // artificial
} Md;


#ifdef	__cplusplus
}
#endif

#endif	/* MD_H */

