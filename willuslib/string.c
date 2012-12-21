/*
** string.c     String/text buffer manipulation/analysis functions.
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
#include <math.h>
#include "willus.h"

static int rest_pmdigits(char *s);
static int rest_digits(char *s);
static void year_adjust(int *year);

/*
** Remove all white space from beginning and end of a string and
** re-pack the string, terminating with '\0'.
** '\n' or '\r' is considered to terminate the input string and
** is removed if found.
**
** White space = ' ', '\t'
**
** E.g. the input string:  "\t\t   Hello there!    \n"
**      becomes the output string:  "Hello there!"
*/
void clean_line(char *buf)

    {
    int     i,j;

    for (i=0;buf[i]!='\n' && buf[i]!='\r' && buf[i]!='\0';i++);
    for (i--;i>=0 && (buf[i]==' ' || buf[i]=='\t');i--);
    buf[i+1]='\0';
    for (i=0;buf[i]==' ' || buf[i]=='\t';i++);
    if (i)
        {
        for (j=0;buf[i]!='\0';j++,i++)
            buf[j]=buf[i];
        buf[j]='\0';
        }
    }


/*
** Get next line from a file, but terminate it with '\0' rather than '\n'.
**
** Returns NULL if EOF, otherwise returns pointer to the string.
**
*/
char *get_line(char *buf,int max,FILE *f)

    {
    int     i;

    if (fgets(buf,max,f)==NULL)
        return(NULL);
    for (i=0;buf[i]!='\n' && buf[i]!='\0';i++);
    if (buf[i]!='\0')
        buf[i]='\0';
    return(buf);
    }


/*
** Get next line from a commented file.
** If the line begins with a semi-colon, it is ignored.  If it has
** a semi-colon in it, the semi-colon is converted to an end-of-string
** character ('\0').
**
** Returns NULL if EOF, otherwise returns pointer to the string.
**
*/
char *get_line_cf(char *buf,int max,FILE *f)

    {
    int     i;

    while (1)
        {
        if (fgets(buf,max,f)==NULL)
            return(NULL);
        if (buf[0]!=';')
            break;
        }
    for (i=0;buf[i]!='\n' && buf[i]!=';' && buf[i]!='\0';i++);
    if (buf[i]!='\0')
        buf[i]='\0';
    return(buf);
    }


int mem_get_line_cf(char *buf,int maxlen,char *cptr,long *cindex,long csize)

    {
    int     i,j;

    i=(*cindex);
    if (i>=csize)
        return(0);
    while (1)
        {
        for (j=0;j<maxlen && i<csize && cptr[i]!='\n';i++)
            {
            if (cptr[i]=='\r')
                continue;
            buf[j++]=cptr[i];
            }
        buf[j]='\0';
        if (i>=csize)
            break;
        if (j>=maxlen)
            for (;i<csize && cptr[i]!='\n';i++);
        if (i>=csize)
            break;
        i++;
        if (buf[0]!=';')
            break;
        }
    (*cindex)=i;
    if (i>=csize)
        return(buf[0]!=';');
    return(1);
    }


/*
** Return first index where pattern occurs in buffer.  Return -1
** if pattern does not occur in buffer.  Match is NOT case sensitive.
*/
int in_string(char *buffer,char *pattern)

    {
    int     i,lp,lb;

    lp=strlen(pattern);
    lb=strlen(buffer);
    if (lb<lp)
        return(-1);
    for (i=0;i<=lb-lp;i++)
        if (!strnicmp(&buffer[i],pattern,lp))
            return(i);
    return(-1);
    }


/*
** Return NZ only if the string s contains a legal integer or floating
** point number (base 10).  Exponential representation is allowed.
**
** Example legal numbers:  105  +.06 -1.007e-10  0020.008  1d0  193478.
** Example illegal numbers:  105a  .e10  105.003.  1e10.3 -2+3e4
**
*/
int is_a_number(char *s)

    {
    int     i,j,dc1;

    i=0;
    if (s[i]=='+' || s[i]=='-')
        i++;
    for (j=i;s[i]>='0' && s[i]<='9';i++);
    dc1=i-j;
    if (dc1==0 && s[i]!='.')
        return(0);
    if (s[i]=='\0')
        return(1);
    if (tolower(s[i])=='e' || tolower(s[i])=='d')
        return(rest_pmdigits(&s[i+1]));
    if (s[i]!='.')
        return(0);
    for (i++,j=i;s[i]>='0' && s[i]<='9';i++);
    if (dc1==0 && i==j)
        return(0);
    if (s[i]=='\0')
        return(1);
    if (tolower(s[i])=='e' || tolower(s[i])=='d')
        return(rest_pmdigits(&s[i+1]));
    return(0);
    }


/*
** Return NZ if the passed string is a valid integer (may begin with
** + or -).  Return zero otherwise.
**
*/
int is_an_integer(char *s)

    {
    int     i;

    i=0;
    if (s[i]=='+' || s[i]=='-')
        i++;
    if (s[i]=='\0')
        return(0);
    for (;s[i]!='\0';i++)
        if (s[i]<'0' || s[i]>'9')
            return(0);
    return(1);
    }


int get_pos_range(char *s,int *n1,int *n2)

    {
    int     i;
    char    buf[80];

    strcpy(buf,s);
    if (buf[0]=='-')
        return(0);
    if (is_an_integer(buf))
        {
        (*n1)=(*n2)=atoi(buf);
        return(1);
        }
    for (i=0;buf[i]!='-' && buf[i]!='\0';i++);
    if (buf[i]=='\0')
        return(0);
    buf[i]='\0';
    if (!is_an_integer(buf))
        return(0);
    (*n1)=atoi(buf);
    if (!is_an_integer(&buf[i+1]))
        return(0);
    (*n2)=atoi(&buf[i+1]);
    return(1);
    }



/* The following str...() functions are already defined in
   DJGPP and TURBOC */

#ifdef NEEDSTRICMP
/*
** Like strcmp(), but without case sensitivity
*/
int stricmp(const char *s1,const char *s2)

    {
    int     i;

    for (i=0;s1[i]!='\0' && s2[i]!='\0';i++)
        if (tolower(s1[i])!=tolower(s2[i]))
            break;
    return(tolower(s1[i])-tolower(s2[i]));
    }


/*
** Convert string to all lower case
*/
void strlwr(char *s)

    {
    int     i;

    for (i=0;s[i]!='\0';s[i]=tolower(s[i]),i++);
    }


/*
** Like strncmp(), but without case sensitivity
*/
int strnicmp(const char *s1,const char *s2,int n)

    {
    int     i;

    for (i=0;i<n && s1[i]!='\0' && s2[i]!='\0';i++)
        if (tolower(s1[i])!=tolower(s2[i]))
            break;
    return(i>=n ? 0 : tolower(s1[i])-tolower(s2[i]));
    }


/*
** Convert string to all upper case
*/
void strupr(char *s)

    {
    int     i;

    for (i=0;s[i]!='\0';s[i]=toupper(s[i]),i++);
    }
#endif /* NEEDSTRICMP */


/*
** Parses a typical long usage list and creates an abbreviated list
** of options.
*/
void pr_short_usage(char *progname,char *usage)

    {
    static char line1[80];
    static char linen[80];
    static char buf[120];
    static char next[100];
    int     i,j,k;

    sprintf(line1,"usage:  %s",progname);
    for (i=0;i<strlen(line1);i++)
        linen[i]=' ';
    linen[i]='\0';
    strcpy(buf,line1);
    for (i=0;usage[i]!='\0';i++)
        {
        if (usage[i]=='-')
            {
            k=0;
            next[k++]=' ';
            next[k++]='[';
            for (j=0;j<80 && usage[i+j]!='\0'
                          && (usage[i+j]!=' ' || usage[i+j+1]=='<'
                                              || usage[i+j+1]=='[');j++)
                next[k++]=usage[i+j];
            next[k++]=']';
            next[k]='\0';
            if (strlen(next)+strlen(buf)>78)
                {
                printf("%s\n",buf);
                strcpy(buf,linen);
                }
            strcat(buf,next);
            }
        for (;usage[i]!='\n' && usage[i]!='\0';i++);
        if (usage[i]=='\0')
            break;
        }
    printf("%s\n",buf);
    }


void decimal_string(char *s,double x,int ndigits,int signspace)

    {
    int is;
    double round;

    if (fabs(x)>=1.0)
        {
        char fmt[16];
        sprintf(fmt,"%%%d.%df",signspace?ndigits+3:ndigits+2,ndigits);
        sprintf(s,fmt,x);
        return;
        }
    is=0;
    if (x>=0. && signspace)
        s[is++]=' ';
    else if (x<0)
        {
        s[is++]='-';
        x = -x;
        }
    round = 5.*pow(10.,(double)-ndigits-1);
    x += round;
    s[is++]='.';
    for (;ndigits>0;ndigits--)
        {
        int ix;

        x *= 10.;
        ix = (int)(x);
        s[is++] = (ix<0 || ix>9) ? '#' : ix+'0';
        x -= (double)ix;
        }
    s[is]='\0';
    }

        
/*
** Prints an integer to 's' with commas separating every three digits.
** E.g. 45,399,350
** Correctly handles negative values.
*/
void comma_print(char *s,long size)

    {
    int  i,m,neg;
    char tbuf[80];

    if (!size)
        {
        s[0]='0';
        s[1]='\0';
        return;
        }
    s[0]='\0';
    neg=0;
    if (size<0)
        {
        size=-size;
        neg=1;
        }
    for (i=0,m=size%1000;size;i++,size=(size-m)/1000,m=size%1000)
        {
        sprintf(tbuf,m==size ? "%d%s":"%03d%s",m,i>0 ? "," : "");
        strcat(tbuf,s);
        strcpy(s,tbuf);
        }
    if (neg)
        {
        strcpy(tbuf,"-");
        strcat(tbuf,s);
        strcpy(s,tbuf);
        }
    }


void comma_dprint(char *s,double size)

    {
    char    tbuf[256];
    char    t2[100];
    char    fmt[10];
    int     negative,c;

    c=0;
    s[0]='\0';
    if (size==0.)
        {
        s[0]='0';
        s[1]='\0';
        return;
        }
    negative = (size<0);
    size = fabs(size);
    while (size)
        {
        double  m;
        int     i;

        if (c>0 && size<0.5)
            break;
        m=fmod(size,1000.);
        if (m!=size)
            strcpy(fmt,"%03d");
        else
            strcpy(fmt,"%d");
        size = (size-m)/1000.;
        i=(int)m;
        m -= i;
        sprintf(tbuf,fmt,i);
        if (c==0 && m!=0.)
            {
            sprintf(t2,"%g",m);
            for (i=0;t2[i]!='.' && t2[i]!='\0';i++);
            strcat(tbuf,&t2[i]);
            }
        if (c)
            strcat(tbuf,",");
        strcat(tbuf,s);
        strcpy(s,tbuf);
        c++;
        }
    if (negative)
        {
        strcpy(tbuf,"-");
        strcat(tbuf,s);
        strcpy(s,tbuf);
        }
    }


/*
** Caution:  buf cannot be constant char *.  It gets temporarily modified
** by this routine.
*/
int string_read_doubles(char *buf,double *a,int nmax)

    {
    int     i,n;

    for (i=n=0;buf[i]!='\0';)
        {
        int     j,c;

        for (;buf[i]=='\n' || buf[i]=='\r' || buf[i]==' ' || buf[i]=='\t' || buf[i]==',' || buf[i]==';';i++);
        if (buf[i]=='\0')
            break;
        j=i;
        for (;buf[i]!='\n' && buf[i]!='\r' && buf[i]!=' ' && buf[i]!='\t' && buf[i]!=','
                           && buf[i]!=';' && buf[i]!='\0';i++);
        c=buf[i];
        if (buf[i]!='\0')
            buf[i]='\0';
        if (!is_a_number(&buf[j]))
            {
            buf[i]=c;
            break;
            }
        a[n++]=string_atof(&buf[j]);
        buf[i]=c;
        if (n>=nmax)
            break;
        }
    return(n);
    }


double string_atof(char *s)

    {
#ifdef WIN32
    return(atof(s));
#else
    char *p;
    int i;
    double x;
    p=malloc(strlen(s)+1);
    if (p==NULL)
        return(atof(s));
    strcpy(p,s);
    for (i=0;p[i]!='\0';i++)
        if (tolower(p[i])=='d')
            p[i]=p[i]+1;
    x=atof(p);
    free(p);
    return(x);
#endif
    }


/*
** Caution:  buf cannot be constant char *.  It gets temporarily modified
** by this routine.
*/
int string_read_integers(char *buf,int *a,int nmax)

    {
    int     i,n;

    for (i=n=0;buf[i]!='\0';)
        {
        int     j,c;

        for (;buf[i]=='\n' || buf[i]=='\r' || buf[i]==' ' || buf[i]=='\t' || buf[i]==',' || buf[i]==';';i++);
        if (buf[i]=='\0')
            break;
        j=i;
        for (;buf[i]!='\n' && buf[i]!='\r' && buf[i]!=' ' && buf[i]!='\t' && buf[i]!=','
                           && buf[i]!=';' && buf[i]!='\0';i++);
        c=buf[i];
        if (buf[i]!='\0')
            buf[i]='\0';
        if (!is_an_integer(&buf[j]))
            {
            buf[i]=c;
            break;
            }
        a[n++]=atoi(&buf[j]);
        buf[i]=c;
        if (n>=nmax)
            break;
        }
    return(n);
    }


/*
**
** string_wild_match_ignore_case(char *pattern,char *name)
**
** Returns 1 if name matches pattern, 0 otherwise.
** Pattern may contain '*' for any number of any characters.
**
** Note:  function is recursive.
**
*/
int string_wild_match_ignore_case(char *pattern,char *name)

    {
    int     i,j,k;

    for (i=0,j=0;pattern[i]!='\0';i++,j++)
        {
        if (pattern[i]=='*' && pattern[i+1]=='\0')
            return(1);
        if (pattern[i]=='*')
            break;
        if (pattern[i]!='?' && tolower((int)pattern[i])!=tolower((int)name[j]))
            return(0);
        if (pattern[i]=='?' && name[j]=='\0')
            return(0);
        }
    if (pattern[i]=='\0' && name[j]=='\0')
        return(1);
    if (pattern[i]=='\0')
        return(0);
    for (k=j;name[k]!='\0';k++)
        if (string_wild_match_ignore_case(&pattern[i+1],&name[k]))
            return(1);
    return(0);
    }


/*
** Converts a string like "1,3,4,6,19-38,45-60" to a double precision
** array sequence.
*/
int range_to_darray(double *a,int maxsize,char *s)

    {
    char *p;
    static char nbuf[256];
    int n,i;
    double x,x1,x2;

    for (n=0,p=strtok(s,", \t");p!=NULL && p[0]!='\0';p=strtok(NULL,", \t"))
        {
        i=in_string(p,"-");
        if (i<=0)
            {
            if (n<maxsize)
                a[n++]=atof(p);
            continue;
            }
        strcpy(nbuf,p);
        nbuf[i]='\0';
        x1=atof(nbuf);
        x2=atof(&nbuf[i+1]);
        for (x=x1;x<=x2;x+=1.0)
            if (n<maxsize)
                a[n++]=x;
        }
    return(n);
    }


void clean_quotes(char *s)

    {
    int c,len;

    c=s[0];
    if (c!='\"' && c!='\'')
        return;
    len=strlen(s);
    memmove(s,&s[1],len);
    len--;
    if (s[len-1]==c)
        s[len-1]='\0';
    }
    

/*
** Converts a string like "1,3,4,6,19-38,45-60" to an integer
** array sequence.
*/
int range_to_iarray(int *a,int maxsize,char *s)

    {
    char *p;
    static char nbuf[256];
    int n,i,i1,i2;

    for (n=0,p=strtok(s,", \t");p!=NULL && p[0]!='\0';p=strtok(NULL,", \t"))
        {
        i=in_string(p,"-");
        if (i<=0)
            {
            if (n<maxsize)
                a[n++]=atoi(p);  
            continue;
            }
        strcpy(nbuf,p);
        nbuf[i]='\0';
        i1=atoi(nbuf);
        i2=atoi(&nbuf[i+1]);
        for (i=i1;i<=i2;i++)
            {
            if (n<maxsize)
                a[n++]=i;
            }
        }
    return(n);
    }


static int rest_pmdigits(char *s)

    {
    int     i;

    i=0;
    if (s[i]=='+' || s[i]=='-')
        i++;
    return(rest_digits(&s[i]));
    }


static int rest_digits(char *s)

    {
    int     i;

    if (s[0]=='\0')
        return(0);
    for (i=0;s[i]>='0' && s[i]<='9';i++);
    return(s[i]=='\0');
    }


void envvar_subst(char *dest,char *src)

    {
    int  i,i0,j,k;
    char envvar[100];
    char *p;

    for (i=j=0;src[i]!='\0';i++)
        {
        if (src[i]=='$')
            {
            i0=i;
            for (i++,k=0;src[i]!='\0' && src[i]!=' ' && src[i]!='\\'
                               && src[i]!=':' && src[i]!='/';i++)
                envvar[k++]=src[i];
            envvar[k]='\0';
            if (envvar[0]!='\0' && (p=getenv(envvar))!=NULL)
                {
                dest[j]='\0';
                strcat(dest,p);
                j=strlen(dest);
                i--;
                continue;
                }
            i=i0;
            }
        dest[j++]=src[i];
        }
    dest[j]='\0';
    }


void double_quote_if_has_spaces(char *s)

    {
    int i,len;

    for (i=0;s[i]!='\0' && s[i]!=' ' && s[i]!='\t';i++);
    if (s[i]=='\0')
        return;
    len=strlen(s);
    memmove(&s[1],s,len);
    s[0]='\"';
    s[len+1]='\"';
    s[len+2]='\0';
    }


/* Guarantee that the exponential representation has only 2 digits. */
void exp_str(char *buf,int maxlen,int decimals,double value)

    {
    char fmt[16];
    int i,j,zero;

    if (maxlen < 1)
        maxlen=1;
    if (maxlen > 99999)
        maxlen=99999;
    if (decimals > 99999)
        decimals = 99999;
    if (decimals < 0)
        decimals=0;
    sprintf(fmt,"%%%d.%de",maxlen,decimals);
    sprintf(buf,fmt,value);
    for (i=0;buf[i]!='e' && buf[i]!='E' && buf[i]!='\0';i++);
    if (buf[i]=='\0' || buf[i+1]=='\0')
        return;
    for (i=i+2,j=i;buf[i]>='0' && buf[i]<='9';i++);
    if (i-j<3)
        return;
    maxlen++;
    sprintf(fmt,"%%%d.%de",maxlen,decimals);
    sprintf(buf,fmt,value);
    for (i=0;buf[i]!='e' && buf[i]!='E' && buf[i]!='\0';i++);
    for (i=i+2,j=i;buf[i]>='0' && buf[i]<='9';i++);
    zero=(buf[j]=='0');
    if (!zero)
        {
        buf[j]='9';
        buf[j+1]='9';
        }
    else
        {
        buf[j]=buf[j+1];
        buf[j+1]=buf[j+2];
        }
    buf[j+2]='\0';
    }


/*
** Parses most date/time formats.  The key assumption is that the time
** part of the string is HH:MM[:SS.SS][am|pm] (i.e. has colons and no spaces,
** though it can have a space before the AM or PM.)
*/
int structtm_from_datetime(struct tm *date,char *datetime)

    {
    char buf[64];
    char time[64];
    int  i,p,i0,status;
    struct tm dt;

    date->tm_year=0;
    date->tm_mon=0;
    date->tm_mday=1;
    date->tm_hour=0;
    date->tm_min=0;
    date->tm_sec=0;

    /* Make temp copy of string */
    strncpy(buf,datetime,63);
    buf[63]='\0';

    /* Clean up string a bit */
    for (i=0;buf[i]!='\0';i++)
        if (buf[i]=='\t' || buf[i]==',')
            buf[i]=' ';
    clean_line(buf);

    /* First check for " am" or " pm" and remove preceding space if found. */
    while (1)
        {
        p=in_string(buf," am");
        if (p<0)
            p=in_string(buf," pm");
        if (p>=0)
            memmove(&buf[p],&buf[p+1],strlen(buf)-p);
        else
            break;
        }

    /* Now find string with colons (time string) */
    for (i=0;buf[i]!='\0' && buf[i]!=':';i++);
    /* If no colons found, assume it's a date only */
    if (buf[i]!=':')
        return(structtm_from_date(date,datetime));
    /* Put time into "time" */
    for (;i>=0 && buf[i]!=' ';i--);
    for (i++,i0=i;buf[i]!=' ' && buf[i]!='\0';i++)
        time[i-i0]=buf[i];
    time[i-i0]='\0';
    /* Remove time string from buf */
    memmove(&buf[i0-1],&buf[i],strlen(buf)-i+1);
    /* Parse time string */
    structtm_from_time(date,time);
    /* Parse date string */
    dt=(*date);
    status=structtm_from_date(date,buf);
    date->tm_hour = dt.tm_hour;
    date->tm_min = dt.tm_min;
    date->tm_sec = dt.tm_sec;
    return(status);
    }


/*
** Accepts most date string formats:
**     JAN DD YYYY|YY
**     DD JAN YYYY|YY
**     YYYY JAN DD
**     YYYY DD JAN
**     DD YYYY JAN
**     YYYY-MM-DD
**     MM-DD-YYYY|YY [Default]
**
** CAUTION!  This function zeros the time fields!
*/
int structtm_from_date(struct tm *date,char *datestr)

    {
    static char *months[]={"jan","feb","mar","apr","may","jun",
                           "jul","aug","sep","oct","nov","dec"};
    char buf[32];
    char tok[3][32];
    int num[3];
    int mon[3];
    int i,j,n,yr;

    strncpy(buf,datestr,31);
    buf[31]='\0';
    date->tm_year=0;
    date->tm_mon=0;
    date->tm_mday=1;
    for (i=0;buf[i]!='\0';i++)
        if (buf[i]=='/' || buf[i]=='-' || buf[i]=='\\' 
                        || buf[i]=='.' || buf[i]==':')
            buf[i]=' ';
    n=sscanf(buf,"%s %s %s",tok[0],tok[1],tok[2]);
    yr = -1;
    for (i=0;i<n;i++)
        {
        mon[i]=0;
        if (is_an_integer(tok[i]))
            {
            num[i]=atoi(tok[i]);
            continue;
            }
        for (j=0;j<12;j++)
            if (!strnicmp(months[j],tok[i],3))
                break;
        if (j<12)
            {
            num[i]=j+1;
            mon[i]=1;
            }
        else
            num[i]=-1;
        }
    if (n>0 && mon[0]==1)
        {
        date->tm_mon=num[0]-1;
        if (n>1)
            date->tm_mday=num[1];
        if (n>2)
            yr=num[2];
        }
    else if (n>1 && mon[1]==1)
        {
        date->tm_mon=num[1]-1;
        if (num[0]>31)
            {
            yr=num[0];
            if (n>2)
                date->tm_mday=num[2];
            }
        else
            {
            date->tm_mday=num[0];
            if (n>2)
                yr=num[2];
            }
        }
    else if (n>2 && mon[2]==1)
        {
        date->tm_mon=num[2]-1;
        if (num[1]>31)
            {
            yr=num[1];
            date->tm_mday=num[0];
            }
        else
            {
            yr=num[0];
            date->tm_mday=num[1];
            }
        }
    else if (n>0 && num[0]>31)
        {
        yr=num[0];
        if (n>1)
            {
            if (num[1]<=12)
                date->tm_mon=num[1]-1;
            else
                date->tm_mday=num[1];
            if (n>2)
                {
                if (num[1]<=12)
                    date->tm_mday=num[2];
                else
                    date->tm_mon=num[2]-1;
                }
            }
        }
    else
        {
        if (n>0)
            {
            if (num[0]>12)
                date->tm_mday=num[0];
            else
                date->tm_mon=num[0]-1;
            if (n>1)
                {
                if (num[0]>12)
                    date->tm_mon=num[1]-1;
                else
                    date->tm_mday=num[1];
                if (n>2)
                    yr=num[2];
                }
            }
        }
    if (date->tm_mon<0 || date->tm_mon>12)
        date->tm_mon=0;
    if (date->tm_mday<1 || date->tm_mday>31)
        date->tm_mday=1;
    if (yr>=0)
        {
        year_adjust(&yr);
        date->tm_year=yr-1900;
        }
    /* Get week day and year day right */
    {
    time_t lt;

    date->tm_hour=0;
    date->tm_min=0;
    date->tm_sec=0;
    date->tm_wday = -1;
    date->tm_yday = -1;
    date->tm_isdst = -1;
/*
printf("date = %d/%02d/%04d, %02d:%02d:%02d, wd=%d, yd=%d, isdst=%d\n",
date->tm_mon+1,date->tm_mday,date->tm_year+1900,date->tm_hour,date->tm_min,
date->tm_sec,date->tm_wday,date->tm_yday,date->tm_isdst);
*/
    lt = mktime(date);
    if (lt>=0)
        (*date)=(*localtime(&lt));
    else
        {
        date->tm_wday=0;
        date->tm_yday=0;
        date->tm_isdst=0;
        }
/*
printf("date = %d/%02d/%04d, %02d:%02d:%02d, wd=%d, yd=%d, isdst=%d\n",
date->tm_mon+1,date->tm_mday,date->tm_year+1900,date->tm_hour,date->tm_min,
date->tm_sec,date->tm_wday,date->tm_yday,date->tm_isdst);
*/
    }
    return(0);
    }


static void year_adjust(int *year)

    {
    time_t now;
    struct tm tnow;

    time(&now);
    tnow=(*localtime(&now));
    if ((*year) < 100)
        {
        int century;
        century=tnow.tm_year/100;
        if ((*year) <= (tnow.tm_year%100)+5)
            (*year) += 1900+century*100;
        else
            (*year) += 1900+(century-1)*100;
        }
    }


/*
** Parses standard time format:  H[H]:M[M][:SS[.SSS]][AM|PM]
*/
int structtm_from_time(struct tm *date,char *time)

    {
    char buf[32];
    int am,pm,n,j;
    double secs;

    date->tm_hour=0;
    date->tm_min=0;
    date->tm_sec=0;
    strncpy(buf,time,31);
    buf[31]='\0';
    for (j=0;buf[j]!='\0';j++)
        if (buf[j]==':')
            buf[j]=' ';
    am=in_string(buf,"am");
    pm=in_string(buf,"pm");
    if (am>=0)
        buf[am]=buf[am+1]=' ';
    if (pm>=0)
        buf[pm]=buf[pm+1]=' ';
    n=sscanf(buf,"%d %d %lf",&date->tm_hour,&date->tm_min,&secs);
    date->tm_sec = (n>2) ? (int)secs : 0;
    if (am>=0 && date->tm_hour>0 && date->tm_hour<=12)
        date->tm_hour = (date->tm_hour==12) ? 0 : date->tm_hour;
    else if (pm>=0 && date->tm_hour>0 && date->tm_hour<=12)
        date->tm_hour = (date->tm_hour!=12) ? date->tm_hour+12 : date->tm_hour;
    return(0);
    }


/*
** Like strtok, but allows empty "cells".  E.g. if there are consecutive
** tabs in a tab-delimited file, this will return an empty string for
** the empty cell.  strtok() won't do that.
*/
char *wstrtok(char *s,char *t)

    {
    static char buf[1024];
    static char buf2[1024];
    static char emptystr[16];
    static int  len;
    int j,i;

    emptystr[0]='\0';
    if (s!=NULL)
        {
        strcpy(buf,s);
        len=strlen(buf);
        i=0;
        }
    else
        i=strlen(buf)+1;
    if (i>=len)
        return(emptystr);
    j=i;
    if (i>0)
        buf[i-1]=t[0];
    for (;buf[i]!='\0' && buf[i]!=t[0];i++);
    buf[i]='\0';
    strcpy(buf2,&buf[j]);
    clean_line(buf2);
    return(buf2);
    }


/*
** If d!=NULL, it gets populated w/unicode values
*/
int utf8_to_unicode(int *d,char *s,int maxlen)

    {
    int i,c;

    for (i=c=0;c<maxlen && s[i]!='\0';i++)
        {
        int x,bits,topbyte;

        for (x=s[i],bits=0;x&0x80;x<<=1,bits++);
        if (bits==0)
            {
            if (d!=NULL)
                d[c]=s[i];
            c++;
            continue;
            }
        topbyte=(x&0xff)>>bits;
        for (x=topbyte,i++,bits--;bits>0;i++,bits--)
            x=(x<<6)|(s[i]&0x3f);
        i--;
        if (d!=NULL)
            d[c]=x;
        c++;
        }
    return(c);
    }
            

