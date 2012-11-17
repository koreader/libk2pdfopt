/*
** wzfile.c     Routines that work on regular or .gz files
**              transparently.  The wz...() functions work identically
**              to the stdio f...() functions (e.g. fopen, fclose, ...),
**              but you use WZFILE instead of FILE for the pointer
**              type.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2012  http://willus.com
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
#include <string.h>

/*
** This library doesn't have much point existing without ZLIB, but without it
** all functions default to uncompressed streams.  Not tested.
*/
#ifdef HAVE_Z_LIB
#include <zlib.h>
#endif

static char *compressed_exts[]={"gz",
                                "tgz",""};
static char *uncompressed_exts[]={"",
                                  "tar",""};
#ifdef HAVE_Z_LIB
static char wzbuffer[4096];
#endif

static int archive_extract(char *archfile,char *file_to_extract,char *tempname);
static int wzfile_status_special(char *archfile,char *filename,char *subarch);

/*
** RECURSIVE
**
** Opens pseudo-archive path.  If the file opened is a temporary file
** (because it had to be extracted from an archive file), then tempname
** contains the temporary name and should be removed with
** wzfile_fully_remove() when it is done being processed, since
** the directory it is contained within is also a temporary file.
**
** Initial call should have archfile=NULL.
**
*/
WZFILE *wzopen_special(char *archfile,char *filename,char *tempname)

    {
    char pathname[MAXFILENAMELEN];
    int i;

// printf("@wzopen_special(%s,%s)\n",archfile==NULL?"NULL":archfile,filename);
    tempname[0]='\0';
    if (archfile!=NULL)
        {
        char subarch[MAXFILENAMELEN];
        int status;

        status=wzfile_status_special(archfile,filename,subarch);
        if (status==1 || status==3)
            {
            if (!archive_extract(archfile,filename,tempname))
                return(NULL);
            return(wzopen(tempname,"rb"));
            }
        if (status==4)
            {
            WZFILE *wf;
            char tempname2[MAXFILENAMELEN];
            char newname[MAXFILENAMELEN];

            strcpy(newname,&filename[strlen(subarch)+1]);
            if (!archive_extract(archfile,subarch,tempname2))
                return(NULL);
            wf=wzopen_special(tempname2,newname,tempname);
            wzfile_fully_remove(tempname2);
            return(wf);
            }
        return(NULL);
        }
    i=0;
    if (filename[0]=='\\' && filename[1]=='\\')
        {
        strcpy(pathname,"\\\\");
        i=2;
        }
    for (;1;)
        {
        char newarch[MAXFILENAMELEN];

        for (;filename[i]!='\0' && filename[i]!='\\' && filename[i]!='/';i++)
            pathname[i]=filename[i];
        pathname[i]='\0';
        if (filename[i]=='\0')
            return(wzopen(pathname,"rb"));
        if (wzfile_status_special(archfile,pathname,NULL)==2)
            {
            pathname[i]=filename[i];
            i++;
            continue;
            }
        if (!wfile_is_archive(pathname))
            return(NULL);
        strcpy(newarch,pathname);
        return(wzopen_special(newarch,&filename[i+1],tempname));
        }
    }

/*
** Removes tempfile and the directory containing it.
*/
void wzfile_fully_remove(char *tempfile)

    {
    char path[MAXFILENAMELEN];

    if (tempfile[0]=='\0')
        return;
    remove(tempfile);
    wfile_basepath(path,tempfile);
    wfile_remove_dir(path,0);
    }


static int archive_extract(char *archfile,char *file_to_extract,char *tempname)

    {
    char archname_full[MAXFILENAMELEN];
    char tempdir[MAXFILENAMELEN];
    char cwd[MAXFILENAMELEN];
    char basefile[MAXFILENAMELEN];
    char cmd[MAXFILENAMELEN+128];

// printf("@archive_extract(%s,%s)\n",archfile,file_to_extract);
    strcpy(archname_full,archfile);
    wfile_make_absolute(archname_full);
    wfile_basespec(basefile,file_to_extract);
    wfile_abstmpnam(tempdir);
    wfile_makedir(tempdir);
    strcpy(cwd,wfile_get_wd());
    wfile_set_wd(tempdir);
    if (!stricmp(wfile_ext(archfile),"7z"))
        sprintf(cmd,"7z -r e \"%s\" \"%s\" 1> nul 2> nul",archname_full,file_to_extract);
    else
#ifdef WIN32
        sprintf(cmd,"unzip -C -o -j \"%s\" \"%s\" 1> nul 2> nul",archname_full,file_to_extract);
#else
        sprintf(cmd,"unzip -o -j \"%s\" \"%s\" 1> nul 2> nul",archname_full,file_to_extract);
#endif
    system(cmd);
    wfile_set_wd(cwd);
    wfile_fullname(tempname,tempdir,basefile);
    if (wfile_status(tempname)!=1)
        {
        printf("Internal error executing command '%s'!\n",cmd);
        exit(10);
        }
    return(1);
    }


static int wzfile_status_special(char *archfile,char *filename,char *subarch)

    {
    FILELIST *fl,_fl;
    int i;

// printf("@wzfile_status_special(%s,%s)\n",archfile,filename);
    if (archfile==NULL)
{
// printf("    Returns %d\n",wfile_status(filename));
        return(wfile_status(filename));
}
    fl=&_fl;
    filelist_init(fl);
    filelist_fill_from_zip(fl,archfile,"*");
    /*
    ** Look for perfect match
    */
    for (i=0;i<fl->n;i++)
        {
#ifdef WIN32
        if (!stricmp(filename,fl->entry[i].name))
#else
        if (!strcmp(filename,fl->entry[i].name))
#endif
            {
            int attr,is_arch;
            attr=fl->entry[i].attr;
            is_arch=wfile_is_archive(fl->entry[i].name);
            filelist_free(fl);
            if (attr & WFILE_DIR)
{
// printf("    Returns 2\n");
                return(2);
}
            if (is_arch)
{
// printf("    Returns 3\n");
                return(3);
}
// printf("    Returns 1\n");
            return(1);
            }
        }
    /*
    ** Look for possible internal archive/folder
    */
    for (i=0;i<fl->n;i++)
        {
        int l1;

        l1=strlen(fl->entry[i].name);
#ifdef WIN32
        if (!strnicmp(filename,fl->entry[i].name,l1) && wfile_eitherslash(filename[l1]))
#else
        if (!strncmp(filename,fl->entry[i].name,l1) && wfile_eitherslash(filename[l1]))
#endif
            {
            int is_arch;

            is_arch=wfile_is_archive(fl->entry[i].name);
            if (is_arch && subarch!=NULL)
                strcpy(subarch,fl->entry[i].name);
            filelist_free(fl);
            if (is_arch)
{
// printf("    Returns 4\n");
                return(4);
}
// printf("    Returns A.0\n");
            return(0);
            }
        l1=strlen(filename);
#ifdef WIN32
        if (!strnicmp(filename,fl->entry[i].name,l1) && wfile_eitherslash(fl->entry[i].name[l1]))
#else
        if (!strncmp(filename,fl->entry[i].name,l1) && wfile_eitherslash(fl->entry[i].name[l1]))
#endif
            {
            filelist_free(fl);
// printf("    Returns 2\n");
            return(2);
            }
        }
    filelist_free(fl);
// printf("    Returns B.0\n");
    return(0);
    }
    

int wfile_is_binary(char *filename,int maxlen)

    {
    WZFILE *f;
    int i,c;

    f=wzopen(filename,"rb");
    if (f==NULL)
        return(0);
    i=0;
    while ((c=wzgetc(f))!=EOF && i<maxlen)
        {
        if (c&0x80)
            {
            wzclose(f);
            return(1);
            }
        i++;
        }
    wzclose(f);
    return(0);
    }


/*
** Should actually do case-sensitive compare for Unix
*/
int wfile_is_gzfile(char *filename)

    {
#ifdef HAVE_Z_LIB
    int i,len,el;

    len=strlen(filename);
    for (i=0;compressed_exts[i][0]!='\0';i++)
        {
        el=strlen(compressed_exts[i]);
        if (len>el+1 && filename[len-el-1]=='.'
                     && !stricmp(&filename[len-el],compressed_exts[i]))
            return(i+1);
        }
#endif
    return(0);
    }


/*
** Should actually do case-sensitive compare for Unix
*/
int wfile_is_special_uncompressed(char *filename)

    {
#ifdef HAVE_Z_LIB
    int i,len,el;

    len=strlen(filename);
    for (i=0;compressed_exts[i][0]!='\0';i++)
        {
        if (uncompressed_exts[i][0]=='\0')
            continue;
        el=strlen(uncompressed_exts[i]);
        if (len>el+1 && filename[len-el-1]=='.'
                     && !stricmp(&filename[len-el],uncompressed_exts[i]))
            return(i+1);
        }
#endif
    return(0);
    }


void wzfile_convert_to_compressed_name(char *dst,char *src)

    {
    int i;

    i=wfile_is_special_uncompressed(src);
    if (i==0)
        {
        strcpy(dst,src);
        strcat(dst,".gz");
        return;
        }
    i--;
    wfile_stripext(dst,src);
    strcat(dst,".");
    strcat(dst,compressed_exts[i]);
    }


void wzfile_convert_to_uncompressed_name(char *dst,char *src)

    {
    int i;

    i=wfile_is_gzfile(src);
    if (i==0)
        {
        strcpy(dst,src);
        return;
        }
    i--;
    wfile_stripext(dst,src);
    if (uncompressed_exts[i][0]!='\0')
        {
        strcat(dst,".");
        strcat(dst,uncompressed_exts[i]);
        }
    }


/*
** Check date of filename or gzipped filename
*/
void wzfile_date(char *filename,struct tm *date)

    {
    static char newname[MAXFILENAMELEN];

    if (wfile_status(filename)==1)
        {
        wfile_date(filename,date);
        return;
        }
    if (wfile_is_gzfile(filename))
        wzfile_convert_to_uncompressed_name(newname,filename);
    else
        wzfile_convert_to_compressed_name(newname,filename);
    wfile_date(newname,date);
    }


/*
** If filename ends in compressed extension, opens as gzipped file.
** Otherwise as regular.
*/
WZFILE *wzopen(char *filename,char *mode)

    {
    WZFILE *wz;
    void *p,*ptr;
    int type;
#ifdef HAVE_Z_LIB
    static char newname[MAXFILENAMELEN];
#endif
    char mode2[16];
    int i;

    strncpy(mode2,mode,14);
    mode2[13]='\0';
    for (i=0;mode2[i]!='\0' && mode2[i]!='b';i++);
    if (mode2[i]!='b')
        strcat(mode2,"b");
#ifdef HAVE_Z_LIB
    if (wfile_is_gzfile(filename))
        {
        p=(void *)gzopen(filename,mode2);
        type=1;
        if (p==NULL)
            {
            wzfile_convert_to_uncompressed_name(newname,filename);
            p=(void *)fopen(newname,mode);
            type=0;
            }
        }
    else
        {
#endif
        p=(void *)fopen(filename,mode);
        type=0;
#ifdef HAVE_Z_LIB
        if (p==NULL)
            {
            wzfile_convert_to_compressed_name(newname,filename);
            p=(void *)gzopen(newname,mode2);
            type=1;
            }
        }
#endif
    if (p==NULL)
        return(NULL);
    willus_mem_alloc_warn(&ptr,sizeof(WZFILE),"wzopen",10);
    wz=(WZFILE *)ptr;
    wz->f=p;
    wz->type=type;
    return(wz);
    }


int wzclose(WZFILE *wz)

    {
    double *ptr;

    if (wz!=NULL)
        {
        int status;

#ifdef HAVE_Z_LIB
        if (wz->type!=0)
            status=gzclose((gzFile)wz->f);
        else
#endif
            status=fclose((FILE *)wz->f);
        ptr=(double *)wz;
        willus_mem_free(&ptr,"wzclose");
        wz=(WZFILE *)ptr;
        return(status);
        }
    return(0);
    }


/*
** Note!  In gzip mode, all \r's are stripped.
*/
char *wzgets(char *buf,int maxlen,WZFILE *wz)

    {
    if (wz==NULL)
        return(NULL);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        {
        char *p;
        int i,j;

        p=gzgets((gzFile)wz->f,buf,maxlen);
        /* Strip \r's from string */
        for (i=j=0;i<maxlen && buf[i]!='\0';i++)
            if (buf[i]!='\r')
                {
                if (i!=j)
                    buf[j]=buf[i];
                j++;
                }
        buf[j]='\0';
        return(p);
        }
#endif
    return(fgets(buf,maxlen,(FILE *)wz->f));
    }


int wzgetc(WZFILE *wz)

    {
    if (wz==NULL)
        return(EOF);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzgetc((gzFile)wz->f));
#endif
    return(fgetc((FILE *)wz->f));
    }


int wzputc(WZFILE *wz,int c)

    {
    if (wz==NULL)
        return(EOF);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzputc((gzFile)wz->f,c));
#endif
    return(fputc(c,(FILE *)wz->f));
    }


int wzwrite(WZFILE *wz,void *ptr,int nbytes)

    {
    if (wz==NULL)
        return(0);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzwrite((gzFile)wz->f,ptr,nbytes));
#endif
    return(fwrite(ptr,1,nbytes,(FILE *)wz->f));
    }


#ifdef WILLUS_HAVE_FILE64
int wzseek(WZFILE *wz,long long position)
#else
int wzseek(WZFILE *wz,long position)
#endif

    {
    if (wz==NULL)
        return(-1);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzseek((gzFile)wz->f,position,0));
#endif
    return(wfile_seek((FILE *)wz->f,position,0));
    }


/* type==2 doesn't work! */
#ifdef WILLUS_HAVE_FILE64
int wzseek2(WZFILE *wz,long long position,int type)
#else
int wzseek2(WZFILE *wz,long position,int type)
#endif

    {
    if (wz==NULL)
        return(-1);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzseek((gzFile)wz->f,position,type));
#endif
    return(wfile_seek((FILE *)wz->f,position,type));
    }


#ifdef WILLUS_HAVE_FILE64
long long wztell(WZFILE *wz)
#else
long wztell(WZFILE *wz)
#endif

    {
    if (wz==NULL)
        return(-1L);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gztell((gzFile)wz->f));
#endif
    return(wfile_tell((FILE *)wz->f));
    }


void wzrewind(WZFILE *wz)

    {
    if (wz==NULL)
        return;
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        {
        gzrewind((gzFile)wz->f);
        return;
        }
#endif
    fseek((FILE *)wz->f,0L,0);
    }


int wzread(WZFILE *wz,void *ptr,int nbytes)

    {
    if (wz==NULL)
        return(0);
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        return(gzread((gzFile)wz->f,ptr,nbytes));
#endif
    return(fread(ptr,1,nbytes,(FILE *)wz->f));
    }


int wzbe_read(WZFILE *wz,void *ptr,int elsize,int nobj)

    {
#ifdef WILLUS_BIGENDIAN
    return(wzread(wz,ptr,elsize*nobj));
#else
    char *a;
    int i,j,status,n2,nread;

    if (elsize<2)
        return(wzread(wz,ptr,elsize*nobj));
    a=(char *)ptr;
    n2=elsize/2;
    nread=0;
    for (i=0,a=(char *)ptr;i<nobj;i++,a+=elsize)
        {
        if ((status=wzread(wz,a,elsize))<elsize) 
            return(nread);
        nread++;
        for (j=0;j<n2;j++)
            {
            char c;
            c=a[j];
            a[j]=a[elsize-j-1];
            a[elsize-j-1]=c; 
            }
        }
    return(nread);
#endif
    }


int wzbe_write(WZFILE *wz,void *ptr,int elsize,int nobj)

    {
#ifdef WILLUS_BIGENDIAN
    return(wzwrite(wz,ptr,elsize*nobj));
#else
    char *a,*b;
    int i,j,status,n2,nwritten;
    static char *funcname="wfile_be_write";
    void *xptr;
    double *d;

    if (elsize<2)
        return(wzwrite(wz,ptr,elsize*nobj));
    a=(char *)ptr;
    n2=elsize/2;
    nwritten=0;
    willus_mem_alloc_warn(&xptr,elsize,funcname,10);
    b=(char *)xptr;
    for (i=0,a=(char *)ptr;i<nobj;i++,a+=elsize)
        {
        memcpy(b,a,elsize);
        for (j=0;j<n2;j++)
            {
            char c;
            c=b[j];
            b[j]=b[elsize-j-1];
            b[elsize-j-1]=c; 
            }
        if ((status=wzwrite(wz,b,elsize))<elsize) 
            {
            d=(double *)b;
            willus_mem_free(&d,funcname);
            return(nwritten);
            }
        nwritten++;
        }
    d=(double *)b;
    willus_mem_free(&d,funcname);
    return(nwritten);
#endif
    }


int wzerror(WZFILE *wz)

    {
#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        {
        int errnum;

        gzerror((gzFile)wz->f,&errnum);
        return(errnum);
        }
#endif
    return(ferror((FILE *)wz->f));
    }


int wzprintf(WZFILE *wz,char *fmt,...)

    {
    va_list args;
    int status;

#ifdef HAVE_Z_LIB
    if (wz->type!=0)
        {
        va_start(args,fmt);
        status=vsprintf(wzbuffer,fmt,args);
        va_end(args);
        if (status<0)
            return(status);
        return(gzwrite((gzFile)wz->f,wzbuffer,strlen(wzbuffer)));
        }
#endif
    va_start(args,fmt);
    status=vfprintf(wz->f,fmt,args);
    va_end(args);
    return(status);
    }


int wzcompressed(WZFILE *wz)

    {
#ifdef HAVE_Z_LIB
    return(wz->type!=0);
#else
    return(0);
#endif
    }


WZFILE *wzuncompressed(FILE *out)

    {
    static WZFILE wz;

    wz.type=0;
    wz.f=(void *)out;
    return(&wz);
    }
