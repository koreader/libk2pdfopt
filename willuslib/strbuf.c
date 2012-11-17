/*
** strbuf.c     Functions to handle STRBUF structure.
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

#include <string.h>
#include "willus.h"

static void strcpy_no_spaces(char *d,char *s);

void strbuf_init(STRBUF *sbuf)

    {
    sbuf->s=NULL;
    sbuf->na=0;
    }


void strbuf_cat(STRBUF *sbuf,char *s)

    {
    if (s!=NULL && s[0]!='\0')
        {
        strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(s)+2);
        if (sbuf->s[0]!='\0')
            strcat(sbuf->s," ");
        strcat(sbuf->s,s);
        }
    }


void strbuf_cat_with_quotes(STRBUF *sbuf,char *s)

    {
    if (s!=NULL && s[0]!='\0')
        {
        int q=0;
        strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(s)+4);
        if (sbuf->s[0]!='\0')
            strcat(sbuf->s," ");
        q=(s[0]!='\"' && (in_string(s," ")>=0 || in_string(s,"\t")>=0));
        if (q)
            strcat(sbuf->s,"\"");
        strcat(sbuf->s,s);
        if (q)
            strcat(sbuf->s,"\"");
        }
    }


void strbuf_cat_no_spaces(STRBUF *sbuf,char *s)

    {
    if (s!=NULL && s[0]!='\0')
        {
        strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(s)+1);
        if (sbuf->s[0]!='\0')
            strcat(sbuf->s," ");
        strcpy_no_spaces(&sbuf->s[strlen(sbuf->s)],s);
        }
    }


void strbuf_cpy(STRBUF *sbuf,char *s)

    {
    if (s!=NULL && s[0]!='\0')
        {
        strbuf_ensure(sbuf,strlen(s)+1);
        strcpy(sbuf->s,s);
        }
    }


void strbuf_clear(STRBUF *sbuf)

    {
    if (sbuf->s!=NULL)
        sbuf->s[0]='\0';
    }


void strbuf_ensure(STRBUF *sbuf,int n)

    {
    static char *funcname="strbuf_ensure";
    if (n>sbuf->na)
        {
        willus_mem_realloc_robust_warn((void**)&sbuf->s,n,sbuf->na,funcname,10);
        if (sbuf->na==0)
            sbuf->s[0]='\0';
        sbuf->na=n;
        }
    }


void strbuf_free(STRBUF *sbuf)

    {
    willus_mem_free((double**)&sbuf->s,"strbuf_free");
    sbuf->s=NULL;
    sbuf->na=0;
    }


void strbuf_sprintf(STRBUF *sbuf,char *fmt,...)

    {
    static char *funcname="strbuf_sprintf";

    if (sbuf!=NULL)
        {
        va_list args;
        char *buf;

        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
        va_start(args,fmt);
        vsprintf(buf,fmt,args);
        va_end(args);
        strbuf_cat(sbuf,buf);
        willus_mem_free((double **)&buf,funcname);
        }
    }


static void strcpy_no_spaces(char *d,char *s)

    {
    int i,j;

    for (j=i=0;s[i]!='\0';i++)
        {
        if (s[i]==' ' || s[i]=='\t')
            continue;
        d[j++]=s[i];
        }
    d[j]='\0';
    }
