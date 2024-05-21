/*
** ocrtess.c   Get OCR of WILLUS bitmap using Tesseract
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2018  http://willus.com
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <leptonica.h>
#include <tesseract.h>
#include "willus.h"


static int ocrtess_lstm_file(char *filename);
static int ocrtess_tess_file(char *filename);
static int file_contains_keyword(char *filename,char *keyword);
static int buf_contains_keyword(char *buf,char *keyword,int n);
/*
static int has_cube_data(char *lang);
*/
static void endian_flip(char *x,int n);

/*
** Returns 0 for success, NZ for failure.
*/
void *ocrtess_init(char *datadir,char *tesspath,int maxtesspathlen,
                   char *lang,FILE *out,char *initstr,int maxlen,int *status)

    {
    char langdef[16];
    void *api;
    char tesspath0[MAXFILENAMELEN];

    ocrtess_datapath(tesspath0,datadir,MAXFILENAMELEN-1);
    if (tesspath!=NULL)
        {
        strncpy(tesspath,tesspath0,maxtesspathlen-1);
        tesspath[maxtesspathlen-1]='\0';
        }
    if (lang==NULL || lang[0]=='\0')
        ocrtess_lang_default(tesspath0,NULL,0,langdef,16,NULL,0,0);
    else
        {
        strncpy(langdef,lang,15);
        langdef[15]='\0';
        }
    /* Tess v4.00 needs only one attempt with ocrtype=0 */
    api=tess_capi_init(tesspath0,langdef,0,out,initstr,maxlen,status);
    return(api);
    }


void ocrtess_lang_default(char *datadir,char *tesspath,int maxtesspathlen,
                          char *langdef,int maxlen,char *tessdebug,int maxdebug,int use_ansi)

    {
    char wildcard[MAXFILENAMELEN+32];
    int j;
    FILELIST *fl,_fl;
    char tesspath0[MAXFILENAMELEN];
    static char *header=
       "File name                          Size         Date      Type*\n"
       "---------------------------------------------------------------------\n";
    /* "12345678901234567890123456789012  XXX.XX MB  XX-XXX-XXXX  [LSTM+TESS] */
    static char *fmt="  %6.2f MB  %2d-%s-%04d  %s";
    static char *month_name[12]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

    if (datadir==NULL)
        ocrtess_datapath(tesspath0,datadir,MAXFILENAMELEN-1);
    else
        {
        strncpy(tesspath0,datadir,MAXFILENAMELEN-1);
        tesspath0[MAXFILENAMELEN-1]='\0';
        }
    if (tesspath!=NULL)
        {
        strncpy(tesspath,tesspath0,maxtesspathlen-1);
        tesspath[maxtesspathlen-1]='\0';
        }
    strncpy(langdef,"eng",maxlen-1);
    langdef[maxlen-1]='\0';
    wfile_fullname(wildcard,tesspath0,"*.traineddata");
    fl=&_fl;
    filelist_init(fl);
    filelist_fill_from_disk_1(fl,wildcard,0,0);
    filelist_sort_by_date(fl);
    for (j=fl->n-1;j>=0;j--)
        {
        char basename[512];
        int i;

        wfile_basespec(basename,fl->entry[j].name);
        if (in_string(basename,"-")>0)
            continue;
        strcpy(wildcard,fl->entry[j].name);
        strncpy(langdef,basename,maxlen-1);
        langdef[maxlen-1]='\0';
        i=in_string(langdef,".");
        if (i>0)
            langdef[i]='\0';
        break;
        }
    if (tessdebug!=NULL)
        {
        filelist_sort_by_name(fl);
        if (strlen(header)+strlen(tessdebug) < maxdebug)
            sprintf(&tessdebug[strlen(tessdebug)],"%s",header);
        for (j=0;j<fl->n;j++)
            {
            char fullname[256];
            char name[256];
            int len1,tess,lstm,len;

            wfile_fullname(fullname,fl->dir,fl->entry[j].name);
            strcpy(name,fl->entry[j].name);
            lstm=ocrtess_lstm_file(fullname);
            tess=ocrtess_tess_file(fullname);
            len1 = strlen(name)>32 ? strlen(name) : 32;
            if (use_ansi)
                len1+=20;
            if (strlen(tessdebug)+len1+46 < maxdebug)
                {
                if (use_ansi)
                    {
                    if (tess || lstm)
                        strcat(tessdebug,ANSI_YELLOW);
                    }
                strcat(tessdebug,name);
                len=strlen(name);
                if (!strcmp(fl->entry[j].name,wildcard))
                    {
                    if (use_ansi)
                        strcat(tessdebug,ANSI_WHITE);
                    strcat(tessdebug," [Def]");
                    len+=6;
                    }
                else
                    if (use_ansi)
                        strcat(tessdebug,ANSI_NORMAL);
                for (;len<32;len++)
                    strcat(tessdebug," ");
                sprintf(&tessdebug[strlen(tessdebug)],fmt,fl->entry[j].size/1024./1024.,
                     fl->entry[j].date.tm_mday,
                     month_name[fl->entry[j].date.tm_mon],
                     fl->entry[j].date.tm_year+1900,
                     tess&&lstm?"[LSTM+TESS]":(tess?"[TESS]":(lstm?"[LSTM]":"(not valid)")));
                if (use_ansi)
                    strcat(tessdebug,ANSI_NORMAL);
                strcat(tessdebug,"\n");
                }
            }
        }
    filelist_free(fl);
    }


/*
** Determine OCR path.  Check both TESSDATA_PREFIX\tessdata AND TESSDATA_PREFIX
*/
void ocrtess_datapath(char *datapath,char *suggested,int maxlen)

    {
    char path1[MAXFILENAMELEN-12];
    char path[MAXFILENAMELEN];
    char *p;

    if (suggested!=NULL)
        {
        strncpy(datapath,suggested,maxlen-1);
        datapath[maxlen-1]='\0';
        return;
        }
    if ((p=getenv("TESSDATA_PREFIX"))==NULL)
        {
        datapath[0]='\0';
        return;
        }
    strncpy(path1,p,MAXFILENAMELEN-13);
    path1[MAXFILENAMELEN-13]='\0';
    wfile_fullname(path,path1,"tessdata");
    if (wfile_status(path)==2)
        {
        strncpy(datapath,path,maxlen-1);
        datapath[maxlen-1]='\0';
        return;
        }
    strncpy(datapath,p,maxlen-1);
    datapath[maxlen-1]='\0';
    }


static int ocrtess_lstm_file(char *filename)

    {
    return(file_contains_keyword(filename,"XYTransLSTM"));
    }


static int ocrtess_tess_file(char *filename)

    {
    return(file_contains_keyword(filename,"NULL 0 NULL 0"));
    }


static int file_contains_keyword(char *filename,char *keyword)

    {
    int pos,bufsize,len;
    FILE *f;
    char *buf;
    static char *funcname="file_contains_keyword";

    f=fopen(filename,"rb");
    if (f==NULL)
        return(0);
    bufsize=1024*1024;
    len=strlen(keyword);
    willus_mem_alloc_warn((void **)&buf,bufsize,funcname,10);
    for (pos=0;1;pos+=bufsize-len)
        {
        int n;

        fseek(f,pos,0);
        n=fread(buf,1,bufsize,f);
        if (buf_contains_keyword(buf,keyword,n))
            {
            willus_mem_free((double **)&buf,funcname);
            fclose(f);
            return(1);
            }
        if (n<bufsize)
            break;
        }
    willus_mem_free((double **)&buf,funcname);
    fclose(f);
    return(0);
    }


static int buf_contains_keyword(char *buf,char *keyword,int n)

    {
    int len,i;

    len=strlen(keyword);
    n=(n-len+1);
    for (i=0;i<n;i++)
        if (buf[i]==keyword[0] && !strncmp(&buf[i],keyword,len))
            return(1);
    return(0);
    }

/*
static int has_cube_data(char *lang)

    {
    char *p;
    char tessdir[512];
    char base[32];
    char wildcard[512];
    FILELIST *fl,_fl;
    int n;

    p=getenv("TESSDATA_PREFIX");
    if (p==NULL)
        return(0);
    wfile_fullname(tessdir,p,"tessdata");
    wfile_reslash(tessdir);
    strncpy(base,lang==NULL || lang[0]=='\0' ? "*" : lang,15);
    base[15]='\0';
    strcat(base,".cube.*");
    wfile_fullname(wildcard,tessdir,base);
    fl=&_fl;
    filelist_init(fl);
    filelist_fill_from_disk_1(fl,wildcard,0,0);
    n=fl->n;
    filelist_free(fl);
    return(n>0);
    }
*/


void ocrtess_end(void *api)

    {
    tess_capi_end(api);
    }

/*
void ocrtesswords_init(OCRTESSWORDS *ocrtesswords)

    {
    ocrtesswords->n=ocrtesswords->na=0;
    ocrtesswords->word=NULL;
    }


void ocrtesswords_free(OCRTESSWORDS *ocrtesswords)

    {
    static char *funcname="ocrtesswords_free";
    int i;

    for (i=ocrtesswords->n;i>=0;i--)
        willus_mem_free((double **)&ocrtesswords->word[i].utf8,funcname);
    willus_mem_free((double **)&ocrtesswords->word,funcname);
    ocrtesswords_init(ocrtesswords);
    }


void ocrtesswords_add_ocrtessword(OCRTESSWORDS *ocrtesswords,int left,int top,
                                  int right,int bottom,int baseline,char *text)

    {
    static char *funcname="ocrtesswords_add_ocrtessword";
    OCRTESSWORD *word;

    if (ocrtesswords->n >= ocrtesswords->na)
        {
        int newsize;

        newsize=ocrtesswords->na<128?256:ocrtesswords->na*2;
        if (ocrtesswords->na==0)
            willus_mem_alloc_warn((void **)&ocrtesswords->word,newsize*sizeof(OCRTESSWORD),
                               funcname,10);
        else
            willus_mem_realloc_robust_warn((void **)&ocrtesswords->word,newsize*sizeof(OCRTESSWORD),
                                   ocrtesswords->na*sizeof(OCRTESSWORD),funcname,10);
        ocrtesswords->na=newsize;
        }
    word=&ocrtesswords->word[ocrtesswords->n];
    word->top=top;
    word->left=left;
    word->right=right;
    word->bottom=bottom;
    word->baseline=baseline;
    willus_mem_alloc_warn((void **)&word->utf8,strlen(text)+1,funcname,10);
    strcpy(word->utf8,text);
    ocrtesswords->n++;
    }
*/
    

/*
** bmp8 must be grayscale
** (x1,y1) and (x2,y2) from top left of bitmap
** Output:
**     "text" gets UTF-8 formatted string of OCR'd text.
**
** segmode:
**   -1 = Default (6)
**    0 = Orientation and script detection (OSD) only.
**    1 = Automatic page segmentation with OSD.
**    2 = Automatic page segmentation, but no OSD, or OCR
**    3 = Fully automatic page segmentation, but no OSD.
**    4 = Assume a single column of text of variable sizes.
**    5 = Assume a single uniform block of vertically aligned text.
**    6 = Assume a single uniform block of text. (Default)
**    7 = Treat the image as a single text line.
**    8 = Treat the image as a single word.
**    9 = Treat the image as a single word in a circle.
**    10 = Treat the image as a single character.
**
*/
void ocrtess_ocrwords_from_bmp8(void *api,OCRWORDS *ocrwords,WILLUSBITMAP *bmp8,
                       int x1,int y1,int x2,int y2,int dpi,
                       int segmode,double downsample,FILE *out)

    {
    PIX *pix;
    WILLUSBITMAP *bmp,_bmp;
    int nw,i,it,w,h,dw,dh,bw;
    unsigned char *src,*dst;
    int *top,*left,*bottom,*right,*ybase;
    char *text;

    if (x1>x2)
        {
        w=x1;
        x1=x2;
        x2=w;
        }
    w=x2-x1+1;

    /* Add a border */
    bw=w/40;
    if (bw<6)
        bw=6;
    dw=w+bw*2;
    dw=(dw+3)&(~3);
    if (y1>y2)
        {
        h=y1;
        y1=y2;
        y2=h;
        }
    h=y2-y1+1;
    dh=h+bw*2;

    /* Store in new bitmap */
    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=dw;
    bmp->height=dh;
    bmp->bpp=8;
    bmp_alloc(bmp);
    for (i=0;i<256;i++)
        bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
    dst=bmp_rowptr_from_top(bmp,0);
    memset(dst,255,dw*dh);
    src=bmp_rowptr_from_top(bmp8,y1)+x1;
    dst=bmp_rowptr_from_top(bmp,bw)+bw;
    for (i=y1;i<=y2;i++,dst+=dw,src+=bmp8->width) 
        memcpy(dst,src,w);
    bmp_set_dpi((double)dpi);

    /* Apply downsample */
    if (downsample > 0. && downsample < 0.9)
        {
        int wnew;

        /* Make sure new width is even multiple of 4 */
        wnew=downsample*bmp->width+0.5;
        wnew=(wnew+3)&(~3);
        downsample = (double)wnew/bmp->width;
        bmp_resize(bmp,downsample);
        dpi=dpi*downsample+0.5;
        bmp_set_dpi((double)dpi);
        }
    else
        downsample=1.0;
/*
{
static int counter=0;
char filename[256];
sprintf(filename,"ocrtext%04d.png",++counter);
bmp_write(bmp,filename,stdout,100);
}
*/
    pix=pixCreate(bmp->width,bmp->height,8);
    src=bmp_rowptr_from_top(bmp,0);
    dst=(unsigned char *)pixGetData(pix);
    memcpy(dst,src,bmp->width*bmp->height);
    bmp_free(bmp);
    endian_flip((char *)dst,pixGetWpl(pix)*pixGetHeight(pix));
    pix->xres = pix->yres = dpi;
    tess_capi_get_ocr_multiword(api,pix,segmode<0 || segmode>10 ? 6 : segmode,
                                &left,&top,&right,&bottom,&ybase,&text,&nw,out);
    pixDestroy(&pix);
    ocrwords_clear(ocrwords);
    for (it=i=0;i<nw;i++)
        {
        OCRWORD word;

        ocrword_init(&word);
        if (downsample < .99)
            {
            word.c=left[i]/downsample-bw+0.5;
            word.r=ybase[i]/downsample-bw+0.5;
            word.maxheight=(ybase[i]-top[i])/downsample+0.5;
            word.w=(right[i]-left[i]+1)/downsample+0.5;
            word.h=(bottom[i]-top[i]+1)/downsample+0.5;
            }
        else
            {
            word.c=(left[i]-bw);
            word.r=(ybase[i]-bw);
            word.maxheight=(ybase[i]-top[i]);
            word.w=(right[i]-left[i]+1);
            word.h=(bottom[i]-top[i]+1);
            }
        word.lcheight=word.maxheight;
        word.rot=0;
        word.text=&text[it];
        it+=strlen(&text[it])+1;
        ocrwords_add_word(ocrwords,&word);
/*
printf("'%s', (%d,%d)-(%d,%d) base=%d\n",&text[it],left[i],top[i],right[i],bottom[i],ybase[i]);
        ocrtesswords_add_ocrtessword(ocrtesswords,left[i]-bw,top[i]-bw,
                                     right[i]-bw,bottom[i]-bw,ybase[i]-bw,
                                     &text[it]);
*/
        }
    free(text);
    free(left);
    }


void ocrtess_from_bmp8(void *api,char *text,int maxlen,WILLUSBITMAP *bmp8,
                       int x1,int y1,int x2,int y2,int dpi,
                       int segmode,int allow_spaces,
                       int std_proc,FILE *out)

    {
    PIX *pix;
    int i,w,h,dw,dh,bw,status;
    unsigned char *src,*dst;

    if (x1>x2)
        {
        w=x1;
        x1=x2;
        x2=w;
        }
    w=x2-x1+1;
    bw=w/40;
    if (bw<6)
        bw=6;
    dw=w+bw*2;
    dw=(dw+3)&(~3);
    if (y1>y2)
        {
        h=y1;
        y1=y2;
        y2=h;
        }
    h=y2-y1+1;
    dh=h+bw*2;
    pix=pixCreate(dw,dh,8);
    src=bmp_rowptr_from_top(bmp8,y1)+x1;
    dst=(unsigned char *)pixGetData(pix);
    memset(dst,255,dw*dh);
    dst=(unsigned char *)pixGetData(pix);
    dst += bw + dw*bw;
    for (i=y1;i<=y2;i++,dst+=dw,src+=bmp8->width) 
        memcpy(dst,src,w);
    endian_flip((char *)pixGetData(pix),pixGetWpl(pix)*pixGetHeight(pix));
    /* Tesseract 3.05.00 -- need to set a resolution */
    pix->xres = pix->yres = dpi;
/*
{
static int counter=0;
WILLUSBITMAP *bmp,_bmp;
char filename[256];
sprintf(filename,"ocrt_%04d.png",++counter);
bmp=&_bmp;
bmp_init(bmp);
bmp_copy(bmp,bmp8);
bmp_promote_to_24(bmp);
bmp_set_dpi((double)dpi);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
}
*/
    status=tess_capi_get_ocr(api,pix,text,maxlen,segmode<0 || segmode>10 ? 6 : segmode,out);
    pixDestroy(&pix);
    if (status<0)
        text[0]='\0';
    /*
    clean_line(text);
    */
    if (std_proc)
        ocr_text_proc(text,allow_spaces);
    }


static void endian_flip(char *x,int n)

    {
    int i;

    for (i=0;i<n;i++,x+=4)
        {
        int c;
        c=x[0];
        x[0]=x[3];
        x[3]=c;
        c=x[1];
        x[1]=x[2];
        x[2]=c;
        }
    }
