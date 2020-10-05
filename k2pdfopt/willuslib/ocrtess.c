/*
** ocrtess.c   Get OCR of WILLUS bitmap using Tesseract
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2020  http://willus.com
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
#include <locale.h>
#include <leptonica.h>
#include <tesseract.h>
#include "willus.h"

char *ocrtess_langnames[] =
    {
    "af","afr","Afrikaans",
    "am","amh","Amharic",
    "ar","ara","Arabic",
    "as","asm","Assamese",
    "az","aze","Azerbaijani",
    "--","aze_cyrl","Azerbaijani Cyr",
    "be","bel","Belarusian",
    "bn","ben","Bengali",
    "bo","bod","Tibetan",
    "bs","bos","Bosnian",
    "br","bre","Breton",
    "bg","bul","Bulgarian",
    "ca","cat","Catalan",
    "ce","ceb","Cebuano",
    "cs","ces","Czech",
    "zh","chi_sim","Chinese Simplified",
    "--","chi_sim_vert","Chinese Simplified Vertical",
    "--","chi_tra","Chinese Traditional",
    "--","chi_tra_vert","Chinese Traditional Vertical",
    "--","chr","Cherokee",
    "cy","cym","Welsh",
    "da","dan","Danish",
    "--","dan_frak","Danish Fraktur",
    "de","deu","German",
    "--","deu_frak","German Fraktur",
    "dz","dzo","Dzongkha",
    "el","ell","Greek",
    "en","eng","English",
    "--","enm","Middle English",
    "--","epo","Esperanto",
    "--","equ","Math Equation",
    "et","est","Estonian",
    "eu","eus","Basque",
    "fa","fas","Persian",
    "fi","fin","Finnish",
    "fr","fra","French",
    "--","frk","Frankish",
    "--","frm","Middle French",
    "gd","gle","Irish",
    "gl","glg","Galician",
    "gr","grc","Ancient Greek",
    "gu","guj","Gujarati",
    "ha","hat","Haitian Print",
    "he","heb","Hebrew",
    "hi","hin","Hindi",
    "hr","hrv","Croatian",
    "hu","hun","Hungarian",
    "ik","iku","Inuktitut",
    "id","ind","Indonesian",
    "is","isl","Icelandic",
    "it","ita","Italian",
    "--","ita_old","Italian Old",
    "--","jav","Javanese",
    "ja","jpn","Japanese",
    "--","jpn_vert","Japanese Vertical",
    "kn","kan","Kannada",
    "ka","kat","Georgian",
    "--","kat_old","Georgian Old",
    "kk","kaz","Kazakh",
    "km","khm","Khmer",
    "ki","kir","Kyrgyz",
    "--","kmr","Kurdish Kumanji",
    "ko","kor","Korean",
    "--","kor_vert","Korean Vertical",
    "ku","kur","Kurdish",
    "lo","lao","Laotian",
    "la","lat","Latin",
    "lv","lav","Latvian",
    "lt","lit","Lithuanian",
    "lu","ltz","Luxembourgish",
    "ml","mal","Malayalam",
    "--","mar","Maharashtra",
    "mk","mkd","Macedonian",
    "mt","mlt","Maltese",
    "mn","mon","Mongolian",
    "mi","mri","Maori",
    "ms","msa","Malay",
    "my","mya","Burmese",
    "ne","nep","Nepali",
    "nl","nld","Dutch",
    "nb","nor","Norwegian",
    "oc","oci","Occitan post 1500",
    "or","ori","Oriya",
    "os","osd","Orientation Script Detection",
    "pa","pan","Panjabi",
    "pl","pol","Polish",
    "pt","por","Portuguese",
    "pu","pus","Pushto",
    "qu","que","Quechua",
    "ro","ron","Romanian",
    "ru","rus","Russian",
    "sa","san","Sanskrit",
    "si","sin","Singhalese",
    "sk","slk","Slovakian",
    "--","slk_frak","Slovakian Fraktur",
    "sl","slv","Slovenian",
    "--","snd","Albanian",
    "es","spa","Spanish",
    "--","spa_old","Spanish Old",
    "sq","sqi","Albanian",
    "sr","srp","Serbian",
    "--","srp_latn","Serbian Latin",
    "su","sun","Sundanese",
    "sw","swa","Swahili",
    "sv","swe","Swedish",
    "sy","syr","Syriac",
    "ta","tam","Tamil",
    "tt","tat","Tatar",
    "te","tel","Telugu",
    "tg","tgk","Tajik",
    "tl","tgl","Tagalog",
    "th","tha","Thai",
    "ti","tir","Tigrinya",
    "to","ton","Tonga",
    "tr","tur","Turkish",
    "ui","uig","Uighur",
    "uk","ukr","Ukrainian",
    "ur","urd","Urdu",
    "uz","uzb","Uzbek",
    "--","uzb_cyrl","Uzbek Cyr",
    "vi","vie","Vietnamese",
    "yi","yid","Yiddish",
    "yo","yor","Yoruba",
    ""
    };
static char *defurl = "raw.githubusercontent.com/tesseract-ocr/tessdata_%s/master";

static int ocrtess_lstm_file(char *filename);
static int ocrtess_tess_file(char *filename);
static int file_contains_keyword(char *filename,char *keyword);
static int buf_contains_keyword(char *buf,char *keyword,int n);
/*
static int has_cube_data(char *lang);
*/
static void endian_flip(char *x,int n);


char *ocrtess_lang_by_index(char *lang,int index)

    {
    static char buf[32];
    char langdef[32];
    char *langstr;
    int i,j,c;

    if (lang==NULL || lang[0]=='\0')
        {
        ocrtess_lang_default(NULL,NULL,0,langdef,31,NULL,0,0);
        langstr=langdef;
        }
    else
        langstr=lang;
    for (i=j=c=0;langstr[i]!='\0';i++)
        {
        if (langstr[i]!='+')
            {
            if (j<31)
                buf[j++]=langstr[i];
            continue;
            }
        if (j>0)
            {
            if (c==index)
                {
                buf[j]='\0';
                return(buf);
                }
            c++;
            j=0;
            }
        }
    if (j>0 && c==index)
        {
        buf[j]='\0';
        return(buf);
        }
    return(NULL);
    }


int ocrtess_lang_count(char *langstr)

    {
    int i;

    for (i=0;ocrtess_lang_by_index(langstr,i)!=NULL;i++);
    return(i);
    }


void ocrtess_set_logfile(char *filename)

    {
    tess_capi_set_logfile(filename);
    }


void ocrtess_debug_message(char *message)

    {
    tess_capi_debug_message(message);
    }


int ocrtess_lang_exists(char *datadir,char *lang)

    {
    char tesspath0[MAXFILENAMELEN];
    char basename[128];
    char fullname[MAXFILENAMELEN];

    ocrtess_datapath(tesspath0,datadir,MAXFILENAMELEN-1);
    sprintf(basename,"%s.traineddata",lang);
    wfile_fullname(fullname,tesspath0,basename);
    return(wfile_status(fullname)==1);
    }


int ocrtess_lang_get_from_github(char *datadir,char *lang)

    {
    int  status,fast,lstm,tess;
    char url[256];
    char httpurl[512];
    char tesspath0[MAXFILENAMELEN];
    char basename[128];
    char urlbasename[128];
    char fullname[MAXFILENAMELEN];

    fast=in_string(lang,"fast")>=0;
    ocrtess_url(url,255,fast);
    ocrtess_datapath(tesspath0,datadir,MAXFILENAMELEN-1);
    ocrtess_baselang(urlbasename,lang,127);
    wfile_newext(basename,lang,"traineddata");
    wfile_newext(urlbasename,urlbasename,"traineddata");
    wfile_fullname(fullname,tesspath0,basename);
    sprintf(httpurl,"%s/%s",url,urlbasename);
    wfile_prepdir(fullname);
    status=inet_httpget(fullname,httpurl);
    if (status)
        return(status);
    lstm=ocrtess_lstm_file(fullname);
    tess=ocrtess_tess_file(fullname);
    if (!lstm && !tess)
        {
        remove(fullname);
        return(-10);
        }
    return(0);
    }


int ocrtess_isfast(char *lang)

    {
    int len;

    len=strlen(lang);
    if ((len>5 && !stricmp(&lang[len-5],"_fast"))
          || (len>5 && !stricmp(&lang[len-5],"-fast")))
        return(len-5);
    else if (len>4 && !stricmp(&lang[len-4],"fast"))
        return(len-4);
    return(0);
    }


void ocrtess_baselang(char *dst,char *src,int maxlen)

    {
    int len;

    if (dst==NULL)
        dst=src;
    else
        xstrncpy(dst,src,maxlen);
    len=ocrtess_isfast(dst);
    if (len>0)
        dst[len]='\0';
    }
        

void ocrtess_url(char *url0,int maxlen,int fast)

    {
    int len;
    char url[256];
    char httpurl[512];
    char envvar[64];
    char *p;

    sprintf(envvar,"TESSDATA%s_URL",fast?"FAST":"");
    p=getenv(envvar);
    if (p!=NULL)
        xstrncpy(url,p,255);
    else
        sprintf(url,defurl,fast?"fast":"best");
    /* Remove trailing slash if exists */
    len=strlen(url);
    if (len>0 && url[len-1]=='/')
        url[len-1]='\0';
    sprintf(httpurl,!strnicmp(url,"http://",7)||!strnicmp(url,"https://",8)?"%s":"https://%s",
                  url);
    xstrncpy(url0,httpurl,maxlen);
    }
        

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
    char *p;
    FILELIST *fl,_fl;
    char tesspath0[MAXFILENAMELEN];
    static char *langdefdef="eng";
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
    p=setlocale(LC_CTYPE,NULL);
    if (p==NULL || !stricmp(p,"c"))
        xstrncpy(langdef,langdefdef,maxlen-1);
    else
        {
        for (j=0;ocrtess_langnames[j*3][0]!='\0';j++)
            if (!strnicmp(p,ocrtess_langnames[j*3],2))
                {
                xstrncpy(langdef,ocrtess_langnames[j*3+1],maxlen-1);
                break;
                }
        if (ocrtess_langnames[j*3][0]=='\0')
            xstrncpy(langdef,langdefdef,maxlen-1);
        }
    wfile_fullname(wildcard,tesspath0,"*.traineddata");
    fl=&_fl;
    filelist_init(fl);
    filelist_fill_from_disk_1(fl,wildcard,0,0);
    /* Don't use most recent training data anymore */
    /*
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
    */
    if (tessdebug!=NULL)
        {
        filelist_sort_by_name(fl);
        if (strlen(header)+strlen(tessdebug) < maxdebug)
            sprintf(&tessdebug[strlen(tessdebug)],"%s",header);
        for (j=0;j<fl->n;j++)
            {
            char fullname[256];
            char name[256];
            char basename[256];
            int len1,tess,lstm,len;

            wfile_fullname(fullname,fl->dir,fl->entry[j].name);
            strcpy(name,fl->entry[j].name);
            wfile_basespec(basename,fl->entry[j].name);
            wfile_newext(basename,basename,"");
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
                if (!strcmp(basename,langdef))
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
#ifdef HAVE_WIN32_API
        p=getenv("USERPROFILE");
#else
        p=getenv("HOME");
#endif
        if (p==NULL)
            {
            datapath[0]='\0';
            return;
            }
        wfile_fullname(path,p,".tessdata");
        xstrncpy(datapath,path,maxlen-1);
        return;
        }
    xstrncpy(path1,p,MAXFILENAMELEN-13);
    wfile_fullname(path,path1,"tessdata");
    if (wfile_status(path)==2)
        {
        xstrncpy(datapath,path,maxlen-1);
        return;
        }
    xstrncpy(datapath,p,maxlen-1);
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
        word.n=utf8_to_unicode(NULL,word.text,-1);
/*
printf("ocrtess: word[%d] = '%s' (%d,%d) %dx%d lc=%d\n",i,word.text,word.c,word.r,word.w,word.h,(int)word.lcheight);
*/
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

#if 0
int ocrtesslangs_get_language_selection(OCRTESSLANGS *otl)

    {
    char tempfile[256];
    char buf[1024];
    static char *tlurl="https://github.com/tesseract-ocr/tessdata";
    OCRTESSLANG *tl1,_tl1;
    FILE *f;
    int status;
  
    tl1=&_tl1;
    otl->n=0;
    wfile_abstmpnam(tempfile);
    status=inet_httpget(tempfile,tlurl,1,1);
    if (status)
        {
        remove(tempfile);
        return(status);
        }
    f=fopen(tempfile,"r");
    if (f==NULL)
        {
        remove(tempfile);
        return(-50);
        }
    while (fgets(buf,1023,f)!=NULL)
        if (!ocrtesslang_from_html(tl1,buf))
            {
            github_fix_url(tl1->langurl,buf);
            xstrncpy(tl1->langurl,buf,255);
            ocrtesslangs_add_one(otl,tl1);
            }
    fclose(f);
    remove(tempfile);
    return(0);
    }


void ocrtesslangs_init(OCRTESSLANGS *otl)

    {
    otl->otl=NULL;
    otl->n=otl->na=0;
    }


void ocrtesslangs_add_one(OCRTESSLANGS *otl,OCRTESSLANG *tl1)

    {
    static char *funcname="ocrtesslangs_add_one";

    if (otl->n>=otl->na)
        {
        int newsize;
        newsize=otl->na<64 ? 128 : otl->na*2;
        willus_mem_realloc_robust_warn((void **)&otl->otl,newsize*sizeof(OCRTESSLANG),
                                       otl->na*sizeof(OCRTESSLANG),funcname,10);
        otl->na=newsize;
        }
    otl->otl[otl->n++]=(*tl1);
    }


void ocrtesslangs_free(OCRTESSLANGS *otl)

    {
    static char *funcname="ocrtesslangs_free";

    willus_mem_free((double **)&otl->otl,funcname);
    otl->na=otl->n=0;
    }


static void github_fix_url(char *s,char *d)

    {
    int i,j;

    if (!strnicmp(s,"http://",7) || !strnicmp(s,"https://",8))
        strcpy(d,s);
    else
        {
        strcpy(d,"https://github.com");
        if (s[0]!='/')
            strcat(d,"/tesseract-ocr/tessdata/");
        strcat(d,s);
        }
    for (i=j=0;d[i]!='\0';i++)
        {
        if (!strncmp(&d[i],"/blob/",6))
            {
            strcpy(&d[j],"/raw/");
            j+=5;
            i+=5;
            }
        else
            {
            if (i!=j)
                d[j]=d[i];
            j++;
            }
        }
    d[j]='\0';
    }


static int ocrtesslang_from_html(OCRTESSLANG *otl,char *buf)

    {
    int i;
    char shortname[16];

    i=in_string(buf,".traineddata</a></span>");
    if (i<0)
        return(-1);
    for (;i>=0 && buf[i]!='>';i--);
    xstrncpy(shortname,&buf[i+1],15);
    for (i=0;shortname[i]!='\0' && shortname[i]!='.';i++);
    shortname[i]='\0';
    xstrncpy(otl->langname,ocrtess_language_name(shortname),47);
    i=in_string(buf,"href=\"");
    if (i<0)
        return(-1);
    xstrncpy(otl->langurl,&buf[i+6],250);
    for (i=0;otl->langurl[i]!='\0' && otl->langurl[i]!='\"';i++);
    otl->langurl[i]='\0';
    return(0);
    }

#endif

char *ocrtess_language_name(char *lang)

    {
    static char langname[56];
    char shortname[32];
    int i,fast;

    xstrncpy(shortname,lang,31);
    i=in_string(shortname,"_fast");
    fast=0;
    if (i>0)
        {
        fast=1;
        shortname[i]='\0';
        }
    xstrncpy(langname,lang,55);
    for (i=0;ocrtess_langnames[i][0]!='\0';i+=3)
        if (!stricmp(shortname,ocrtess_langnames[i+1]))
            {
            xstrncpy(langname,ocrtess_langnames[i+2],55);
            if (fast)
                xstrncat(langname," (Fast)",55);
            break;
            }
    return(langname);
    }
