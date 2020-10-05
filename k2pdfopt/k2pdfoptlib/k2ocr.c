/*
** k2ocr.c       k2pdfopt OCR functions
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

#ifdef WIN32
#include <windows.h>
#endif
#include "k2pdfopt.h"
#include <locale.h>
#include <pthread.h>

#ifdef HAVE_OCR_LIB
static int k2ocr_gocr_inited=0;
static int maxthreads=0;
static double ocr_cpu_time_secs=0.;
#if (defined(HAVE_TESSERACT_LIB))
static void **ocrtess_api;
static void *otinit(void *data);
static int k2ocr_tess_inited=0;
static int k2ocr_tess_status=0;
typedef struct
    {
    K2PDFOPT_SETTINGS *k2settings;
    int index;
    int ni;
    char initstr[256];
    } OCRTESSINITINFO;
#endif
static void k2ocr_show_envvar(char *buf,char *color,char *var);
static void k2ocr_status_line(char *buf,char *color,char *label,char *string);
static void k2ocr_tesslang_init(char *lang,int assume_yes);
static void k2ocr_ocrwords_add_subregion_to_queue(MASTERINFO *masterinfo,OCRWORDS *words,
                                        BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
#endif /* HAVE_OCR_LIB */

/* Functions to support extracting text from PDF using MuPDF lib */
#ifdef HAVE_MUPDF_LIB
static void k2ocr_ocrwords_get_from_ocrlayer(MASTERINFO *masterinfo,OCRWORDS *words,
                                             BMPREGION *region,K2PDFOPT_SETTINGS *k2settings);
static int ocrword_map_to_bitmap(OCRWORD *word,MASTERINFO *masterinfo,BMPREGION *region,
                                 K2PDFOPT_SETTINGS *k2settings,int *index,int *i2);
static int wrectmap_srcword_inside(WRECTMAP *wrectmap,OCRWORD *word,BMPREGION *region,int *i2);
static int k2ocr_ocrword_verify_boundingbox(OCRWORD *word,BMPREGION *region);
static void wtextchars_group_by_words(WTEXTCHARS *wtcs,OCRWORDS *words,
                                      K2PDFOPT_SETTINGS *k2settings);
static void wtextchars_add_one_row(WTEXTCHARS *wtcs,int i0,int i1,OCRWORDS *words);
static int wtextchars_ligature_pattern(WTEXTCHARS *wtcs,int index);
#endif

static char *k2ocr_logfile=NULL;


void k2ocr_init(K2PDFOPT_SETTINGS *k2settings,char *initstr)

    {
#ifdef HAVE_OCR_LIB
    static char *funcname="k2ocr_init";
    static char ocrinitmessage[256];
    static char logfilename[256];

    initstr[0]='\0';
    if (maxthreads==0)
        {
        if (k2settings->nthreads<0)
            maxthreads=wsys_num_cpus()*abs(k2settings->nthreads)/100;
        else
            maxthreads=k2settings->nthreads;
        if (maxthreads<1)
            maxthreads=1;
        }
    if (!k2settings->dst_ocr)
        return;
#if (!defined(HAVE_TESSERACT_LIB) && defined(HAVE_GOCR_LIB))
    if (k2settings->dst_ocr=='t')
        {
        static char *tessnot="** Tesseract not compiled into this version.  Using GOCR. **";

        strcpy(initstr,tessnot);
        strcat(initstr,"\n");
        k2printf(TTEXT_WARN "\a%s" TTEXT_NORMAL "\n\n",tessnot);
        k2settings->dst_ocr='g';
        }
#endif
#if (defined(HAVE_TESSERACT_LIB) && !defined(HAVE_GOCR_LIB))
    if (k2settings->dst_ocr=='g')
        {
        static char *gocrnot="** GOCR not compiled into this version.  Using Tesseract. **";
         
        strcpy(initstr,gocrnot);
        strcat(initstr,"\n");
        k2printf(TTEXT_WARN "\a%s" TTEXT_NORMAL "\n\n",gocrnot);
        k2settings->dst_ocr='t';
        }
#endif
#ifdef HAVE_TESSERACT_LIB
#ifdef HAVE_GOCR_LIB
    if (k2settings->dst_ocr=='t')
        {
#endif
        /* v2.15 fix--specific variable for Tesseract init status */
        if (!k2ocr_tess_inited)
            {
            int i,j,ni;
            pthread_t *thread;
            OCRTESSINITINFO *otii;
            char *istr;

            /* Check that languages exist--download if they don't */
            k2ocr_tesslang_init(k2settings->dst_ocr_lang,k2settings->assume_yes);

            /* Capture error messages from Tesseract library to a logfile */
            wfile_abstmpnam(logfilename);
            k2ocr_logfile=logfilename;
            ocrtess_set_logfile(k2ocr_logfile);

            /* v2.40 -- multithreaded init */
            willus_mem_alloc_warn((void **)(&ocrtess_api),sizeof(void*)*maxthreads,funcname,10); 
            if (maxthreads>=8)
                ni=4;
            else if (maxthreads>=2)
                ni=2;
            else
                ni=1;
            willus_mem_alloc_warn((void**)&thread,sizeof(pthread_t)*ni,funcname,10);
            willus_mem_alloc_warn((void**)&otii,sizeof(OCRTESSINITINFO)*ni,funcname,10);
            if (maxthreads>1)
                k2printf("Initializing OCR for %d threads ",maxthreads);
            for (i=0;i<ni;i++)
                {
                otii[i].k2settings=k2settings;
                otii[i].ni=ni;
                otii[i].index=i;
                otii[i].initstr[0]='\0';
                pthread_create(&thread[i],NULL,otinit,&otii[i]);
                }
            for (istr=NULL,i=0;i<ni;i++)
                {
                pthread_join(thread[i],NULL);
                if (istr==NULL && otii[i].initstr[0]!='\0')
                    istr=otii[i].initstr;
                }
            for (i=j=0;i<maxthreads;i++)
                {
                if (ocrtess_api[i]==NULL)
                    {
                    if (maxthreads>1)
                        k2printf(TTEXT_WARN "x" TTEXT_NORMAL);
                    continue;
                    }
                if (i!=j)
                    ocrtess_api[j]=ocrtess_api[i];
                j++;
                }
            ocrtess_set_logfile(NULL); /* Close debugging file */
            if (maxthreads>1)
                k2printf("\n");
            if (j>0)
                {
                if (istr!=NULL)
                    xstrncpy(ocrinitmessage,istr,255);
                else
                    strcpy(ocrinitmessage,"Tesseract initialized (no init message returned).");
                k2printf("%s%s%s\n",TTEXT_BOLD,ocrinitmessage,TTEXT_NORMAL);
                if (j<maxthreads)
                    {
                    k2printf(TTEXT_WARN "** Only able to initialize %d instances of Tesseract. **"
                             TTEXT_NORMAL "\n",j);
                    maxthreads=j;
                    }
                k2ocr_tess_status=0;
                }
            else
                {
                sprintf(ocrinitmessage,"Could not initialize any Tesseract threads.\n"
                     "Possibly could not find Tesseract data (env var TESSDATA_PREFIX = %s).\n"
                     "Using GOCR v0.50.\n\n",
                     getenv("TESSDATA_PREFIX")==NULL?"(not assigned)":getenv("TESSDATA_PREFIX"));
                k2ocr_tess_status=-1;
                maxthreads=1;
                k2printf(TTEXT_WARN "%s" TTEXT_NORMAL,ocrinitmessage);
                k2ocr_showlog();
                }
            strcat(initstr,ocrinitmessage);
            willus_mem_free((double **)&otii,funcname);
            willus_mem_free((double **)&thread,funcname);
            k2ocr_tess_inited=1;
            }
        else
            strcat(initstr,ocrinitmessage);
#ifdef HAVE_GOCR_LIB
        }
    else
#endif
#endif
#ifdef HAVE_GOCR_LIB
        {
        if (k2settings->dst_ocr=='g')
            {
            if (!k2ocr_gocr_inited)
                {
                strcpy(ocrinitmessage,"GOCR v0.50 OCR Engine");
                k2printf(TTEXT_BOLD "%s" TTEXT_NORMAL "\n\n",ocrinitmessage);
                k2ocr_gocr_inited=1;
                }
            strcat(initstr,ocrinitmessage);
            maxthreads=1;
            }
        }
#endif
#ifdef HAVE_MUPDF_LIB
    /* Could announce MuPDF virtual OCR here, but I think it will just confuse people. */
    /*
    if (k2settings->dst_ocr=='m')
        k2printf(TTEXT_BOLD "Using MuPDF \"Virtual\" OCR" TTEXT_NORMAL "\n\n");
    */
#endif
#endif /* HAVE_OCR_LIB */
    }


#ifdef HAVE_TESSERACT_LIB
void ocrtess_debug_info(char **buf0,int use_ansi)

    {
    static char *funcname="ocrtess_debug_info";
    char *buf;
    char langdef[16];
    char color[16];
    char datapath[MAXFILENAMELEN];
    char ocrurl[256];

    ocrtess_datapath(datapath,NULL,MAXFILENAMELEN-1);
    willus_mem_alloc_warn((void **)buf0,8192,funcname,10);
    buf=(*buf0);
    buf[0]='\0';
    xstrncpy(color,use_ansi?ANSI_MAGENTA:"",15);
    k2ocr_show_envvar(buf,color,"TESSDATA_PREFIX");
    k2ocr_status_line(buf,color,"Tesseract data folder",datapath);
    k2ocr_show_envvar(buf,color,"TESSDATA_URL");
    k2ocr_show_envvar(buf,color,"TESSDATAFAST_URL");
    ocrtess_url(ocrurl,255,0);
    k2ocr_status_line(buf,color,"Tesseract data URI",ocrurl);
    k2ocr_status_line(buf,color,"Locale",setlocale(LC_CTYPE,NULL));
    strcat(buf,"\nContents of ");
    if (use_ansi)
        strcat(buf,ANSI_MAGENTA);
    strcat(buf,datapath);
    if (use_ansi)
        strcat(buf,ANSI_NORMAL);
    strcat(buf,":\n");
    ocrtess_lang_default(NULL,NULL,0,langdef,16,buf,6000,use_ansi);
    strcat(buf,"* - LSTM = \"Long Short-Term Memory\" training data.\n"
               "    LSTM is the latest, most accurate OCR method used by Tesseract v4.x.\n"
               "    TESS = Tesseract v3.x compatible (can be used by v4.x).\n");
    }


static void k2ocr_show_envvar(char *buf,char *color,char *var)

    {
    char label[256];

    sprintf(label,"%s environment variable",var);
    k2ocr_status_line(buf,color,label,getenv(var));
    }


static void k2ocr_status_line(char *buf,char *color,char *label,char *string)

    {
    strcat(buf,label);
    strcat(buf,":  ");
    if (color!=NULL && color[0]!='\0' && string!=NULL)
        strcat(buf,color);
    strcat(buf,string==NULL ? "(not set)" : string);
    if (color!=NULL && color[0]!='\0' && string!=NULL)
        strcat(buf,ANSI_NORMAL);
    strcat(buf,"\n");
    }


void k2ocr_tesslang_selected(char *lang,int maxlen,K2PDFOPT_SETTINGS *k2settings)

    {
    xstrncpy(lang,ocrtess_lang_by_index(k2settings->dst_ocr_lang,0),maxlen);
    }


static void k2ocr_tesslang_init(char *lang,int assume_yes)

    {
    int i;
    char *longname,*p;
    char datapath[256];

    ocrtess_datapath(datapath,NULL,255);
    for (i=0;(p=ocrtess_lang_by_index(lang,i))!=NULL;i++)
        {
        int yes,status;
        char buf[512];

        if (ocrtess_lang_exists(NULL,p))
            continue;
        longname=ocrtess_language_name(p);
        if (!assume_yes)
            {
#ifdef HAVE_K2GUI
            if (k2gui_active())
                {
                status=k2gui_yesno("OCR training file not found",
                       "Tesseract training file for %s not found.  Download from github?",
                       longname);
                yes = (status==1);
                }
            else
                {
#endif
            k2printf("Tesseract training file for " TTEXT_BOLD "%s" TTEXT_NORMAL " not found.\n"
                     "  Download from github (y[es]/n[o])? " TTEXT_INPUT,longname);
            k2gets(buf,16,"y");
            k2printf(TTEXT_NORMAL);
            clean_line(buf);
            yes=(tolower(buf[0])=='y' || buf[0]=='\0');
#ifdef HAVE_K2GUI
               }
#endif
            }
        else
            yes=1;
        if (!yes)
            continue;
        k2printf("Downloading %s to folder %s...\n",longname,datapath);
        status=ocrtess_lang_get_from_github(NULL,p);
#ifdef HAVE_K2GUI
        if (k2gui_active() && !assume_yes)
            {
            if (status)
                {
                sprintf(buf,"Download of training file for language %s to folder %s failed.  "
                            "Error %d.",longname,datapath,status);
                k2gui_redbox(0,"Github download",buf);
                }
            else
                {
                sprintf(buf,"Download of training file for language %s to folder %s successful.",
                             longname,datapath);
                k2gui_messagebox(0,"Github download",buf);
                }
            }
#endif
        if (status)
            k2printf(TTEXT_WARN " ... download failed.  Error %d." TTEXT_NORMAL "\n",status);
        else
            k2printf(TTEXT_NORMAL " ... download successful." TTEXT_NORMAL "\n");
        }
    }
            

    
static void *otinit(void *data)

    {
    OCRTESSINITINFO *otii;
    char *lang;
    char initstr[256];
    int ntries,i,status;

    otii=(OCRTESSINITINFO*)data;
    lang=otii->k2settings->dst_ocr_lang;
    for (i=otii->index;i<maxthreads;i+=otii->ni)
        ocrtess_api[i]=NULL;
    for (ntries=0;ntries<5;ntries++)
        {
        int done=1;

        for (i=otii->index;i<maxthreads;i+=otii->ni)
            {
            if (ocrtess_api[i]!=NULL)
                continue;
            ocrtess_api[i]=ocrtess_init(NULL,NULL,0,
                                        lang[0]=='\0'?NULL:lang,NULL,initstr,255,&status);
            if (ocrtess_api[i]==NULL)
                done=0;
            else
                {
                if (otii->initstr[0]=='\0')
                    strcpy(otii->initstr,initstr);
                if (maxthreads>1)
                    k2printf(".");
                }
//            if (ntries>3)
//                wsys_sleep_ms(250);
            }
        if (done)
            break;
//        if (ntries>2)
//            wsys_sleep_ms(250);
        }
    pthread_exit(NULL);
    return(NULL);
    }
#endif


/*
** Dump Tesseract library debugging log using k2printf()
** Only print unique line once (due to multi-threading)
*/
void k2ocr_showlog(void)

    {
    static char *funcname="k2ocr_showlog";

    if (k2ocr_tess_status<0 && k2ocr_logfile!=NULL && wfile_status(k2ocr_logfile)==1
                            && wfile_size(k2ocr_logfile)>0.)
        {
        FILE *f;
        char *bigbuf;
        char *lines[128];
        int i,n,nmax;
        char buf[256];
        static char *divider="---------------------------\n";
        static char *header=TTEXT_BOLD "%s%-5s Tesseract library log\n%s" TTEXT_NORMAL;

        nmax=128;
        n=0;
        bigbuf=NULL;
        willus_mem_alloc_warn((void **)&bigbuf,256*nmax,funcname,10);
        for (i=0;i<nmax;i++)
            lines[i]=&bigbuf[256*i];
        k2printf(header,"","Start",divider);
        k2printf("Starting multi-threaded init.\n");
        f=fopen(k2ocr_logfile,"r");
        if (f!=NULL)
            {
            while (fgets(buf,255,f)!=NULL)
                {
                for (i=0;i<n;i++)
                    if (!stricmp(buf,lines[i]))
                        break;
                if (i<n)
                    continue;
                k2printf("%s",buf);
                if (i<nmax)
                    {
                    strcpy(lines[i],buf);
                    n++;
                    }
                }
            fclose(f);
            }
        k2printf(header,divider,"End","\n");
        willus_mem_free((double **)&bigbuf,funcname);
        }
    }
            

/*
** v2.15--call close/free functions even if k2settings->dst_ocr not set--may have
**        been called from previous GUI conversion.
*/
void k2ocr_end(K2PDFOPT_SETTINGS *k2settings)

    {
    if (k2ocr_logfile!=NULL)
        remove(k2ocr_logfile);
#ifdef HAVE_OCR_LIB
#ifdef HAVE_TESSERACT_LIB
    static char *funcname="k2ocr_end";
    if (k2ocr_tess_inited)
        {
        int i;

        for (i=maxthreads-1;i>=0;i--)
            {
            ocrtess_end(ocrtess_api[i]);
            ocrtess_api[i]=NULL;
            }
        willus_mem_free((double **)&ocrtess_api,funcname);
        k2ocr_tess_inited=0;
        }
#endif
#endif /* HAVE_OCR_LIB */
    }


#ifdef HAVE_OCR_LIB
/*
** Find words in src bitmap and put into words structure.
** If ocrcolumns > maxcols, looks for multiple columns
**
** wrectmaps has information that maps the bitmap pixel space to the original
** source page space so that the location of the words can be used to extract
** the text from a native PDF document if possible (dst_ocr=='m').
**
*/
void k2ocr_ocrwords_add_to_queue(MASTERINFO *masterinfo,OCRWORDS *words,BMPREGION *region,
                                 K2PDFOPT_SETTINGS *k2settings)

    {
    PAGEREGIONS *pageregions,_pageregions;
    int i,maxlevels;

#if (WILLUSDEBUGX & 32)
{
static int ifr=0;
char filename[256];
sprintf(filename,"ocrwords_fill_in_%04d.png",++ifr);
bmpregion_write(region,filename);
printf("k2ocr_ocrwords_fill_in_ex #%d\n",ifr);
}
#endif

    if (k2settings->ocr_max_columns<0 || k2settings->ocr_max_columns <= k2settings->max_columns
           || (k2settings->ocr_detection_type=='p' && k2settings->dst_ocr=='t'))
        {
#if (WILLUSDEBUGX & 32)
printf("Call #1. k2ocr_ocrwords_fill_in\n");
#endif
        k2ocr_ocrwords_add_subregion_to_queue(masterinfo,words,region,k2settings);
        return;
        }
    /* Parse region into columns for OCR */
    pageregions=&_pageregions;
    pageregions_init(pageregions);
    if (k2settings->ocr_max_columns==2 || k2settings->max_columns>1)
        maxlevels = 2;
    else
        maxlevels = 3;
    pageregions_find_columns(pageregions,region,k2settings,masterinfo,maxlevels);
    for (i=0;i<pageregions->n;i++)
        {
        /*
        ** If using built-in source-file OCR layer, don't need to scan bitmap
        ** for text rows.
        */
#if (WILLUSDEBUGX & 32)
printf("\nwrectmaps->n=%d, dst_ocr='%c'\n",region->wrectmaps->n,k2settings->dst_ocr);
#endif
        if (k2settings->dst_ocr!='m' || region->wrectmaps->n!=1)
            bmpregion_find_textrows(&pageregions->pageregion[i].bmpregion,k2settings,0,1,-1.0,0);
        pageregions->pageregion[i].bmpregion.wrectmaps = region->wrectmaps;
#if (WILLUSDEBUGX & 32)
printf("Call #2. k2ocr_ocrwords_fill_in, pr = %d of %d\n",i+1,pageregions->n);
#endif
        k2ocr_ocrwords_add_subregion_to_queue(masterinfo,words,&pageregions->pageregion[i].bmpregion,k2settings);
        }
    pageregions_free(pageregions);
    }


/*
** Queue up words for OCR-ing.  Store bitmaps to be OCR'd in queue.
*/
static void k2ocr_ocrwords_add_subregion_to_queue(MASTERINFO *masterinfo,OCRWORDS *words,
                                             BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    int i;
/*
k2printf("@ocrwords_fill_in (%d x %d)...tr=%d\n",region->bmp->width,region->bmp->height,region->textrows.n);
if (region->textrows.n==0)
{
bmpregion_write(region,"out.png");
exit(10);
}
*/
#if (WILLUSDEBUGX & 32)
k2printf("@ocrwords_fill_in... type='%c'\n",type);
#endif
/*
{
char filename[MAXFILENAMELEN];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/

    if (region->bmp->width<=0 || region->bmp->height<=0)
        return;

#if (WILLUSDEBUGX & 32)
k2printf("OCR REGION:  @%3d,%3d = %3d x %3d\n",
region->c1, region->r1, region->c2-region->c1+1, region->r2-region->r1+1);
k2printf("  %d row%s of text, dst_ocr='%c'\n",region->textrows.n,region->textrows.n==1?"":"s",k2settings->dst_ocr);
#endif
#if (defined(HAVE_MUPDF_LIB) || defined(HAVE_DJVU_LIB))
    if (k2settings->dst_ocr=='m')
        {
        k2ocr_ocrwords_get_from_ocrlayer(masterinfo,words,region,k2settings);
        return;
        }
#endif

    /* Queue bitmaps to be processed for OCR */
    if (k2settings->dst_ocr=='t' && 
          (k2settings->ocr_detection_type=='p' || k2settings->ocr_detection_type=='c'))
        /* Queue entire page at once */
        ocrwords_queue_bitmap(words,region->bmp8,region->dpi,
                                region->c1,region->r1,region->c2,region->r2,-1);
    else /* Use k2pdfopt engine to parse row by row */
        {
        /*
        ** Go text row by text row--store bitmap for each word into ocrresults
        ** structure which will then be processed with multiple parallel threads.
        ** (v2.40)
        */
        for (i=0;i<region->textrows.n;i++)
            {
            BMPREGION _newregion,*newregion;
            int j,r1,r2;
            double lcheight;
            TEXTWORDS *textwords;

            newregion=&_newregion;
            bmpregion_init(newregion);
            bmpregion_copy(newregion,region,0);
            r1=newregion->r1=region->textrows.textrow[i].r1;
            r2=newregion->r2=region->textrows.textrow[i].r2;
#if (WILLUSDEBUGX & 32)
printf("  Row %2d:  @%3d,%3d = %3d x %3d\n",i+1,
region->textrows.textrow[i].c1,
region->textrows.textrow[i].r1,
region->textrows.textrow[i].c2-region->textrows.textrow[i].c1+1,
region->textrows.textrow[i].r2-region->textrows.textrow[i].r1+1);
#endif
            newregion->bbox.type = REGION_TYPE_TEXTLINE;
            lcheight=region->textrows.textrow[i].lcheight;
            /* Sanity check on lcheight */
            if (lcheight/(r2-r1) < .33)
                lcheight = 0.33*(r2-r1);
            if (k2settings->dst_ocr=='t' && k2settings->ocr_detection_type=='l')
                {
                /* New in v2.50:  Use Tesseract to parse entire line of text */
                /* Don't OCR if line height exceeds spec */
                if ((double)(region->textrows.textrow[i].r2-region->textrows.textrow[i].r1+1)
                              / region->dpi > k2settings->ocr_max_height_inches)
                    continue;
                ocrwords_queue_bitmap(words,region->bmp8,region->dpi,
                                        region->textrows.textrow[i].c1,
                                        region->textrows.textrow[i].r1,
                                        region->textrows.textrow[i].c2,
                                        region->textrows.textrow[i].r2,(int)(lcheight+.5));
                continue;
                }

            /* Use k2pdfopt engine to break text row into words (also establishes lcheight) */
            bmpregion_one_row_find_textwords(newregion,k2settings,0);
            textwords = &newregion->textrows;
            /* Add each word */
            for (j=0;j<textwords->n;j++)
                {
                /* Don't OCR if "word" height exceeds spec */
                if ((double)(textwords->textrow[j].r2-textwords->textrow[j].r1+1)/region->dpi
                         > k2settings->ocr_max_height_inches)
                    continue;
                ocrwords_queue_bitmap(words,region->bmp8,region->dpi,
                                        textwords->textrow[j].c1,
                                        textwords->textrow[j].r1,
                                        textwords->textrow[j].c2,
                                        textwords->textrow[j].r2,(int)(lcheight+.5));
                }
            bmpregion_free(newregion);
            } /* text row loop */
        } /* If detection type == 'p' */

#if (WILLUSDEBUGX & 32)
{
char filename[256];
int i,j;
WILLUSBITMAP *bmp,_bmp;
static int iregion=0;

bmp=&_bmp;
bmp_init(bmp);
for (j=0;j<ocrresults->n;j++)
{
OCRRESULT *ocrresult;

ocrresult=&ocrresults->ocrresult[j];
bmp->width=ocrresult->c2-ocrresult->c1+1;
bmp->height=ocrresult->r2-ocrresult->r1+1;
bmp->bpp=8;
bmp_alloc(bmp);
for (i=0;i<256;i++)
bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
for (i=0;i<bmp->height;i++)
{
unsigned char *s,*d;
s=bmp_rowptr_from_top(ocrresult->bmp,ocrresult->r1+i)+ocrresult->c1;
d=bmp_rowptr_from_top(bmp,i);
memcpy(d,s,bmp->width);
}
sprintf(filename,"ocr_region_%04d.png",iregion+1);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
iregion++;
}
}
#endif
#if (WILLUSDEBUGX & 32)
printf("Done k2ocr_ocrwords_add_subregion_to_queue()\n");
#endif
    }


void k2ocr_multithreaded_ocr(OCRWORDS *words,K2PDFOPT_SETTINGS *k2settings)

    {
    ocr_cpu_time_secs += ocrwords_multithreaded_ocr(words,ocrtess_api,maxthreads,
                                                    k2settings->dst_ocr,
                                                    k2settings->ocr_dpi);
    }


double k2ocr_cpu_time_secs(void)

    {
    return(ocr_cpu_time_secs);
    }


void k2ocr_cpu_time_reset(void)

    {
    ocr_cpu_time_secs=0.;
    }


int k2ocr_max_threads(void)

    {
    return(maxthreads);
    }
#endif /* HAVE_OCR_LIB */


/*
** In a contiguous rectangular region that is mapped to the PDF source file,
** find rows of text assuming a single column of text.
*/
#if (defined(HAVE_MUPDF_LIB) || defined(HAVE_DJVU_LIB))
static void k2ocr_ocrwords_get_from_ocrlayer(MASTERINFO *masterinfo,OCRWORDS *dwords,
                                             BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    static OCRWORDS *words=NULL;
    static OCRWORDS _words;
    static int pageno=-1;
    static char pdffile[512];
    int i;

#if (WILLUSDEBUGX & 0x10000)
printf("@k2ocr_ocrwords_get_from_ocrlayer.\n");
#endif
    if (words==NULL)
        {
        words=&_words;
        ocrwords_init(words);
        pdffile[0]='\0';
        }
    if (pageno!=masterinfo->pageinfo.srcpage || strcmp(pdffile,masterinfo->srcfilename))
        {
        static WTEXTCHARS *wtcs=NULL;
        static WTEXTCHARS _wtcs;

        if (wtcs==NULL)
            {
            wtcs=&_wtcs;
            wtextchars_init(wtcs);
            }
        ocrwords_free(words);
        wtextchars_clear(wtcs);
        k2ocr_wtextchars_fill_from_page(wtcs,masterinfo->srcfilename,masterinfo->pageinfo.srcpage,"",0);
        /* v2.52 bug fix--scale text chars w/document scale factor */
        if (masterinfo->document_scale_factor!=1)
            wtextchars_scale_page(wtcs,masterinfo->document_scale_factor);
#if (WILLUSDEBUGX & 0x10000)
{
FILE *f;
int i;
f=fopen("chars.ep","w");
fprintf(f,"/sc on\n/sd off\n/sm off\n");
for (i=0;i<wtcs->n;i++)
{
WTEXTCHAR *wtc;
wtc=&wtcs->wtextchar[i];
printf("Char[%2d]:  ucs='%c'(%d), xp=%7.3f, yp=%7.3f, x1=%7.3f, x2=%7.3f, y1=%7.3f, y2=%7.3f\n",i,
wtc->ucs,wtc->ucs,wtc->xp,wtc->yp,wtc->x1,wtc->x2,wtc->y1,wtc->y2);
fprintf(f,"/sa m 2 2\n%g %g\n%g %g\n%g %g\n%g %g\n%g %g\n//nc\n",
wtc->x1,792.-wtc->y1,wtc->x2,792.-wtc->y1,wtc->x2,792.-wtc->y2,wtc->x1,792.-wtc->y2,wtc->x1,792.-wtc->y1);
}
fclose(f);
}
#endif
        wtextchars_rotate_clockwise(wtcs,360-(int)masterinfo->pageinfo.srcpage_rot_deg);
        wtextchars_group_by_words(wtcs,words,k2settings);
#if (WILLUSDEBUGX2==4)
ocrwords_to_easyplot(words,"words3.ep",0,NULL);
printf("Wrote %d words to words3.ep.\n",words->n);
wfile_written_info("words3.ep",stdout);
#endif
#if (WILLUSDEBUGX & 0x10000)
{
int i;
FILE *f;
for (i=0;i<words->n;i++)
{
printf("Word[%4d]: (%6.2f,%6.2f) %6.2f x %6.2f '%s'=",i,words->word[i].x0,words->word[i].y0,words->word[i].w0,words->word[i].h0,words->word[i].text);
utf8_vals_to_stream(words->word[i].text,stdout);
printf("\n");
}
for (i=0;i<words->n;i++)
printf("Word[%4d]: (%d,%d) %d x %d mh=%g '%s'\n",i,words->word[i].c,words->word[i].r,words->word[i].w,words->word[i].h,words->word[i].maxheight,words->word[i].text);
f=fopen("chars.ep","a");
for (i=0;i<words->n;i++)
{
OCRWORD *word;
word=&words->word[i];
fprintf(f,"/sa m 1 2\n%g %g\n%g %g\n%g %g\n%g %g\n%g %g\n//nc\n",
word->x0,792.-word->y0,word->x0+word->w0,792.-word->y0,word->x0+word->w0,792.-(word->y0+word->h0),word->x0,792.-(word->y0+word->h0),word->x0,792.-word->y0);
}
fclose(f);
}
#endif
        pageno=masterinfo->pageinfo.srcpage;
        strncpy(pdffile,masterinfo->srcfilename,511);
        pdffile[511]='\0';
        }
#if (WILLUSDEBUGX & 0x10000)
{
FILE *f;
int y0;
static int count=0;
char filename[256];
WRECTMAPS *wrectmaps;
sprintf(filename,"bmp%03d.png",count+1);
if (count==0)
bmp_write(region->bmp8,filename,stdout,100);
printf("    region = (%d,%d) - (%d,%d) pixels\n",region->c1,region->r1,region->c2,region->r2);
wrectmaps=region->wrectmaps;
printf("\n");
if (wrectmaps!=NULL)
{
printf("    Got %d word regions.\n",wrectmaps->n);
printf("    region = (%d,%d) - (%d,%d) pixels\n",region->c1,region->r1,region->c2,region->r2);
for (i=0;i<wrectmaps->n;i++)
{
printf("    wrectmap[%d]= srcorg=(%g,%g); dstorg=(%g,%g) , %g x %g\n",i,
wrectmaps->wrectmap[i].coords[0].x,
wrectmaps->wrectmap[i].coords[0].y,
wrectmaps->wrectmap[i].coords[1].x,
wrectmaps->wrectmap[i].coords[1].y,
wrectmaps->wrectmap[i].coords[2].x,
wrectmaps->wrectmap[i].coords[2].y);
printf("        srcpageno=%d\n",wrectmaps->wrectmap[i].srcpageno);
printf("        srcwidth=%d\n",wrectmaps->wrectmap[i].srcwidth);
printf("        srcheight=%d\n",wrectmaps->wrectmap[i].srcheight);
printf("        srcdpiw=%g\n",wrectmaps->wrectmap[i].srcdpiw);
printf("        srcdpih=%g\n",wrectmaps->wrectmap[i].srcdpih);
printf("        srcrot=%d\n",wrectmaps->wrectmap[i].srcrot);
}
}
y0=region->r2;
f=fopen("words.ep",count==0?"w":"a");
count++;
fprintf(f,"; srcdpi=%d, dstdpi=%d\n",k2settings->src_dpi,k2settings->dst_dpi);
fprintf(f,"/sm off\n/sc on\n/sd off\n");
fprintf(f,"/sa l \"Region (pixels)\" 2\n");
fprintf(f,"%d %d\n",region->c1,y0-region->r1);
fprintf(f,"%d %d\n",region->c2,y0-region->r1);
fprintf(f,"%d %d\n",region->c2,y0-region->r2);
fprintf(f,"%d %d\n",region->c1,y0-region->r2);
fprintf(f,"%d %d\n",region->c1,y0-region->r1);
fprintf(f,"//nc\n");
#endif

    /*
    ** Map word PDF positions (in source file) to destination bitmap pixels and
    ** keep only the words that are within the destination region.
    **
    ** I'm not sure this will work entirely correctly with right-to-left text
    */
#if (WILLUSDEBUGX & 0x10000)
printf("words->n=%d\n",words->n);
#endif
    for (i=0;i<words->n;i++)
        {
        int index,i2;

#if (WILLUSDEBUGX & 0x10000)
printf("**Word[%4d] = '%s' = ",i,words->word[i].text);
utf8_vals_to_stream(words->word[i].text,stdout);
printf("!\n");
#endif
        for (i2=-1,index=0;1;index++)
            {
            OCRWORD _word,*word;
            int n,dn;

            /*
            ** Make copy since the function call below can modify the value inside "word"
            */
            word=&_word;
            ocrword_copy(word,&words->word[i]); /* Allocates new memory */
#if (WILLUSDEBUGX & 0x10000)
printf("   (word = '%s' = ",word->text);
utf8_vals_to_stream(word->text,stdout);
printf(")\n");
#endif
            if (i2>=0)
                {
                ocrword_truncate(word,i2+1,word->n-1);
                dn = words->word[i].n-word->n;
#if (WILLUSDEBUGX & 0x10000)
printf("truncated word (index %d, len %d) = '%s' = ",i2+1,word->n,word->text);
utf8_vals_to_stream(word->text,stdout);
printf("!\n");
#endif
                }
            else
                dn=0;
            n=word->n;
            if (!ocrword_map_to_bitmap(word,masterinfo,region,k2settings,&index,&i2))
                {
                ocrword_free(word);
                break;
                }
            if (k2settings->ocrvbb)
                k2ocr_ocrword_verify_boundingbox(word,region);
#if (WILLUSDEBUGX & 0x10000)
printf("i2 = %d (dn=%d, n=%d); word='%s' (len=%d)\n",i2,dn,n,word->text,word->n);
printf("        (word = ");
utf8_vals_to_stream(word->text,stdout);
printf(")\n");
fprintf(f,"/sa m 2 2\n");
fprintf(f,"/sa l \"'%s'\" 2\n",word->text);
fprintf(f,"%d %d\n",word->c,y0-word->r);
fprintf(f,"%d %d\n",word->c+word->w,y0-word->r);
fprintf(f,"%d %d\n",word->c+word->w,y0-(word->r-word->h));
fprintf(f,"%d %d\n",word->c,y0-(word->r-word->h));
fprintf(f,"%d %d\n",word->c,y0-word->r);
fprintf(f,"//nc\n");
#endif
            ocrwords_add_word(dwords,word);
            ocrword_free(word);
#if (WILLUSDEBUGX & 0x10000)
printf("i2=%d, n=%d\n",i2,n);
#endif
            if (i2>=n-1)
                break;
#if (WILLUSDEBUGX & 0x10000)
printf("continuing\n");
#endif
            i2+=dn;
            }
        }
    /* These are static--keep them around for the next call in case it's on the same page */
    /*
    ocrwords_free(words);
    wtextchars_free(wtcs);
    */
#if (WILLUSDEBUGX & 0x10000)
fclose(f);
wfile_written_info("words.ep",stdout);
/*
if (count==5)
exit(10);
*/
}
printf("ALL DONE.\n");
#endif
#if (WILLUSDEBUGX2==4)
{
static int count=0;
static int ymin1=0;
ocrwords_to_easyplot(dwords,"words2.ep",count!=0,&ymin1);
count++;
}
#endif
    }


/*
** Determine pixel locations of OCR word on source bitmap region.
** Return 1 if any part of word is within region, 0 if not.
**
** If returns 1, then:
**
**     wrectmaps->wrectmap[(*index)] is the region containing the word
**
**     (*i2) gets the index of the "last" letter of the word which is still within
**        the wrectmaps->wrectmap[(*index)].
**
** WARNING:  Contents of "word" may be modified.
**
*/
static int ocrword_map_to_bitmap(OCRWORD *word,MASTERINFO *masterinfo,BMPREGION *region,
                                 K2PDFOPT_SETTINGS *k2settings,int *index,int *i2)

    {
    int i;
    WRECTMAPS *wrectmaps;

    wrectmaps=region->wrectmaps;
/*
#if (WILLUSDEBUGX & 0x10000)
printf("wrectmaps->n = %d\n",wrectmaps->n);
#endif
*/
#if (WILLUSDEBUGX2==4)
printf("ocrword_map_to_bitmap(%d, n=%d)\n",(*index),wrectmaps->n);
#endif
    for (i=(*index);i<wrectmaps->n;i++)
{
#if (WILLUSDEBUGX2==4)
printf("    Trying wrectmap[%d].\n",i);
#endif
        if (wrectmap_srcword_inside(&wrectmaps->wrectmap[i],word,region,i2))
            {
            word->rot0_deg=masterinfo->pageinfo.srcpage_rot_deg;
            word->pageno=masterinfo->pageinfo.srcpage;
            (*index)=i;
            return(1);
            }
}
    return(0);
    }


#if (WILLUSDEBUGX & 0x10000)
static void wrectmaps_echo(WRECTMAPS *wrectmaps)

    {
    int i;
    for (i=0;i<wrectmaps->n;i++)
        {
        printf("wrectmap %d of %d\n",i+1,wrectmaps->n);
        printf("    source bm top left = (%g,%g)\n",wrectmaps->wrectmap[i].coords[0].x,
                                                    wrectmaps->wrectmap[i].coords[0].y);
        printf("    wrapbmp top left = (%g,%g)\n",wrectmaps->wrectmap[i].coords[1].x,
                                                  wrectmaps->wrectmap[i].coords[1].y);
        printf("    w/h of region = (%g,%g)\n",wrectmaps->wrectmap[i].coords[2].x,
                                               wrectmaps->wrectmap[i].coords[2].y);
        }
    }
#endif


/*
** WARNING:  Contents of "word" may be modified.
**
** If 1 is returned, (*index2) gets the index to the "last" letter of the word that 
** is still within the wrectmap.
*/
static int wrectmap_srcword_inside(WRECTMAP *wrectmap,OCRWORD *word,BMPREGION *region,int *index2)

    {
    double cx_word,cy_word;
    double x0_wrect,x1_wrect,y0_wrect,y1_wrect;
    double cx_wrect,cy_wrect;

#if (WILLUSDEBUGX2==4)
/*
printf("@wrectmap_srcword_inside('%s',pos=%g,%g, c1=%g,%g, wxh=%gx%g, bmp=%dx%d)\n",
word->text,wrectmap->coords[0].x,wrectmap->coords[0].y,
wrectmap->coords[1].x,wrectmap->coords[1].y,
wrectmap->coords[2].x,wrectmap->coords[2].y,
region->bmp8->width,region->bmp8->height);
printf("    region->c1,r1-c2,r2 = %d,%d-%d,%d\n",region->c1,region->r1,region->c2,region->r2);
*/
#endif
    (*index2)=-1;
    /* compute word and wrectmap in src coords (pts) */
    /*
    ** (cx_word, cy_word) = position of center of word on source page, from top left,
    **                      in points.
    */
    cx_word=word->x0+word->w0/2.;
    cy_word=word->y0+word->h0/2.;
    /*
    ** x0_wrect, y0_wrect = position of upper-left region box on source page, in points.
    ** y1_wrect, y1_wrect = position of lower-right region box on source page, in points.
    */
    /* Original formula from before 7-13-2014--seems off.
    x0_wrect=(wrectmap->coords[0].x+(region->c1-wrectmap->coords[1].x))*72./wrectmap->srcdpiw;
    x1_wrect=(wrectmap->coords[0].x+(region->c2+1-wrectmap->coords[1].x))*72./wrectmap->srcdpiw;
    y0_wrect=(wrectmap->coords[0].y+(region->r1-wrectmap->coords[1].y))*72./wrectmap->srcdpih;
    y1_wrect=(wrectmap->coords[0].y+(region->r2+1-wrectmap->coords[1].y))*72./wrectmap->srcdpih;
    */
    x0_wrect=wrectmap->coords[0].x*72./wrectmap->srcdpiw;
    x1_wrect=x0_wrect + wrectmap->coords[2].x*72./wrectmap->srcdpiw;
    y0_wrect=wrectmap->coords[0].y*72./wrectmap->srcdpih;
    y1_wrect=y0_wrect + wrectmap->coords[2].y*72./wrectmap->srcdpih;
    cx_wrect=(x0_wrect+x1_wrect)/2.;
    cy_wrect=(y0_wrect+y1_wrect)/2.;
#if (WILLUSDEBUGX2==4)
{
printf("@scrword_inside('%s',pos=%g,%g, c1=%g,%g, wxh=%gx%g, bmp=%dx%d)\n",
word->text,wrectmap->coords[0].x,wrectmap->coords[0].y,
wrectmap->coords[1].x,wrectmap->coords[1].y,
wrectmap->coords[2].x,wrectmap->coords[2].y,
region->bmp8->width,region->bmp8->height);
printf("    region->c1,r1-c2,r2 = %d,%d-%d,%d\n",region->c1,region->r1,region->c2,region->r2);
printf("    wrect=(%g,%g)-(%g,%g), word=(%g,%g)-(%g,%g)\n",
   x0_wrect,y0_wrect,x1_wrect,y1_wrect,word->x0,word->y0,word->x0+word->w0,word->y0+word->h0);
}
#endif

    /* If word box is completely outside, skip */
    if (word->x0 >= x1_wrect || (word->x0+word->w0) <= x0_wrect 
                             || word->y0 >= y1_wrect || (word->y0+word->h0) <= y0_wrect)
{
#if (WILLUSDEBUGX2==4)
printf("    No overlap.\n");
#endif
        return(0);
}
#if (WILLUSDEBUGX2==4)
{
static int count=1;
printf("Partially overlapping.\n");
if (wrectmap->coords[2].x>1 && wrectmap->coords[2].y>1)
wrectmap_write_bmp(wrectmap,count,region);
printf("wrectmap index: %d\n",count);
count++;
}
#endif
    /* v2.53:  Check anything partially overlapping in x */
    /*
    if (((cx_word >= x0_wrect && word->x0 <= x1_wrect) || (cx_wrect >= word->x0 && cx_wrect <= word->x0+word->w0))
            && ((cy_word >= y0_wrect && cy_word <= y1_wrect) || (cy_wrect >= word->y0 && cy_wrect <= word->y0+word->h0)))
    */
    if ((cy_word >= y0_wrect && cy_word <= y1_wrect) 
            || (cy_wrect >= word->y0 && cy_wrect <= word->y0+word->h0))
        {
        int i,i1,i2;

#if (WILLUSDEBUGX2==4)
printf("Checking char by char...\n");
#endif
        /* Find out which letters are in/out */
        /* From left... */
#if (WILLUSDEBUGX & 0x10000)
printf("Word:  '%s'\n",word->text);
printf("    ");
utf8_vals_to_stream(word->text,stdout);
printf("\n");
#endif
        for (i=0;i<word->n;i++)
            {
            double x0,x1,x2,dx;

            x0=(i==0)?0.:word->cpos[i-1];
            dx=word->cpos[i]-x0;
            x1=word->x0+x0+dx*0.2;
            x2=word->x0+x0+dx*0.8;
#if (WILLUSDEBUGX & 0x10000)
printf("    i1=%d, x1=%g, x2=%g, x0_rect=%g, x1_rect=%g\n",i,x1,x2,x0_wrect,x1_wrect);
#endif
            /*
            ** v2.53 -- add third conditional to check if single character completely
            **          spans wrectmap
            */
            if ((x1>=x0_wrect && x1<=x1_wrect) || (x2>=x0_wrect && x2<=x1_wrect)
                     || (x1<=x0_wrect && x2>=x1_wrect))
                break;
            }
        i1=i;
        if (i1>=word->n)
            return(0);
        /* From right... */
        for (i=word->n-1;i>i1;i--)
            {
            double x0,x1,x2,dx;

            x0=(i==0)?0.:word->cpos[i-1];
            dx=word->cpos[i]-x0;
            x1=word->x0+x0+dx*0.2;
            x2=word->x0+x0+dx*0.8;
#if (WILLUSDEBUGX & 0x10000)
printf("    i2=%d, x1=%g, x2=%g, x0_rect=%g, x1_rect=%g\n",i,x1,x2,x0_wrect,x1_wrect);
#endif
            /*
            ** v2.53 -- add third conditional to check if single character completely
            **          spans wrectmap
            */
            if ((x1>=x0_wrect && x1<=x1_wrect) || (x2>=x0_wrect && x2<=x1_wrect)
                     || (x1<=x0_wrect && x2>=x1_wrect))
                break;
            }
        i2=i;
        (*index2)=i2;
#if (WILLUSDEBUGX & 0x10000)
printf("    i1=%d, i2=%d, n=%d\n",i1,i2,word->n);
#endif
        if (i2<i1)
            return(0);
#if (WILLUSDEBUGX & 0x10000)
printf("    Chars %d to %d\n",i1+1,i2+1);
printf("    Region = (%g,%g) - (%g,%g)\n",x0_wrect,y0_wrect,x1_wrect,y1_wrect);
printf("    Word = (%g,%g) - (%g,%g)\n",word->x0,word->y0,word->x0+word->w0,word->y0+word->h0);
#endif
        word->c=word->x0*wrectmap->srcdpiw/72.-wrectmap->coords[0].x+wrectmap->coords[1].x+.5;
        word->r=((double)word->r/100.)*wrectmap->srcdpih/72.-wrectmap->coords[0].y+wrectmap->coords[1].y+.5;
        word->w=(int)(word->w0*wrectmap->srcdpiw/72.+.5);
        word->h=(int)(word->h0*wrectmap->srcdpih/72.+.5);
#if (WILLUSDEBUGX & 0x10000)
printf("    Word mapped to (%d,%d), %d x %d\n",word->c,word->r,word->w,word->h);
printf("    Word mxheight = %g\n",word->maxheight);
#endif
        word->maxheight=word->maxheight*wrectmap->srcdpih/72.;
        word->lcheight=word->maxheight/1.7;
        word->rot=0;
        if (i1>0 || i2<word->n-1)
            {
            ocrword_truncate(word,i1,i2);
#if (WILLUSDEBUGX & 0x10000)
aprintf(ANSI_YELLOW "    Truncated='%s' (len=%d)" ANSI_NORMAL "\n",word->text,word->n);
printf("    Word = (%g,%g) - (%g,%g)\n",word->x0,word->y0,word->x0+word->w0,word->y0+word->h0);
printf("    Word mapped to (%d,%d), %d x %d\n",word->c,word->r,word->w,word->h);
printf("    Word mxheight = %g\n",word->maxheight);
#endif
            }
        return(1);
        }
    return(0);
    }


/*
** Find actual word bounding box graphically
*/
static int k2ocr_ocrword_verify_boundingbox(OCRWORD *word,BMPREGION *region)

    {
    int r,c,bw,totpx,lesspx,wt;
    int r1,r2,c1,c2;
    WILLUSBITMAP *bmp;

    r1=word->r-word->h+1;
    r2=word->r;
    c1=word->c;
    c2=word->c+word->w-1;
    bw=bmp_bytewidth(region->bmp8);
    totpx=word->w*word->h;
    wt=region->bgcolor<0 ? 192 : region->bgcolor;
    bmp=region->bmp8;
#if (WILLUSDEBUGX&0x10000)
printf("    bgc = %d\n",region->bgcolor);
printf("    r sweeping %d to %d\n",word->r,word->r+word->h-1);
printf("    c sweeping %d to %d\n",word->c,word->c+word->w-1);
printf("    bw=%d, totpx=%d\n",bw,totpx);
#endif
    lesspx=0;
    if (r2<0 || r1>=bmp->height || c2<0 || c1>=bmp->width)
        return(-1);
    if (r1<0)
        {
        lesspx += (-r1)*(c2-c1+1);
        r1=0;
        }
    if (c1<0)
        {
        lesspx += (-c1)*(r2-r1+1);
        c1=0;
        }
    if (r2>=bmp->height)
        {
        lesspx += (r2-(bmp->height-1))*(c2-c1+1);
        r2=bmp->height-1;
        }
    if (c2>=bmp->width)
        {
        lesspx += (c2-(bmp->width-1))*(r2-r1+1);
        c2=bmp->width-1;
        }
    /* Bot */
    for (r=r2;r>=r1;r--)
        {
        unsigned char *p;
        int j;
        p=bmp_rowptr_from_top(bmp,r)+c1;
        for (j=c1;j<=c2;j++,p++)
            if (p[0]<wt)
                break;
        if (j<=c2)
            break;
        }
    /* Is word rectangle completely blank?  Return -1 */
    if (r<r1)
        return(-1);
    lesspx += (r2-r)*(c2-c1+1);
    r2=r;
    /* Top */
    for (r=r1;r<=r2;r++)
        {
        unsigned char *p;
        int j;
        p=bmp_rowptr_from_top(bmp,r)+c1;
        for (j=c1;j<=c2;j++,p++)
            if (p[0]<wt)
                break;
        if (j<=c2)
            break;
        }
    lesspx += (r-r1)*(c2-c1+1);
    r1=r;
    /* Left */
    for (c=c1;c<=c2;c++)
        {
        unsigned char *p;
        int j;
        p=bmp_rowptr_from_top(bmp,r1)+c;
        for (j=r1;j<=r2;j++,p+=bw)
            if (p[0]<wt)
                break;
        if (j<=r2)
            break;
        }
    if (c>c2)
        return(-1);
    lesspx += (c-c1)*(r2-r1+1);
    c1=c;
    /* Right */
    for (c=c2;c>=c1;c--)
        {
        unsigned char *p;
        int j;
        p=bmp_rowptr_from_top(bmp,r1)+c;
        for (j=r1;j<=r2;j++,p+=bw)
            if (p[0]<wt)
                break;
        if (j<=r2)
            break;
        }
    lesspx += (c2-c)*(r2-r1+1);
    c2=c;
    word->r=r2;
    word->c=c1;
    word->h=r2-r1+1;
    word->w=c2-c1+1;
    {
    int percentage_reduction;
    percentage_reduction=(int)(lesspx*100./totpx+.5);
    return(percentage_reduction);
    }
    }


static void wtextchars_group_by_words(WTEXTCHARS *wtcs,OCRWORDS *words,
                                      K2PDFOPT_SETTINGS *k2settings)

    {
    double vert_spacing_threshold;
    static char *funcname="wtextchars_group_by_words";
    int i,i0;

    if (wtcs->n<=0)
        return;    
    /* Manually sort characters in OCR layer */
    if (k2settings->ocrsort)
        {
        double *ch; /* population of character heights */

        willus_dmem_alloc_warn(39,(void **)&ch,sizeof(double)*wtcs->n,funcname,10);
        /* Sort by position */
        /* v2.53--smarter(?) sort -- second arg = 3 */
        wtextchars_sort_vertically_by_position(wtcs,3);
        /* Create population character heights to determine row spacing threshold */
        for (i=0;i<wtcs->n-1;i++)
            ch[i]=wtcs->wtextchar[i].y2-wtcs->wtextchar[i].y1;
        sortd(ch,wtcs->n);
        vert_spacing_threshold=0.67*ch[wtcs->n/10];
        willus_dmem_free(39,&ch,funcname);
        }
    else
        vert_spacing_threshold=-1;
    /* Group characters row by row and add one row at a time */
    for (i0=0,i=1;i<wtcs->n;i++)
        {
        double dx,dy;
        int newrow;

        newrow=0;
        dy=wtcs->wtextchar[i].yp - wtcs->wtextchar[i-1].yp;
        dx=wtcs->wtextchar[i].xp - wtcs->wtextchar[i-1].xp;
        if (vert_spacing_threshold>0)
            {
            double h;

            h=wtcs->wtextchar[i].y2 - wtcs->wtextchar[i].y1;
            if (dy >= h*.75 || dy >= vert_spacing_threshold)
                {
                /* v2.53--fix sorting bug.  Change i-i0+1 to i-i0 */
                if (i-1>i0)
                    wtextchar_array_sort_horizontally_by_position(&wtcs->wtextchar[i0],i-i0);
                newrow=1;
                }
            }
        else
            {
            if (dx<0 || dy<0)
                newrow=1;
            }
        if (newrow)
            {
            wtextchars_add_one_row(wtcs,i0,i-1,words);
            i0=i;
            }
        }
    wtextchars_add_one_row(wtcs,i0,wtcs->n-1,words);
    }


static void wtextchars_add_one_row(WTEXTCHARS *wtcs,int i0,int i1,OCRWORDS *words)

    {
    int i,j0;
    OCRWORD word;
    int *u16str;
    static char *funcname="wtextchars_add_one_row";

#if (WILLUSDEBUGX & 0x10000)
printf("@wtextchars_add_one_row, i0=%d, i1=%d (of %d)\n",i0,i1,wtcs->n);
#endif
    ocrword_init(&word);
    willus_dmem_alloc_warn(40,(void **)&u16str,(i1-i0+1)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(41,(void **)&word.text,(i1-i0+2)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(42,(void **)&word.cpos,(i1-i0+2)*sizeof(double),funcname,10);
    for (j0=i=i0;i<=i1;i++)
        {
        int c,space;
        double h,dx;

        c=wtcs->wtextchar[i].ucs;
        /* If letters are physically separated, assume a space */
        if (i<i1)
            {
            h = fabs((wtcs->wtextchar[i+1].y2 - wtcs->wtextchar[i+1].y1)
                      +(wtcs->wtextchar[i].y2 - wtcs->wtextchar[i].y1)) / 2.;
            dx = wtcs->wtextchar[i+1].x1 - wtcs->wtextchar[i].x2;
            space = (dx > h*.05);
            }
        else
            {
            dx = 0.;
            space = 0;
            }
        /*
        ** Look for ligatured pattern--v2.33 with MuPDF v1.7
        ** Ligature pattern, e.g. "fi"
        ** As of MuPDF 1.7, a ligured "fi" is like this:
        ** f has zero width, followed by space at same position w/zero width,
        ** followed by i at same position, followed by space of zero width.
        */
        if (wtextchars_ligature_pattern(wtcs,i))
            {
            i+=3;
            continue;
            }
        if (space || c==' ' || i==i1)
            {
#if (WILLUSDEBUGX & 0x10000)
printf("    space at %d\n",i);
#endif
            if (c==' ' && i==j0)
                j0++;
            else
                {
                int j1,j,ligature,nc;
                double dx;

                j1=(c==' ')?i-1:i;
                word.r=(int)(wtcs->wtextchar[j0].yp*100.+.5);
                word.x0=wtcs->wtextchar[j0].x1;
                word.y0=wtcs->wtextchar[j0].y1;
                word.w0=wtcs->wtextchar[j1].x2-word.x0;
                word.h0=wtcs->wtextchar[j0].y2-word.y0;
                /*
                ** Narrow up the words slightly so that it is physically separated
                ** from other words--this makes selection work better in Sumatra.
                ** Note that only w0 is narrowed--the cpos[] values are not. (v2.20)
                */
                dx = word.h0*.05;
                if (word.w0<dx*4.)
                    dx = word.w0/10.;
                word.x0 += dx;
                word.w0 -= 2*dx;

                word.maxheight=0.;
                for (ligature=0,nc=0,j=j0;j<=j1;j++)
                    {
                    word.cpos[nc]=wtcs->wtextchar[j].x2-word.x0;
                    u16str[nc]=wtcs->wtextchar[j].ucs;
                    nc++;
#if (WILLUSDEBUGX & 0x10000)
printf("    Checking maxheight=%g against yp-y1=%g\n",word.maxheight,wtcs->wtextchar[j].yp-wtcs->wtextchar[j].y1);
#endif
                    if (wtcs->wtextchar[j].yp-wtcs->wtextchar[j].y1 > word.maxheight)
                        word.maxheight=(wtcs->wtextchar[j].yp-wtcs->wtextchar[j].y1);
                    /* Ligature check--new with MuPDF v1.7 */
                    if (ligature)
                        {
                        word.cpos[nc-1] += (wtcs->wtextchar[j+1].x2-wtcs->wtextchar[j].x2)/2.;
                        ligature=0;
                        j++;
                        }
                    else if (j<j1-2 && wtextchars_ligature_pattern(wtcs,j))
                        {
                        ligature=1;
                        j++;
                        }
                    }
                unicode_to_utf8(word.text,u16str,nc);
                word.n=nc;
                ocrwords_add_word(words,&word);
                j0= i+1;
                }
            }
        }
    willus_dmem_free(42,(double **)&word.cpos,funcname);
    willus_dmem_free(41,(double **)&word.text,funcname);
    willus_dmem_free(40,(double **)&u16str,funcname);
    }

/*
** For MuPDF v1.7 and up
*/
static int wtextchars_ligature_pattern(WTEXTCHARS *wtcs,int index)

    {
    return(index<wtcs->n-3
             && wtcs->wtextchar[index].ucs!=' ' 
             && wtcs->wtextchar[index+1].ucs==' '
             && wtcs->wtextchar[index+2].ucs!=' '
             && wtcs->wtextchar[index+3].ucs==' '
             && fabs(wtcs->wtextchar[index+1].x1-wtcs->wtextchar[index].x1)<.01
             && fabs(wtcs->wtextchar[index+2].x1 - wtcs->wtextchar[index].x1)<.01);
    }


int k2ocr_wtextchars_fill_from_page(WTEXTCHARS *wtcs,char *filename,int pageno,char *password,
                                   int boundingbox)

    {
    int src_type;

    src_type=get_source_type(filename);
#ifdef HAVE_DJVU_LIB
    if (src_type==SRC_TYPE_DJVU)
{
#if (WILLUSDEBUGX & 0x10000)
printf("Calling wtextchars_fill_from_djvu_page()\n");
#endif
        return(wtextchars_fill_from_djvu_page(wtcs,filename,pageno,boundingbox));
}
#endif
#ifdef HAVE_MUPDF_LIB
    return(wtextchars_fill_from_page_ex(wtcs,filename,pageno,password,boundingbox));
#else
    return(-1);
#endif
    }
    
#endif /* HAVE_MUPDF_LIB */
