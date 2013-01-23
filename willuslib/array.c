/*
** array.c      Functions to manipulate/analyze 1-D double precision and
**              single precision arrays.
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
#include <time.h>
#include <math.h>
#include "willus.h"


static double findminfitd1(double *x,double *y,int n,double x0,double dx,
                           double *rmserr,int *npfit,double *ymin);
static int array_recfft(double *xr,double *xi,int length);
static int arrayf_recfft(float *xr,float *xi,int length);
static double digit_reverse(int x,int base);

/*
**
** Compute mean and standard deviation
**
*/
double array_mean(double *a,int n,double *mean,double *stddev)

    {
    int     i;
    double  sum,avg,sum_sq;

    if (n<1)
        return(0.);
    for (sum=sum_sq=i=0;i<n;i++)
        sum += a[i];
    avg = sum/n;
    if (mean!=NULL)
        (*mean) = avg;
    if (stddev!=NULL)
        {
        double sum_sq;

        for (sum_sq=i=0;i<n;i++)
            sum_sq += (a[i]-avg)*(a[i]-avg);
        (*stddev) = sqrt(sum_sq/n);
        }
    return(avg);
    }


double arrayi_mean(int *a,int n,double *mean,double *stddev)

    {
    int     i;
    double  sum,avg,sum_sq;

    if (n<1)
        return(0.);
    for (sum=sum_sq=i=0;i<n;i++)
        sum += a[i];
    avg = sum/n;
    if (mean!=NULL)
        (*mean) = avg;
    if (stddev!=NULL)
        {
        double sum_sq;

        for (sum_sq=i=0;i<n;i++)
            sum_sq += (a[i]-avg)*(a[i]-avg);
        (*stddev) = sqrt(sum_sq/n);
        }
    return(avg);
    }


double array_weighted_mean(double *a,double *w,int n)

    {
    int i;
    double sum,wsum;

    if (n<=0)
        return(0.);
    for (wsum=sum=0.,i=0;i<n;i++)
        {
        wsum += w[i];
        sum += a[i]*w[i];
        }
    return(wsum==0. ? 0. : sum/wsum);
    }


double array_weighted_stddev(double *a,double *w,int n)

    {
    int i;
    double sumsq,sum,wsum,mean,meansq;

    if (n<=0)
        return(0.);
    for (sumsq=wsum=sum=0.,i=0;i<n;i++)
        {
        wsum += w[i];
        sum += a[i]*w[i];
        sumsq += a[i]*a[i]*w[i];
        }
    if (wsum==0.)
        return(0.);
    mean = sum/wsum;
    meansq = sumsq/wsum;
    return(sqrt(fabs(meansq-mean*mean)));
    }
    

void array_force_weighted_stddev(double *a,double *w,int n,double newstddev)

    {
    double mean,oldstddev,f;
    int i;

    mean=array_weighted_mean(a,w,n);
    oldstddev=array_weighted_stddev(a,w,n);
    f=newstddev/oldstddev;
    for (i=0;i<n;i++)
        a[i] = mean + (a[i]-mean)*f;
    }


void array_copy(double *dst,double *src,int n)

    {
    memcpy((void *)dst,(void *)src,n*sizeof(double));
    }


void array_fabs(double *a,int n)

    {
    int i;

    for (i=0;i<n;i++)
        a[i]=fabs(a[i]);
    }


void array_set(double *a,int n,double value)

    {
    int i;

    for (i=0;i<n;i++)
        a[i]=value;
    }


void array_scale(double *a,int n,double scale_factor,double offset)

    {
    int i;

    for (i=0;i<n;a[i]=a[i]*scale_factor+offset,i++);
    }


void array_center(double *a,int n)

    {
    double min,max,delta;

    min=array_min(a,n);
    max=array_max(a,n);
    delta=(min+(1-max))/2.-min;
    array_scale(a,n,1.0,delta);
    }


double array_rms(double *a,int n)

    {
    int     i;
    double  sum;

    for (sum=i=0;i<n;i++)
        sum += a[i]*a[i];
    return(sqrt(sum/n));
    }


double array_max(double *a,int n)

    {
    double max;
    int  i;

    for (max=a[0],i=1;i<n;i++)
        if (max<a[i])
            max=a[i];
    return(max);
    }


double array_min(double *a,int n)

    {
    double min;
    int  i;

    for (min=a[0],i=1;i<n;i++)
        if (min>a[i])
            min=a[i];
    return(min);
    }


void array_sort(double *a,int n)

    {
    sortd(a,(long)n);
    }


/*
** n must be >= sw.
** The array is shifted so that a_new[n] = a_old[n+hw],
** where hw = (int)((sw-1)/2)
** e.g. if the sliding window width is 3, then hw = 1.
** The size of the array is reduced by sw-1.
*/
int array_sliding_window(double *a,int n,int sw)

    {
    double sum;
    int i,j;

    /* hw=(sw-1)/2; */
    for (sum=0.,i=0;i<sw && i<n;i++)
        sum += a[i];
    if (i>n)
        return(0);
    for (j=0;i<=n;i++,j++)
        {
        double t;
        t=sum/sw;
        if (i<n)
            sum+=a[i]-a[i-sw];
        a[j]=t;
        }
    return(j);
    }


/*
**
** Loads an n-element array (0..n-1) uniformly with a Hammersley's
** sequence.
**
** If load_type=="all_one", the array is zeroed.
**
** If load_type=="random", the array is loaded entirely randomly with values
**             x, such that 0 <= x < 1, using the rand() function.
**
** If load_type=="uniform", the array is loaded uniformly from array[0]=0/n to
**             array[n-1]=(n-1)/n.
**
** If load_type=="hbase_n", the arrays is loaded with Hammersley sequence
**                     n.  E.g. load_type=="hbase_3".
**
** Explanation of Hammersley sequence of base 3 (see also digit_reverse):
** -----------------------------------------------------------------------
** Let m be the smallest integer such that 3^m >= n.
** Let M = 3^m.
** Then for each element i, convert i to base 3, reverse the m
** least significant base 3 digits, and divide by M.  Assign that
** value to that element.
**
** The best number of array elements to use for a Hammersley's
** sequence is base^n, where n is an integer.
**
*/
void array_load(double *array,int n,char *loadtype)

    {
    static char *err= "array_load:  ";
    int     i,base;
    static int randomized=0;

    if (!strcmp(loadtype,"all_one"))
        {
        array_set(array,n,0.);
        return;
        }
    if (!strcmp(loadtype,"random"))
        {
        if (!randomized)
            {
            srand((unsigned)time(NULL)); /* seed the random number generator */
            rand();
            randomized=1;
            }
        for (i=0;i<n;i++)
            array[i]=(double)rand()/((double)RAND_MAX+1.);
        return;
        }
    if (!strcmp(loadtype,"uniform"))
        {
        for (i=0;i<n;i++)
            array[i]=(double)i/n;
        return;
        }
    if (strncmp(loadtype,"hbase",5))
        {
        fprintf(stderr,"%sUnknown load type %s.\n",err,loadtype);
        return;
        }
    base = atoi(loadtype[5]=='_' ? &loadtype[6] : &loadtype[5]);
    if (base<2)
        {
        fprintf(stderr,"%sBad Hammersley base %d.\n",err,base);
        return;
        }
    for (i=0;i<n;i++)
        {
        // The (i+base-1)%n part seems to keep multiple arrays from
        // having a correlated 0 index, otherwise all values at index 0
        // are the smallest in the distribution.
        array[i]=digit_reverse((i+base-1)%n,base);
#ifdef _CRAY
        if (array[i]<0 || array[i]>1)
            printf("array[%ld]=%g!  Bad Hammersley loading!\n",i,array[i]);
#endif
        }
    }


/*
**
** digit_reverse(long x,int base)
**
** Treats x as base "base" and flips the digits about the decimal
** point.  These values, when computed for a consecutive sequence
** of integers for a given base, are known as Hammersley's sequence
** for that base.
**
** Examples:  If x==1234 and base==10, then the returned value is
**            0.4321.
**
**            If x==34 (base 10) and base==3, then x base 3 is 1021,
**            so the returned value is 0.1201 base 3 = 1/3+2/9+1/81 base 10.
**
*/
static double digit_reverse(int x,int base)

    {
    double    result;
    int     inverse_multiplier;

    result=0.;
    inverse_multiplier=base;
    while (x)
        {
        result += (double)(x%base)/(double)inverse_multiplier;
        inverse_multiplier *= base;
        x /= base;
        }
    return(result);
    }




/*
** Find the x-position minimum of an (x,y) curve by fitting a parabola
** about the minimum point in the array.  If the parabolic fit does
** not work well, then the x[] array value corresponding to the minimum
** y[] point is returned.
** dxmax is the maximum x-range that will be fitted over.
** errmax is the maximum allowable normalized rms error for the fit.
** If this error is exceeded, the minimum point will be returned
** without using any fitting.
**
** If err is not NULL, it gets the normalized rms error of the fit.
** If npf is not NULL, it gets the number of points fitted.
** If ymin is not NULL, it gets ymin (according to parabolic fit).
** NOTE:  The arrays do NOT need to be sorted in x or y.
**
*/
double array_findminfitd(double *x,double *y,int n,double dxmax,double errmax,
                         double *err,int *npf,double *ymin)


    {
    int  i,ipos,i0,i1,ib1,ib2,imin,npfbest,npf3;
    double xb1,xb2,dxmean,xpos,xmin0,ymin0,dy;
    double *xd,*yd;
    double xmin,confbest,conf3,xmin3,ymin3,yminbest;
    static char *funcname="array_findminfitd";

/*
printf("@array_findminfitd...\n");
printf("    n=%d\n",n);
printf("    x[0]=%g, y[0]=%g\n",x[0],y[0]);
printf("    x[%d]=%g, y[%d]=%g\n",n/2,x[n/2],n/2,y[n/2]);
printf("    x[%d]=%g, y[%d]=%g\n",n-1,x[n-1],n-1,y[n-1]);
*/
    if (n<1)
        return(0.);
    imin=array_findminindexd(y,n);
    xmin0 = x[imin];
    ymin0 = y[imin];
/*
printf("    imin = %d, xmin0 = %g\n",imin,xmin0);
*/
    if (n<4)
        {
        if (ymin!=NULL)
            (*ymin) = ymin0;
        return(xmin0);
        }
    if (!willus_mem_alloc(&xd,sizeof(double)*2*n,funcname))
        {
        fprintf(stderr,"willuslib %s willus_mem_alloc fails (%d elements)\n",
                         funcname,2*n);
        exit(10);
        }
    yd = &xd[n];
    for (i=0;i<n;i++)
        {
        xd[i]=x[i];
        yd[i]=y[i];
        }
    sortxyd(xd,yd,n);
    imin = array_findminindexd(yd,n);
    /* Get slightly better initial guess of xmin0 */
    ib1=ib2=imin;
    for (;ib1>=0 && yd[ib1]==yd[imin];ib1--);
    ib1++;
    for (;ib2<=n-1 && yd[ib2]==yd[imin];ib2++);
    ib2--;
    if (ib1<=0 || ib2>=n-1)
        {
        willus_mem_free(&xd,funcname);
        if (ymin!=NULL)
            (*ymin) = ymin0;
        return(xmin0);
        }
    i0 = ib1-1;
    i1 = ib2+1;
    dy = yd[i0]-yd[imin] > yd[i1]-yd[imin] ? yd[i0]-yd[imin] : yd[i1]-yd[imin];
    if (yd[i0]-yd[imin] < 0.2*dy)
        ib1--;
    else if (yd[i1]-yd[imin] < 0.2*dy)
        ib2++;
    i0=ib1;
    i1=ib2;
    /* Recompute xmin0 */
    for (xmin0=ymin0=0,i=i0;i<=i1;i++)
        {
        xmin0 += xd[i];
        ymin0 += yd[i];
        }
    xmin0 /= (i1-i0+1);
    ymin0 /= (i1-i0+1);

/*
printf("    xmin0 = %g\n",xmin0);
*/
    ib1 = i0>2   ? i0-3 : 0;
    ib2 = i1<n-3 ? i1+3 : n-1;
    if (ib1==ib2)
        {
        willus_mem_free(&xd,funcname);
        if (ymin!=NULL)
            (*ymin)=ymin0;
        return(xmin0);
        }
    xb1 = xd[ib1];
    xb2 = xd[ib2];
    dxmean = (xb2-xb1)/(ib2-ib1);
    if (dxmean<=0.)
        {
        willus_mem_free(&xd,funcname);
        if (ymin!=NULL)
            (*ymin)=ymin0;
        return(xmin0);
        }
    confbest=conf3= -1;
    npfbest=npf3=0;
    xmin = xmin3 = xmin0;
    yminbest = ymin3 = ymin0;
    /* Try some different parabolic fits around the minimum point */
    /* and choose the best one.                                   */
    for (ipos=-2;ipos<=2;ipos++)
        {
        int   i0;
        xpos = xmin0+ipos*dxmean;
        i0 = fabs((double)ipos)+1.1;
        for (i=i0;dxmean*i*2.<=dxmax;i++)
            {
            double   x1,conf,rmserr,ymin1;
            int      npfit;
            npfit=0; /* Avoid compiler warning. */
            ymin1=rmserr=0.; /* Avoid compiler warning. */
            x1 = findminfitd1(xd,yd,n,xpos,dxmean*(i+.6),&rmserr,&npfit,&ymin1);
            if (x1<x[0] || x1<xpos-dxmean*i || x1>xpos+dxmean*i)
                continue;
            conf = rmserr; /*  / (npfit-1); */
/*
printf("xpos = %g, dx=%g, np=%d, rmserr=%g, conf=%g\n",
xpos,dxmean*i,npfit,rmserr,conf);
printf("%g %g %d\n",x1,conf,npfit);
*/
            if (npfit>3 && conf<errmax && (confbest < 0 || conf < confbest))
                {
                confbest = conf;
                xmin = x1;
                npfbest = npfit;
                yminbest = ymin1;
                }
            else if (npfit==3 && conf<errmax && (conf3 <0 || conf < conf3))
                {
                conf3 = conf;
                xmin3 = x1;
                npf3 = npfit;
                ymin3 = ymin1;
                }
            }
        }
    willus_mem_free(&xd,funcname);
    if (confbest<0 && conf3>=0.)
        {
        xmin = xmin3;
        confbest = conf3;
        npfbest = npf3;
        yminbest = ymin3;
        }
    if (err!=NULL)
        (*err) = confbest;
    if (npf!=NULL)
        (*npf) = npfbest;
    if (ymin!=NULL)
        (*ymin) = yminbest;
/*
printf("confbest = %g, conf3 = %g, xmin = %g\n",confbest,conf3,xmin);
*/
    return(xmin);
    }


int array_findminindexd(double *y,int n)

    {
    int  i,imin;

    for (imin=0,i=1;i<n;i++)
        if (y[i] < y[imin])
            imin=i;
    return(imin);
    }


/*
** Finds minimum of (x,y) curve (in y) by fitting parabola about
** points near x0.
** The fit range is determined +/-dx.
** The x[] array must be sorted (monotonically increasing from 0 to n-1).
** (*rmserr) gets the normalized rms error of the fit.
** [The normalization is to divide it by dy, the total range in y]
** (*npfit) gets the number of points fitted.
** The routine will fail if it cannot
** find at least 3 (in which case the rms error will be zero).
*/
static double findminfitd1(double *x,double *y,int n,double x0,double dx,
                           double *rmserr,int *npfit,double *ymin)

    {
    double  c[3];
    int     i,i1,i2;
    double  errval,dy;

    errval = x[0] - (x[n-1]-x[0]) - 1.0;
    for (i=0;i<n;i++)
        if (x[i] >= x0-dx)
            break;
    i1=i;
    for (;i<n;i++)
        if (x[i] > x0+dx)
            break;
    i2=i-1;
    if (i2-i1+1 < 3)
        return(errval);
    if (npfit!=NULL)
        (*npfit) = (i2-i1+1);
    dy = array_max(&y[i1],i2-i1+1) - array_min(&y[i1],i2-i1+1);
    if (dy<=0.)
        return(errval);
    wpolyfitd(&x[i1],&y[i1],i2-i1+1,2,c);
    if (fabs(c[2])<1e-10)
        return(errval);
    if (rmserr!=NULL)
        {
        double err;
        err=0;
        for (i=i1;i<=i2;i++)
            {
            double yf;
            yf = x[i]*x[i]*c[2] + x[i]*c[1] + c[0];
            err += (yf-y[i])*(yf-y[i]);
            }
        (*rmserr) = sqrt(err/(i2-i1+1))/dy;
        }
    if (ymin!=NULL)
        (*ymin) = c[0] - c[1]*c[1]/(4*c[2]);
    return(-c[1]/(2.*c[2]));
    }


/*
** Returns 1 for success, 0 if memory allocation error
**
** NOTE: if the input is totally real (all xi[] values are zero),
**       then the FFT will have an even real part and an odd imaginary part.
*/
int array_fft(double *xr,double *xi,int n)

    {
    return(array_recfft(xr,xi,n));
    }


/*
** Returns 1 for success, 0 if memory allocation error
*/
int array_ifft(double *xr,double *xi,int n)

    {
    int     i;

    for (i=0;i<n;i++)
        xi[i]=-xi[i];
    if (!array_fft(xr,xi,n))
        return(0);
    for (i=0;i<n;i++)
        {
        xr[i]/=n;
        xi[i]/=-n;
        }
    return(1);
    }




/*
**
** RECFFT    Recursive FFT function used to calculate the FFT of a
**           sequence.  The sequence can be ANY length, however, if
**           the length doesn't factor well, the calculations will
**           approach normal DFT speeds.
**
** willus.com, July 20, 1989
**
**
*/
static int array_recfft(double *xr,double *xi,int length)

    {
    static int      len=0;
    static double  *wr,*wi,*txr,*txi;
    static int      j,i1,i2,index;
    int             i,step,f1,f2;
    static char *funcname="array_recfft";

    if (length==1)
        return(1);
    if (length<=0)
        return(0);

/*
** Initialization code, first pass of the function
*/

    if (len==0)
        {
        if (!willus_mem_alloc(&wr,4L*sizeof(double)*length,funcname))
            return(0);
        wi=&wr[length];
        txr=&wi[length];
        txi=&txr[length];
        for (i=0;i<length;i++)
            {
            wi[i]=sin(2*PI*(double)i/(double)length);
            wr[i]=cos(2*PI*(double)i/(double)length);
            wi[i]=-wi[i];
            }
        len=length;
        }

/*
** Factor the length into two smaller multiples
*/

    if (length%2==0)
        f1=2;
    else
        if (length%3==0)
            f1=3;
        else
            {
            j=sqrt((double)length)+1;
            for (i=5;i<=j;i+=4)
                {
                if (length%i==0)
                    break;
                i+=2;
                if (i>j)
                    break;
                if (length%i==0)
                    break;
                }
            f1=(i>j)?length:i;
            }
    f2=length/f1;

/*
** Now do the FFT's of the factors, calling this function recursively.
*/

    step=len/length;
    if (f2>1)
        for (i=0;i<f1;i++)
            array_recfft(&xr[i*step],&xi[i*step],f2);

/*
** Recombine the FFT's of the factors to get the final result.
*/

    for (i=0;i<length;i++)
        {
        txr[i]=0;
        txi[i]=0;
        index=i%f2*step*f1;
        for (j=0;j<f1;j++)
            {
            i1=index+j*step;
            i2=i*j%length*step;
            txr[i] += xr[i1]*wr[i2] - xi[i1]*wi[i2];
            txi[i] += xr[i1]*wi[i2] + xi[i1]*wr[i2];
            }
        }
    for (i=0,j=0;i<length;i++,j+=step)
        {
        xr[j]=txr[i];
        xi[j]=txi[i];
        }

/*
** Clean up
*/

    if (length==len)
        {
        willus_mem_free(&wr,funcname);
        len=0;
        }
    return(1);
    }








/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */



/*               SINGLE PRECISION STARTS HERE                           */



/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */
/* ==================================================================== */


int arrayf_find_max_point(float *x0,float *y0,float *x,float *y,int n)

	{
	double  c[3];
	double *tx,*ty;
	double  xoff,yoff;
	int		i;
	
	willus_mem_alloc_warn((void **)&tx,sizeof(double)*2*n,"arrayf_find_max_point",10);
    ty = &tx[n];
    for (xoff=yoff=0.,i=0;i<n;i++)
        {
        xoff += x[i];
        yoff += y[i];
        }
    xoff /= n;
    yoff /= n;
    for (i=0;i<n;i++)
        {
        tx[i] = x[i]-xoff;
        ty[i] = y[i]-yoff;
        }
	wpolyfitd(tx,ty,n,2,c);
    willus_mem_free((double **)&tx,"array_find_max_point");
	if (c[2]==0)
		return(-1);
	(*x0)= (float)(-c[1]/(2.*c[2]));
	(*y0)= (float)(c[2]*(*x0)*(*x0) + c[1]*(*x0) + c[0] + yoff);
    (*x0) = (*x0) + xoff;
	return(0);
	}
	
	
/*
// Find y = m*x + b equation for line fit
*/
void arrayf_get_lsq_slope(float *m,float *b,float *x,float *y,int n)

    {
    double	c[2];
	double  *tx,*ty;
	double	xoff,yoff;
	int		i;
	
	willus_mem_alloc_warn((void **)&tx,sizeof(double)*2*n,"arrayf_get_lsq_slope",10);
    ty = &tx[n];
    for (xoff=yoff=0.,i=0;i<n;i++)
        {
        xoff += x[i];
        yoff += y[i];
        }
    xoff /= n;
    yoff /= n;
    for (i=0;i<n;i++)
        {
        tx[i] = x[i]-xoff;
        ty[i] = y[i]-yoff;
        }
    wpolyfitd(tx,ty,n,1,c);
  	willus_mem_free((double **)&tx,"arrayf_get_lsq_slope");
    (*m)=(float)c[1];
    (*b)=(float)(c[0]+yoff);
    }

	
	
int arrayf_is_linear(float *x,int n)

	{
	int		i;
	float	xp,x0,x1,xav;
	
	if (n<3)
		return(1);
	x0=x[0];
	x1=x[n-1];
	xav=(x0+x1)/2.;
	if (xav==0)
		xav=(x1-x0);
	if (xav==0)
		xav=1;
	for (i=1;i<n-1;i++)
		{
		xp=x0+(x1-x0)*i/(n-1);
		if (fabs((xp-x[i])/xav)>1e-5)
			return(0);
		}
	return(1);
	}
	

/*
** Linearize
*/
int arrayf_linearize_xyz(float *x,float *y,float *z,int n,int newn)

	{
	float	*x0,*y0,*z0;
	int		i;
	float	xx;
	
	if (!willus_mem_alloc((double **)&x0,sizeof(float)*3*newn,"arrayf_linearize_xyz"))
		return(1);
	y0=&x0[newn];
	z0=&y0[newn];
	for (i=0;i<newn;i++)
		{
		xx = x[0] + (x[n-1]-x[0])*(double)i/(newn-1);
		x0[i]=xx;
		y0[i]=interpxy(xx,x,y,n);
		z0[i]=interpxy(xx,x,z,n);
		}
	for (i=0;i<newn;i++)
		{
		x[i]=x0[i];
		y[i]=y0[i];
		z[i]=z0[i];
		}
	willus_mem_free((double **)&x0,"arrayf_linearize_xyz");
	return(0);
	}


int arrayf_max_index(float *x,int n)

	{
	int		i,imax;
	
	for (imax=0,i=1;i<n;i++)
		if (x[imax]<x[i])
			imax=i;
	return(imax);
	}



float arrayf_maxdev(float *x,int n)

    {
    return(arrayf_max(x,n)-arrayf_min(x,n));
    }




/*
**
** Compute mean and standard deviation
**
*/
void arrayf_mean(float *a,int n,double *mean,double *stddev)

    {
    int     i;
    double  sum,avg,sum_sq;

    if (n<1)
        return;
    for (sum=sum_sq=i=0;i<n;i++)
        sum += a[i];
    avg = sum/n;
    if (mean!=NULL)
        (*mean) = avg;
    if (stddev!=NULL)
        {
        float sum_sq;

        for (sum_sq=i=0;i<n;i++)
            sum_sq += (a[i]-avg)*(a[i]-avg);
        (*stddev) = sqrt(sum_sq/n);
        }
    }


double arrayf_rms(float *a,int n)

    {
    int     i;
    double  sum;

    for (sum=i=0;i<n;i++)
        sum += a[i]*a[i];
    return(sqrt(sum/n));
    }


float arrayf_max(float *a,int n)

    {
    float max;
    int  i;

    for (max=a[0],i=1;i<n;i++)
        if (max<a[i])
            max=a[i];
    return(max);
    }


float arrayf_min(float *a,int n)

    {
    float min;
    int  i;

    for (min=a[0],i=1;i<n;i++)
        if (min>a[i])
            min=a[i];
    return(min);
    }


void arrayf_sort(float *a,int n)

    {
    sort(a,(long)n);
    }


/*
** Returns 1 for success, 0 if memory allocation error
*/
int arrayf_fft(float *xr,float *xi,int n)

    {
    return(arrayf_recfft(xr,xi,n));
    }


/*
** Returns 1 for success, 0 if memory allocation error
*/
int arrayf_ifft(float *xr,float *xi,int n)

    {
    int     i;

    for (i=0;i<n;i++)
        xi[i]=-xi[i];
    if (!arrayf_fft(xr,xi,n))
        return(0);
    for (i=0;i<n;i++)
        {
        xr[i]/=n;
        xi[i]/=-n;
        }
    return(1);
    }


/*
**
** RECFFT    Recursive FFT function used to calculate the FFT of a
**           sequence.  The sequence can be ANY length, however, if
**           the length doesn't factor well, the calculations will
**           approach normal DFT speeds.
**
** willus.com, July 20, 1989
**
**
*/
static int arrayf_recfft(float *xr,float *xi,int length)

    {
    static int      len=0;
    static float  *wr,*wi,*txr,*txi;
    static int      j,i1,i2,index;
    int             i,step,f1,f2;
    static char *funcname="arrayf_recfft";

    if (length==1)
        return(1);
    if (length<=0)
        return(0);

/*
** Initialization code, first pass of the function
*/

    if (len==0)
        {
        if (!willus_mem_alloc((double **)&wr,sizeof(float)*4L*length,funcname))
            return(0);
        wi=&wr[length];
        txr=&wi[length];
        txi=&txr[length];
        for (i=0;i<length;i++)
            {
            double s,c;
            s=sin(2.*PI*i/length);
            c=cos(2.*PI*i/length);
            wr[i]=(float)c;
            wi[i]=-(float)s;
            }
        len=length;
        }

/*
** Factor the length into two smaller multiples
*/

    if (length%2==0)
        f1=2;
    else
        if (length%3==0)
            f1=3;
        else
            {
            j=sqrt((float)length)+1;
            for (i=5;i<=j;i+=4)
                {
                if (length%i==0)
                    break;
                i+=2;
                if (i>j)
                    break;
                if (length%i==0)
                    break;
                }
            f1=(i>j)?length:i;
            }
    f2=length/f1;

/*
** Now do the FFT's of the factors, calling this function recursively.
*/

    step=len/length;
    if (f2>1)
        for (i=0;i<f1;i++)
            arrayf_recfft(&xr[i*step],&xi[i*step],f2);

/*
** Recombine the FFT's of the factors to get the final result.
*/

    for (i=0;i<length;i++)
        {
        txr[i]=0;
        txi[i]=0;
        index=i%f2*step*f1;
        for (j=0;j<f1;j++)
            {
            i1=index+j*step;
            i2=i*j%length*step;
            txr[i] += xr[i1]*wr[i2] - xi[i1]*wi[i2];
            txi[i] += xr[i1]*wi[i2] + xi[i1]*wr[i2];
            }
        }
    for (i=0,j=0;i<length;i++,j+=step)
        {
        xr[j]=txr[i];
        xi[j]=txi[i];
        }

/*
** Clean up
*/

    if (length==len)
        {
        willus_mem_free((double **)&wr,funcname);
        len=0;
        }
    return(1);
    }


