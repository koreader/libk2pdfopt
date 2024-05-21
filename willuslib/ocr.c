/*
** ocr.c   Routines involving optical character recognition (OCR)
**         (Not specific to one particular engine.)
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2022  http://willus.com
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
#include <pthread.h>
#include "willus.h"


/* Structures to capture multithreaded OCR results */
typedef struct
    {
    WILLUSBITMAP *bmp;
    int    dpi; /* bitmap dpi */
    int    c1,r1;
    int    index; /* index into ocrwords array that this came from */
    double lcheight;
    pthread_mutex_t mutex;
    int done;
    double scale;
    OCRWORDS ocrwords;
    } OCRRESULT;

typedef struct
    {
    OCRRESULT *ocrresult;
    int thindex;
    int n;
    int na;
    } OCRRESULTS;

static void **global_ocr_api;
static int global_ocr_type;
/*
** If < 0, then | global_ocr_target_dpi | = the desired light of a lowercase letter
** in pixels.
*/
static int global_ocr_target_dpi;

/*
** Support funcs for multithreaded OCR 
*/
static void  ocrresult_init_from_ocrword(OCRRESULT *ocrresult,OCRWORD *word,int index);
static void  ocrresults_make_room(OCRRESULTS *ocrresults);
static void *ocrword_multithreaded_procbitmaps(void *data);
static void  ocrresult_proc_bitmap(void *api,OCRRESULT *ocrresult);

static int  vowel(int c0);
static int  not_usually_after_T(int c0);
static int  not_usually_after_n(int c0);
static int  not_usually_after_l(int c0);
static int  not_usually_after_r(int c0);


/*
** Bitmap must be 8-bit grayscale
*/
void ocrwords_queue_bitmap(OCRWORDS *words,WILLUSBITMAP *bmp8,int dpi,
                           int c1,int r1,int c2,int r2,int lcheight)

    {
    static char *funcname="ocrword_queue_bitmap";
    OCRWORD *word,_word;

    word=&_word;
    ocrword_init(word);
    ocrwords_add_word(words,word);
    word=&words->word[words->n-1];
    word->c=c1;
    word->r=r1;
    word->lcheight=lcheight;
    word->dpi=dpi;
    word->rot=0;
    ocrword_bitmap8_copy_cropped(word,bmp8,c1,r1,c2,r2);
    }


static void ocrresult_init_from_ocrword(OCRRESULT *ocrresult,OCRWORD *word,int index)

    {
    ocrresult->bmp=ocrword_bitmap_ptr(word);
    ocrresult->dpi=word->dpi;
    ocrresult->c1=word->c;
    ocrresult->r1=word->r;
    ocrresult->lcheight=word->lcheight;
    ocrresult->index=index;
    ocrresult->done=0;
    ocrresult->scale=word->bmpscale;
    pthread_mutex_init(&ocrresult->mutex,NULL);
    ocrwords_init(&ocrresult->ocrwords);
    }


static void ocrresults_make_room(OCRRESULTS *ocrresults)

    {
    static char *funcname="ocrresults_make_room";

    if (ocrresults->n>=ocrresults->na)
        {
        willus_mem_realloc_robust_warn((void **)&ocrresults->ocrresult,
                                       ocrresults->na*2*sizeof(OCRRESULT),
                                       ocrresults->na*sizeof(OCRRESULT),funcname,10);
        ocrresults->na*=2;
        }
    }


/*
** Perform multithreaded OCR on all queued words
*/
double ocrwords_multithreaded_ocr(OCRWORDS *words,void **ocr_api,int nthreads,int type,int target_dpi)

    {
    OCRRESULTS _ocrresults,*ocrresults;
    OCRRESULTS *ocrr;
    pthread_t *thread;
    clock_t start,stop;
    double ocr_cpu_time_secs;
    int i;
    static char *funcname="ocrwords_multithreaded_ocr";

    global_ocr_api=ocr_api;
    global_ocr_type=type; /* 'g' for GOCR or 't' for Tesseract */
    global_ocr_target_dpi=target_dpi;
    willus_mem_alloc_warn((void**)&ocrr,sizeof(OCRRESULTS)*nthreads,funcname,10);
    willus_mem_alloc_warn((void**)&thread,sizeof(pthread_t)*nthreads,funcname,10);
    ocrresults=&_ocrresults;
    ocrresults->n=0;
    ocrresults->na=16;
    willus_mem_alloc_warn((void **)&ocrresults->ocrresult,sizeof(OCRRESULT)*ocrresults->na,funcname,10);

    /* Queue up all conversions to be done into OCRRESULTS structure */
    for (i=0;i<words->n;i++)
        {
        OCRWORD *word;

        word=&words->word[i];
        if (ocrword_bitmap_ptr(word)==NULL)
            continue;
        ocrresults_make_room(ocrresults);
        ocrresult_init_from_ocrword(&ocrresults->ocrresult[ocrresults->n++],word,i);
        }

    /* Perform OCR */
    start=clock();
    for (i=0;i<nthreads;i++)
        {
        ocrr[i]=(*ocrresults);
        ocrr[i].thindex=i;
        pthread_create(&thread[i],NULL,ocrword_multithreaded_procbitmaps,&ocrr[i]);
        }
    for (i=0;i<nthreads;i++)
        pthread_join(thread[i],NULL);
    stop=clock();
    ocr_cpu_time_secs=(double)stop/CLOCKS_PER_SEC - (double)start/CLOCKS_PER_SEC;

    /* Process results */
    for (i=0;i<ocrresults->n;i++)
        {
        OCRRESULT *ocrresult;
        int j;

        ocrresult=&ocrresults->ocrresult[i];
/*
printf("ocrresult %d of %d: c1=%d, r1=%d, n=%d\n",i,ocrresults->n,ocrresult->c1,ocrresult->r1,ocrresult->ocrwords.n);
*/
        if (ocrresult->ocrwords.n==0)
            {
            OCRWORD *word;
            word=&words->word[ocrresult->index];
            ocrword_free(word);
            }
        for (j=0;j<ocrresult->ocrwords.n;j++)
            {
            OCRWORD *word,_word;

            if (j==0)
                {
                word=&words->word[ocrresult->index];
                ocrword_free(word);
                }
            else
                {
                word=&_word;
                ocrword_init(word);
                }
/*
printf("    word[%d] (%4d,%4d) %dx%d = '%s' (n=%d)\n",j,ocrresult->ocrwords.word[j].c,ocrresult->ocrwords.word[j].r,ocrresult->ocrwords.word[j].w,ocrresult->ocrwords.word[j].h,ocrresult->ocrwords.word[j].text,ocrresult->ocrwords.word[j].n);
*/
            ocrword_copy(word,&ocrresult->ocrwords.word[j]);
            if (j>0)
                ocrwords_add_word(words,word);
            }
        }

    /* Remove any null results */
    for (i=0;i<words->n;i++)
        if (words->word[i].text==NULL || strlen(words->word[i].text)==0)
            {
            ocrwords_remove_words(words,i,i);
            i--;
            }
        else
            words->word[i].n=utf8_to_unicode(NULL,words->word[i].text,-1);

    /* Order by position */
    ocrwords_sort_by_position(words);

    /* Clean up */
    for (i=ocrresults->n-1;i>=0;i--)
        ocrwords_free(&ocrresults->ocrresult[i].ocrwords);
    willus_mem_free((double**)&ocrresults->ocrresult,funcname);
    willus_mem_free((double**)&thread,funcname);
    willus_mem_free((double**)&ocrr,funcname);
    return(ocr_cpu_time_secs);
    }


static void *ocrword_multithreaded_procbitmaps(void *data)

    {
    OCRRESULTS *ocrresults;
    int i;

    ocrresults=(OCRRESULTS *)data;
    for (i=0;i<ocrresults->n;i++)
        {
        OCRRESULT *ocrresult;

        ocrresult=&ocrresults->ocrresult[i];
        pthread_mutex_lock(&ocrresult->mutex);
        if (ocrresult->done==1)
            {
            pthread_mutex_unlock(&ocrresult->mutex);
            continue;
            }
        ocrresult->done=1;
        pthread_mutex_unlock(&ocrresult->mutex);
        ocrresult_proc_bitmap(global_ocr_api[ocrresults->thindex],ocrresult);
        }
    pthread_exit(NULL);
    return(NULL);
    }


/*
** In tests I did, the ...no_mutex function is never faster
** than the ..._mutex() function.
*/
#if 0
static void *ocrword_multithreaded_procbitmaps_no_mutex(void *data)

    {
    OCRRESULTS *ocrresults;
    int i;

    ocrresults=(OCRRESULTS *)data;
    for (i=ocrresults->thindex;i<ocrresults->n;i+=maxthreads)
        {
        OCRRESULT *ocrresult;

        ocrresult=&ocrresults->ocrresult[i];
        ocrresult->done=1;
        ocr_proc_bitmap(ocrtess_api[ocrresults->thindex],ocrresult);
        }
    pthread_exit(NULL);
    return(NULL);
    }
#endif



static void ocrresult_proc_bitmap(void *api,OCRRESULT *ocrresult)

    {
    switch (global_ocr_type)
        {
#ifdef HAVE_TESSERACT_LIB
        case 't':
            {
            double downsample;
            OCRWORDS *ocrwords;

            ocrwords=&ocrresult->ocrwords;
            if (ocrresult->lcheight > 0. && global_ocr_target_dpi < 0
                      && ocrresult->lcheight > -global_ocr_target_dpi)
                downsample = (double)-global_ocr_target_dpi / ocrresult->lcheight;
            else if (ocrresult->dpi > 0 && global_ocr_target_dpi > 0
                      && ocrresult->dpi > global_ocr_target_dpi)
                downsample = (double)global_ocr_target_dpi / ocrresult->dpi;
            else
                downsample = 1.;
/*
{
static int count=0;
char filename[256];
sprintf(filename,"ocrbmp%03d.png",count++);
bmp_write(ocrresult->bmp,filename,stdout,100);
wfile_written_info(filename,stdout);
printf("Calling ocrtess_ocrwords, bmp=%dx%d\n",ocrresult->bmp->width,ocrresult->bmp->height);
}
*/
            ocrtess_ocrwords_from_bmp8(api,ocrwords,ocrresult->bmp,
                                       0,0,ocrresult->bmp->width-1,ocrresult->bmp->height-1,
                                       ocrresult->dpi,-1,downsample,NULL);
            ocrwords_scale(ocrwords,ocrresult->scale);
            ocrwords_offset(ocrwords,ocrresult->c1,ocrresult->r1);
/*
printf("    Result:  %d words.\n",ocrresult->ocrwords.n);
{ int i;
for (i=0;i<ocrresult->ocrwords.n;i++)
printf("        word[%d]='%s' (bs=%g)\n",i,ocrresult->ocrwords.word[i].text,ocrresult->ocrwords.word[i].bmpscale);
}
*/
/*
#if (DEBUG)
{
int i;
printf("      OCR TESS:  (%d,%d)-(%d,%d) -- got %d words.\n",
ocrresult->c1,ocrresult->r1,ocrresult->c2,ocrresult->r2,ocrresult->ocrwords.n);
printf("                 ocrresult=%p, ocrresult->ocrwords.word=%p\n",ocrresult,ocrresult->ocrwords.word);
for (i=0;i<ocrresult->ocrwords.n;i++)
{
OCRWORD *word;
word=&ocrresult->ocrwords.word[i];
printf("        %2d. '%s' (%d,%d) w=%d, h=%d\n",i+1,word->text,word->c,word->r,word->w,word->h);
}
}
#endif
*/
            break;
            }
#endif
#ifdef HAVE_GOCR_LIB
        case 'g':
            gocr_ocrwords_from_bmp8(&ocrresult->ocrwords,ocrresult->bmp,0,0,
                                    ocrresult->bmp->width-1,ocrresult->bmp->height-1,0,1);
            ocrwords_offset(&ocrresult->ocrwords,ocrresult->c1,ocrresult->r1);
            break;
#endif
        default:
            /*
            wordbuf[0]='m';
            wordbuf[1]='\0';
            */
            break;
        }

    /*
    willus_mem_alloc_warn((void **)&ocrresult->word,strlen(wordbuf)+1,"ocr_proc_bitmap",10);
    strcpy(ocrresult->word,wordbuf);
    */
    }


void ocr_text_proc(char *s,int allow_spaces)

    {
    static char *word_swaps[] =
        {
        "neld","field",
        "_eld","field",
        "PaPeC","paper",
        "successrul","successful",
        "_or","for",
        "rrequency","frequency",
        "out_Ut","output",
        "_un","gun",
        "worh","work",
        "bene_t","benefit",
        "sign_cantly","significantly",
        "_oal","goal",
        ""
        };
    int i,j;

    if (!allow_spaces)
        {
        for (i=j=0;s[i]!='\0';i++)
            if (s[i]!=' ')
                {
                if (j!=i)
                    s[j]=s[i];
                j++;
                }
        s[j]='\0';
        }
    /* Specific word swaps */
    for (i=0;word_swaps[i][0]!='\0';i+=2)
        if (!strcmp(s,word_swaps[i]))
            {
            strcpy(s,word_swaps[i+1]);
            return;
            }

    /* Heuristic rules */

    /* Starting letter */
    if (s[0]=='T' && not_usually_after_T(s[1]))
        s[0]='I';
    else if (s[0]=='n' && not_usually_after_n(s[1]))
        {
        memmove(&s[2],&s[1],strlen(s)-1);
        s[0]='f';
        s[1]='i';
        }
    else if (s[0]=='l' && not_usually_after_l(s[1]))
        s[0]='i';
    else if (s[0]=='r' && not_usually_after_r(s[1]))
        s[0]='f';
    else if (s[0]=='h' && tolower(s[1])=='l')
        s[0]='k';
    /* I's and O's to 1's and 0's */
    if (strcmp(s,"OI") && strcmp(s,"O") && strcmp(s,"I"))
        {
        for (i=0;s[i]!='\0';i++)
            {
            if (s[i]!='I' && s[i]!='O' && (s[i]<'0' || s[i]>'9'))
                break;
            }
        if (s[i]=='\0')
            {
            for (i=0;s[i]!='\0';i++)
                {
                if (s[i]=='I')
                    s[i]='1';
                if (s[i]=='O')
                    s[i]='0';
                }
            }
        }
    /* Digits to letters */
    for (i=0;s[i]!='\0';i++)
        {
        if (s[i]>='0' && s[i]<='9' 
                      && (i==0 || (s[i-1]>='a' && s[i-1]<='z'))
                      && (s[i+1]=='\0' || (s[i+1]>='a' && s[i+1]<='z')))
            {
            if (s[i]=='1')
                s[i]='l';
            if (s[i]=='4')
                s[i]='u'; /* strange but true */
            }
        }
    /* Caps in the middle of lowercase letters */
    for (i=0;s[i]!='\0';i++)
        {
        if (i>0 && (s[i]>='A' && s[i]<='Z') 
                && (s[i-1]>='a' && s[i-1]<='z')
                && (s[i+1]>='a' && s[i+1]<='z'))
            {
            if (s[i]=='I')
                s[i]='l';
            else
                s[i]=tolower(s[i]);
            }
        }
    /* in_ to ing */
    while ((i=in_string(s,"in_"))>=0)
        s[i+2]='g';
    }


static int vowel(int c0)

    {
    int c;
    c=tolower(c0);
    return(c=='a' || c=='e' || c=='i' || c=='o' || c=='u' || c=='y');
    }


static int not_usually_after_T(int c0)

    {
    int c;
    c=tolower(c0);
    return(!(vowel(c) || c=='h' || c=='r' || c=='s' || c=='w'));
    }


static int not_usually_after_n(int c0)

    {
    int c;
    c=tolower(c0);
    return(!(vowel(c) || c=='g'));
    }


static int not_usually_after_l(int c0)

    {
    int c;
    c=tolower(c0);
    return(!(vowel(c) || c=='h' || c=='l'));
    }


static int not_usually_after_r(int c0)

    {
    int c;
    c=tolower(c0);
    return(c=='r' || c=='l');
    }
