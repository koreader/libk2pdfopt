/*
** math.c       Common calculation routines such as sorting, interpolating, etc.
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
#include <math.h>
#include <math.h>
#include "willus.h"

static double interp_high_order_dx_1(double x0,double *x,double *y,int n,
                                     int order,double dx,int ex);




void freexyz(double **x)

    {
    willus_mem_free(x,"freexyz");
    }


/*
** Frees dynamically allocated memory that was allocated by readxy().
*/
void freexy(double **x)

    {
    willus_mem_free(x,"freexy");
    }


/*
** Frees dynamically allocated memory that was allocated by readx().
*/
void freex(double **x)

    {
    willus_mem_free(x,"freex");
    }


/*
** Same as below, but with floating point (single precision) arrays.
*/
float interpxy(float x0,float *x,float *y,long n)

    {
    long    i;

    i=indexx(x0,x,n);
    if (i<0)
        return(y[0]);
    if (i>=n-1)
        return(y[n-1]);
    return(y[i] + (x0-x[i])*(y[i+1]-y[i])/(x[i+1]-x[i]));
    }


/*
** Expects x[] array to be monotonically increasing, and returns
** the index, i, such that x[i] <= x0 < x[i+1].  Uses binomial
** search to find the point quickly.  If x0 < x[0], returns -1.
** If x0 >= x[n-1], returns n-1.
**
** If two or more consecutive x[] values have the same value, and that value
** is x0, the index of the _last_ of the same x[] values (the highest index)
** will be returned.
*/
long indexx(float x0,float *x,long n)

    {
    long    i,delta;

    if (x0<x[0])
        return(-1);
    if (x0>=x[n-1])
        return(n-1);
    i=0;
    if (n>10)
        {
        delta=n>>1;
        while (delta>5)
            {
            for (;i<n && x0>=x[i];i+=delta);
            i-=delta;
            delta>>=1;
            }
        }
    for (;i<n && x0>=x[i];i++);
    return(i-1);
    }


/*
** Expects x[] array to be monotonically increasing, and returns
** the index, i, such that x[i] <= x0 < x[i+1].  Uses binomial
** search to find the point quickly.  If x0 < x[0], returns -1.
** If x0 >= x[n-1], returns n-1.
**
** If two or more consecutive x[] values have the same value, and that value
** is x0, the index of the _last_ of the same x[] values (the highest index)
** will be returned.
*/
long indexxd(double x0,double *x,long n)

    {
    long    i,delta;

    if (x0<x[0])
        return(-1);
    if (x0>=x[n-1])
        return(n-1);
    i=0;
    if (n>10)
        {
        delta=n>>1;
        while (delta>5)
            {
            for (;i<n && x0>=x[i];i+=delta);
            i-=delta;
            delta>>=1;
            }
        }
    for (;i<n && x0>=x[i];i++);
    return(i-1);
    }


/*
** Assumes that x[] is monotonically increasing and returns the
** value y0 corresponding to y0=f(x) where f(x) describes the
** set of line segments connecting all of the points (x[0],y[0]),
** (x[1],y[1]), ..., (x[n-1],y[n-1]).  If x0 is outside the
** bounds of the x[] array, either y[0] or y[n-1] is returned.
**
** Uses binomial search to jump quickly through the x[] array.
**
** NOTE:  x[] array MUST BE MONOTONICALLY INCREASING
**
*/
double interpxyd(double x0,double *x,double *y,long n)

    {
    long    i;

    i=indexxd(x0,x,n);
    if (i<0)
        return(y[0]);
    if (i>=n-1)
        return(y[n-1]);
    return(y[i] + (x0-x[i])*(y[i+1]-y[i])/(x[i+1]-x[i]));
    }




/*
** Reads a double precision (x,y) array from a file,
** dynamically allocating the array space needed.
**
** (*x) returns the x-array pointer
** (*y) returns y-array pointer
** filename is the input file
** (*n) returns the number of points.
** Set output=NULL if you want no printed output, otherwise error
** messages are printed to output (so typically you'd use stderr).
**
** See also:  freexy()
**
** Returns:  0 if all went well.   (*n) gets the number of points.
**          -1 if not enough memory.  (*n) gets the number of points in file.
**          -2 if file would not open.
**          -3 if the first pass and second pass don't jive.
**           If no points in file, returns 0, but (*n)= 0 and no memory
**           is allocated.
**           If anything other than 0 is returned, no memory is allocated.
**
*/

int readxy(char *filename,double **x,double **y,int *n,FILE *output)

    {
    return(readxy_ex(filename,x,y,n,output,1));
    }


int readxy_ex(char *filename,double **x,double **y,int *n,FILE *output,
              int ignore_after_semicolon)

    {
    FILE *f;
    static char *notenough="There is not enough memory to read in file %s.\n";
    static char *internal="Internal error re-reading file %s.\n";
    char    buf[200];
    long    memsize;
    int     i;
    size_t  memsizest;
    static char *funcname="readxy";

    /* Count points in file */

    (*x)=NULL;
    (*y)=NULL;
    (*n)=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        if (output!=NULL)
            fprintf(output,"Cannot open file %s for reading array pairs.\n",
                    filename);
        return(-2);
        }
    while (1)
        {
        double v[2];
        void *ptr;

        if (ignore_after_semicolon)
            ptr=get_line_cf(buf,199,f);
        else
            ptr=fgets(buf,199,f);
        if (ptr==NULL)
            break;
        clean_line(buf);
        if (string_read_doubles(buf,v,2)==2)
            (*n)++;
        }
    fclose(f);
    if ((*n)==0)
        {
        if (output!=NULL)
            fprintf(output,"There are no array pairs in file %s.\n",filename);
        return(0);
        }
    memsize=(long)sizeof(double)*(*n)*2L;
    memsizest=(size_t)memsize;
    if (memsize!=memsizest)
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    if (!willus_mem_alloc(x,memsizest,funcname))
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    (*y)= &(*x)[(*n)];
    i=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        freexy(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    while (1)
        {
        double v[2];
        void *ptr;

        if (ignore_after_semicolon)
            ptr=get_line_cf(buf,199,f);
        else
            ptr=fgets(buf,199,f);
        if (ptr==NULL)
            break;
        clean_line(buf);
        if (string_read_doubles(buf,v,2)==2)
            {
            if (i>=(*n))
                {
                fclose(f);
                freexy(x);
                if (output!=NULL)
                    fprintf(output,internal,filename);
                return(-3);
                }
            (*x)[i]=v[0];
            (*y)[i]=v[1];
            i++;
            }
        }
    fclose(f);
    if (i!=(*n))
        {
        freexy(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    return(0);
    }


int readx(char *filename,double **x,int *n,FILE *output)

    {
    FILE *f;
    static char *notenough="There is not enough memory to read in file %s.\n";
    static char *internal="Internal error re-reading file %s.\n";
    char    buf[200];
    long    memsize;
    int     i;
    size_t  memsizest;
    static char *funcname="readx";

    /* Count points in file */

    (*x)=NULL;
    (*n)=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        if (output!=NULL)
            fprintf(output,"Cannot open file %s for reading array pairs.\n",
                    filename);
        return(-2);
        }
    while (get_line_cf(buf,199,f)!=NULL)
        {
        clean_line(buf);
        if (is_a_number(buf))
            (*n)++;
        }
    fclose(f);
    if ((*n)==0)
        {
        if (output!=NULL)
            fprintf(output,"There are no array pairs in file %s.\n",filename);
        return(0);
        }
    memsize=(long)sizeof(double)*(*n);
    memsizest=(size_t)memsize;
    if (memsize!=memsizest)
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    if (!willus_mem_alloc(x,memsizest,funcname))
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    i=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        freex(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    while (get_line_cf(buf,199,f)!=NULL)
        {
        clean_line(buf);
        if (is_a_number(buf))
            {
            if (i>=(*n))
                {
                fclose(f);
                freex(x);
                if (output!=NULL)
                    fprintf(output,internal,filename);
                return(-3);
                }
            (*x)[i]=atof(buf);
            i++;
            }
        }
    fclose(f);
    if (i!=(*n))
        {
        freexy(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    return(0);
    }


/*
** Reads double precision (x,y,z) triplets from a file into x[],y[],z[]
** arrays, dynamically allocating the array space needed.  See readxy().
*/
int readxyz(char *filename,double **x,double **y,double **z,int *n,FILE *output)

    {
    return(readxyz_ex(filename,x,y,z,n,output,1));
    }


int readxyz_ex(char *filename,double **x,double **y,double **z,int *n,FILE *output,
               int ignore_after_semicolon)

    {
    FILE *f;
    static char *notenough="There is not enough memory to read in file %s.\n";
    static char *internal="Internal error re-reading file %s.\n";
    char    buf[200];
    long    memsize;
    int     i;
    size_t  memsizest;
    static char *funcname="readxyz";

    /* Count points in file */

    (*x)=NULL;
    (*y)=NULL;
    (*z)=NULL;
    (*n)=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        if (output!=NULL)
            fprintf(output,"Cannot open file %s for reading array triplets.\n",
                    filename);
        return(-2);
        }
    while (1)
        {
        void *ptr;
        double v[3];

        if (ignore_after_semicolon)
            ptr=get_line_cf(buf,199,f);
        else
            ptr=fgets(buf,199,f);
        if (ptr==NULL)
            break;
        clean_line(buf);
        if (string_read_doubles(buf,v,3)==3)
            (*n)++;
        }
    fclose(f);
    if ((*n)==0)
        {
        if (output!=NULL)
            fprintf(output,"There are no array triplets in file %s.\n",filename);
        return(0);
        }
    memsize=(long)sizeof(double)*(*n)*3L;
    memsizest=(size_t)memsize;
    if (memsize!=memsizest)
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    if (!willus_mem_alloc(x,memsizest,funcname))
        {
        if (output!=NULL)
            fprintf(output,notenough,filename);
        return(-1);
        }
    (*y)= &(*x)[(*n)];
    (*z)= &(*y)[(*n)];
    i=0;
    f=fopen(filename,"r");
    if (f==NULL)
        {
        freexyz(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    while (1)
        {
        double v[3];
        void *ptr;

        if (ignore_after_semicolon)
            ptr=get_line_cf(buf,199,f);
        else
            ptr=fgets(buf,199,f);
        if (ptr==NULL)
            break;
        clean_line(buf);
        if (string_read_doubles(buf,v,3)==3)
            {
            if (i>=(*n))
                {
                fclose(f);
                freexyz(x);
                if (output!=NULL)
                    fprintf(output,internal,filename);
                return(-3);
                }
            (*x)[i]=v[0];
            (*y)[i]=v[1];
            (*z)[i]=v[2];
            i++;
            }
        }
    fclose(f);
    if (i!=(*n))
        {
        freexyz(x);
        if (output!=NULL)
            fprintf(output,internal,filename);
        return(-3);
        }
    return(0);
    }




void sort(float *x,int n)

    {
    int top,n1;
    float   x0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


void sortd(double *x,int n)

    {
    int top,n1;
    double  x0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


void sorti(int *x,int n)

    {
    int top,n1;
    int x0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }



void sortxyi(int *x,int *y,int n)

    {
    int     top,n1;
    int     x0,y0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        }
        }
    }


void sortxyzi(int *x,int *y,int *z,int n)

    {
    int     top,n1;
    int     x0,y0,z0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            z0=z[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            z0=z[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            z[n1]=z[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                z[0]=z0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        }
        }
    }


void sortxy(float *x,float *y,int n)

    {
    int     top,n1;
    float   x0,y0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        }
        }
    }


void sortxyz(float *x,float *y,float *z,int n)

    {
    int     top,n1;
    float   x0,y0,z0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            z0=z[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            z0=z[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            z[n1]=z[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                z[0]=z0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        }
        }
    }




void sortxyzd(double *x,double *y,double *z,int n)

    {
    int     top,n1;
    double  x0,y0,z0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            z0=z[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            z0=z[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            z[n1]=z[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                z[0]=z0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        }
        }
    }


void sortxyzwvd(double *x,double *y,double *z,double *w,double *v,int n)

    {
    int     top,n1;
    double  x0,y0,z0,w0,v0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            z0=z[top];
            w0=w[top];
            v0=v[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            z0=z[n1];
            w0=w[n1];
            v0=v[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            z[n1]=z[0];
            w[n1]=w[0];
            v[n1]=v[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                z[0]=z0;
                w[0]=w0;
                v[0]=v0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                w[parent]=w[child];
                v[parent]=v[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        w[parent]=w0;
        v[parent]=v0;
        }
        }
    }


void sortxyzwd(double *x,double *y,double *z,double *w,int n)

    {
    int     top,n1;
    double  x0,y0,z0,w0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            z0=z[top];
            w0=w[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            z0=z[n1];
            w0=w[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            z[n1]=z[0];
            w[n1]=w[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                z[0]=z0;
                w[0]=w0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                w[parent]=w[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        w[parent]=w0;
        }
        }
    }


/*
** Generic sort from 1 to 8 double-precision arrays.  All double precision
** array arguments except the first one can be NULL.  Lots of if-statements,
** so slower than other sort functions in this module.
*/
void sort8d(double *x,double *y,double *z,double *a,double *b,double *c,double *e,double *f,int n)

    {
    int     top,n1;
    double  x0,y0,z0,a0,b0,c0,e0,f0;
 
    x0=y0=z0=a0=b0=c0=e0=f0=0.; /* Avoid compiler warning */
    if (n<2)
        return;
    if (x==NULL)
        return;
    /* Use faster sorts if available */
    if (y==NULL && z==NULL && a==NULL && b==NULL && c==NULL && e==NULL && f==NULL)
        {
        sortd(x,n);
        return;
        }
    if (y!=NULL && z==NULL && a==NULL && b==NULL && c==NULL && e==NULL && f==NULL)
        {
        sortxyd(x,y,n);
        return;
        }
    if (y!=NULL && z!=NULL && a==NULL && b==NULL && c==NULL && e==NULL && f==NULL)
        {
        sortxyzd(x,y,z,n);
        return;
        }
    if (y!=NULL && z!=NULL && a!=NULL && b==NULL && c==NULL && e==NULL && f==NULL)
        {
        sortxyzwd(x,y,z,a,n);
        return;
        }
    if (y!=NULL && z!=NULL && a!=NULL && b!=NULL && c==NULL && e==NULL && f==NULL)
        {
        sortxyzwvd(x,y,z,a,b,n);
        return;
        }
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            if (y!=NULL)
                y0=y[top];
            if (z!=NULL)
                z0=z[top];
            if (a!=NULL)
                a0=a[top];
            if (b!=NULL)
                b0=b[top];
            if (c!=NULL)
                c0=c[top];
            if (e!=NULL)
                e0=e[top];
            if (f!=NULL)
                f0=f[top];
            }
        else
            {
            x0=x[n1];
            if (y!=NULL)
                y0=y[n1];
            if (z!=NULL)
                z0=z[n1];
            if (a!=NULL)
                a0=a[n1];
            if (b!=NULL)
                b0=b[n1];
            if (c!=NULL)
                c0=c[n1];
            if (e!=NULL)
                e0=e[n1];
            if (f!=NULL)
                f0=f[n1];
            x[n1]=x[0];
            if (y!=NULL)
                y[n1]=y[0];
            if (z!=NULL)
                z[n1]=z[0];
            if (a!=NULL)
                a[n1]=a[0];
            if (b!=NULL)
                b[n1]=b[0];
            if (c!=NULL)
                c[n1]=c[0];
            if (e!=NULL)
                e[n1]=e[0];
            if (f!=NULL)
                f[n1]=f[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                if (y!=NULL)
                    y[0]=y0;
                if (z!=NULL)
                    z[0]=z0;
                if (a!=NULL)
                    a[0]=a0;
                if (b!=NULL)
                    b[0]=b0;
                if (c!=NULL)
                    c[0]=c0;
                if (e!=NULL)
                    e[0]=e0;
                if (f!=NULL)
                    f[0]=f0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                if (y!=NULL)
                    y[parent]=y[child];
                if (z!=NULL)
                    z[parent]=z[child];
                if (a!=NULL)
                    a[parent]=a[child];
                if (b!=NULL)
                    b[parent]=b[child];
                if (c!=NULL)
                    c[parent]=c[child];
                if (e!=NULL)
                    e[parent]=e[child];
                if (f!=NULL)
                    f[parent]=f[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        if (y!=NULL)
            y[parent]=y0;
        if (z!=NULL)
            z[parent]=z0;
        if (a!=NULL)
            a[parent]=a0;
        if (b!=NULL)
            b[parent]=b0;
        if (c!=NULL)
            c[parent]=c0;
        if (e!=NULL)
            e[parent]=e0;
        if (f!=NULL)
            f[parent]=f0;
        }
        }
    }


void sortxyd(double *x,double *y,int n)

    {
    int     top,n1;
    double  x0,y0;

    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            y0=y[top];
            }
        else
            {
            x0=x[n1];
            y0=y[n1];
            x[n1]=x[0];
            y[n1]=y[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                y[0]=y0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child]<x[child+1])
                child++;
            if (x0<x[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        y[parent]=y0;
        }
        }
    }


/*
** Compress algorithm.  Averages duplicate inputs.  x[] array is expected
** to be monotonically increasing.
**
*/
void compressxy(double *x,double *y,int *n)

    {
    int     delta,i,j;
    double    sum;

    for (i=0;i<(*n)-1;i++)
        {
        if (x[i]!=x[i+1])
            continue;
        sum=y[i]+y[i+1];
        for (j=i+2;j<(*n) && x[j]==x[i];j++)
            sum += y[j];
        delta=j-i-1;
        y[i] = sum/(double)(delta+1);
        for (;j<(*n);j++)
            {
            x[j-delta]=x[j];
            y[j-delta]=y[j];
            }
        (*n) -= delta;
        }
    }

/*
** Compress algorithm.  Averages duplicate inputs.  x[] array is expected
** to be monotonically increasing.
**
*/
void compressxyz(double *x,double *y,double *z,int *n)

    {
    int     delta,i,j;
    double    sumy,sumz;

    for (i=0;i<(*n)-1;i++)
        {
        if (x[i]!=x[i+1])
            continue;
        sumy=y[i]+y[i+1];
        sumz=z[i]+z[i+1];
        for (j=i+2;j<(*n) && x[j]==x[i];j++)
            {
            sumy += y[j];
            sumz += z[j];
            }
        delta=j-i-1;
        y[i] = sumy/(double)(delta+1);
        z[i] = sumz/(double)(delta+1);
        for (;j<(*n);j++)
            {
            x[j-delta]=x[j];
            y[j-delta]=y[j];
            z[j-delta]=z[j];
            }
        (*n) -= delta;
        }
    }


/*
** Compress algorithm.  Removes duplicates.   x[] array is expected
** to be monotonically increasing.
**
*/
void compressx(double *x,int *n)

    {
    int     delta,i,j;

    for (i=0;i<(*n)-1;i++)
        {
        if (x[i]!=x[i+1])
            continue;
        for (j=i+2;j<(*n) && x[j]==x[i];j++);
        delta=j-i-1;
        for (;j<(*n);j++)
            x[j-delta]=x[j];
        (*n) -= delta;
        }
    }

/*
** Return 3D distance between point (x,y,z) and line segment
** (x1,y1,z1)-(x2,y2,z2).
*/
double line_segment_dist_3d(double x1,double y1,double z1,
                            double x2,double y2,double z2,
                            double x,double y,double z)

    {
    double   d1,d2,d3,xx,yy;
    d1=point_distance_3d(x,y,z,x1,y1,z1);
    d2=point_distance_3d(x,y,z,x2,y2,z2);
    d3=point_distance_3d(x1,y1,z1,x2,y2,z2);
    xx=(d2*d2-d1*d1-d3*d3)/(-2*d3);
    yy=d1*d1-xx*xx;
    yy= yy<0 ? 0. : sqrt(yy);
    return(line_segment_dist_2d(0.,0.,d3,0.,xx,yy));
    }


/*
** Return 2D distance between point (z,r) and line segment (z0,r0)-(z1,r1).
*/
double line_segment_dist_2d(double z0,double r0,double z1,double r1,
                            double z,double r)

    {
    return(line_segment_dist_2dx(z0,r0,z1,r1,z,r,NULL));
    }


/*
** Return 2D distance between point (z,r) and line segment (z0,r0)-(z1,r1).
** If f is not NULL, it gets the fractional point along the segment
** that the point is closest to.  E.g. f==0 corresponds to (z0,r0),
** and f==1 corresonds to (z1,r1), and f==0.5 corresonds to the midpoint.
*/
double line_segment_dist_2dx(double z0,double r0,double z1,double r1,
                             double z,double r,double *f)

    {
    double  x,y,x0,y0,x1,y1,xmin,ymin,m,b;
    int     swapped;

    if (z0==z1 && r0==r1)
        {
        if (f!=NULL)
            (*f)=0.;
        return(point_distance_2d(z0,r0,z,r));
        }

    if (fabs(z0-z1)<fabs(r0-r1))
        { x0=r0; x1=r1; x=r; y0=z0; y1=z1; y=z; }
    else
        { x0=z0; x1=z1; x=z; y0=r0; y1=r1; y=r; }

    if (x0>x1)
        {
        double t;
        t=x0;
        x0=x1;
        x1=t;
        t=y0;
        y0=y1;
        y1=t;
        swapped=1;
        }
    else
        swapped=0;
    m=(y1-y0)/(x1-x0);
    b=y0-m*x0;
    xmin=(x+m*y-m*b)/(m*m+1);
    if (xmin<x0)
        {
        if (f!=NULL)
            (*f)=swapped ? 1. : 0.;
        return(point_distance_2d(x0,y0,x,y));
        }
    if (xmin>x1)
        {
        if (f!=NULL)
            (*f)=swapped ? 0. : 1.;
        return(point_distance_2d(x1,y1,x,y));
        }
    ymin=m*xmin+b;
    if (f!=NULL)
        {
        (*f) = point_distance_2d(xmin,ymin,x0,y0)
               / point_distance_2d(x1,y1,x0,y0);
        if (swapped)
            (*f) = 1.-(*f);
        }
    return(point_distance_2d(xmin,ymin,x,y));
    }


/*
** Return distance between 3D points (x1,y1,z1) and (x2,y2,z2)
*/
double point_distance_3d(double x1,double y1,double z1,
                         double x2,double y2,double z2)

    {
    return(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)));
    }


/*
** Return distance between 2D points (x1,y1) and (x2,y2)
*/
double point_distance_2d(double x1,double y1,double x2,double y2)

    {
    return(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
    }


/*
** High order interpolation routine.  Interpolates a set of x[],y[]
** points (n of them) using polynomials of order "order", with a
** sliding fit over "nlocal" points at a time.
*/
double interp_high_order(double x0,double *x,double *y,int n,int order,
                         int nlocal)

    {
    int     i0,maxfits,i,j;
    double  c[16];
    double  f,wsum,ysum,ymean,xx;

    if (order>15)
        order=15;
    i0 = indexxd(x0,x,n);
    if (i0<0)
        return(y[0]);
    if (i0>=n-1)
        return(y[n-1]);
    if (x[i0]==x[i0+1])
        f=0.;
    else
        f=(x0-x[i0])/(x[i0+1]-x[i0]);
    maxfits = nlocal-order+1;
    for (ysum=0.,wsum=0.,i=0;i<maxfits;i++)
        {
        int     istart;
        double  w0,y0;

        istart = i0+i+1-maxfits;
        if (istart<0 || istart+nlocal-1>n-1)
            continue;
        wpolyfitd(&x[istart],&y[istart],nlocal,order,c);
        w0 = 0.5-fabs((i0+f-istart)/(nlocal-1.) - 0.5);
        for (y0=0.,xx=1.,j=0;j<=order;y0+=xx*c[j],xx*=x0,j++);
        ysum += y0*w0;
        wsum += w0;
        }
    if (wsum==0.)
        return(interpxyd(x0,x,y,n));
    ymean=ysum/wsum;
    return(ymean);
    }


double interp_high_order_dx(double x0,double *x,double *y,int n,int order,
                            double dx)

    {
    return(interp_high_order_dx_1(x0,x,y,n,order,dx,0));
    }


double interp_high_order_dx_ex(double x0,double *x,double *y,int n,int order,
                               double dx)

    {
    return(interp_high_order_dx_1(x0,x,y,n,order,dx,1));
    }


/*
** Similar to interp_high_order(), but uses dx range rather than a
** number of points range.
*/
static double interp_high_order_dx_1(double x0,double *x,double *y,int n,
                                     int order,double dx,int ex)

    {
    int     i0,i1,j,nlocal;
    double  c[16];
    double  xi,xx,y0;

    if (order>15)
        order=15;
    dx=fabs(dx)/2.;
    xi=x0;
    if (x0<x[0])
        x0=x[0];
    else if (x0>x[n-1])
        x0=x[n-1];
    i0 = indexxd(x0,x,n);
    if (i0<0)
        {
        if (ex)
            i0=0;
        else
            return(y[0]);
        }
    if (i0>=n-1)
        {
        if (ex)
            i0=n-1;
        else
            return(y[n-1]);
        }
    for (i1=i0;i0>=0 && x[i0]+dx >= x0;i0--);
    i0++;
    for (;i1<n && x[i1]-dx <= x0;i1++);
    i1--;
    nlocal = (i1-i0)+1;
    if (nlocal < 2)
        return(interpxyd(x0,x,y,n));
    if (nlocal < order+1)
        order=nlocal-1;
    wpolyfitd(&x[i0],&y[i0],nlocal,order,c);
    if (ex)
        x0=xi;
    for (y0=0.,xx=1.,j=0;j<=order;y0+=xx*c[j],xx*=x0,j++);
    return(y0);
    }


/*
** Similar to interp_high_order(), but uses dx range rather than a
** number of points range.
*/
double interp_high_order_dxf(double x0,float *x,float *y,int n,int order,
                             double dx)

    {
    int     i0,i1,j,nlocal;
    double  c[16];
    double  xx,y0;
    void   *ptr;
    double *x1,*y1;
    static char *funcname="interp_high_order_dxf";

    if (order>15)
        order=15;
    dx=fabs(dx)/2.;
    if (x0<x[0])
        x0=x[0];
    else if (x0>x[n-1])
        x0=x[n-1];
    i0 = indexx(x0,x,n);
    if (i0<0)
        return(y[0]);
    if (i0>=n-1)
        return(y[n-1]);
    for (i1=i0;i0>=0 && x[i0]+dx >= x0;i0--);
    i0++;
    for (;i1<n && x[i1]-dx <= x0;i1++);
    i1--;
    nlocal = (i1-i0)+1;
    if (nlocal < 2)
        return(interpxy(x0,x,y,n));
    if (nlocal < order+1)
        order=nlocal-1;
    willus_mem_alloc_warn(&ptr,sizeof(double)*2*nlocal,funcname,10);
    x1=(double *)ptr;
    y1=&x1[nlocal];
    for (j=0;j<nlocal;j++)
        {
        x1[j]=x[j+i0];
        y1[j]=y[j+i0];
        }
    wpolyfitd(x1,y1,nlocal,order,c);
    willus_mem_free(&x1,funcname);
    for (y0=0.,xx=1.,j=0;j<=order;y0+=xx*c[j],xx*=x0,j++);
    return(y0);
    }


/*
** Similar to interp_high_order(), but uses dx range rather than a
** number of points range.
*/
double interp_high_order_slope(double x0,double *x,double *y,int n,int order,
                               double dx)

    {
    int     i0,i1,j,nlocal;
    double  c[16];
    double  xx,y0;
    void   *ptr;
    double *x1,*y1;
    static char *funcname="interp_high_order_slope";

    if (order>15)
        order=15;
    dx=fabs(dx)/2.;
    if (x0<x[0])
        x0=x[0];
    else if (x0>x[n-1])
        x0=x[n-1];
    i0 = indexxd(x0,x,n);
    if (i0<0)
        i0=0;
    if (i0>n-1)
        i0=n-1;
    for (i1=i0;i0>=0 && x[i0]+dx >= x0;i0--);
    i0++;
    for (;i1<n && x[i1]-dx <= x0;i1++);
    i1--;
    nlocal = (i1-i0)+1;
    if (nlocal < 2)
        {
        if (i0>=n-1)
            i0--;
        if (i0<0)
            return(0.);
        return((y[i0+1]-y[i0])/(x[i0+1]-x[i0]));
        }
    if (nlocal < order+1)
        order=nlocal-1;
    willus_mem_alloc_warn(&ptr,sizeof(double)*2*nlocal,funcname,10);
    x1=(double *)ptr;
    y1=&x1[nlocal];
    for (j=0;j<nlocal;j++)
        {
        x1[j]=x[j+i0];
        y1[j]=y[j+i0];
        }
    wpolyfitd(x1,y1,nlocal,order,c);
    willus_mem_free(&x1,funcname);
    for (y0=0.,xx=1.,j=1;j<=order;y0+=j*xx*c[j],xx*=x0,j++);
    return(y0);
    }


/*
** Similar to interp_high_order(), but uses dx range rather than a
** number of points range.
*/
double interp_high_order_slopef(double x0,float *x,float *y,int n,int order,
                                double dx)

    {
    int     i0,i1,j,nlocal;
    double  c[16];
    double  xx,y0;
    void   *ptr;
    double *x1,*y1;
    static char *funcname="interp_high_order_slopef";

    if (order>15)
        order=15;
    dx=fabs(dx)/2.;
    if (x0<x[0])
        x0=x[0];
    else if (x0>x[n-1])
        x0=x[n-1];
    i0 = indexx(x0,x,n);
    if (i0<0)
        i0=0;
    if (i0>n-1)
        i0=n-1;
    for (i1=i0;i0>=0 && x[i0]+dx >= x0;i0--);
    i0++;
    for (;i1<n && x[i1]-dx <= x0;i1++);
    i1--;
    nlocal = (i1-i0)+1;
    if (nlocal < 2)
        {
        if (i0>=n-1)
            i0--;
        if (i0<0)
            return(0.);
        return((y[i0+1]-y[i0])/(x[i0+1]-x[i0]));
        }
    if (nlocal < order+1)
        order=nlocal-1;
    willus_mem_alloc_warn(&ptr,sizeof(double)*2*nlocal,funcname,10);
    x1=(double *)ptr;
    y1=&x1[nlocal];
    for (j=0;j<nlocal;j++)
        {
        x1[j]=x[j+i0];
        y1[j]=y[j+i0];
        }
    wpolyfitd(x1,y1,nlocal,order,c);
    willus_mem_free(&x1,funcname);
    for (y0=0.,xx=1.,j=1;j<=order;y0+=j*xx*c[j],xx*=x0,j++);
    return(y0);
    }


int willusmath_is_inf(double x)

    {
    double y;

    if (x==0.)
        return(0.);
    y=1./x;
    return(y==0.);
    }


int willusmath_is_nan(double x)

    {
    int isz,isgt,islt;
    isz = (x==0.);
    isgt = (x>0.);
    islt = (x<0.);
    return((isz && isgt) || (isz && islt) || (isgt && islt) || (!isz && !isgt && !islt));
    }


double hammersley(int index,int base)

    {
    int   N,sum;

    for (N=1,sum=0;index>0;)
        {
        int digit;
        digit = index%base;
        sum = sum*base + digit;
        N *= base;
        index = (int)(index/base);
        }
    return((double)sum/N);
    }


void vector_alloc(void **ptr,int element_size,int d1)

    {
    int dims[1];
    dims[0]=d1;
    vector_nd_alloc(ptr,element_size,dims,1);
    }


void vector_free(void **ptr,int d1)

    {
    int dims[1];
    dims[0]=d1;
    vector_nd_free(ptr,dims,1);
    }


void vector_2d_alloc(void **ptr,int element_size,int d1,int d2)

    {
    int dims[2];
    dims[0]=d1;
    dims[1]=d2;
    vector_nd_alloc(ptr,element_size,dims,2);
    }


void vector_2d_free(void **ptr,int d1,int d2)

    {
    int dims[2];
    dims[0]=d1;
    dims[1]=d2;
    vector_nd_free(ptr,dims,2);
    }

/*
** Sample use:
** double ***array3d;
** vector_3d_alloc((void **)&array3d,sizeof(double),16,16,16);
** array3d[i][j][k] = i*j+k;
** vector_3d_free((void **)&array3d,16,16,16);
*/
void vector_3d_alloc(void **ptr,int element_size,int d1,int d2,int d3)

    {
    int dims[3];
    dims[0]=d1;
    dims[1]=d2;
    dims[2]=d3;
    vector_nd_alloc(ptr,element_size,dims,3);
    }


void vector_3d_free(void **ptr,int d1,int d2,int d3)

    {
    int dims[3];
    dims[0]=d1;
    dims[1]=d2;
    dims[2]=d3;
    vector_nd_free(ptr,dims,3);
    }


/*
** Recursive function.
*/
void vector_nd_free(void **ptr,int *dims,int n)

    {
    static char *funcname="vector_nd_free";
    int i;
    void **p2;

    if (n==1)
        {
        willus_mem_free((double **)ptr,funcname);
        return;
        }
    p2=(void **)(*ptr);
    for (i=0;i<dims[0];i++)
        vector_nd_free(&p2[i],&dims[1],n-1);
    willus_mem_free((double **)ptr,funcname);
    }


/*
** Recursive function.
*/
void vector_nd_alloc(void **ptr,int element_size,int *dims,int n)

    {
    static char *funcname="vector_nd_alloc";
    int i;
    void **p2;

    if (n==1)
        {
        willus_mem_alloc_warn(ptr,element_size*dims[0],funcname,10);
        return;
        }
    willus_mem_alloc_warn(ptr,sizeof(void *)*dims[0],funcname,10);
    p2=(void **)(*ptr);
    for (i=0;i<dims[0];i++)
        vector_nd_alloc(&p2[i],element_size,&dims[1],n-1);
    }


/*
** Return x mod modval.  Returned value is between 0 and modval even
** if x is negative.  For example, afmod(-0.5,2.0) = 1.5
*/
double afmod(double x,real modval)

    {
    x=fmod(x,modval);
    if (x<0)
        x+=modval;
    return(x);
    }
