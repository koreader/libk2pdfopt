/*
** ocr.c   Routines involving optical character recognition (OCR)
**         (Not specific to one particular engine.)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "willus.h"

static void ocrword_init(OCRWORD *word);
static void ocrword_free(OCRWORD *word);
static int  vowel(int c0);
static int  not_usually_after_T(int c0);
static int  not_usually_after_n(int c0);
static int  not_usually_after_l(int c0);
static int  not_usually_after_r(int c0);


static void ocrword_init(OCRWORD *word)

    {
    word->text=NULL;
    }


static void ocrword_free(OCRWORD *word)

    {
    static char *funcname="ocrword_free";

    willus_mem_free((double **)&word->text,funcname);
    }


void ocrwords_init(OCRWORDS *words)

    {
    words->word=NULL;
    words->n=words->na=0;
    }


/*
** Allocates new space for text buffer.
*/
void ocrwords_add_word(OCRWORDS *words,OCRWORD *word)

    {
    static char *funcname="ocrwords_add_word";

    if (words->n>=words->na)
        {
        int i,newsize;
      
        newsize = words->na<512 ? 1024 : words->na*2;
        willus_mem_realloc_robust_warn((void **)&words->word,newsize*sizeof(OCRWORD),
                                    words->na*sizeof(OCRWORD),funcname,10);
        for (i=words->na;i<newsize;i++)
            ocrword_init(&words->word[i]);
        words->na=newsize;
        }
    words->word[words->n]=(*word);
    words->word[words->n].text=NULL;
    willus_mem_alloc_warn((void **)&words->word[words->n].text,strlen(word->text)+1,funcname,10);
    strcpy(words->word[words->n].text,word->text);
    words->n++;
    }

/*
** Remove words from index i1 through i2
*/
void ocrwords_remove_words(OCRWORDS *words,int i1,int i2)

    {
    int i,dn;

    if (i2>words->n-1)
        i2=words->n-1;
    if (i2<0)
        i2=0;
    if (i1>words->n-1)
        i1=words->n-1;
    if (i1<0)
        i1=0;
    if (i1>i2)
        {
        int t;
        t=i1;
        i1=i2;
        i2=t;
        }
    dn=i2-i1+1;
    for (i=i2;i>=i1;i--)
        ocrword_free(&words->word[i]);
    for (i=i1;i<words->na-dn;i++)
        words->word[i]=words->word[i+dn];
    for (i=words->na-dn;i<words->na;i++)
        ocrword_init(&words->word[i]);
    words->n -= dn;
    }


void ocrwords_clear(OCRWORDS *words)

    {
    int i;

    for (i=words->na-1;i>=0;i--)
        ocrword_free(&words->word[i]);
    words->n=0;
    }


void ocrwords_free(OCRWORDS *words)

    {
    int i;
    static char *funcname="ocrwords_free";

    for (i=words->na-1;i>=0;i--)
        ocrword_free(&words->word[i]);
    willus_mem_free((double **)&words->word,funcname);
    words->n=0;
    words->na=0;
    }


void ocrwords_offset(OCRWORDS *words,int dx,int dy)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        words->word[i].c += dx;
        words->word[i].r += dy;
        }
    }


void ocrwords_scale(OCRWORDS *words,double srat)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        int c2,r2;
        c2 = (words->word[i].c+words->word[i].w-1)*srat;
        r2 = (words->word[i].r+words->word[i].h-1)*srat;
        words->word[i].c = words->word[i].c*srat;
        words->word[i].r = words->word[i].r*srat;
        words->word[i].maxheight = words->word[i].maxheight*srat;
        words->word[i].lcheight = words->word[i].lcheight*srat;
        words->word[i].w = c2-words->word[i].c+1;
        words->word[i].h = r2-words->word[i].r+1;
        }
    }


void ocrwords_int_scale(OCRWORDS *words,int ndiv)

    {
    int i;

    for (i=0;i<words->n;i++)
        {
        int c2,r2;
        c2 = (words->word[i].c+words->word[i].w-1)/ndiv;
        r2 = (words->word[i].r+words->word[i].h-1)/ndiv;
        words->word[i].c = words->word[i].c/ndiv;
        words->word[i].r = words->word[i].r/ndiv;
        words->word[i].maxheight = words->word[i].maxheight/ndiv;
        words->word[i].lcheight = words->word[i].lcheight/ndiv;
        words->word[i].w = c2-words->word[i].c+1;
        words->word[i].h = r2-words->word[i].r+1;
        }
    }


void ocrwords_concatenate(OCRWORDS *dst,OCRWORDS *src)

    {
    int i;

    for (i=0;i<src->n;i++)
        ocrwords_add_word(dst,&src->word[i]);
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
