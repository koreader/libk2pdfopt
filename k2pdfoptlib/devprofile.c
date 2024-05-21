/*
** devprofile.c    Handle device profiles.
**
** Copyright (C) 2018  http://willus.com
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

/*
** v2.32:  Added new device dims from screen shots on mobileread.com
** v2.42:  Added Kobo Aura One
** v2.50:  Added Kindle Oasis 2nd-gen; Combined Voyager/PW3+/Oasis
*/
static DEVPROFILE devprof[21] =
    {
    {"Kindle 1-5","k2",560,735,167,0,1,{0,0,3,4}},
    {"Kindle DX","dx",800,1180,167,0,1,{0,0,0,0}},
    /* v2.02:  Changed kpw to 658 x 889 based on e-mail feedback, 9-15-13 */
    {"Kindle Paperwhite","kpw",658,889,212,0,1,{0,0,3,4}},
    /* v2.31:  Added PW2 based on user feedback and Voyage */
    /* Voyage = 1072 x 1448--guessed at viewable PDF resolution */
    /* PW2 dims from Doitsu's screen shots = 718 x 964 */
    /* http://www.mobileread.com/forums/showthread.php?p=3013298#post3013298 */
    {"Kindle Paperwhite 2","kp2",718,965,212,0,1,{0,0,3,4}},
    /* PW3 (released Summer 2015) = 1016 x 1364 = Kindle Voyage (7-5-15 on MR.com) */
    {"Kindle Paperwhite 3","kp3",1016,1364,300,0,1,{0,0,3,4}},
    /* Voyage dims = 1016 x 1364 */
    /* http://www.mobileread.com/forums/showthread.php?p=3012815#post3012815 */
    /* http://www.mobileread.com/forums/showthread.php?p=3018484#post3018484 */
    {"Kindle Voyage/PW3+/Oasis","kv",1016,1364,300,0,1,{0,0,3,4}},
    /* Kindle Oasis 2nd-gen released Oct 2017--4-6-18 on MR.com */ 
    {"Kindle Oasis 2","ko2",1200,1583,300,0,1,{0,0,3,4}},
    /* Pocketbook Basic 2 = 600 x 800 (6-20-15 on MR.com) */
    {"Pocketbook Basic 2","pb2",600,800,167,0,1,{0,0,3,4}},
    {"Nook Simple Touch","nookst",552,725,167,0,1,{0,0,0,0}},
    /* Kobo:  http://www.mobileread.com/forums/showthread.php?p=3012925#post3012925 */
    {"Kobo Touch","kbt",600,730,167,0,1,{0,0,3,4}},
    {"Kobo Glo","kbg",758,942,213,0,1,{0,0,3,4}},
    /* E-mail 7-23-15: Kobo Glo HD */
    {"Kobo Glo HD","kghd",1072,1328,250,0,1,{0,0,3,4}},
    {"Kobo Glo HD Full Screen","kghdfs",1072,1448,250,0,1,{0,0,3,4}},
    /* v2.13:  Added Kobo mini */
    {"Kobo Mini","kbm",600,730,200,0,1,{0,0,3,4}},
    {"Kobo Aura","kba",758,932,211,0,1,{0,0,3,4}},
    {"Kobo Aura HD","kbhd",1080,1320,250,0,1,{0,0,3,4}},
    {"Kobo H2O","kbh2o",1080,1309,265,0,1,{0,0,3,4}},
    {"Kobo H2O Full Screen","kbh2ofs",1080,1429,265,0,1,{0,0,3,4}},
    {"Kobo Aura One","kao",1404,1713,300,0,0,{0,0,3,4}},
    /* Nexus 7 */
    /* http://www.mobileread.com/forums/showthread.php?p=3181143#post3181143 */
    {"Nexus 7","nex7",1187,1811,323,1,1,{0,0,3,4}},
    {"","",0,0,167,0,1,{0,0,0,0}}
    };


int devprofiles_count(void)

    {
    int i;

    for (i=0;devprof[i].name[0]!='\0';i++);
    return(i);
    }


char *devprofile_alias(int index)

    {
    return(devprof[index].alias);
    }


char *devprofile_name(int index)

    {
    return(devprof[index].name);
    }


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
        {
        if (!stricmp(devprof[i].name,name) || !stricmp(devprof[i].alias,name))
            {
            c=1;
            i0=i;
            break;
            }
        if (in_string(devprof[i].name,name)>=0 || in_string(devprof[i].alias,name)>=0)
            {
            c++;
            i0=i;
            }
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

        k2printf("Select your e-reader type:\n");
        for (i=0;devprof[i].name[0]!='\0';i++)
            k2printf("    %s%2d%s. %s (%s)\n",TTEXT_BOLD,i+1,TTEXT_NORMAL,devprof[i].name,devprof[i].alias);
        k2printf("    %s%2d%s. Other (specify width, height, etc.)\n\n",TTEXT_BOLD,i+1,TTEXT_NORMAL);
        n=userinput_integer("Enter selection",1,&x,1,i+1);
        if (n<0)
            return(q);
        if (x==i+1)
            break;
        return(devprof[x-1].alias);
        }
    return(NULL);
    }
