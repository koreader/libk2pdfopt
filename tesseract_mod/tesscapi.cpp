/*
** tesscapi.cpp    willus.com attempt at C wrapper for tesseract.
**                 (Butchered from tesseractmain.cpp)
**                 Last udpated 9-1-12
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

/*
#include "mfcpch.h"
*/
// #define USE_VLD //Uncomment for Visual Leak Detector.
#if (defined _MSC_VER && defined USE_VLD)
#include <vld.h>
#endif

// Include automatically generated configuration file if running autoconf
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#include <locale.h>
#ifdef USING_GETTEXT
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) (x)
#endif

#include "allheaders.h"
#include "baseapi.h"
#include "strngs.h"
#include "params.h"
#include "blobs.h"
#include "simddetect.h"
#include "tesseractclass.h"
/*
#include "notdll.h"
*/

/* C Wrappers */
#include "tesseract.h"

// static tesseract::TessBaseAPI api[4];

/*
** ocr_type=0:  OEM_DEFAULT
** ocr_type=1:  OEM_TESSERACT_ONLY
** ocr_type=2:  OEM_LSTM_ONLY
** ocr_type=3:  OEM_TESSERACT_LSTM_COMBINED
*/
void *tess_capi_init(char *datapath,char *language,int ocr_type,FILE *out,
                     char *initstr,int maxlen,int *status)

    {
    char original_locale[256];
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI;
/*
printf("@tess_capi_init\n");
printf("    datapath='%s'\n",datapath);
printf("    language='%s'\n",language);
printf("    ocr_type=%d\n",ocr_type);
*/
#ifdef USE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    /* willus mod, 11-24-16 */
    /* Tesseract needs "C" locale to correctly parse all data .traineddata files. */
/*
printf("locale='%s'\n",setlocale(LC_ALL,NULL));
printf("ctype='%s'\n",setlocale(LC_CTYPE,NULL));
printf("numeric='%s'\n",setlocale(LC_NUMERIC,NULL));
*/
    strncpy(original_locale,setlocale(LC_ALL,NULL),255);
    original_locale[255]='\0';
/*
printf("original_locale='%s'\n",original_locale);
*/
    setlocale(LC_ALL,"C");
/*
printf("new locale='%s'\n",setlocale(LC_ALL,NULL));
printf("new ctype='%s'\n",setlocale(LC_CTYPE,NULL));
printf("new numeric='%s'\n",setlocale(LC_NUMERIC,NULL));
*/
    // fprintf(stderr, "tesseract %s\n", tesseract::TessBaseAPI::Version());
    // Make the order of args a bit more forgiving than it used to be.
    const char* lang = "eng";
    tesseract::PageSegMode pagesegmode = tesseract::PSM_SINGLE_BLOCK;
    if (language!=NULL && language[0]!='\0')
        lang = language;
    /*
    if (output == NULL)
        {
        fprintf(stderr, _("Usage:%s imagename outputbase [-l lang] "
                      "[-psm pagesegmode] [configfile...]\n"), argv[0]);
        fprintf(stderr,
            _("pagesegmode values are:\n"
              "0 = Orientation and script detection (OSD) only.\n"
              "1 = Automatic page segmentation with OSD.\n"
              "2 = Automatic page segmentation, but no OSD, or OCR\n"
              "3 = Fully automatic page segmentation, but no OSD. (Default)\n"
              "4 = Assume a single column of text of variable sizes.\n"
              "5 = Assume a single uniform block of vertically aligned text.\n"
              "6 = Assume a single uniform block of text.\n"
              "7 = Treat the image as a single text line.\n"
              "8 = Treat the image as a single word.\n"
              "9 = Treat the image as a single word in a circle.\n"
              "10 = Treat the image as a single character.\n"));
        fprintf(stderr, _("-l lang and/or -psm pagesegmode must occur before any"
                      "configfile.\n"));
        exit(1);
        }
    */
/*
printf("SSE = %s\n",SIMDDetect::IsSSEAvailable() ? "AVAILABLE" : "NOT AVAILABLE");
printf("AVX = %s\n",SIMDDetect::IsAVXAvailable() ? "AVAILABLE" : "NOT AVAILABLE");
*/
/*
v4.00 loads either TESSERACT enginer, LSTM engine, or both.  No CUBE.
*/
    ocr_type=0; /* Ignore specified and use default */
    api->SetOutputName(NULL);
    (*status)=api->Init(datapath,lang,
              ocr_type==0 ? tesseract::OEM_DEFAULT :
                (ocr_type==1 ? tesseract::OEM_TESSERACT_ONLY :
                   (ocr_type==2 ? tesseract::OEM_LSTM_ONLY :
                                  (tesseract::OEM_TESSERACT_LSTM_COMBINED))));
    if ((*status)!=0)
        {
        /* willus mod, 11-24-16 */
        setlocale(LC_ALL,original_locale);
        api->End();
        delete api;
        return(NULL);
        }
    /*
    api.Init("tesscapi",lang,tesseract::OEM_DEFAULT,
           &(argv[arg]), argc - arg, NULL, NULL, false);
    */
    // We have 2 possible sources of pagesegmode: a config file and
    // the command line. For backwards compatability reasons, the
    // default in tesseract is tesseract::PSM_SINGLE_BLOCK, but the
    // default for this program is tesseract::PSM_AUTO. We will let
    // the config file take priority, so the command-line default
    // can take priority over the tesseract default, so we use the
    // value from the command line only if the retrieved mode
    // is still tesseract::PSM_SINGLE_BLOCK, indicating no change
    // in any config file. Therefore the only way to force
    // tesseract::PSM_SINGLE_BLOCK is from the command line.
    // It would be simpler if we could set the value before Init,
    // but that doesn't work.
    if (api->GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK)
        api->SetPageSegMode(pagesegmode);

    /*
    ** Initialization message
    */
    {
    char istr[1024];
    int sse,avx;

// printf("tessedit_ocr_engine_mode = %d\n",tessedit_ocr_engine_mode);
    sprintf(istr,"%s",api->Version());
    sse=SIMDDetect::IsSSEAvailable();
    avx=SIMDDetect::IsAVXAvailable();
    if (sse || avx)
        sprintf(&istr[strlen(istr)]," [%s]",sse&&avx?"SSE+AVX":(sse?"SSE":"AVX"));
    sprintf(&istr[strlen(istr)],"\n    Tesseract data folder = '%s'",datapath==NULL?getenv("TESSDATA_PREFIX"):datapath);
    strcat(istr,"\n    Tesseract languages: ");
    GenericVector<STRING> languages;
    api->GetLoadedLanguagesAsVector(&languages);
/*
printf("OEM=%d\n",api->oem());
printf("Langs='%s'\n",api->GetInitLanguagesAsString());
printf("AnyTessLang()=%d\n",(int)api->tesseract()->AnyTessLang());
printf("AnyLSTMLang()=%d\n",(int)api->tesseract()->AnyLSTMLang());
printf("num_sub_langs()=%d\n",api->tesseract()->num_sub_langs());
printf("languages.size()=%d\n",(int)languages.size());
*/
    
    for (int i=0;i<=api->tesseract()->num_sub_langs();i++)
        {
        tesseract::Tesseract *lang1;
        int eng;
        lang1 = i==0 ? api->tesseract() : api->tesseract()->get_sub_lang(i-1);
        eng=(int)lang1->tessedit_ocr_engine_mode;
        sprintf(&istr[strlen(istr)],"%s%s [%s]",i==0?"":", ",lang1->lang.string(),
                 eng==2?"LSTM+Tess":(eng==1?"LSTM":"Tess"));
        }
/*
printf("%d. '%s'\n",i+1,languages[i].string());
printf("    sublang[%d].oem_engine = %d\n",i+1,(int)api->tesseract()->get_sub_lang(i)->tessedit_ocr_engine_mode);
*/

    /*
    if (ocr_type==0 || ocr_type==3)
        sprintf(&istr[strlen(istr)],"[LSTM+] (lang=");
    else if (ocr_type==2)
        sprintf(&istr[strlen(istr)],"[LSTM] (lang=");
    strncpy(&istr[strlen(istr)],language,253-strlen(istr));
    istr[253]='\0';
    strcat(istr,")");
    */
    if (out!=NULL)
        fprintf(out,"%s\n",istr);
    if (initstr!=NULL)
        {
        strncpy(initstr,istr,maxlen-1);
        initstr[maxlen-1]='\0';
        }
    }


    /* Turn off LSTM debugging output */
    api->SetVariable("lstm_debug_level","0");
#if (WILLUSDEBUG & 1)
    api->SetVariable("lstm_debug_level","9");
    api->SetVariable("paragraph_debug_level","9");
    api->SetVariable("tessdata_manager_debug_level","9");
    api->SetVariable("tosp_debug_level","9");
    api->SetVariable("wordrec_debug_level","9");
    api->SetVariable("segsearch_debug_level","9");
#endif
    /* willus mod, 11-24-16 */
    setlocale(LC_ALL,original_locale);
    return((void *)api);
    }


int tess_capi_get_ocr(void *vapi,PIX *pix,char *outstr,int maxlen,int segmode,FILE *out)

    {
    tesseract::TessBaseAPI *api;
    static int old_segmode=-1;

    api=(tesseract::TessBaseAPI *)vapi;
    if (old_segmode != segmode)
        {
        old_segmode=segmode;
        api->SetPageSegMode((tesseract::PageSegMode)segmode);
        }
    if (!api->ProcessPage(pix,0,NULL,NULL,0,NULL))
        {
        /* pixDestroy(&pix); */
        if (out!=NULL)
            fprintf(out,"tesscapi:  Error during bitmap processing.\n");
        api->Clear();
        return(-1);
        }
    strncpy(outstr,api->GetUTF8Text(),maxlen-1);
    outstr[maxlen-1]='\0';
    api->Clear();
    return(0);
    }


int tess_capi_get_ocr_multiword(void *vapi,PIX *pix,int segmode,
                                int **left,int **top,int **right,int **bottom,
                                int **ybase,char **text,int *nw,
                                FILE *out)

    {
    tesseract::TessBaseAPI *api;
    static int old_segmode=-1;

    api=(tesseract::TessBaseAPI *)vapi;
    if (old_segmode != segmode)
        {
        old_segmode=segmode;
        api->SetPageSegMode((tesseract::PageSegMode)segmode);
        }
    if (!api->ProcessPage(pix,0,NULL,NULL,0,NULL))
        {
        if (out!=NULL)
            fprintf(out,"tesscapi:  Error during bitmap processing.\n");
        api->Clear();
        (*nw)=0;
        return(-1);
        }
    (*nw)=api->GetOCRWords(left,top,right,bottom,ybase,text);
    api->Clear();
    return(0);
    }


void tess_capi_end(void *vapi)

    {
    tesseract::TessBaseAPI *api;

    if (vapi==NULL)
        return;
    api=(tesseract::TessBaseAPI *)vapi;
    api->End();
    delete api;
    }
