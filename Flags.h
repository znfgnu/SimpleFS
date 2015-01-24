/* 
 * File:   Flags.h
 * Author: konrad
 *
 * Created on 18 stycznia 2015, 15:33
 */

#ifndef FLAGS_H
#define	FLAGS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define SFS_READ O_RDONLY
#define SFS_WRITE O_RDWR
#define SFS_CREAT (O_RDWR|O_CREAT|O_EXCL)


#ifdef	__cplusplus
}
#endif

#endif	/* FLAGS_H */

