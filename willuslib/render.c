/*
** render.c       Bitmap rendering functions.
**                The bitmap pixel numbering starts at the bottom left
**                and goes to the top right.  The coordinate of the bottom
**                left point on the bottom left pixel is (0,0) and the
**                coordinate of the top right point of the top right pixel
**                is (1,1).  The pixels go in integers from (0,0) [bottom
**                left] to (bmap->width-1,bmap->height-1) [top right]
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
#include <math.h>
#include <string.h>
#include <time.h>
#include "willus.h"

static double rtol = 1e-10;

#define min2(a,b)    ((a)<(b) ? (a) : (b))
#define min3(a,b,c)  ((a)<(b) ? min2(a,c) : min2(b,c))
#define max2(a,b)    ((a)>(b) ? (a) : (b))
#define max3(a,b,c)  ((a)>(b) ? max2(a,c) : max2(b,c))

static RENDER_CLIPBOX  _clipbox,_oldclipbox;
static RENDER_CLIPBOX  *clipbox=NULL;

static WILLUSBITMAP *lbmp;
static RENDER_COLOR lfgc,lbgc;
static int lrtype;
static double width_pts,height_pts; /* width/height of bitmap in points */
static double pbx0,pby0; /* pts */
static double dash_lengths[16]; /* points.  starts w/0 for solid */
static double dash_index; /* points */
static double linewidth; /* points */
static int    linecap; /* 0 = butt, 1 = round, 2 = projecting square */

static int render_p2d_same(POINT2D p1,POINT2D p2);
static void render_triangle_1(WILLUSBITMAP *bmap,TRIANGLE2D *srctri,
                              RENDER_COLOR *rcolor,
                              RENDER_COLOR *bgcolor,int render_type);
static void render_horizontal_line(WILLUSBITMAP *bmp,int x0,int y,int x1,
                                   RENDER_COLOR *color);
static void render_horizontal_3d(WILLUSBITMAP *bmp,double *zbuffer,
                                 int x0,int x1,int y,double z0,double z1,
                                 RENDER_COLOR *color,int edgeflag,
                                 RENDER_COLOR *edgecolor);
static void render_triangle_2(WILLUSBITMAP *bmp,TRIANGLE2D *srctri,
                              RENDER_COLOR *color);
static int render_pixel_contained(WILLUSBITMAP *bmp,TRIANGLE2D *tri,int row,
                                  int col);
static int render_row(WILLUSBITMAP *bmap,double y);
static int render_col(WILLUSBITMAP *bmap,double x);
static void vertex_sort(POINT2D *x,int *y,double *z,int n);
static void render_pixelset(WILLUSBITMAP *bmap,int px,int py,RENDER_COLOR *color,
                         int render_type,RENDER_COLOR *bgcolor,double weight);
static unsigned char *bmp_dataptr(WILLUSBITMAP *bmap,int px,int py);
static int corner_after(int clipedge);
static int corner_before(int clipedge);
static double tri2d_intersection_area(WILLUSBITMAP *bmap,TRIANGLE2D *tri,
                                      int row,int col);
static double render_dash_length_pts(void);
static void render_solid_line_pts(double x0,double y0,double x1,double y1);



void render_init(void)

    {
    clipbox=&_clipbox;
    clipbox->p[0] = p2d_point(0.,0.);
    clipbox->p[1] = p2d_point(1.,1.);
    _oldclipbox = _clipbox;
    }


void render_set_clipbox(double x1,double y1,double x2,double y2)

    {
    _oldclipbox = (*clipbox);
    clipbox->p[0] = p2d_point(min2(x1,x2),min2(y1,y2));
    clipbox->p[1] = p2d_point(max2(x1,x2),max2(y1,y2));
    }


void render_set_clipbox_pts(double x1,double y1,double x2,double y2)

    {
    render_set_clipbox(x1/width_pts,y1/height_pts,x2/width_pts,y2/height_pts);
    }


void render_restore_clipbox(double x1,double y1,double x2,double y2)

    {
    (*clipbox) = _oldclipbox;
    }


void render_get_clipbox(double *x1,double *y1,double *x2,double *y2)

    {
    (*x1) = clipbox->p[0].x;
    (*y1) = clipbox->p[0].y;
    (*x2) = clipbox->p[1].x;
    (*y2) = clipbox->p[1].y;
    }


POINT2D p2d_point(double x,double y)

    {
    POINT2D p;
    p.x=x;
    p.y=y;
    return(p);
    }


TRIANGLE2D tri2d_triangle(POINT2D p1,POINT2D p2,POINT2D p3)

    {
    TRIANGLE2D t;
    t.p[0]=p1;
    t.p[1]=p2;
    t.p[2]=p3;
    return(t);
    }


RENDER_COLOR render_color(double r,double g,double b)

    {
    RENDER_COLOR rc;
    rc.rgb[0]=r;
    rc.rgb[1]=g;
    rc.rgb[2]=b;
    return(rc);
    }


void render_circle(WILLUSBITMAP *bmp,double xc,double yc,double xradius,
                   int nsteps,RENDER_COLOR *rcolor,RENDER_COLOR *bgcolor,
                   int render_type)

    {
    int i;
    double ar;

    if (nsteps<0)
        {
        nsteps=xradius*bmp->width*1.33; // Makes a pretty good circle
        if (nsteps<8)
            nsteps=8;
        }
    ar=(double)bmp->width/bmp->height;
    for (i=0;i<nsteps;i++)
        {
        double th1,th2;
        TRIANGLE2D tri;

        th1=i*2.*PI/nsteps;
        th2=(i==nsteps-1)?0.:(i+1)*2.*PI/nsteps;
        tri.p[0].x = xc;
        tri.p[0].y = yc;
        tri.p[1].x = xc+xradius*cos(th1);
        tri.p[1].y = yc+xradius*ar*sin(th1);
        tri.p[2].x = xc+xradius*cos(th2);
        tri.p[2].y = yc+xradius*ar*sin(th2);
        render_triangle(bmp,&tri,rcolor,bgcolor,render_type);
        }
    }


void render_box(WILLUSBITMAP *bmp,double x1,double y1,
                               double x2,double y2,
                               double btx,double bty,
                               RENDER_COLOR *fgc,
                               RENDER_COLOR *bgc,int render_type)

    {
    render_rect(bmp,x1+btx/2.,y1-bty/2.,x2-btx/2.,y1+bty/2.,fgc,bgc,render_type);
    render_rect(bmp,x1+btx/2.,y2-bty/2.,x2-btx/2.,y2+bty/2.,fgc,bgc,render_type);
    render_rect(bmp,x1-btx/2.,y1-bty/2.,x1+btx/2.,y2+bty/2.,fgc,bgc,render_type);
    render_rect(bmp,x2-btx/2.,y1-bty/2.,x2+btx/2.,y2+bty/2.,fgc,bgc,render_type);
    }


void render_rect(WILLUSBITMAP *bmp,double x1,double y1,
                                double x2,double y2,
                                RENDER_COLOR *fgc,RENDER_COLOR *bgc,
                                int render_type)

    {
    TRIANGLE2D t;
/*
    RENDER_COLOR fg,bg;

    bg.rgb[0]=bg.rgb[1]=bg.rgb[2]=1.0;
    fg.rgb[0]=r/255.;
    fg.rgb[1]=g/255.;
    fg.rgb[2]=b/255.;
*/
    t.p[0]=p2d_point(x1,y1);
    t.p[1]=p2d_point(x1,y2);
    t.p[2]=p2d_point(x2,y2);
// printf("(%g,%g) - (%g,%g)\n",x1*bmp->width,y1*bmp->height,x2*bmp->width,y2*bmp->height);
    render_triangle(bmp,&t,fgc,bgc,render_type);
    t.p[0]=p2d_point(x1,y1);
    t.p[1]=p2d_point(x2,y1);
    t.p[2]=p2d_point(x2,y2);
    render_triangle(bmp,&t,fgc,bgc,render_type);
    }



/*
** For render.c, the bitmap pixel numbering starts at the bottom
** left and goes to the top right.  The coordinate of the bottom
** left point on the bottom left pixel is (0,0) and the coordinate
** of the top right point of the top right pixel is (1,1).
** The pixels go in integers from (0,0) [bottom left]
**                             to (bmap->width-1,bmap->height-1) [top right]
**
*/
void render_triangle(WILLUSBITMAP *bmap,TRIANGLE2D *tri,RENDER_COLOR *rcolor,
                     RENDER_COLOR *bgcolor,int render_type)

    {
    TRIANGLE2D    newtri;
    static POINT2D clippath[16];
    int         i,nc;

/*
printf(";render_triangle\n");
for (i=0;i<4;i++)
printf("%6.4f %6.4f\n",tri->p[i%3].x,tri->p[i%3].y);
printf("//nc\n");
*/
    if (clipbox==NULL)
        render_init();
    render_clipped_triangle(clippath,&nc,clipbox,tri);
    for (i=1;i<nc-1;i++)
        {
        newtri = tri2d_triangle(clippath[0],clippath[i],clippath[i+1]);
        render_triangle_1(bmap,&newtri,rcolor,bgcolor,render_type);
        }
    }


/*
** Use the clip box to generate a series of triangles that
** together make up the clipped portion of the source triangle.
** "clippath" should have at least 16 allocated elements.
*/
int render_clipped_triangle(POINT2D *clippath,int *nc,
                            RENDER_CLIPBOX *box,TRIANGLE2D *triangle)

    {
    TRIANGLE2D _tri,*tri;
    double  xmin,xmax;
    static POINT2D vertex[10],cp[4];
    static int     clipedge[10],vin[4];
    static double  position[10];
    int     iv,nv,i,j,ic;
    double  x,y;

    tri=&_tri;
    (*tri) = (*triangle);
    (*nc)=0;
    tri2d_sort_ypoints(tri);
    rtol=p2d_dist(box->p[0],box->p[1])/1e5;
    if (tri->p[0].y >= box->p[1].y || tri->p[2].y <= box->p[0].y)
        return(0);
    xmin = min3(tri->p[0].x,tri->p[1].x,tri->p[2].x);
    xmax = max3(tri->p[0].x,tri->p[1].x,tri->p[2].x);
    if (xmin >= box->p[1].x ||  xmax <= box->p[0].x)
        return(0);
    iv=0;
    for (i=0;i<10;i++)
        clipedge[i]=0;
    for (i=0;i<4;i++)
        vin[i]=0;
    for (i=0;i<3;i++)
        {
        int     i1,i2;

        i1 = i;
        i2 = (i+1)%3;
        if (tri->p[i1].x >= box->p[0].x && tri->p[i1].x <= box->p[1].x
              && tri->p[i1].y >= box->p[0].y && tri->p[i1].y <= box->p[1].y)
            {
            vertex[iv] = tri->p[i1];
            position[iv] = i;
            if (tri->p[i1].x == box->p[0].x)
                clipedge[iv] |= 2;
            else if (tri->p[i1].x == box->p[1].x)
                clipedge[iv] |= 8;
            else if (tri->p[i1].y == box->p[0].y)
                clipedge[iv] |= 1;
            else if (tri->p[i1].y == box->p[1].y)
                clipedge[iv] |= 4;
            else
                clipedge[iv] = 0;
            iv++;
            }
        for (j=0;j<2;j++)
            {
            /* side 1 */
            if ((tri->p[i1].x < box->p[j].x && tri->p[i2].x > box->p[j].x)
                 || (tri->p[i1].x > box->p[j].x && tri->p[i2].x < box->p[j].x))
                {
                POINT2D pp1,pp2;

                y = p2d_y_intercept(box->p[j].x,tri->p[i1],tri->p[i2]);
                pp1.x = pp2.x = box->p[j].x;
                pp1.y = y;
                pp2.y = box->p[0].y;
                /* Check if it's close to a corner. */
                if (render_p2d_same(pp1,pp2))
                    y=box->p[0].y;
                else
                    {
                    pp2.y = box->p[1].y;
                    if (render_p2d_same(pp1,pp2))
                        y=box->p[1].y;
                    }
                if (y>=box->p[0].y && y<= box->p[1].y)
                    {
                    vertex[iv].x = box->p[j].x;
                    vertex[iv].y = y;
                    clipedge[iv] |= 1<<(j*2+1);
                    position[iv] = i+(box->p[j].x-tri->p[i1].x)
                                      /(tri->p[i2].x-tri->p[i1].x);
                    iv++;
                    }
                }
            /* side 2 */
            if ((tri->p[i1].y < box->p[j].y && tri->p[i2].y > box->p[j].y)
                 || (tri->p[i1].y > box->p[j].y && tri->p[i2].y < box->p[j].y))
                {
                POINT2D pp1,pp2;
                
                x = p2d_x_intercept(box->p[j].y,tri->p[i1],tri->p[i2]);
                pp1.x = x;
                pp1.y = pp2.y = box->p[j].y;
                pp2.x = box->p[0].x;
                /* Check if it's close to a corner. */
                if (render_p2d_same(pp1,pp2))
                    x=box->p[0].x;
                else
                    {
                    pp2.x = box->p[1].x;
                    if (render_p2d_same(pp1,pp2))
                        x=box->p[1].x;
                    }
                if (x>= box->p[0].x && x<= box->p[1].x)
                    {
                    vertex[iv].x = x;
                    vertex[iv].y = box->p[j].y;
                    clipedge[iv] |= 1<<(j*2);
                    position[iv] = i+(box->p[j].y-tri->p[i1].y)
                                   /(tri->p[i2].y-tri->p[i1].y);
                    iv++;
                    }
                }
            }
        }
    nv=iv;
    cp[0] = box->p[0];
    cp[1] = p2d_point(box->p[0].x,box->p[1].y);
    cp[2] = box->p[1];
    cp[3] = p2d_point(box->p[1].x,box->p[0].y);
    /* Check any unevaluated corners to see if they're in. */
    for (ic=i=0;i<4;i++)
        {
        if (!vin[i])
            vin[i] = tri2d_point_inside(tri,cp[i]);
        if (vin[i])
            {
            ic++;
            }
        }
    vertex_sort(vertex,clipedge,position,nv);
    /* Remove duplicate points */
    for (i=0;i<nv;i++)
        for (j=i+1;j<nv;j++)
            if (render_p2d_same(vertex[i],vertex[j]))
                {
                int k;
                for (k=j+1;k<nv;k++)
                    {
                    vertex[k-1]=vertex[k];
                    clipedge[k-1]=clipedge[k];
                    position[k-1]=position[k];
                    }
                j--;
                nv--;
                }
    if (ic==4 || nv<2)
        {
        if (ic>1)
            {
            clippath[0] = box->p[0];
            clippath[1] = p2d_point(box->p[0].x,box->p[1].y);
            clippath[2] = box->p[1];
            clippath[3] = p2d_point(box->p[1].x,box->p[0].y);
            (*nc)=4;
            return(2);
            }
        else
            return(0);
        }

    /* Now figure out the clip path */
    iv=0;
    clippath[iv++] = vertex[0];
    for (i=1;i<=nv;i++)
        {
        int     i2;

        i2=i%nv;
        if (clipedge[i-1]>0 && clipedge[i2]>0
                            && (clipedge[i-1]&clipedge[i2])==0)
            {
            int   jend;

            jend = (corner_before(clipedge[i2])+1)%4;
            for (j=corner_after(clipedge[i-1]);j!=jend;j=(j+1)%4)
                if (!vin[j])
                    break;
            if (j==jend)
                {
                for (j=corner_after(clipedge[i-1]);j!=jend;j=(j+1)%4)
                    {
                    if (render_p2d_same(cp[j],clippath[iv-1]))
                        continue;
                    clippath[iv++]=cp[j];
                    }
                }
            else
                {
                jend = (corner_after(clipedge[i2])+3)%4;
                for (j=corner_before(clipedge[i-1]);j!=jend;j=(j+3)%4)
                    if (!vin[j])
                        break;
                if (j==jend)
                    {
                    for (j=corner_before(clipedge[i-1]);j!=jend;j=(j+3)%4)
                        {
                        if (render_p2d_same(cp[j],clippath[iv-1]))
                            continue;
                        clippath[iv++]=cp[j];
                        }
                    }
                }
            }
        if (!render_p2d_same(clippath[iv-1],vertex[i2]))
            clippath[iv++]=vertex[i2];
        if (i==nv-1 && nv==2)
            {
            iv++;
            break;
            }
        }
    nv = iv-1;
    for (i=0;i<nv;i++)
        for (j=i+1;j<nv;j++)
            if (render_p2d_same(clippath[i],clippath[j]))
                {
                int k;
                for (k=j+1;k<nv;k++)
                    clippath[k-1]=clippath[k];
                j--;
                nv--;
                }
    /* Add any corners that are in but weren't included */
    for (i=0;i<4;i++)
        if (vin[i])
            {
            for (j=0;j<nv;j++)
                if (render_p2d_same(clippath[j],cp[i]))
                    break;
            if (j>=nv)
                clippath[nv++] = cp[i];
            }
    (*nc)=nv;
    return(1);
    }


static int render_p2d_same(POINT2D p1,POINT2D p2)

    {
    return(p2d_dist(p1,p2)<rtol);
    }


/*
** Triangles passed to this function have already been clipped by
** the clip box.
*/
static void render_triangle_1(WILLUSBITMAP *bmap,TRIANGLE2D *srctri,
                              RENDER_COLOR *rcolor,
                              RENDER_COLOR *bgcolor,int render_type)

    {
    TRIANGLE2D *tri,_tri;
    int     row,bottom_row,top_row;

/*
{
int i;
printf("@render_triangle_1\n");
for (i=0;i<4;i++)
printf("%6.4f %6.4f\n",srctri->p[i%3].x*bmap->width,srctri->p[i%3].y*bmap->height);
printf("//nc\n");
}
*/
    if (render_type==RENDER_TYPE_SET)
        {
        render_triangle_2(bmap,srctri,rcolor);
        return;
        }
    tri=&_tri;
    if (tri2d_zero_area(srctri))
        return;
    (*tri)=(*srctri);
    tri2d_sort_ypoints(tri);
    bottom_row = render_row(bmap,tri->p[0].y);
    top_row = render_row(bmap,tri->p[2].y);
    for (row=bottom_row;row<=top_row;row++)
        {
        int     nx,i,j,k,col,left_col,right_col;
        double  y0,y1;
        static double x[9];

// printf("row=%d\n",row);
        y0 = (double)row/bmap->height;
        y1 = (double)(row+1)/bmap->height;
        i=0;
        /* Create array of possible extreme x-coords */
        /* Triangle vertices */
        for (j=0;j<3;j++)
            if (y0<=tri->p[j].y && y1>=tri->p[j].y)
                x[i++] = tri->p[j].x;
        /* Segments intercepting y0 */
        for (j=0;j<2;j++)
            for (k=j+1;k<3;k++)
                if (tri->p[j].y < y0 && tri->p[k].y > y0)
                    x[i++] = p2d_x_intercept(y0,tri->p[j],tri->p[k]);
        /* Segments intercepting y1 */
        for (j=0;j<2;j++)
            for (k=j+1;k<3;k++)
                if (tri->p[j].y < y1 && tri->p[k].y > y1)
                    x[i++] = p2d_x_intercept(y1,tri->p[j],tri->p[k]);
        nx=i;
        left_col  = render_col(bmap,array_min(x,nx));
        right_col = render_col(bmap,array_max(x,nx));
// printf("    %d to %d\n",left_col,right_col);
        for (col=left_col;col<=right_col;col++)
            {
            if (render_type==RENDER_TYPE_ANTIALIASED)
                render_pixelset(bmap,col,row,rcolor,
                       render_type,bgcolor,
                       tri2d_intersection_area(bmap,tri,row,col));
            else
                if (render_pixel_contained(bmap,tri,row,col))
                    render_pixelset(bmap,col,row,rcolor,
                                      render_type,bgcolor,1.);
            }
        }
    }


static void render_horizontal_line(WILLUSBITMAP *bmp,int x0,int y,int x1,
                                   RENDER_COLOR *color)

    {
    int     x;
    unsigned char *p;
    int     i,ci[3];

    for (i=0;i<3;i++)
        ci[i] = bmp->type==WILLUSBITMAP_TYPE_WIN32 ? 2-i : i;
    for (x=x0,p=bmp_dataptr(bmp,x,y);x<=x1;x++,p+=3)
        for (i=0;i<3;i++)
            p[ci[i]] = color->rgb[i]*255.99;
    }


#define    pswap(x1,y1,x2,y2) {int t; t=x1;x1=x2;x2=t;t=y1;y1=y2;y2=t;}
#define    dswap(x1,y1,x2,y2) {double t; t=x1;x1=x2;x2=t;t=y1;y1=y2;y2=t;}
/*
** Uses G-drive non-anti-aliased method to draw triangle.  Set only for now.
*/
static void render_triangle_2(WILLUSBITMAP *bmp,TRIANGLE2D *srctri,
                              RENDER_COLOR *color)

    {
    double  x1,y1,x2,y2,x3,y3,ylast;
    int    *pattern;
    int     def[2] = {1,0xffff};
    double  px1,py1,px2,py2,px3,py3;
    double  ldy,rdy;
    double  x1clip,x2clip,y1clip,y2clip;
    double  ldx,rdx;
    int     lx,rx,y,yi,yf,yinc;
/*
printf("@rt2 (%6.4f,%6.4f)-(%6.4f,%6.4f)-(%6.4f,%6.4f)\n",
srctri->p[0].x,srctri->p[0].y,
srctri->p[1].x,srctri->p[1].y,
srctri->p[2].x,srctri->p[2].y);
*/

    pattern = NULL;
    x1 = srctri->p[0].x;
    y1 = srctri->p[0].y;
    x2 = srctri->p[1].x;
    y2 = srctri->p[1].y;
    x3 = srctri->p[2].x;
    y3 = srctri->p[2].y;
    x1clip=0;
    x2clip=bmp->width;
    y1clip=0;
    y2clip=bmp->height;
/*
    px1=render_col(bmp,x1);
    py1=render_row(bmp,y1);
    px2=render_col(bmp,x2);
    py2=render_row(bmp,y2);
    px3=render_col(bmp,x3);
    py3=render_row(bmp,y3);
*/
    px1=x1*bmp->width;
    py1=y1*bmp->height;
    px2=x2*bmp->width;
    py2=y2*bmp->height;
    px3=x3*bmp->width;
    py3=y3*bmp->height;
    if (py1>py2)
        dswap(px1,py1,px2,py2);
    if (py2>py3)
        dswap(px2,py2,px3,py3);
    if (py1>py2)
        dswap(px1,py1,px2,py2);
    if (py1>y2clip || py3<y1clip)
        return;
    if (pattern==NULL)
        pattern=def;
    if (py1==py2 && py2==py3)
        {
        lx = min3(px1,px2,px3);
        rx = max3(px1,px2,px3);
        if (lx>x2clip || rx<x1clip)
            return;
        if (lx<x1clip)
            lx=x1clip;
        if (rx>x2clip)
            rx=x2clip;
        }
    if (py1==py3)
        x1=(double)(px1+px3)/2.;
    else
        x1=px1+(double)(px3-px1)*(double)(py2-py1)/(double)(py3-py1);
    yinc=1;
// printf("py1=%7.2f, py2=%7.2f\n",py1,py2);
    if (py2>=y1clip && py2!=py1)
        {
        yi = floor((py1>y1clip ? py1 : y1clip)+.5);
        yf = floor((py2<y2clip ? py2 : y2clip)-.5);
// printf("yi=%d, yf=%d\n",yi,yf);
        if (x1>(double)px2)
            {
            ldx=px2-px1;
            rdx=px3-px1;
            ldy=py2-py1;
            rdy=py3-py1;
            }
        else
            {
            ldx=px3-px1;
            rdx=px2-px1;
            ldy=py3-py1;
            rdy=py2-py1;
            }
        for (y=yi;y<=yf;y+=yinc)
            {
            lx=floor((px1+ldx*(y+.5-py1)/ldy)+.5);
            rx=floor((px1+rdx*(y+.5-py1)/rdy)-.5);
// printf("lx,rx[%d] = %d, %d\n",y,lx,rx);
            if (lx>rx)
                continue;
            if (lx>x2clip || rx<x1clip)
                continue;
            if (lx<x1clip)
                lx=x1clip;
            if (rx>x2clip)
                rx=x2clip;
            if (lx>rx)
                continue;
            render_horizontal_line(bmp,lx,y,rx,color);
                /*
                if ((status=hlinepat(lx,y,rx,pen_color,pattern[y%pattern[0]+1]))!=NO_ERROR)
                    return(status);
                */
            }
        }
    ylast=py2;
// printf("ylast=%7.2f, py3=%7.2f\n",ylast,py3);
    if (ylast<=y2clip && py2!=py3)
        {
        yi = floor((ylast>y1clip ? ylast : y1clip)+.5);
        yf = floor((py3<y2clip ? py3 : y2clip)-.5);
// printf("yi=%d, yf=%d\n",yi,yf);
        if (x1>px2)
            {
            ldx=px2-px3;
            rdx=px1-px3;
            ldy=py3-py2;
            rdy=py3-py1;
            }
        else
            {
            ldx=px1-px3;
            rdx=px2-px3;
            ldy=py3-py1;
            rdy=py3-py2;
            }
// printf("px3=%g, ldx=%g, rdx=%g, ldy=%g, rdy=%g\n",px3,ldx,rdx,ldy,rdy);
        for (y=yi;y<=yf;y+=yinc)
            {
            lx=floor((px3+ldx*(py3-(y+.5))/ldy)+.5);
            rx=floor((px3+rdx*(py3-(y+.5))/rdy)-.5);
// printf("lx,rxdp[%d] = %15.10f, %15.10f\n",y,px3+ldx*(py3-(y+.5))/ldy,px3+rdx*(py3-(y+.5))/rdy);
// printf("lx,rx[%d] = %d, %d\n",y,lx,rx);
            if (lx>x2clip || rx<x1clip)
                continue;
            if (lx<x1clip)
                lx=x1clip;
            if (rx>x2clip)
                rx=x2clip;
            if (lx>rx)
                continue;
            render_horizontal_line(bmp,lx,y,rx,color);
            /*
            if ((status=hlinepat(lx,y,rx,pen_color,pattern[y%pattern[0]+1]))!=NO_ERROR)
                return(status);
            */
            }
        }
    }


#define pswap3d(x1,y1,z1,x2,y2,z2) {double d; d=x1;x1=x2;x2=d;d=y1;y1=y2;y2=d;d=z1;z1=z2;z2=d;}
/*
**
** Flushed a lot of bugs out of this on 9-26-06.
**
** Draw 3D triangle using Z-buffer to correctly display triangles in front
** of each other.
**
*/
void render_triangle_3d(WILLUSBITMAP *bmp,double *zbuffer,
                        TRIANGLE3D *srctri,RENDER_COLOR *color,
                        RENDER_COLOR *edge_color)

    {
    double  x1,y1,z1,x2,y2,z2,x3,y3,z3,ylast;
    double  px1,py1,px2,py2,px3,py3,ldx,rdx,ldy,rdy,ldz,rdz;
    double  x1clip,x2clip,y1clip,y2clip;
    double  lz,rz;
    int     yi,yf,lx,rx,y,edgeflag;

// printf("render_triangle_3d.\n");
    x1 = srctri->p[0].x;
    y1 = srctri->p[0].y;
    z1 = srctri->p[0].z;
    x2 = srctri->p[1].x;
    y2 = srctri->p[1].y;
    z2 = srctri->p[1].z;
    x3 = srctri->p[2].x;
    y3 = srctri->p[2].y;
    z3 = srctri->p[2].z;
// printf("/td xy.\n");
// printf("    %9.5f %9.5f %9.5f\n",x1,y1,z1);
// printf("    %9.5f %9.5f %9.5f\n",x2,y2,z2);
// printf("    %9.5f %9.5f %9.5f\n",x3,y3,z3);
// printf("    %9.5f %9.5f %9.5f\n",x1,y1,z1);
// printf("//nc\n");
    x1clip=0;
    x2clip=bmp->width;
    y1clip=0;
    y2clip=bmp->height;

    px1=x1*bmp->width;
    py1=y1*bmp->height;
    px2=x2*bmp->width;
    py2=y2*bmp->height;
    px3=x3*bmp->width;
    py3=y3*bmp->height;
    if (py1>py2)
        pswap3d(px1,py1,z1,px2,py2,z2);
    if (py2>py3)
        pswap3d(px2,py2,z2,px3,py3,z3);
    if (py1>py2)
        pswap3d(px1,py1,z1,px2,py2,z2);
    if (py1>y2clip || py3<y1clip)
        return;
    if (py1==py2 && py2==py3)
        {
        lx = min3(px1,px2,px3);
        rx = max3(px1,px2,px3);
        if (lx>x2clip || rx<x1clip)
            return;
        if (lx<x1clip)
            lx=x1clip;
        if (rx>x2clip)
            rx=x2clip;
        }
    if (py1==py3)
        x1=(double)(px1+px3)/2.;
    else
        x1=px1+(double)(px3-px1)*(double)(py2-py1)/(double)(py3-py1);
    if (py2>=y1clip && py2!=py1)
        {
        yi = floor((py1>y1clip ? py1 : y1clip)+.5);
        yf = floor((py2<y2clip ? py2 : y2clip)-.5);
        if (x1>(double)px2)
            {
            ldx=px2-px1;
            rdx=px3-px1;
            ldy=py2-py1;
            rdy=py3-py1;
            ldz=z2-z1;
            rdz=z3-z1;
            }
        else
            {
            ldx=px3-px1;
            rdx=px2-px1;
            ldy=py3-py1;
            rdy=py2-py1;
            ldz=z3-z1;
            rdz=z2-z1;
            }
        for (y=yi;y<=yf;y++)
            {
            double lx1,rx1,lz1,rz1,dx,dz;

            lx1=px1+ldx*(y-py1)/ldy;
            rx1=px1+rdx*(y-py1)/rdy;
            dx=rx1-lx1;
            lx=floor((px1+ldx*(y+.5-py1)/ldy)+.5);
            rx=floor((px1+rdx*(y+.5-py1)/rdy)-.5);
            if (lx>rx)
                continue;
            if (lx>x2clip || rx<x1clip)
                continue;
            edgeflag=3;
            if (lx<x1clip)
                {
                edgeflag&=(~1);
                lx=x1clip;
                }
            if (rx>x2clip)
                {
                edgeflag&=(~2);
                rx=x2clip;
                }
            if (lx>rx)
                continue;
            // lz = ldx==0 ? (z1+ldz/2.) : z1 + ldz*(lx-px1)/ldx;
            // rz = rdx==0 ? (z1+rdz/2.) : z1 + rdz*(rx-px1)/rdx;
            lz1 = z1+ldz*(y-py1)/ldy;
            rz1 = z1+rdz*(y-py1)/rdy;
            dz = rz1-lz1;
            if (dx==0)
                {
                lz=lz1;
                rz=rz1;
                }
            else
                {
                lz=lz1+(lx-lx1)*dz/dx;
                rz=lz1+(rx-lx1)*dz/dx;
                }
            if ((y==py1 && y==py2) || (y==py2 && y==py3))
                edgeflag|=4;
            render_horizontal_3d(bmp,zbuffer,lx,rx,y,lz,rz,color,
                                 edgeflag,edge_color);
            }
        }
    ylast=py2;
    if (ylast<=y2clip && py2!=py3)
        {
        yi = floor((ylast>y1clip ? ylast : y1clip)+.5);
        yf = floor((py3<y2clip ? py3 : y2clip)-.5);
        if (x1>px2)
            {
            ldx=px2-px3;
            rdx=px1-px3;
            ldy=py3-py2;
            rdy=py3-py1;
            ldz=z2-z3;
            rdz=z1-z3;
            }
        else
            {
            ldx=px1-px3;
            rdx=px2-px3;
            ldy=py3-py1;
            rdy=py3-py2;
            ldz=z1-z3;
            rdz=z2-z3;
            }
        for (y=yi;y<=yf;y++)
            {
            double lx1,rx1,lz1,rz1,dx,dz;

            lx1=px3+ldx*(py3-y)/ldy;
            rx1=px3+rdx*(py3-y)/rdy;
            dx=rx1-lx1;
            lx=floor((px3+ldx*(py3-(y+.5))/ldy)+.5);
            rx=floor((px3+rdx*(py3-(y+.5))/rdy)-.5);
            edgeflag=3;
            if (lx>x2clip || rx<x1clip)
                continue;
            if (lx<x1clip)
                {
                edgeflag&=(~1);
                lx=x1clip;
                }
            if (rx>x2clip)
                {
                edgeflag&=(~2);
                rx=x2clip;
                }
            if (lx>rx)
                continue;
            // lz = ldx==0 ? (z3+ldz/2.) : z3 + ldz*(lx-px3)/ldx;
            // rz = rdx==0 ? (z3+rdz/2.) : z3 + rdz*(rx-px3)/rdx;
            lz1 = z3+ldz*(py3-y)/ldy;
            rz1 = z3+rdz*(py3-y)/rdy;
            dz = rz1-lz1;
            if (dx==0)
                {
                lz=lz1;
                rz=rz1;
                }
            else
                {
                lz=lz1+(lx-lx1)*dz/dx;
                rz=lz1+(rx-lx1)*dz/dx;
                }
            if ((y==py1 && y==py2) || (y==py2 && y==py3))
                edgeflag|=4;
            render_horizontal_3d(bmp,zbuffer,lx,rx,y,lz,rz,color,
                                 edgeflag,edge_color);
            }
        }
    }


static void render_horizontal_3d(WILLUSBITMAP *bmp,double *zbuffer,
                                 int x0,int x1,int y,double z0,double z1,
                                 RENDER_COLOR *color,int edgeflag,
                                 RENDER_COLOR *edgecolor)

    {
    int     x,i,ci[3];
    unsigned char *p;
    double  *zb;
    double  z,dz;

    dz = x1==x0 ? 0. : (z1-z0)/(x1-x0);
    for (i=0;i<3;i++)
        ci[i] = bmp->type==WILLUSBITMAP_TYPE_WIN32 ? 2-i : i;
    for (z=z0,x=x0,zb=&zbuffer[y*bmp->width+x0],p=bmp_dataptr(bmp,x,y);x<=x1;x++,p+=3,z+=dz,zb++)
        if (z < (*zb))
            {
            if ((edgeflag&4)
                 || (x==x0 && (edgeflag&1))
                 || (x==x1 && (edgeflag&2)))
                for (i=0;i<3;i++)
                    p[ci[i]] = edgecolor->rgb[i]*255.99;
            else
                for (i=0;i<3;i++)
                    p[ci[i]] = color->rgb[i]*255.99;
            (*zb) = z;
            }
    }


static int render_pixel_contained(WILLUSBITMAP *bmp,TRIANGLE2D *tri,int row,
                                  int col)

    {
    return(tri2d_intersection_area(bmp,tri,row,col) > 0.);
    }


static int render_row(WILLUSBITMAP *bmap,double y)

    {
    int     row;

    row=floor(y*bmap->height);
    if (row<0)
        row=0;
    if (row>bmap->height-1)
        row=bmap->height-1;
    return(row);
    }


static int render_col(WILLUSBITMAP *bmap,double x)

    {
    int     col;

    col=floor(x*bmap->width);
    if (col<0)
        col=0;
    if (col>bmap->width-1)
        col=bmap->width-1;
    return(col);
    }


static int corner_after(int clipedge)

    {
    if ((clipedge&0xc)==4)
        return(2);
    else if ((clipedge&0x6)==2)
        return(1);
    else if ((clipedge&0x3)==1)
        return(0);
    return(3);
    }


static int corner_before(int clipedge)

    {
    if ((clipedge&0xc)==8)
        return(2);
    else if ((clipedge&0x6)==4)
        return(1);
    else if ((clipedge&0x3)==2)
        return(0);
    return(3);
    }


static void vertex_sort(POINT2D *x,int *y,double *z,int n)

    {
    int     top,n1;
    POINT2D x0;
    int     y0;
    double  z0;

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
            if (child<n1 && z[child]<z[child+1])
                child++;
            if (z0<z[child])
                {
                x[parent]=x[child];
                y[parent]=y[child];
                z[parent]=z[child];
                parent=child;
                child+=(parent+1);
                }
            else
                child=n1+1;
            }
        x[parent]=x0;
        y[parent]=y0;
        z[parent]=z0;
        }
        }
    }


/*
** py is from BOTTOM of bitmap!
*/
static void render_pixelset(WILLUSBITMAP *bmap,int px,int py,RENDER_COLOR *color,
                         int render_type,RENDER_COLOR *bgcolor,double weight)

    {
    unsigned char *p;
    int     i,ci[3];
    double  d1,d2,d;

    p = bmp_dataptr(bmap,px,py);
    for (i=0;i<3;i++)
        ci[i] = bmap->type==WILLUSBITMAP_TYPE_WIN32 ? 2-i : i;
    switch (render_type)
        {
        case RENDER_TYPE_SET:
            for (i=0;i<3;i++)
                p[ci[i]] = color->rgb[i]*255.99;
            break;
        case RENDER_TYPE_OR:
            for (i=0;i<3;i++)
                {
                d1 = fabs(p[ci[i]]/255. - bgcolor->rgb[i]);
                d2 = fabs(color->rgb[i] - bgcolor->rgb[i]);
                if (d2>d1)
                    p[ci[i]] = color->rgb[i]*255.99;
                }
            break;
        case RENDER_TYPE_XOR:
            for (i=0;i<3;i++)
                {
                d1 = fabs(p[ci[i]]/255. - bgcolor->rgb[i]);
                d2 = fabs(color->rgb[i] - bgcolor->rgb[i]);
                if (bgcolor->rgb[i]>0.5)
                    d = bgcolor->rgb[i] - fabs(d2-d1);
                else
                    d = bgcolor->rgb[i] + fabs(d2-d1);
                if (d<0.)
                    d = 0.;
                else if (d>1.)
                    d = 1.;
                p[ci[i]] = d*255.99;
                }
            break;
        case RENDER_TYPE_ADD:
            for (i=0;i<3;i++)
                {
                d1 = p[ci[i]]/255. - bgcolor->rgb[i];
                d2 = color->rgb[i] - bgcolor->rgb[i];
                d=d1+d2+bgcolor->rgb[i];
                if (d<0.)
                    d=0.;
                else if (d>1.)
                    d=1.;
                p[ci[i]] = d*255.99;
                }
            break;
        case RENDER_TYPE_ANTIALIASED:
            if (weight>0.)
                for (i=0;i<3;i++)
                    {
                    double d2,d;

                    d2 = color->rgb[i] - bgcolor->rgb[i];
                    if (d2==0.)
                        continue;
                    d = p[ci[i]]/255. + d2*weight;
                    if (d<0.)
                        d=0.;
                    else if (d>1.)
                        d=1.;
                    p[ci[i]] = d*255.99;
                    }
            break;
        }
    }


static unsigned char *bmp_dataptr(WILLUSBITMAP *bmap,int px,int py)

    {
    if (bmap->type==WILLUSBITMAP_TYPE_NATIVE)
        return(&bmap->data[(bmap->height-1-py)*bmp_bytewidth(bmap)+px*3]);
    return(&bmap->data[py*bmp_bytewidth(bmap)+px*3]);
    }


/*
** Return area of intersection between triangle "tri" and
** bitmap rectangular coordinate at (row,col).
*/
static double tri2d_intersection_area(WILLUSBITMAP *bmap,TRIANGLE2D *tri,
                                      int row,int col)

    {
    TRIANGLE2D    newtri;
    RENDER_CLIPBOX     box;
    int         i,status;
    double      area;
    static POINT2D clippath[16];
    int nc;

    box.p[0].y = (double)row/bmap->height;
    box.p[1].y = (double)(row+1)/bmap->height;
    box.p[0].x = (double)col/bmap->width;
    box.p[1].x = (double)(col+1)/bmap->width;
    status=render_clipped_triangle(clippath,&nc,&box,tri);
    if (status==2)
        return(1.0);
    for (area=0.,i=1;i<nc-1;i++)
        {
        double a;

        newtri=tri2d_triangle(clippath[0],clippath[i],clippath[i+1]);
        a=tri2d_area(&newtri);
        area += a;
        }
    return(area*bmap->height*bmap->width);
    }


void rendercolor_rainbow_colorscale(RENDER_COLOR *rcolor,double scale)

    {
    if (scale<0.5)
        rcolor->rgb[2]=1.-scale*2.;
    else
        rcolor->rgb[2]=0.;
    if (scale<0.5)
        rcolor->rgb[1]=scale*2.;
    else
        rcolor->rgb[1]=1.-(scale-0.5)*2.;
    if (scale>0.5)
        rcolor->rgb[0]=(scale-0.5)*2.;
    else
        rcolor->rgb[0]=0.;
    }


int intcolor_from_rendercolor(RENDER_COLOR *rcolor)

    {
    int i,ibest;
    RENDER_COLOR rbest;
    double dbest;

    ibest=0;
    rbest=rendercolor_from_intcolor(ibest);
    dbest=rendercolor_diff(&rbest,rcolor);
    for (ibest=0,i=1;i<16;i++)
        {
        RENDER_COLOR rc;
        double d;
        rc=rendercolor_from_intcolor(i);
        d=rendercolor_diff(&rc,rcolor);
        if (d < dbest)
            {
            ibest=i;
            rbest=rc;
            dbest=d;
            }
        }
    return(ibest);
    }


RENDER_COLOR rendercolor_from_intcolor(int i)

    {
    int r,g,b,br;
    RENDER_COLOR rc;

    br=(i&8)>>3;
    r=(i&4)>>2;
    g=(i&2)>>1;
    b=i&1;
    if (i==8)
        {
        rc.rgb[0]=.5;
        rc.rgb[1]=.5;
        rc.rgb[2]=.5;
        return(rc);
        }
    if (br)
        {
        rc.rgb[0]=r;
        rc.rgb[1]=g;
        rc.rgb[2]=b;
        }
    else
        {
        rc.rgb[0]=r*.5;
        rc.rgb[1]=g*.5;
        rc.rgb[2]=b*.5;
        }
    return(rc);
    }


double rendercolor_diff(RENDER_COLOR *rc1,RENDER_COLOR *rc2)

    {
    return(sqrt((rc1->rgb[0]-rc2->rgb[0])*(rc1->rgb[0]-rc2->rgb[0])
              + (rc1->rgb[1]-rc2->rgb[1])*(rc1->rgb[1]-rc2->rgb[1])
              + (rc1->rgb[2]-rc2->rgb[2])*(rc1->rgb[2]-rc2->rgb[2])));
    }


void render_set_point_size(WILLUSBITMAP *bmp,double width,double height)

    {
    if (bmp!=NULL)
        lbmp=bmp;
    if (width>0)
        width_pts = width;
    if (height>0)
        height_pts = height;
    }


void render_set_fg_bg_rtype(RENDER_COLOR *fgc,RENDER_COLOR *bgc,
                                   int render_type)

    {
    if (fgc!=NULL)
        lfgc = (*fgc);
    if (bgc!=NULL)
        lbgc = (*bgc);
    if (render_type >= 0)
        lrtype = render_type;
    }


int render_get_rtype(void)

    {
    return(lrtype);
    }


void render_setlinewidth_pts(double lw)

    {
    linewidth=lw;
    }


/* 0=butt, 1=round, 2=projecting square */
void render_setlinecap(int lc)

    {
    linecap=lc;
    }


void render_setdash_pts(double *dash,double offset)

    {
    int i;

    if (dash==NULL)
        {
        dash_lengths[0]=0.;
        dash_index=0.;
        return;
        }
    for (i=0;i<15 && dash[i]>0.;i++)
        dash_lengths[i]=dash[i];
    dash_lengths[i]=0.;
    dash_index=offset;
    }


double render_dash_index(void)

    {
    return(dash_index);
    }


void render_position_pixels(double *xpx,double *ypx)

    {
    if (xpx!=NULL)
        (*xpx)=pbx0*lbmp->width/width_pts;
    if (ypx!=NULL)
        (*ypx)=pby0*lbmp->height/height_pts;
    }
  
 
void render_position_pts(double *xpts,double *ypts)

    {
    if (xpts!=NULL)
        (*xpts)=pbx0;
    if (ypts!=NULL)
        (*ypts)=pby0;
    }


void render_moveto_pts(double x,double y) /* pts */

    {
    pbx0=x;
    pby0=y;
    }


void render_dash_index_and_offset(double index,int *ii,double *dashoff)

    {
    double len;
    int i;

    len=render_dash_length_pts();
    index=fmod(index,len);
    for (i=0,len=0.;i<16 && dash_lengths[i]>0 && len+dash_lengths[i]<index;i++)
        len += dash_lengths[i];
    (*ii)=i;
    (*dashoff) = index-len;
    }


static double render_dash_length_pts(void)

    {
    int i;
    double len;

    for (i=0,len=0.;i<16 && dash_lengths[i]>0;i++)
        len+=dash_lengths[i];
    if (i==0)
        return(0.);
    if ((i&1)==1)
        len+=dash_lengths[i-1];
    return(len);
    }


void render_lineto_pts(double x,double y) /* pts */

    {
    int id1;
    double do1,x1,y1,dx,dy,len,dlen;

    dx = x-pbx0;
    dy = y-pby0;
    len = sqrt(dx*dx+dy*dy);
    if (dash_lengths[0]<=0.)
        {
        render_solid_line_pts(pbx0,pby0,x,y);
        pbx0=x;
        pby0=y;
        return;
        }
    dlen = render_dash_length_pts();
    render_dash_index_and_offset(dash_index,&id1,&do1);
// printf("rlt(%7.2f,%7.2f) [di=%7.2f pts, len=%7.2f pts, dlen=%7.2f pts] id1=%d, do1=%7.2f\n",x,y,dash_index,len,dlen,id1,do1);
    dash_index=fmod(dash_index+len,dlen);
    while (1)
        {
        double dl,ltg;

        dl = dash_lengths[id1]==0 ? dash_lengths[id1-1] : dash_lengths[id1];
        ltg = dl-do1;
        if (len <= ltg)
            {
            if (!(id1&1))
                render_solid_line_pts(pbx0,pby0,x,y);
            break;
            }
        x1 = pbx0 + dx*ltg/len;
        y1 = pby0 + dy*ltg/len;
        if (!(id1&1))
            render_solid_line_pts(pbx0,pby0,x1,y1);
        pbx0 = x1;
        pby0 = y1;
        dx = x-pbx0;
        dy = y-pby0;
        len = sqrt(dx*dx+dy*dy);
        do1=0.;
        if (dash_lengths[id1]==0)
            id1=0;
        else
            {
            id1++;
            if (dash_lengths[id1]<=0 && (id1&1)==0)
                id1=0;
            }
        }
    pbx0=x;
    pby0=y;
    }


void render_line_pts(double x1,double y1,double x2,double y2)

    {
    render_moveto_pts(x1,y1);
    render_lineto_pts(x2,y2);
    }


void render_outline_rect_pts(double x1,double y1,double x2,double y2)

    {
    render_moveto_pts(x1,y1);
    render_lineto_pts(x2,y1);
    render_lineto_pts(x2,y2);
    render_lineto_pts(x1,y2);
    render_lineto_pts(x1,y1);
    }


/*
** x0,y0,x1,y1 in points
*/
static void render_solid_line_pts(double x0,double y0,double x1,double y1)

    {
    double theta;
    double dx,dy,r;


    dx=x1-x0;
    dy=y1-y0;
    r=linewidth/2.;
    if (fabs(dx)>1e-8 || fabs(dy)>1e-8)
        theta=atan2(dy,dx);
    else
        theta=0.;
    switch (linecap)
        {
        case 0: /* butt */
        case 1: /* round */
            {
            double xa,ya,xb,yb,xc,yc,xd,yd;
            xa=x0+r*cos(theta-PI/2.);
            ya=y0+r*sin(theta-PI/2.);
            xb=x0+r*cos(theta+PI/2.);
            yb=y0+r*sin(theta+PI/2.);
            xc=x1+r*cos(theta+PI/2.);
            yc=y1+r*sin(theta+PI/2.);
            xd=x1+r*cos(theta-PI/2.);
            yd=y1+r*sin(theta-PI/2.);
            render_rect_pts(xa,ya,xb,yb,xc,yc,xd,yd);
            if (linecap==0)
                break;
            render_partial_circle_pts(x0,y0,r,theta+PI/2.,theta+3.*PI/2.,-1);
            render_partial_circle_pts(x1,y1,r,theta-PI/2.,theta+PI/2.,-1);
            break;
            }
        case 2: /* project */
            {
            double xa,ya,xb,yb,xc,yc,xd,yd;
            double r2;
            r2=r*SQRT2;
            xa=x0+r2*cos(theta-1.5*PI/2.);
            ya=y0+r2*sin(theta-1.5*PI/2.);
            xb=x0+r2*cos(theta+1.5*PI/2.);
            yb=y0+r2*sin(theta+1.5*PI/2.);
            xc=x1+r2*cos(theta+PI/4.);
            yc=y1+r2*sin(theta+PI/4.);
            xd=x1+r2*cos(theta-PI/4.);
            yd=y1+r2*sin(theta-PI/4.);
            render_rect_pts(xa,ya,xb,yb,xc,yc,xd,yd);
            break;
            }
        }
    }


void render_rect_pts(double x1,double y1,double x2,double y2,
                     double x3,double y3,double x4,double y4)

    {
    render_tri_pts(x1,y1,x2,y2,x4,y4);
    render_tri_pts(x4,y4,x2,y2,x3,y3);
    }


void render_tri_pts(double x1,double y1,double x2,double y2,
                    double x3,double y3)

    {
    TRIANGLE2D tri;

    tri.p[0]=p2d_point(x1/width_pts,y1/height_pts);
    tri.p[1]=p2d_point(x2/width_pts,y2/height_pts);
    tri.p[2]=p2d_point(x3/width_pts,y3/height_pts);
    render_triangle(lbmp,&tri,&lfgc,&lbgc,lrtype);
    }


void render_partial_circle_pts(double xc,double yc,double radius,
                               double theta0,double theta1,int nsteps)

    {
    int i;

    if (theta0==0. && theta1==0.)
        theta1=2.*PI;
    if (nsteps<1)
        {
        int n1,n2;

        n1=lbmp->width*1.33*radius/width_pts;
        n2=lbmp->height*1.33*radius/height_pts;
        nsteps = n1<n2 ? n2 : n1;
        if (nsteps<8)
            nsteps=8;
        nsteps *= fabs(theta1-theta0)/(2.*PI);
        if (nsteps<1)
            nsteps=1;
        }
    for (i=0;i<nsteps;i++)
        {
        double th1,th2;

        th1=theta0+i*(theta1-theta0)/nsteps;
        th2=(i==nsteps-1)?theta1:theta0+(i+1)*(theta1-theta0)/nsteps;
        render_tri_pts(xc,yc,xc+radius*cos(th1),yc+radius*sin(th1),
                             xc+radius*cos(th2),yc+radius*sin(th2));
        }
    }
