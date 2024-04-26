/*
** strbuf.c     Functions to handle STRBUF structure.
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

#include <string.h>
#include "willus.h"

static char *count_from_top(char *s,int index);
static char *count_from_bottom(char *s,int index);
static void strcpy_no_spaces(char *d,char *s);

/*
** Return char pointer to line # line_index in the buffer (using \n as line end.)
** Use negative value for line_index to count from the end of the buffer backwards.
*/
char *strbuf_lineno(STRBUF *sbuf,int line_index)

    {
    if (sbuf->s==NULL)
        return(sbuf->s);
    if (line_index==0)
        return(sbuf->s);
    if (line_index>0)
        return(count_from_top(sbuf->s,line_index));
    return(count_from_bottom(sbuf->s,-line_index));
    }


static char *count_from_top(char *s,int index)

    {
    int i;

    if (index==1)
        return(s);
    for (i=0,index--;s[i]!='\0';i++)
        {
        if (s[i]=='\n')
            {
            index--;
            if (index<=0)
                break;
            }
        }
    if (s[i]=='\n')
        {
        i++;
        if (s[i]=='\r')
            i++;
        }
    return(&s[i]);
    }

            
static char *count_from_bottom(char *s,int index)

    {
    int i,len;

    if (s[0]=='\0')
        return(s);
    len=strlen(s)-1;
    if (len>0 && s[len]=='\r')
        len--;
    if (len>0 && s[len]=='\n')
        len--;
    for (i=len;i>0;i--)
        {
        if (s[i]=='\n')
            {
            index--;
            if (index<=0)
                break;
            }
        }
    if (s[i]=='\n')
        {
        i++;
        if (s[i]=='\r')
            i++;
        }
    return(&s[i]);
    }
            

void strbuf_init(STRBUF *sbuf)

    {
    sbuf->s=NULL;
    sbuf->na=0;
    }


void strbuf_cat_ex(STRBUF *sbuf,char *s)

    {
    if (s!=NULL && s[0]!='\0')
        {
        strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(s)+2);
        strcat(sbuf->s,s);
        }
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
    if (s!=NULL)
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


void strbuf_dsprintf(STRBUF *sbuf,STRBUF *sbuf2,char *fmt,...)

    {
    static char *funcname="strbuf_dsprintf";

    if (sbuf!=NULL || sbuf2!=NULL)
        {
        va_list args;
        char *buf;

        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
        va_start(args,fmt);
        vsprintf(buf,fmt,args);
        va_end(args);
        if (sbuf!=NULL && sbuf2==NULL)
            strbuf_cat(sbuf,buf);
        if (sbuf2!=NULL)
            strbuf_cat(sbuf2,buf);
        willus_mem_free((double **)&buf,funcname);
        }
    }


void strbuf_sprintf_no_space(STRBUF *sbuf,char *fmt,...)

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
        if (buf[0]!='\0')
            {
            strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(buf)+2);
            strcat(sbuf->s,buf);
            }
        willus_mem_free((double **)&buf,funcname);
        }
    }


void strbuf_dsprintf_no_space(STRBUF *sbuf,STRBUF *sbuf2,char *fmt,...)

    {
    static char *funcname="strbuf_sprintf";

    if (sbuf!=NULL || sbuf2!=NULL)
        {
        va_list args;
        char *buf;

        willus_mem_alloc_warn((void **)&buf,1024,funcname,10);
        va_start(args,fmt);
        vsprintf(buf,fmt,args);
        va_end(args);
        if (buf[0]!='\0')
            {
            if (sbuf!=NULL && sbuf2==NULL)
                {
                strbuf_ensure(sbuf,(sbuf->s==NULL?0:strlen(sbuf->s))+strlen(buf)+2);
                strcat(sbuf->s,buf);
                }
            if (sbuf2!=NULL)
                {
                strbuf_ensure(sbuf2,(sbuf2->s==NULL?0:strlen(sbuf2->s))+strlen(buf)+2);
                strcat(sbuf2->s,buf);
                }
            }
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
