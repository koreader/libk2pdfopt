/*
** bmpregion.c    Functions to handle BMPREGION structure.  These
**                are more-or-less generic functions that don't depend
**                heavily on k2pdfopt settings.
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

static void trim_to(int *count,int *i1,int i2,double gaplen,int dpi,double defect_size_pts);
static int height2_calc(int *rc,int n);


int bmpregion_row_black_count(BMPREGION *region,int r0)

    {
    unsigned char *p;
    int i,nc,c;

    p=bmp_rowptr_from_top(region->bmp8,r0)+region->c1;
    nc=region->c2-region->c1+1;
    for (c=i=0;i<nc;i++,p++)
        if (p[0]<region->bgcolor)
            c++;
    return(c);
    }


int bmpregion_col_black_count(BMPREGION *region,int c0)

    {
    unsigned char *p;
    int i,nr,c,bw;

    bw=bmp_bytewidth(region->bmp8);
    p=bmp_rowptr_from_top(region->bmp8,region->r1)+c0;
    nr=region->r2-region->r1+1;
    for (c=i=0;i<nr;i++,p+=bw)
        if (p[0]<region->bgcolor)
            c++;
    return(c);
    }


#if (defined(WILLUSDEBUGX) || defined(WILLUSDEBUG))
void bmpregion_write(BMPREGION *region,char *filename)

    {
    int i,bpp;
    WILLUSBITMAP *bmp,_bmp;

    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=region->c2-region->c1+1;
    bmp->height=region->r2-region->r1+1;
    bmp->bpp=region->bmp->bpp;
    bpp=bmp->bpp==8?1:3;
    bmp_alloc(bmp);
    for (i=0;i<256;i++)
        bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *s,*d;
        s=bmp_rowptr_from_top(region->bmp,region->r1+i)+region->c1*bpp;
        d=bmp_rowptr_from_top(bmp,i);
        memcpy(d,s,bmp->width*bpp);
        }
    bmp_write(bmp,filename,stdout,97);
    bmp_free(bmp);
    }
#endif

void bmpregion_row_histogram(BMPREGION *region)

    {
    static char *funcname="bmpregion_row_histogram";
    WILLUSBITMAP *src;
    FILE *out;
    static int *rowcount;
    static int *hist;
    int i,j,nn;

    willus_dmem_alloc_warn(6,(void **)&rowcount,(region->r2-region->r1+1)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(7,(void **)&hist,(region->c2-region->c1+1)*sizeof(int),funcname,10);
    src=region->bmp8;
    for (j=region->r1;j<=region->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(src,j)+region->c1;
        rowcount[j-region->r1]=0;
        for (i=region->c1;i<=region->c2;i++,p++)
            if (p[0]<region->bgcolor)
                rowcount[j-region->r1]++;
        }
    for (i=region->c1;i<=region->c2;i++)
        hist[i-region->c1]=0;
    for (i=region->r1;i<=region->r2;i++)
        hist[rowcount[i-region->r1]]++;
    for (i=region->c2-region->c1+1;i>=0;i--)
        if (hist[i]>0)
            break;
    nn=i;
    out=fopen("hist.ep","w");
        for (i=0;i<=nn;i++)
            fprintf(out,"%5d %5d\n",i,hist[i]);
    fclose(out);
    out=fopen("rowcount.ep","w");
        for (i=0;i<region->r2-region->r1+1;i++)
            fprintf(out,"%5d %5d\n",i,rowcount[i]);
    fclose(out);
    willus_dmem_free(7,(double **)&hist,funcname);
    willus_dmem_free(6,(double **)&rowcount,funcname);
    }


/*
** Return 0 if there are dark pixels in the region.  NZ otherwise.
*/
int bmpregion_is_clear(BMPREGION *region,int *row_black_count,int *col_black_count,
                       int *col_pix_count,int rpc,double gt_in)

    {
    int nr,nc,r,c,pt,mindim;

    pt=(int)(gt_in*region->dpi*(region->c2-region->c1+1)+.5);
    if (pt<0)
        pt=0;
    /*
    ** Fast way to count dark pixels, but requires big array
    */
    if (col_pix_count!=NULL && rpc>0)
        {
        int i;
        if (region->r1>0)
            for (c=0,i=region->c1;i<=region->c2;i++)
                {
                c += col_pix_count[i*rpc+region->r2] - col_pix_count[i*rpc+(region->r1-1)];
                if (c>pt)
                    return(0);
                }
        else
            for (c=0,i=region->c1;i<=region->c2;i++)
                {
                c += col_pix_count[i*rpc+region->r2];
                if (c>pt)
                    return(0);
                }
        return(pt<=0 ? 1 : 1+(int)10*c/pt);
        }
        
    /*
    ** row_black_count[] doesn't necessarily match up to this particular region's columns.
    ** So if row_black_count[] == 0, the row is clear, otherwise it has to be counted.
    ** because the columns are a subset.
    */
    nr=region->r2-region->r1+1;
    nc=region->c2-region->c1+1;
    mindim = nr>nc ? nc : nr;
    if (mindim > 5)
        {
        int i,bcc,brc;

        /*
        ** Determine most efficient way to see if the shaft is clear
        */
        for (bcc=0,i=region->c1;i<=region->c2;i++)
            if (col_black_count[i]==0)
                bcc++;
        for (brc=0,i=region->r1;i<=region->r2;i++)
            if (row_black_count[i]==0)
                brc++;

        /*
        ** Count dark pixels by columns
        */
        if (bcc*(region->r2-region->r1+1) > 2*brc*(region->c2-region->c1+1))
            {
            int col;

            for (c=0,col=region->c1;col<=region->c2;col++)
                {
                if (col<0 || col>=region->bmp8->width)
                    continue;
                if (col_black_count[col]==0)
                    continue;
                c+=bmpregion_col_black_count(region,col);
                if (c>pt)
                    return(0);
                }
            return(pt<=0 ? 1 : 1+(int)10*c/pt);
            }
        }

    /*
    ** Count dark pixels by rows
    */
    for (c=0,r=region->r1;r<=region->r2;r++)
        {
        if (r<0 || r>=region->bmp8->height)
            continue;
        if (row_black_count[r]==0)
            continue;
        c+=bmpregion_row_black_count(region,r);
        if (c>pt)
            return(0);
        }
/*
printf("(%d,%d)-(%d,%d):  c=%d, pt=%d (gt_in=%g)\n",
region->c1,region->r1,region->c2,region->r2,c,pt,gt_in);
*/
    return(pt<=0 ? 1 : 1+(int)10*c/pt);
    }


void bmpregion_trim_to_crop_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings)

    {
    int n;

    if (k2settings->mar_left<0
         || k2settings->mar_right<0
         || k2settings->mar_top<0
         || k2settings->mar_bot<0)
        k2pdfopt_settings_sanity_check(k2settings);
    n=(int)(0.5+k2settings->mar_left*region->dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c1=n;
    n=(int)(0.5+k2settings->mar_right*region->dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c2=region->bmp->width-1-n;
    n=(int)(0.5+k2settings->mar_top*region->dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r1=n;
    n=(int)(0.5+k2settings->mar_bot*region->dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r2=region->bmp->height-1-n;
    }

/*
** 1 = column 1 too short
** 2 = column 2 too short
** 3 = both too short
** 0 = both okay
** Both columns must pass height requirement.
**
** Also, if gap between columns > max_column_gap_inches, fails test. (8-31-12)
**
*/
int bmpregion_column_height_and_gap_test(BMPREGION *column,BMPREGION *region,
                                        K2PDFOPT_SETTINGS *k2settings,
                                        int r1,int r2,
                                        int cmid,int *colcount,int *rowcount)

    {
    int min_height_pixels,status;

    status=0;
    min_height_pixels=k2settings->min_column_height_inches*region->dpi;
    column[0]=(*region);
    column[0].r1=r1;
    column[0].r2=r2;
    column[0].c2=cmid-1;
    bmpregion_trim_margins(&column[0],k2settings,colcount,rowcount,0xf);
/*
printf("    COL1:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[0].r2-column[0].r1+1 < min_height_pixels)
        status |= 1;
    column[1]=(*region);
    column[1].r1=r1;
    column[1].r2=r2;
    column[1].c1=cmid;
    column[1].c2=region->c2;
    bmpregion_trim_margins(&column[1],k2settings,colcount,rowcount,0xf);
/*
printf("    COL2:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[1].r2-column[1].r1+1 < min_height_pixels)
        status |= 2;
    /* Make sure gap between columns is not too large */
    if (k2settings->max_column_gap_inches>=0. && column[1].c1-column[0].c2-1 > k2settings->max_column_gap_inches*region->dpi)
        status |= 4;
    return(status);
    }


/*
** flags&1  : trim c1
** flags&2  : trim c2
** flags&4  : trim r1
** flags&8  : trim r2
** flags&16 : Find rowbase, font size, etc.
**
** Row base is where row dist crosses 50% on r2 side.
** Font size is where row dist crosses 5% on other side (r1 side).
** Lowercase font size is where row dist crosses 50% on r1 side.
**
** For 12 pt font:
**     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
**     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
**     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
** Mean line spacing = 1.15 - 1.22 (~1.16)
** Mean cap height = 0.68
** Mean small letter height = 0.49
** 
*/
void bmpregion_trim_margins(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                            int *colcount0,int *rowcount0,int flags)

    {
    int i,j,n; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int *colcount,*rowcount;
    static char *funcname="bmpregion_trim_margins";

    /* To detect a hyphen, we need to trim and calc text base row */
    if (flags&32)
        flags |= 0x1f;
    if (colcount0==NULL)
        willus_dmem_alloc_warn(10,(void **)&colcount,sizeof(int)*(region->c2+1),funcname,10);
    else
        colcount=colcount0;
    if (rowcount0==NULL)
        willus_dmem_alloc_warn(11,(void **)&rowcount,sizeof(int)*(region->r2+1),funcname,10);
    else
        rowcount=rowcount0;
    n=region->c2-region->c1+1;
/*
printf("Trim:  reg=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
if (region->c2+1 > cca || region->r2+1 > rca)
{
printf("A ha 0!\n");
exit(10);
}
*/
    memset(colcount,0,(region->c2+1)*sizeof(int));
    memset(rowcount,0,(region->r2+1)*sizeof(int));
    for (j=region->r1;j<=region->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(region->bmp8,j)+region->c1;
        for (i=0;i<n;i++,p++)
            if (p[0]<region->bgcolor)
                {
                rowcount[j]++;
                colcount[i+region->c1]++;
                }
        }
    /*
    ** Trim excess margins
    */
    if (flags&1)
        trim_to(colcount,&region->c1,region->c2,k2settings->src_left_to_right ? 2.0 : 4.0,
                region->dpi,k2settings->defect_size_pts);
    if (flags&2)
        trim_to(colcount,&region->c2,region->c1,k2settings->src_left_to_right ? 4.0 : 2.0,
                region->dpi,k2settings->defect_size_pts);
    if (colcount0==NULL)
        willus_dmem_free(10,(double **)&colcount,funcname);
    if (flags&4)
        trim_to(rowcount,&region->r1,region->r2,4.0,region->dpi,k2settings->defect_size_pts);
    if (flags&8)
        trim_to(rowcount,&region->r2,region->r1,4.0,region->dpi,k2settings->defect_size_pts);
    if (flags&16)
        {
        int maxcount,mc2,h2;
        double f;

        maxcount=0;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > maxcount)
                maxcount = rowcount[i];
        mc2 = maxcount / 2;
        for (i=region->r2;i>=region->r1;i--)
            if (rowcount[i] > mc2)
                break;
        region->rowbase = i;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > mc2)
                break;
        region->h5050 = region->lcheight = region->rowbase-i+1;
        mc2 = maxcount / 20;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > mc2)
                break;
        region->capheight = region->rowbase-i+1;
        /*
        ** Sanity check capheight and lcheight
        */
        h2=height2_calc(&rowcount[region->r1],region->r2-region->r1+1);
#if (WILLUSDEBUGX & 8)
if (region->c2-region->c1 > 1500)
printf("reg %d x %d (%d,%d) - (%d,%d) h2=%d ch/h2=%g\n",region->c2-region->c1+1,region->r2-region->r1+1,region->c1,region->r1,region->c2,region->r2,h2,(double)region->capheight/h2);
#endif
        if (region->capheight < h2*0.75)
            region->capheight = h2;
        f=(double)region->lcheight/region->capheight;
        if (f<0.55)
            region->lcheight = (int)(0.72*region->capheight+.5);
        else if (f>0.85)
            region->lcheight = (int)(0.72*region->capheight+.5);
#if (WILLUSDEBUGX & 8)
if (region->c2-region->c1 > 1500)
printf("    lcheight final = %d\n",region->lcheight);
#endif
#if (WILLUSDEBUGX & 10)
if (region->c2-region->c1 > 1500 && region->r2-region->r1 < 100)
{
static int append=0;
FILE *f;
int i;
f=fopen("textrows.ep",append==0?"w":"a");
append=1;
for (i=region->r1;i<=region->r2;i++)
fprintf(f,"%d %g\n",region->rowbase-i,(double)rowcount[i]/maxcount);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
        }
    else
        {
        region->h5050 = region->r2-region->r1+1;
        region->capheight = 0.68*(region->r2-region->r1+1);
        region->lcheight = 0.5*(region->r2-region->r1+1);
        region->rowbase = region->r2;
        }
#if (WILLUSDEBUGX & 2)
printf("trim:\n    reg->c1=%d, reg->c2=%d\n",region->c1,region->c2);
printf("    reg->r1=%d, reg->r2=%d, reg->rowbase=%d\n\n",region->r1,region->r2,region->rowbase);
#endif
    if (rowcount0==NULL)
        willus_dmem_free(11,(double **)&rowcount,funcname);
    }


static void trim_to(int *count,int *i1,int i2,double gaplen,int dpi,double defect_size_pts)

    {
    int del,dcount,igaplen,clevel,dlevel,defect_start,last_defect;

    igaplen=(int)(gaplen*dpi/72.);
    if (igaplen<1)
        igaplen=1;
    /* clevel=(int)(defect_size_pts*dpi/72./3.); */
    clevel=0;
    dlevel=(int)(pow(defect_size_pts*dpi/72.,2.)*PI/4.+.5);
    del=i2>(*i1) ? 1 : -1;
    defect_start=-1;
    last_defect=-1;
    dcount=0;
    for (;(*i1)!=i2;(*i1)=(*i1)+del)
        {
        if (count[(*i1)]<=clevel)
            {
            dcount=0;  /* Reset defect size */
            continue;
            }
        /* Mark found */
        if (dcount==0)
            {
            if (defect_start>=0)
                last_defect=defect_start;
            defect_start=(*i1);
            }
        dcount += count[(*i1)];
        if (dcount >= dlevel)
            {
            if (last_defect>=0 && abs(defect_start-last_defect)<=igaplen)
                (*i1)=last_defect;
            else
                (*i1)=defect_start;
            return;
            }
        }
    if (defect_start<0)
        return;
    if (last_defect<0)
        {
        (*i1)=defect_start;
        return;
        }
    if (abs(defect_start-last_defect)<=igaplen)
        (*i1)=last_defect;
    else
        (*i1)=defect_start;
    }

/*
** Calculate weighted height of a rectangular region.
** This weighted height is intended to be close to the height of
** a capital letter, or the height of the majority of the region.
**
*/
static int height2_calc(int *rc,int n)

    {
    int i,thresh,i1,h2;
    int *c;
    static char *funcname="height2_calc";
#if (WILLUSDEBUGX & 8)
    int cmax;
#endif

    if (n<=0)
        return(1);
    willus_dmem_alloc_warn(12,(void **)&c,sizeof(int)*n,funcname,10);
    memcpy(c,rc,n*sizeof(int));
    sorti(c,n);
#if (WILLUSDEBUGX & 8)
    cmax=c[n-1];
#endif
    for (i=0;i<n-1 && c[i]==0;i++);
    thresh=c[(i+n)/3];
    willus_dmem_free(12,(double **)&c,funcname);
    for (i=0;i<n-1;i++)
        if (rc[i]>=thresh)
            break;
    i1=i;
    for (i=n-1;i>i1;i--)
        if (rc[i]>=thresh)
            break;
#if (WILLUSDEBUGX & 8)
// printf("thresh = %g, i1=%d, i2=%d\n",(double)thresh/cmax,i1,i);
#endif
    h2=i-i1+1; /* Guaranteed to be >=1 */
    return(h2);
    }


/*
** Does region end in a hyphen?  If so, fill in HYPHENINFO structure.
*/
void bmpregion_hyphen_detect(BMPREGION *region,int hyphen_detect,int left_to_right)

    {
    int i,j; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int width;
    int *r0,*r1,*r2,*r3;
    int rmin,rmax,rowbytes,nrmid,rsum;
    int cstart,cend,cdir;
    unsigned char *p;
    static char *funcname="bmpregion_hyphen_detect";

#if (WILLUSDEBUGX & 16)
static int count=0;
char pngfile[256];
FILE *out;

count++;
printf("@bmpregion_hyphen_detect count=%d\n",count);
sprintf(pngfile,"word%04d.png",count);
bmpregion_write(region,pngfile);
sprintf(pngfile,"word%04d.txt",count);
out=fopen(pngfile,"w");
fprintf(out,"c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
fprintf(out,"lcheight=%d\n",region->lcheight);
#endif

    region->hyphen.ch = -1;
    region->hyphen.c2 = -1;
    if (hyphen_detect)
        return;
    width=region->c2-region->c1+1;
    if (width<2)
        return;
    willus_dmem_alloc_warn(27,(void **)&r0,sizeof(int)*4*width,funcname,10);
    r1=&r0[width];
    r2=&r1[width];
    r3=&r2[width];
    for (i=0;i<width;i++)
        r0[i]=r1[i]=r2[i]=r3[i]=-1;
    rmin=region->rowbase-region->capheight-region->lcheight*.04;
    if (rmin < region->r1)
        rmin = region->r1;
    rmax=region->rowbase+region->lcheight*.04;
    if (rmax > region->r2)
        rmax = region->r2;
    rowbytes=bmp_bytewidth(region->bmp8);
    p=bmp_rowptr_from_top(region->bmp8,0);
    nrmid=rsum=0;
    if (left_to_right)
        {
        cstart=region->c2;
        cend=region->c1-1;
        cdir=-1;
        }
    else
        {
        cstart=region->c1;
        cend=region->c2+1;
        cdir=1;
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   j     r0     r1     r2     r3\n");
#endif
    for (j=cstart;j!=cend;j+=cdir)
        {
        int r,rmid,dr,drmax;

// printf("j=%d\n",j);
        rmid=(rmin+rmax)/2;
// printf("   rmid=%d\n",rmid);
        drmax=region->r2+1-rmid > rmid-region->r1+1 ? region->r2+1-rmid : rmid-region->r1+1;
        /* Find dark region closest to center line */
        for (dr=0;dr<drmax;dr++)
            {
            if (rmid+dr<=region->r2 && p[(rmid+dr)*rowbytes+j]<region->bgcolor)
                break;
            if (rmid-dr>=region->r1 && p[(rmid-dr)*rowbytes+j]<region->bgcolor)
                {
                dr=-dr;
                break;
                }
            }
#if (WILLUSDEBUGX & 16)
fprintf(out,"    dr=%d/%d, rmid+dr=%d, rmin=%d, rmax=%d, nrmid=%d\n",dr,drmax,rmid+dr,rmin,rmax,nrmid);
#endif
        /* No dark detected or mark is outside hyphen region? */
        /* Termination criterion #1 */
        if (dr>=drmax || (nrmid>2 && (double)nrmid/region->lcheight>.1 
                               && (rmid+dr<rmin || rmid+dr>rmax)))
            {
            if (region->hyphen.ch>=0 && dr>=drmax)
                continue;
            if (nrmid>2 && (double)nrmid/region->lcheight > .35)
                {
                region->hyphen.ch = j-cdir;
                region->hyphen.r1 = rmin;
                region->hyphen.r2 = rmax;
                }
            if (dr<drmax)
                {
                region->hyphen.c2=j;
                break;
                }
            continue;
            }
        if (region->hyphen.ch>=0)
            {
            region->hyphen.c2=j;
            break;
            }
        nrmid++;
        rmid += dr;
        /* Dark spot is outside expected hyphen area */
        /*
        if (rmid<rmin || rmid>rmax)
            {
            if (nrmid>0)
                break;
            continue;
            }
        */
        for (r=rmid;r>=region->r1;r--)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r1[j-region->c1]=r+1;
        r0[j-region->c1]=-1;
        if (r>=region->r1)
            {
            for (;r>=region->r1;r--)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r>=region->r1)
                r0[j-region->c1]=r;
            }
        for (r=rmid;r<=region->r2;r++)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r2[j-region->c1]=r-1;
        r3[j-region->c1]=-1;
        if (r<=region->r2)
            {
            for (;r<=region->r2;r++)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r<=region->r2)
                r3[j-region->c1]=r;
            }
#if (WILLUSDEBUGX & 16)
fprintf(out," %4d  %4d  %4d  %4d  %4d\n",j,r0[j-region->c1],r1[j-region->c1],r2[j-region->c1],r3[j-region->c1]);
#endif
        if (region->hyphen.c2<0 && (r0[j-region->c1]>=0 || r3[j-region->c1]>=0))
            region->hyphen.c2=j;
        /* Termination criterion #2 */
        if (nrmid>2 && (double)nrmid/region->lcheight > .35
                && (r1[j-region->c1] > rmax || r2[j-region->c1] < rmin))
            {
            region->hyphen.ch = j-cdir;
            region->hyphen.r1 = rmin;
            region->hyphen.r2 = rmax;
            if (region->hyphen.c2<0)
                region->hyphen.c2=j;
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Termination #2\n");
#endif
            break;
            }
        // rc=(r1[j-region->c1]+r2[j-region->c1])/2;
        /* DQ possible hyphen if r1/r2 out of range */
        if (nrmid>1)
           {
           /* Too far away from last values? */
           if ((double)(rmin-r1[j-region->c1])/region->lcheight > .1
               || (double)(r2[j-region->c1]-rmax)/region->lcheight > .1)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too far from last values.\n");
#endif
               break;
}
           if ((double)nrmid/region->lcheight > .1 && nrmid>1)
               {
               if ((double)fabs(rmin-r1[j-region->c1])/region->lcheight > .1
                   || (double)(rmax-r2[j-region->c1])/region->lcheight > .1)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too far from last values (2).\n");
#endif
                   break;
}
               }
           }
        if (nrmid==1 || r1[j-region->c1]<rmin)
            rmin=r1[j-region->c1];
        if (nrmid==1 || r2[j-region->c1]>rmax)
            rmax=r2[j-region->c1];
        if ((double)nrmid/region->lcheight > .1 && nrmid>1)
            {
            double rmean;

            /* Can't be too thick */
            if ((double)(rmax-rmin+1)/region->lcheight > .55
                    || (double)(rmax-rmin+1)/region->lcheight < .05)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Too thick or too thin:  rmax=%d, rmin=%d, lch=%d rat=%g (.05 - .55).\n",
rmax,rmin,region->lcheight,(double)(rmax-rmin+1)/region->lcheight);
#endif
                break;
}
            /* Must be reasonably well centered above baseline */
            rmean=(double)(rmax+rmin)/2;
            if ((double)(region->rowbase-rmean)/region->lcheight < 0.35
                  || (double)(region->rowbase-rmean)/region->lcheight > 0.85)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Not well centered (1).\n");
#endif
                break;
}
            if ((double)(region->rowbase-rmax)/region->lcheight < 0.2
                  || (double)(region->rowbase-rmin)/region->lcheight > 0.92)
{
#if (WILLUSDEBUGX & 16)
fprintf(out,"  Not well centered (2).\n");
#endif
                break;
}
            }
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   ch=%d, c2=%d, r1=%d, r2=%d\n",region->hyphen.ch,region->hyphen.c2,region->hyphen.r1,region->hyphen.r2);
fclose(out);
#endif
    /* More sanity checks--better to miss a hyphen than falsely detect it. */
    if (region->hyphen.ch>=0)
        {
        double ar;
        /* If it's only a hyphen, then it's probably actually a dash--don't detect it. */
        if (region->hyphen.c2<0)
            region->hyphen.ch = -1;
        /* Check aspect ratio */
        ar=(double)(region->hyphen.r2-region->hyphen.r1)/nrmid;
        if (ar<0.08 || ar > 0.75)
            region->hyphen.ch = -1;
        }
    willus_dmem_free(27,(double **)&r0,funcname);
#if (WILLUSDEBUGX & 16)
if (region->hyphen.ch>=0)
printf("\n\n   GOT HYPHEN.\n\n");
printf("   Exiting bmpregion_hyphen_detect\n");
#endif
    }


int bmpregion_is_centered(BMPREGION *region,K2PDFOPT_SETTINGS *k2settings,
                          BREAKINFO *breakinfo,int i1,int i2,int *th)

    {
    int j,i,cc,n1,ntr;
    int textheight;

#if (WILLUSDEBUGX & 1)
printf("@bmpregion_is_centered:  region=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    nrows = %d\n",i2-i1+1);
#endif
    ntr=i2-i1+1;
    for (j=0;j<3;j++)
        {
        for (n1=textheight=0,i=i1;i<=i2;i++)
            {
            TEXTROW *textrow;
            double ar,rh;

            textrow=&breakinfo->textrow[i];
            if (textrow->c2<textrow->c1)
                ar = 100.;
            else
                ar = (double)(textrow->r2-textrow->r1+1)/(double)(textrow->c2-textrow->c1+1);
            rh = (double)(textrow->r2-textrow->r1+1)/region->dpi;
            if (j==2 || (j>=1 && rh<=k2settings->no_wrap_height_limit_inches)
                     || (j==0 && rh<=k2settings->no_wrap_height_limit_inches && ar<=k2settings->no_wrap_ar_limit))
                {
                textheight += textrow->rowbase - textrow->r1+1;
                n1++;
                }
            }
        if (n1>0)
            break;
        }
    textheight = (int)((double)textheight/n1+.5);
    if (th!=NULL)
        {
        (*th)=textheight;
#if (WILLUSDEBUGX & 1)
printf("    textheight assigned (%d)\n",textheight);
#endif
        return(breakinfo->centered);
        }

    /*
    ** Does region appear to be centered?
    */
    for (cc=0,i=i1;i<=i2;i++)
        {
        double indent1,indent2;

#if (WILLUSDEBUGX & 1)
printf("    tr[%d].c1,c2 = %d, %d\n",i,breakinfo->textrow[i].c1,breakinfo->textrow[i].c2);
#endif
        indent1 = (double)(breakinfo->textrow[i].c1-region->c1) / textheight;
        indent2 = (double)(region->c2 - breakinfo->textrow[i].c2) / textheight;
#if (WILLUSDEBUGX & 1)
printf("    tr[%d].indent1,2 = %g, %g\n",i,indent1,indent2);
#endif
        /* If only one line and it spans the entire region, call it centered */
        /* Sometimes this won't be the right thing to to. */
        if (i1==i2 && indent1<.5 && indent2<.5)
{
#if (WILLUSDEBUGX & 1)
printf("    One line default to bigger region (%s).\n",breakinfo->centered?"not centered":"centered");
#endif
            return(1);
}
        if (fabs(indent1-indent2) > 1.5)
{
#if (WILLUSDEBUGX & 1)
printf("    Region not centered.\n");
#endif
            return(0);
}
        if (indent1 > 1.0)
            cc++;
        }
#if (WILLUSDEBUGX & 1)
printf("Region centering:  i=%d, i2=%d, cc=%d, ntr=%d\n",i,i2,cc,ntr);
#endif
    if (cc>ntr/2)
{
#if (WILLUSDEBUGX & 1)
printf("    Region is centered (enough obviously centered lines).\n");
#endif
        return(1);
}
#if (WILLUSDEBUGX & 1)
printf("    Not centered (not enough obviously centered lines).\n");
#endif
    return(0);
    }


