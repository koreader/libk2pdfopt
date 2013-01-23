/*
** devprofile.c    Handle device profiles.
**
** Copyright (C) 2013  http://willus.com
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

static DEVPROFILE devprof[5] =
    {
    {"Kindle 2","k2",560,735,167,0,1,{0,0,3,4}},
    {"Nook Simple Touch","nookst",552,725,167,0,1,{0,0,0,0}},
    {"Kindle DX","dx",800,1180,167,0,1,{0,0,0,0}},
    {"Kindle Paperwhite","kpw",758,1024,212,0,1,{0,0,3,4}},
    {"","",0,0,167,0,1,{0,0,0,0}}
    };

void devprofiles_echo(FILE *out)

    {
    int i;

    fprintf(out,"\nAvailable devices:\n");
    for (i=0;devprof[i].width>0;i++)
        {
        afprintf(out,"    %s%s%s",TTEXT_BOLD,devprof[i].name,TTEXT_NORMAL);
        if (devprof[i].alias[0]!='\0')
            afprintf(out," (alias %s%s%s)",TTEXT_BOLD,devprof[i].alias,TTEXT_NORMAL);
        fprintf(out,": %d x %d, %d dpi\n",devprof[i].width,
                                          devprof[i].height,devprof[i].dpi);
        fprintf(out,"        Mark corners=%d, Padding (l,t,r,b)=%d,%d,%d,%d\n\n",
            devprof[i].mark_corners,
            devprof[i].padding[0],devprof[i].padding[1],
            devprof[i].padding[2],devprof[i].padding[3]);
        }
    fprintf(out,"\n");
    }


DEVPROFILE *devprofile_get(char *name)

    {
    int i,i0,c;

    for (i0=i=c=0;devprof[i].width>0;i++)
        if (in_string(devprof[i].name,name)>=0 || in_string(devprof[i].alias,name)>=0)
            {
            c++;
            i0=i;
            }
    if (c==1)
        return(&devprof[i0]);
    return(NULL);
    }


char *devprofile_select(void)

    {
    while (1)
        {
        int i,n,x;
        static char *q="q";

        aprintf("Select the device:\n");
        for (i=0;devprof[i].name[0]!='\0';i++)
            aprintf("    %s%2d%s. %s (%s)\n",TTEXT_BOLD,i+1,TTEXT_NORMAL,devprof[i].name,devprof[i].alias);
        aprintf("    %s%2d%s. Other (specify width, height, etc.)\n\n",TTEXT_BOLD,i+1,TTEXT_NORMAL);
        n=userinput_integer("Enter selection",1,&x,1,i+1);
        if (n<0)
            return(q);
        if (x==i+1)
            break;
        return(devprof[x-1].alias);
        }
    return(NULL);
    }
