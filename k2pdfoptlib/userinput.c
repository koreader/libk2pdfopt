/*
** userinput.c    User input functions for k2pdfopt.c.
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

#include "k2pdfopt.h"


int userinput_float(char *message,double defval,double *dstval,int nmax,
               double min,double max,char *extramessage)

    {
    char buf[256];
    int i,na;
    double v[8];

    if (nmax>8)
        nmax=8;
    while (1)
        {
        if (extramessage!=NULL && extramessage[0]!='\0')
            aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL "\n",extramessage);
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL,message);
        if (defval > -1e9)
            aprintf(" [%g]",defval);
        aprintf(": ");
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            dstval[0]=defval;
            return(1);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        na=string_read_doubles(buf,v,nmax);
        if (na<=0)
            {
            aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,buf);
            continue;
            }
        for (i=0;i<na;i++)
            {
            if (atof(buf)<min || atof(buf)>max)
                {
                aprintf(TTEXT_WARN "\aThe response must be between %g and %g.\n\n" TTEXT_NORMAL,min,max);
                break;
                }
            }
        if (i<na)
            continue;
        for (i=0;i<na;i++)
            dstval[i]=v[i];
        return(na);
        }
    }


int userinput_integer(char *message,int defval,int *dstval,int min,int max)

    {
    char buf[256];

    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " [%d]: " TTEXT_INPUT,
                message,defval);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            (*dstval)=defval;
            return(0);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        if (!is_an_integer(buf))
            {
            aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,buf);
            continue;
            }
        if (atoi(buf)<min || atoi(buf)>max)
            {
            aprintf(TTEXT_WARN "\aThe response must be between %d and %d.\n\n" TTEXT_NORMAL,min,max);
            continue;
            }
        (*dstval)=atoi(buf);
        return(0);
        }
    }


int userinput_any_string(char *message,char *dstval,int maxlen,char *defname)

    {
    char buf[1024];

    if (maxlen>1023)
        maxlen=1023;
    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " [%s]: " TTEXT_INPUT,
                message,defname);
        fgets(buf,maxlen,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            dstval[0]='\0';
            return(0);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        strncpy(dstval,buf,maxlen-1);
        dstval[maxlen-1]='\0';
        return(0);
        }
    }


int userinput_string(char *message,char *selection[],char *def)

    {
    char buf[256];
    int i;

    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " (",message);
        for (i=0;selection[i][0]!='\0';i++)
            aprintf("%s" TTEXT_BOLD "%c" TTEXT_NORMAL "%s",
                    i>0 ? ", " : "",selection[i][0],&selection[i][1]);
        aprintf(") [%c]: " TTEXT_INPUT,def[0]);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            strcpy(buf,def);
        if (tolower(buf[0])=='q')
            return(-1);
        for (i=0;selection[i][0]!='\0';i++)
            if (tolower(buf[0])==tolower(selection[i][0]))
                return(i);
        aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,
                buf);
        }
    }



int get_ttyrows(void)

    {
    int i,j;

    if (ansi_rows_cols(stdout,&i,&j))
        return(i);
#if (defined(WIN32) || defined(WIN64))
    return(25);
#else
    return(24);
#endif
    }
