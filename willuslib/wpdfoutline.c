/*
** wpdfoutline.c  Routines to manipulate PDF bookmarks / outlines
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2023  http://willus.com
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
#include <math.h>
#include "willus.h"

#ifdef HAVE_Z_LIB
#include <zlib.h>
#endif

/* Outline support functions */
static int wpdfoutline_within(WPDFOUTLINE *outline,WPDFOUTLINE *outline1);


void wpdfoutline_init(WPDFOUTLINE *wpdfoutline)

    {
    wpdfoutline->title=NULL;
    wpdfoutline->next=NULL;
    wpdfoutline->down=NULL;
    wpdfoutline->srcpage=-1;
    wpdfoutline->dstpage=-1;
    }


void wpdfoutline_free(WPDFOUTLINE *wpdfoutline)

    {
    static char *funcname="wpdfoutline_free";

    if (wpdfoutline==NULL)
        return;
    wpdfoutline_free(wpdfoutline->down);
    willus_mem_free((double **)&wpdfoutline->down,funcname);
    wpdfoutline_free(wpdfoutline->next);
    willus_mem_free((double **)&wpdfoutline->next,funcname);
    willus_mem_free((double **)&wpdfoutline->title,funcname);
    wpdfoutline->srcpage=-1;
    wpdfoutline->dstpage=-1;
    }


void wpdfoutline_append(WPDFOUTLINE *outline1,WPDFOUTLINE *outline2)

    {
    WPDFOUTLINE *p;

    for (p=outline1;p->next!=NULL;p=p->next);
    p->next=outline2;
    }


void wpdfoutline_add_to_srcpages(WPDFOUTLINE *outline,int pagecount)

    {
    if (outline==NULL)
        return;
    outline->srcpage += pagecount;
    wpdfoutline_add_to_srcpages(outline->next,pagecount);
    wpdfoutline_add_to_srcpages(outline->down,pagecount);
    }


WPDFOUTLINE *wpdfoutline_read_from_text_file(char *filename)

    {
    FILE *f;
    WPDFOUTLINE *outline0,*outline,*parent[16];
    char buf[512];
    int level,count;
    static char *funcname="wpdfoutline_read_from_text_file";

    for (level=0;level<16;level++)
        parent[level]=NULL;
    f=wfile_fopen_utf8(filename,"r");
    if (f==NULL)
        return(NULL);
    outline0=outline=NULL;
    level=0;
    count=0;
    while (fgets(buf,511,f)!=NULL)
        {
        int llev,i,j;
        WPDFOUTLINE *oline;

        clean_line(buf);
        for (llev=i=0;buf[i]!='\0' && (buf[i]<'0' || buf[i]>'9');i++)
            if (buf[i]=='+')
               llev++;
        for (j=i;buf[j]>='0' && buf[j]<='9';j++);
        count++;
        willus_mem_alloc_warn((void **)&oline,sizeof(WPDFOUTLINE),funcname,10);
        wpdfoutline_init(oline);
        oline->srcpage=atoi(&buf[i])-1;
        clean_line(&buf[j]);
        willus_mem_alloc_warn((void **)&oline->title,strlen(&buf[j])+1,funcname,10);
        strcpy(oline->title,&buf[j]);
        oline->dstpage=-1;
        if (count==1)
            {
            level=llev;
            outline0=outline=oline;
            parent[0]=outline;
            continue;
            }
        if (llev > 15)
            {
            printf("pdfwrite:  Exceeded max outline sub-levels (15).\n");
            exit(10);
            }
        if (llev > level)
            {
            parent[level]=outline;
            outline->down=oline;
            outline=outline->down;
            level=llev;
            continue;
            }
        if (llev!=level)
            {
            for (i=llev;i>=0;i--)
                if (parent[i]!=NULL)
                    {
                    outline=parent[i];
                    break;
                    }
            level=i;
            }
        outline->next=oline;
        outline=outline->next;
        parent[llev]=outline;
        }
    fclose(f);
    return(outline0);
    }
        
        
/*
** srcpage and dstpage start at 1!
*/
void wpdfoutline_set_dstpage(WPDFOUTLINE *outline,int srcpage,int dstpage)

    {
/*
printf("\n@wpdfoutline_set_dstpage(srcpage=%d,dstpage=%d)\n",srcpage,dstpage);
*/
    if (outline==NULL)
        return;
    if (outline->dstpage<0 || outline->srcpage>=srcpage-1)
        outline->dstpage=dstpage-1;
    wpdfoutline_set_dstpage(outline->down,srcpage,dstpage);
    wpdfoutline_set_dstpage(outline->next,srcpage,dstpage);
    }


/*
** pageno starts at 1
**
** Returns outline level where page number is found.  Higher level = more sub-bulleted.
**
*/
int wpdfoutline_includes_srcpage(WPDFOUTLINE *outline,int pageno,int level)

    {
    int status;

    if (outline==NULL)
        return(0);
    if (outline->srcpage==pageno-1)
        return(level);
    if ((status=wpdfoutline_includes_srcpage(outline->down,pageno,level+1))!=0)
        return(status);
    return(wpdfoutline_includes_srcpage(outline->next,pageno,level));
    }


void wpdfoutline_echo(WPDFOUTLINE *outline,int level,int count,FILE *out)

    {
    int i;

    if (outline==NULL)
        return;
    for (i=0;i<level;i++)
        fprintf(out,"    ");
    fprintf(out,"%2d. %s (sp. %d, dp. %d)\n",count,outline->title,outline->srcpage+1,outline->dstpage+1);
    wpdfoutline_echo(outline->down,level+1,1,out);
    wpdfoutline_echo(outline->next,level,count+1,out);
    }


void wpdfoutline_echo2(WPDFOUTLINE *outline,int level,FILE *out)

    {
    int i;

    if (outline==NULL)
        return;
    for (i=0;i<level;i++)
        fprintf(out,"+");
    fprintf(out,"%d %s\n",outline->srcpage+1,outline->title);
    wpdfoutline_echo2(outline->down,level+1,out);
    wpdfoutline_echo2(outline->next,level,out);
    }


int wpdfoutline_num_anchors_on_level(WPDFOUTLINE *outline,int *rc)

    {
    int i,na;

    for (na=i=0;outline!=NULL;i++,outline=outline->next)
        if (outline->next!=NULL)
            na += wpdfoutline_num_anchors_recursive(outline->down)+1;
    if (rc!=NULL)
        (*rc)=na;
    return(i);
    }


int wpdfoutline_num_anchors_recursive(WPDFOUTLINE *outline)

    {
    if (outline==NULL)
        return(0);
    return(wpdfoutline_num_anchors_recursive(outline->down)
            + wpdfoutline_num_anchors_recursive(outline->next)
            + 1);
    }


static int wpdfoutline_within(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    if (outline==NULL)
        return(0);
    if (outline==outline1)
        return(1);
    if (wpdfoutline_within(outline->down,outline1))
        return(1);
    return(wpdfoutline_within(outline->next,outline1));
    }


int wpdfoutline_index(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    int na;

    if (outline==NULL || outline==outline1)
        return(0);
    if (wpdfoutline_within(outline->down,outline1))
        return(1+wpdfoutline_index(outline->down,outline1));
    na=wpdfoutline_num_anchors_recursive(outline->down);
    return(1+na+wpdfoutline_index(outline->next,outline1));
    }


WPDFOUTLINE *wpdfoutline_by_index(WPDFOUTLINE *outline,int n)

    {
    int nr;

    if (n==0 || outline==NULL)
        return(outline);
    if (outline->down!=NULL)
        {
        nr=wpdfoutline_num_anchors_recursive(outline->down);
        if (nr>=n)
            return(wpdfoutline_by_index(outline->down,n-1));
        n-=nr;
        }
    return(wpdfoutline_by_index(outline->next,n-1));
    }


WPDFOUTLINE *wpdfoutline_previous(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    if (outline==NULL)
        return(NULL);
    if (outline->next==outline1)
        return(outline);
    if (wpdfoutline_within(outline->down,outline1))
        return(wpdfoutline_previous(outline->down,outline1));
    return(wpdfoutline_previous(outline->next,outline1));
    }


WPDFOUTLINE *wpdfoutline_parent(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    if (outline==NULL)
        return(NULL);
    if (outline->down==outline1)
        return(outline);
    if (wpdfoutline_within(outline->down,outline1))
        return(wpdfoutline_parent(outline->down,outline1));
    return(wpdfoutline_parent(outline->next,outline1));
    }


int wpdfoutline_fill_in_blank_dstpages(WPDFOUTLINE *outline,int pageno)

    {
    if (outline==NULL)
        return(pageno);
    if (outline->dstpage<0)
        outline->dstpage=pageno-1;
    else
        pageno=outline->dstpage+1;
    pageno=wpdfoutline_fill_in_blank_dstpages(outline->down,pageno);
    return(wpdfoutline_fill_in_blank_dstpages(outline->next,pageno));
    }




/*
int wpdfoutline_includes_srcpage(WPDFOUTLINE *outline,int srcpage)

    {
    if (outline==NULL)
        return(0);
    if (outline->srcpage==srcpage-1)
        return(1);
    if (wpdfoutline_includes_srcpage(outline->down,srcpage))
        return(1);
    return(wpdfoutline_includes_srcpage(outline->next,srcpage));
    }
*/
