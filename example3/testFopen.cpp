#if        defined(_POSIX_SOURCE)
#include        <sys/types.h>
#endif
#include        <stdio.h>
#include        <stdlib.h>
#include        "loc_incl.h"

#define        PMODE                0666
#define        O_RDONLY        0
#define        O_WRONLY        1
#define        O_RDWR                2
#define        O_CREAT                0x010
#define        O_TRUNC                0x020
#define        O_APPEND        0x040
int _open(const char *path, int flags);
int _creat(const char *path, Mode_t mode);
int _close(int d);
FILE *
fopen(const char *name, const char *mode)
{
        register int i;
        int rwmode = 0, rwflags = 0;
        FILE *stream;
        int fd, flags = 0;
        for (i = 0; __iotab[i] != 0 ; i++)
                if ( i >= FOPEN_MAX-1 )
                        return (FILE *)NULL;
        switch(*mode++) {
        case 'r':
                flags |= _IOREAD | _IOREADING;        
                rwmode = O_RDONLY;
                break;
        case 'w':
                flags |= _IOWRITE | _IOWRITING;
                rwmode = O_WRONLY;
                rwflags = O_CREAT | O_TRUNC;
                break;
        case 'a':
                flags |= _IOWRITE | _IOWRITING | _IOAPPEND;
                rwmode = O_WRONLY;
                rwflags |= O_APPEND | O_CREAT;
                break;         
        default:
                return (FILE *)NULL;
        }
        while (*mode) {
                switch(*mode++) {
                case 'b':
                        continue;
                case '+':
                        rwmode = O_RDWR;
                        flags |= _IOREAD | _IOWRITE;
                        continue;
               
                default:
                        break;
                }
                break;
        }
        if ((rwflags & O_TRUNC)
            || (((fd = _open(name, rwmode)) < 0)
                    && (rwflags & O_CREAT))) {
                if (((fd = _creat(name, PMODE)) > 0) && flags  | _IOREAD) {
                        (void) _close(fd);
                        fd = _open(name, rwmode);
                }
                        
        }
        if (fd < 0) return (FILE *)NULL;
        if (( stream = (FILE *) malloc(sizeof(FILE))) == NULL ) {
                _close(fd);
                return (FILE *)NULL;
        }
        if ((flags & (_IOREAD | _IOWRITE))  == (_IOREAD | _IOWRITE))
                flags &= ~(_IOREADING | _IOWRITING);
        stream->_count = 0;
        stream->_fd = fd;
        stream->_flags = flags;
        stream->_buf = NULL;
        __iotab[i] = stream;
        return stream;
}