/*
** token.c  Get next token from a file or buffer.
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "willus.h"

static int allow_escapes = 1;
static int inited = 0;
static char allowed_white[64];

#define QUOTE(x)    ((x)=='\"' || (x)=='\'')
#define TRUE        1
#define FALSE       0

static int token_space(int ch);
static int token_nextline(FILE *f,int *linenum);
static void token_incline(int *linenum);
static void token_decline(int *linenum);


void cmdlineinput_init(CMDLINEINPUT *cl,int argc,char *argv[],char *opt_string)

    {
    cl->p = opt_string;
    cl->index = 0;
    cl->i=1;
    cl->argv=argv;
    cl->argc=argc;
    cl->cmdarg[0]='\0';
    }


char *cmdlineinput_next(CMDLINEINPUT *cl)

    {
    if (cl->p!=NULL)
        {
        if (token_next_from_string(cl->cmdarg,cl->p,&cl->index,1024))
            return(cl->cmdarg);
        else
            cl->p=NULL;
        }
    if (cl->i>=cl->argc)
        return(NULL);
    strncpy(cl->cmdarg,cl->argv[cl->i],1023);
    cl->cmdarg[1023]='\0';
    cl->i++;
    return(cl->cmdarg);
    }
        
    
/*
** Currently does not handle a double-quote within a double-quote.
** (e.g. a back-slashed double-quote)
*/
int token_next_from_string(char *dst,char *src,int *index,int max)

    {
    int     i,j;

    i=(*index);
    for (;src[i]==' ' || src[i]=='\t' || src[i]=='\n' || src[i]=='\r';i++);
    (*index)=i;
    if (src[i]=='\0')
        return(0);

    /* Is it quoted? */
    if (src[i]=='\"')
        {
        for (j=0,i++;src[i]!='\"' && src[i]!='\0';i++)
            {
            if (j<max-1)
                dst[j++]=src[i];
            }
        dst[j]='\0';
        if (src[i]=='\"')
            i++;
        }
    else
        {
        /* Not quoted */
        for (j=0;src[i]!=' ' && src[i]!='\t' && src[i]!='\n' 
                             && src[i]!='\r' && src[i]!='\0';i++)
            if (j<max-1)
                dst[j++]=src[i];
        dst[j]='\0';
        }
    for (;src[i]==' ' || src[i]=='\t' || src[i]=='\n' || src[i]=='\r';i++);
    (*index)=i;
    return(-1);
    }


void token_set_white(char *s)

    {
    strcpy(allowed_white,s);
    inited=1;
    }


void token_set_escapes(int status)

    {
    allow_escapes = status;
    }


/*
** nexttoken(FILE *stream, char *string, int max,int sameline)
**
** 1.  Skips past any white space (including ";" comments) to the
**     next character in the file.  NOTE that = and , are treated
**     as white space.
**
**     If the end of the file is encountered, a zero-length string
**     is returned, and the boolean value FALSE is returned.
**
**     If sameline==1 and a '\n' is encountered in the white
**     space, a zero-length string is returned, but TRUE is returned.
**     The file pointer is left pointing to the beginning of the
**     next line.
**
** 2.  Collects the next set of contiguous characters into the
**     string variable.  If the next contiguous string has more
**     characters than "max", then ALL of the characters are
**     read (i.e. the token is exhausted), but only the first
**     "max" characters are stored to the string.
**
**     Examples of a contiguous string:
**
**     a.  this_is_one_token
**     b.  "this is one token"
**     c.  'this is one token'
**     d.  'this " is " one " token'
**     e.  "this is one "\  "token"
**     f.  "this is one "\  any text without quotes can go here
**         and will be ignored.  "token"
**     g.   this_is_one_\   (a linefeed must immediately follow the \)
**          token
**     Note:  e., f., g. only if token_set_escapes(0) NOT called.
**
**     If a token splits across lines using the "\ method,
**     the value of sameline has no effect.
**
**     Tokens may not split across lines unless a \ is put at the
**     end of the line or after the last quote mark.
**
**     IF token_set_escapes(0) was NOT CALLED, these will also
**     be interpreted:
**     Special characters:  \n puts a linefeed in the string
**                          \f puts a formfeed in the string
**                          \" puts a double quote in the string
**                          \xHH puts binary byte HH (hex) into the string
**                             There MUST be two and only two digits
**                             specified for the hex value.
**                          \\ puts a backslash in the string
**
** 3.  The file pointer is left pointing to the first
**     white-space character which terminates the string.
**
**     If a token has quotes around it, the file pointer is left
**     pointing to the character just after the quote.
**
*/
int token_next(FILE *f,char *string,int max,int sameline,int *linenum,int *quoted)

    {
    int     c,j,d1,d2,qc;

    if (!inited)
        {
        strcpy(allowed_white," \n\f\t,=\x1a");
        inited=1;
        }
    string[0]='\0';
    if (quoted!=NULL)
        (*quoted)=0;
    for (c=fgetc(f);c!=EOF;c=fgetc(f))
        {
        if (c=='\n')
            token_incline(linenum);
        if (c=='\n' && sameline)
            return(TRUE);
        if (c==';')
            {
            if (!token_nextline(f,linenum))
                return(FALSE);
            if (sameline)
                return(TRUE);
            continue;
            }
        if (!token_space(c))
            break;
        }
    if (c==EOF)
        return(FALSE);
    j=0;
    if (!QUOTE(c))
        {
        string[j++]=c;
        for (c=fgetc(f);c!=EOF && c!=';' && !token_space(c);c=fgetc(f))
            if (j<max)
                string[j++]=c;
        if (c=='\n')
            token_incline(linenum);
        }
    else
        {
        if (quoted!=NULL)
            (*quoted)=1;
        qc=c;
        while (1)
            {
            for (c=fgetc(f);c!=EOF && c!=qc && c!='\n';c=fgetc(f))
                {
                if (allow_escapes && c=='\\')
                    {
                    c=fgetc(f);
                    if (c==EOF)
                        break;
                    if (c=='\n')
                        {
                        token_incline(linenum);
                        continue;
                        }
                    else if (c=='n')
                        {
                        if (j<max)
                            string[j++]='\n';
                        }
                    else if (c=='f')
                        {
                        if (j<max)
                            string[j++]='\f';
                        }
                    else if (c=='\"')
                        {
                        if (j<max)
                            string[j++]='\"';
                        }
                    else if (c=='x')
                        {
                        d1=tolower(fgetc(f));
                        d2=tolower(fgetc(f));
                        if (d1>'9')
                            d1=d1-'a'+10;
                        else
                            d1=d1-'0';
                        if (d2>'9')
                            d2=d2-'a'+10;
                        else
                            d2=d2-'0';
                        string[j++]=(char)((d1<<4)+d2);
                        }
                    else
                        {
                        if (j<max)
                            string[j++]=c;
                        }
                    continue;
                    }
                else
                    {
                    if (j<max)
                        string[j++]=c;
                    }
                }
            if (c=='\n')
                token_incline(linenum);
            if (c==qc)
                {
                c=fgetc(f);
                if (c=='\n')
                    token_incline(linenum);
                if (allow_escapes && c=='\\')
                    {
                    for (c=fgetc(f);1;c=fgetc(f))
                        if (c==EOF || c=='\"' || (c==';' && !token_nextline(f,linenum)))
                            break;
                    if (c==qc)
                        continue;
                    }
                }
            break;
            }
        }
    string[j]='\0';
    /* Note that fseek(f,-1L,1) on a non-binary opened file can be risky. */
    fseek(f,-1L,1);
    c=fgetc(f);
    fseek(f,-1L,1);
    if (c=='\n')
        token_decline(linenum);
    return(TRUE);
    }


static int token_space(int ch)

    {
    int i;

    for (i=0;i<64 && allowed_white[i]!='\0';i++)
        if (ch==allowed_white[i])
            return(-1);
    return(0);
    }


static int token_nextline(FILE *f,int *linenum)

    {
    int     c;

    while ((c=fgetc(f))!=EOF && c!='\n');
    if (c=='\n')
        token_incline(linenum);
    return(c!=EOF);
    }


static void token_incline(int *linenum)

    {
    if (linenum!=NULL)
        (*linenum) = (*linenum)+1;
    }


static void token_decline(int *linenum)

    {
    if (linenum!=NULL)
        (*linenum) = (*linenum)-1;
    }
