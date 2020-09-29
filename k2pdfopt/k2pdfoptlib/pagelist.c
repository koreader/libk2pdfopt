/*
** pagelist.c    Functions to parse comma-delimited page-list string.
**
** Copyright (C) 2016  http://willus.com
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

static void pagelist_n1n2_adjust(int *n1,int *n2,int flags);
static int pagelist_next_pages(char *pagelist,int maxpages,int *index,
                               int *n1,int *n2,int *flags);
static int evenoddcheck(int c,int *flags);


int pagelist_valid_page_range(char *s)

    {
    int i;

    for (i=0;s[i]!='\0';i++)
        {
        if (s[i]==' ' || s[i]=='\t' || s[i]==',' || s[i]=='-' 
             || tolower(s[i])=='o' || tolower(s[i])=='e' || (s[i]>='0' && s[i]<='9'))
            continue;
        else
            break;
        }
    return(s[i]=='\0');
    }


int pagelist_includes_page(char *pagelist,int pageno,int maxpages)

    {
    int i,n;

    /* Sort of arbitrary */
    if (maxpages < 0)
        maxpages = 99999;

    /* v2.34--see if cover image included */
    if (pageno<0 && in_string(pagelist,"c")>=0)
        return(1);
    if (!stricmp(pagelist,"c") && pageno>0)
        return(0);

    n=pagelist_count(pagelist,maxpages);
/*
#ifdef WILLUSDEBUGX
printf("pagelist_count('%s',%d) = %d\n",pagelist,maxpages,pagelist_count(pagelist,maxpages));
#endif
*/
    for (i=0;i<n;i++)
        if (pagelist_page_by_index(pagelist,i,maxpages)==pageno)
             return(1);
    return(0);
    }


/*
** Store page list into integer array.
** Terminates with -2 if should go to max page.
** Terminates with -1 if not.
*/
void pagelist_get_array(int **pagelist,char *asciilist)

    {
    int n1,n2,i,j,k,nn,s,flags;
    int maxpages;
    int *pl;

    maxpages=999999;
    pl=(*pagelist)=NULL;
    if (asciilist[0]=='\0')
        return;
    for (k=0;k<2;k++)
        {
        int last;

        i=0;
        nn=0;
        last=-1;
        while (pagelist_next_pages(asciilist,maxpages,&i,&n1,&n2,&flags))
            {
            if (n1<=0 && n2<=0)
                continue;
            if (n1>=maxpages-1)
                continue;
            s = (n2>=n1) ? 1 : -1;
            if (flags!=3)
                s *= 2;
            n2 += s;
            if (n2>=maxpages-2)
                {
                if (k==1)
                    pl[nn]=n1;
                nn++;
                if (k==1)
                    pl[nn]=n1+s;
                nn++;
                if (k==1)
                    pl[nn]=-2;
                last = -2;
                nn++;
                break;
                }
            for (j=n1;j!=n2;j+=s)
                {
                if (j<1)
                    continue;
                if (k==1)
                    pl[nn]=j;
                nn++;
                }
            }
        if (nn>0 && last!=-2)
            {
            if (k==1)
                pl[nn]=-1;
            nn++;
            }
        if (k==0)
            {
            if (nn<=0)
                break;
            pl=(*pagelist)=malloc(sizeof(int)*nn);
            if (pl==NULL)
                break;
            }
        }
    }


/*
** Return the page number of the zero-based index'th page in the page list.
*/
int pagelist_page_by_index(char *pagelist,int index,int maxpages)

    {
    int n1,n2,i,j,s,flags;

// printf("@pagelist_page_by_index('%s',%d,%d)\n",pagelist,index,maxpages);
    if (pagelist[0]=='\0')
        return(index+1);
    i=0;
    while (pagelist_next_pages(pagelist,maxpages,&i,&n1,&n2,&flags))
        {
        if (n1<=0 && n2<=0)
            continue;
        s = (n2>=n1) ? 1 : -1;
        if (flags!=3)
            s *= 2;
        n2 += s;
        for (j=n1;j!=n2;j+=s)
            {
            if (j<1)
                continue;
            if (maxpages>0 && j>maxpages)
                continue;
            if (index==0)
                return(j);
            index--;
            }
        }
    return(-1);
    }


int double_pagelist_page_by_index(char *pagelist,char *pagexlist,int index,int maxpages)

    {
    int ntot,i,j,page;

    if (pagexlist==NULL || pagexlist[0]=='\0')
        return(pagelist_page_by_index(pagelist,index,maxpages));
    ntot=double_pagelist_count(pagelist,pagexlist,maxpages);
    if (index>=ntot)
        return(-1);
    page=-1;
    for (i=j=0;i<=index;j++)
        {
        page=pagelist_page_by_index(pagelist,j,maxpages);
        if (!pagelist_includes_page(pagexlist,page,maxpages))
            i++;
        }
    return(page);
    }


int double_pagelist_count(char *pagelist,char *pagexlist,int maxpages)

    {
    int i,n,ntot;

    ntot=n=pagelist_count(pagelist,maxpages);
    if (pagexlist!=NULL && pagexlist[0]!='\0')
        for (i=0;i<n;i++)
            {
            int page;

            page=pagelist_page_by_index(pagelist,i,maxpages);
            if (pagelist_includes_page(pagexlist,page,maxpages))
                ntot--;
            }
    return(ntot);
    }


int pagelist_count(char *pagelist,int maxpages)

    {
    int n1,n2,i,count,flags;
/*
#ifdef WILLUSDEBUGX
printf("@pagelist_count('%s',%d)\n",pagelist,maxpages);
#endif
*/
    if (pagelist[0]=='\0')
        return(maxpages);
    count=0;
    i=0;
    while (pagelist_next_pages(pagelist,maxpages,&i,&n1,&n2,&flags))
        {
/*
#ifdef WILLUSDEBUGX
printf("   count=%d, i=%d, n1=%d, n2=%d\n",count,i,n1,n2);
#endif
*/
        if (n1<=0 && n2<=0)
            continue;
        if (n1>n2)
            {
            int t;
            t=n1;
            n1=n2;
            n2=t;
            }
/*
#ifdef WILLUSDEBUGX
printf("   maxpages=%d, n1=%d, n2=%d\n",maxpages,n1,n2);
#endif
*/
        if ((maxpages>0 && n1>maxpages) || n2<1)
            continue;
        if (n1<1)
            n1=1;
        if (maxpages>0 && n2>maxpages)
            n2=maxpages;
        if (flags==3)
            count += n2-n1+1;
        else if (n2>=n1)
            count += (n2-n1+2)/2;
        }
    return(count);
    }


static void pagelist_n1n2_adjust(int *n1,int *n2,int flags)

    {
    if (flags==2)
        {
        if ((*n2)>=(*n1))
            {
            (*n1)=((*n1)+1)&(~1);
            (*n2)=(*n2)&(~1);
            }
        else
            {
            (*n1)=(*n1)&(~1);
            (*n2)=((*n2)+1)&(~1);
            }
        }
    else if (flags==1)
        {
        if ((*n2)>=(*n1))
            {
            (*n1)=(*n1)|1;
            (*n2)=((*n2)-1)|1;
            }
        else
            {
            (*n1)=((*n1)-1)|1;
            (*n2)=(*n2)|1;
            }
        }
    }

/*
** (*flags)&1 ==> Odds
** (*flags)&2 ==> Evens
*/
static int pagelist_next_pages(char *pagelist,int maxpages,int *index,
                               int *n1,int *n2,int *flags)

    {
    int i,j;
    char buf[128];

    i=(*index);
    (*flags)=3; /* Even and odd */
    if (evenoddcheck(pagelist[i],flags))
        {
        i++;
        buf[0]='\0';
        }
    else
        {
        for (j=0;j<126 && pagelist[i]>='0' && pagelist[i]<='9';i++)
            buf[j++]=pagelist[i];
        buf[j]='\0';
        if (evenoddcheck(pagelist[i],flags))
            i++;
        }
    if (buf[0]=='\0')
        {
        if (pagelist[i]=='-')
            (*n1)=1;
        else
            {
            (*n1)=-1;
            (*n2)=-1;
            (*index)=i;
            if ((*flags)!=3)
                {
                (*n1)=1;
                (*n2)=maxpages;
                pagelist_n1n2_adjust(n1,n2,(*flags));
                if (pagelist[i]!='\0')
                    (*index)=(*index)+1;
                return(1);
                }
            if (pagelist[i]=='\0')
                return(0);
            (*index)=(*index)+1;
            return(1);
            }
        }
    else
        (*n1)=atoi(buf);
    if (pagelist[i]!='-')
        (*n2)=(*n1);
    else
        {
        if (evenoddcheck(pagelist[i+1],flags))
            {
            i+=2;
            buf[0]='\0';
            }
        else
            {
            for (i++,j=0;j<126 && pagelist[i]>='0' && pagelist[i]<='9';i++)
                buf[j++]=pagelist[i];
            buf[j]='\0';
            if (evenoddcheck(pagelist[i],flags))
                i++;
            }
        if (buf[0]=='\0')
            {
            (*n2)=maxpages;
            /*
            ** If the user specifies a starting number that is beyond the max with an open
            ** hyphen, set (*n2) to that number also--otherwise it will count backwards.
            ** (v2.32 bug fix)
            */
            if ((*n2)<(*n1))
                (*n2)=(*n1);
            }
        else
            (*n2)=atoi(buf);
        }
    if (pagelist[i]!='\0')
        i++;
    (*index)=i;
    pagelist_n1n2_adjust(n1,n2,(*flags));
    return(1);
    }


static int evenoddcheck(int c,int *flags)

    {
    c=tolower(c);
    if (c=='e')
        {
        (*flags) = 2;
        return(1);
        }
    if (c=='o')
        {
        (*flags) = 1;
        return(1);
        }
    return(0);
    }
