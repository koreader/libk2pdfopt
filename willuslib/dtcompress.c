/*
** dtcompress.c  Routines to write a compressed stream with zlib header
**               into pdf files (from Dirk Thierbach).
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2014  http://willus.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Affero General Public License as
** published by the Free Software Foundation, either version 3 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
*/
#include "willus.h"

/*
** If zlib is not available, streams are not compressed.
*/
#ifdef HAVE_Z_LIB
#include <zlib.h>

/*
** This uses (unpatched) zlib, writes a zlib header, and can be used for
** uncompressed writing, too.
**
** Usage: 
** For compression, wrap with compress_init and compress_done:
**
**   FILE* f;
**   compress_handle h;
**
**   ...
**   h = compress_start(f, 7); // compression level = 7
**   for (...)
**       {
**       ...
**       n_written = compress_write(f, h, buf, sizeof(buf));
**       }
**   compress_done(f, h);
**   ...
**
** For uncompressed write, just pass NULL as handle:
**
**   compress_write(f, NULL, buf, sizeof(buf));
**
** The handle will be set to NULL by compress_start if zlib is not available, 
** so there's no need for #ifdef's in the actual code
**
** In case of error, compress_write will return a negative number. In that
** case, the internal zlib state and allocations should be cleaned up
** by calling compress_done with a NULL file, to keep it from attempting
** to write out whatever remains in the buffer.
**
*/

#define COMPRESS_CHUNK 16384

typedef struct compress_handle_s
    {
    z_stream strm;
    unsigned char in[COMPRESS_CHUNK];
    unsigned char out[COMPRESS_CHUNK];
    } compress_handle_t;

typedef compress_handle_t *compress_handle_p;

compress_handle compress_start(FILE *f,int level)

    {
    compress_handle_p h;
    int ret;
    static char *funcname="compress_start";

    willus_mem_alloc_warn((void **)&h,sizeof(compress_handle_t),funcname,10);
    h->strm.zalloc = Z_NULL;
    h->strm.zfree = Z_NULL;
    h->strm.opaque = Z_NULL;
    h->strm.total_in = 0;
    h->strm.total_out = 0;
    h->strm.avail_in = 0;
    h->strm.next_in = &h->in[0];
    ret = deflateInit2(&h->strm,level,Z_DEFLATED,MAX_WBITS,8,Z_DEFAULT_STRATEGY);
    /* memory level 8 (default) = 128K */
    if (ret != Z_OK) /* Error */
        return NULL;
    return ((compress_handle)h);
    }

/*
** In: strm out empty, next_in and avail_in set 
** Out:Return negative value on error, else bytes written.
*/
static size_t compress_out(FILE *f,compress_handle_p h,int flush) 

    {
    int ret;
    size_t have,written;

    /*
    ** run deflate() on input until output buffer not full, finish
    ** compression if all of source has been read in
    */
    written = 0;
    do
        {
        h->strm.avail_out = COMPRESS_CHUNK;
        h->strm.next_out = &h->out[0];
        ret = deflate(&h->strm,flush);  /* no bad return value */
        if (ret==Z_STREAM_ERROR)
            {
            fprintf(stderr,"Internal error in compress_out.  Z_STREAM_ERROR.\n"
                           "Program aborted.\n");
            exit(99);
            }
        have = COMPRESS_CHUNK - h->strm.avail_out; // size of output produced
        if (fwrite(&h->out,1,have,f)!=have || ferror(f)) 
            {
            (void)deflateEnd(&h->strm);
            return Z_ERRNO;
            }
        written += have;
        } while (h->strm.avail_out==0);  /* full output, there may be more */
    /* all input must have been used now */
    if (h->strm.avail_in!=0)
        {
        fprintf(stderr,"Internal error in compress_out.  Not all input used.\n"
                       "Program aborted.\n");
        exit(99);
        }
    return(written);
    }


/*
** Call with NULL filehandle in case of error etc., to clean up zlib state 
** and allocations.
*/
void compress_done(FILE *f,compress_handle *hh) 

    {
    static char *funcname="compress_done";

    compress_handle_p h = (compress_handle_p)(*hh);
    if (h)
        {
        if (f)
            compress_out(f,h,Z_FINISH);
        deflateEnd(&h->strm);
        /* Fix memory leak, 2-2-14 */
        willus_mem_free((double **)hh,funcname);
        }
    }


/*
** Inv: strm out empty, avail_in is bytes stored so far, next_in set
*/
size_t compress_write(FILE *f,compress_handle hh,const void *buf,size_t size)

    {
    compress_handle_p h = (compress_handle_p) hh;
    size_t n,written;

    if (!h)
        return fwrite(buf, 1, size, f);
    else
        {
        written = 0;
        while (size > 0) 
            {
            n = COMPRESS_CHUNK - h->strm.avail_in;
            if (n > size)
                n = size;
            memcpy(h->in+h->strm.avail_in,buf,n);
            h->strm.avail_in += n;
            if (h->strm.avail_in >= COMPRESS_CHUNK)
                {
                if (compress_out(f,h,Z_NO_FLUSH) < 0) 
                    {
                    /* ERROR; */
                    return Z_ERRNO;
                    }
                h->strm.avail_in = 0;
                h->strm.next_in = &h->in[0];
                }
            written += n;
            size -= n;
            buf += n;
            }
        return written;
        }
    }

#else /* HAVE_Z_LIB */

compress_handle compress_start(FILE *f,int level) 

    {
    return NULL;
    }

void compress_done(FILE *f,compress_handle *h) 

    {
    }

size_t compress_write(FILE *f,compress_handle h,const void *buf,size_t size)

    {
    return(fwrite(buf,1,size,f));
    }

#endif /* HAVE_Z_LIB */
