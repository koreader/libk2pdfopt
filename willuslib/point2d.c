/*
** point2d.c    Functions to work on a 2D point/vector type.
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
#include "willus.h"
#include <math.h>


int p2d_is_zero(POINT2D *p)

    {
    return(p->x==0. && p->y==0.);
    }


double p2d_magnitude(VECTOR2D *v)

    {
    return(sqrt(v->x*v->x + v->y*v->y));
    }


double p2d_dot_product(VECTOR2D *v1,VECTOR2D *v2)

    {
    return(v1->x*v2->x+v1->y*v2->y);
    }


void p2d_swap(POINT2D *a,POINT2D *b)

    {
    POINT2D p;

    p=(*a);
    (*a) = (*b);
    (*b) = p;
    }


/*
** If triangle t has potential v[] at each of the vertices,
** this function returns the potential at p0 inside the triangle.
*/
double tri2d_point_interp(TRIANGLE2D *t,POINT2D p0,double *val)

    {
    TRIANGLE2D tri2d;
    VECTOR2D v00,v0,v,xhat,yhat,vv;
    double dvx,dvy;

    xhat=p2d_normalize(p2d_vector(&t->p[0],&t->p[1]));
    yhat.x=-xhat.y;
    yhat.y=xhat.x;
    tri2d.p[0].x = 0.;
    tri2d.p[0].y = 0.;
    vv=p2d_vector(&t->p[0],&t->p[1]);
    tri2d.p[1].x = p2d_magnitude(&vv);
    tri2d.p[1].y = 0.;
    v = p2d_vector(&t->p[0],&t->p[2]);
    tri2d.p[2].x = p2d_dot_product(&v,&xhat);
    tri2d.p[2].y = p2d_dot_product(&v,&yhat);
    v00 = p2d_vector(&t->p[0],&p0);
    v0.x=p2d_dot_product(&v00,&xhat);
    v0.y=p2d_dot_product(&v00,&yhat);
    dvx=(val[1]-val[0])/tri2d.p[1].x;
    dvy=(val[2]-(tri2d.p[2].x*dvx)-val[0])/tri2d.p[2].y;
    return(val[0]+dvx*v0.x+dvy*v0.y);
    }


/*
** Returns sine of angle from v1 to v2.
*/
double p2d_sine_angle_between(VECTOR2D *v1,VECTOR2D *v2)

    {
    double  m2,sinth,costh_x_m2;

    if (p2d_is_zero(v1) || p2d_is_zero(v2))
        return(0.);
    m2 = p2d_magnitude(v1)*p2d_magnitude(v2);
    sinth=(v1->x*v2->y - v1->y*v2->x)/m2;
    costh_x_m2 = (v1->x*v2->x + v1->y*v2->y);
    return(costh_x_m2 > 0 ? sinth : (sinth>0 ? 2.-sinth : -2.-sinth));
    }


void p2d_min_angles(POINT2D *p,int n,double *mostneg_deg,
                                     double *closest_to_zero_deg)

    {
    VECTOR2D v[2];
    double sum,angle,mostneg,czero;
    int i;

    czero=1e10;
    mostneg=1e10;
    sum=0.;
    for (i=0;i<n;i++)
        {
        v[0]=p2d_vector(&p[i],&p[(i+1)%n]);
        v[1]=p2d_vector(&p[(i+1)%n],&p[(i+2)%n]);
        angle=p2d_angle_between_deg(&v[0],&v[1]);
        sum+=angle;
        }
    for (i=0;i<n;i++)
        {
        v[0]=p2d_vector(&p[i],&p[(i+1)%n]);
        v[1]=p2d_vector(&p[(i+1)%n],&p[(i+2)%n]);
        angle=p2d_angle_between_deg(&v[0],&v[1]);
        if (sum<0)
            angle = -angle;
        if (fabs(angle)<fabs(czero))
            czero=angle;
        if (angle < mostneg)
            mostneg=angle;
        }
    if (mostneg_deg != NULL)
        (*mostneg_deg) = mostneg;
    if (closest_to_zero_deg !=NULL)
        (*closest_to_zero_deg) = czero;
    }


/*
** Returns angle from v1 to v2 in degrees
*/
double p2d_angle_between_deg(VECTOR2D *v1,VECTOR2D *v2)

    {
    double  m2,sinth,costh_x_m2;

    if (p2d_is_zero(v1) || p2d_is_zero(v2))
        return(0.);
    m2 = p2d_magnitude(v1)*p2d_magnitude(v2);
    sinth=(v1->x*v2->y - v1->y*v2->x)/m2;
    costh_x_m2 = (v1->x*v2->x + v1->y*v2->y);
    return(costh_x_m2 > 0 ? asin(sinth)*180/PI 
                          : (sinth>0 ? 180.-asin(sinth)*180./PI
                                     : -180.+asin(-sinth)*180./PI));
    }


POINT2D p2d_vector(POINT2D *p1,POINT2D *p2)

    {
    VECTOR2D    v;

    v.x = (p2->x-p1->x);
    v.y = (p2->y-p1->y);
    return(v);
    }


VECTOR2D p2d_normalize(VECTOR2D v)

    {
    VECTOR2D vn;
    double len;

    len=p2d_magnitude(&v);
    vn=v;
    if (len>0)
        {
        vn.x/=len;
        vn.y/=len;
        }
    return(vn);
    }


int tri2d_point_inside(TRIANGLE2D *tri,POINT2D p)

    {
    int     i;

    for (i=0;i<3;i++)
        {
        int     i1,i2,i3;
        VECTOR2D    v1,v2,v3;
        double  sin12,sin13;

        i1 = i;
        i2 = (i+1)%3;
        i3 = (i+2)%3;
        v1 = p2d_vector(&tri->p[i1],&tri->p[i2]);
        v2 = p2d_vector(&tri->p[i1],&tri->p[i3]);
        v3 = p2d_vector(&tri->p[i1],&p);
        sin12 = p2d_sine_angle_between(&v1,&v2);
        sin13 = p2d_sine_angle_between(&v1,&v3);
        if (sin12 < 0)
            {
            sin12 = -sin12;
            sin13 = -sin13;
            }
        /* Give benefit of doubt to points that are close (within 1e-10) */
        if (sin13>sin12+1e-10 || sin13<-1e-10)
/*
{
if (tidebug)
printf("    point is NOT in.\n");
*/
            return(0);
/*
}
*/
        }
/*
if (tidebug)
printf("    point is IN.\n");
*/
    return(1);
    }


double p2d_point_line_distance(LINE2D *line,POINT2D *point)

    {
    double z0,r0,z1,r1,z,r;
    double  x,y,x0,y0,x1,y1,xmin,ymin,m,b;

    z0=line->p[0].x;
    r0=line->p[0].y;
    z1=line->p[1].x;
    r1=line->p[1].y;
    z=point->x;
    r=point->y;
    if (z0==z1 && r0==r1)
        return(sqrt((z0-z)*(z0-z)+(r0-r)*(r0-r)));
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
        }
    m=(y1-y0)/(x1-x0);
    b=y0-m*x0;
    xmin=(x+m*y-m*b)/(m*m+1);
    ymin=m*xmin+b;
    return(sqrt((xmin-x)*(xmin-x)+(ymin-y)*(ymin-y)));
    }


/*
** See p3d_tri_line_intersect.
** Returns 0 for no intersection, 1 for intersection of a vertex,
**         and 2 for a two point intersection.
*/
int p2d_tri_line_intersect(TRIANGLE2D *tri,double *val,LINE2D *line,
                           POINT2D *pout,double *vout)

    {
    int side;
    int count,vc;

    count=vc=0;
    for (side=0;side<3;side++)
        {
        LINE2D tline;
        POINT2D p;
        double x0,dx,y0,dy,f;
        int i1,i2,n;

        i1=side;
        i2=(side+1)%3;
        tline.p[0]=tri->p[i1];
        tline.p[1]=tri->p[i2];
        n=p2d_line_line_intersection(&tline,line,&p);
        if (n==-1)
            {
            pout[0]=tri->p[i1];
            pout[1]=tri->p[i2];
            vout[0]=val[i1];
            vout[1]=val[i2];
            return(2);
            }
        if (n==0)
            continue;
        x0=tline.p[0].x;
        y0=tline.p[0].y;
        dx=tline.p[1].x-x0;
        dy=tline.p[1].y-y0;
        if (fabs(dx)>fabs(dy))
            f=(p.x-x0)/dx;
        else
            f=(p.y-y0)/dy;
        if (f<0. || f>1.)
            continue;
        if (f==0. || f==1.)
            {
            if (vc>0)
                continue;
            vc++;
            }
        pout[count].x=x0+f*dx;
        pout[count].y=y0+f*dy;
        vout[count]=val[i1]+f*(val[i2]-val[i1]);
        count++;
        }
    return(count);
    }


VECTOR2D p2d_line_to_vector(LINE2D *line)

    {
    VECTOR2D v;

    v.x=line->p[1].x-line->p[0].x;
    v.y=line->p[1].y-line->p[0].y;
    return(v);
    }


/*
** Returns 0 for no intersection, -1 if the two lines are coincident,
** and 1 if the intersection is a single point.
*/
int p2d_line_line_intersection(LINE2D *line1,LINE2D *line2,POINT2D *point)

    {
    VECTOR2D v1,v2;
    double dp;
    double x0,y0,dx0,dy0,x1,y1,dx1,dy1,t1;

    v1=p2d_line_to_vector(line1);   
    v2=p2d_line_to_vector(line2);
    dp=p2d_dot_product(&v1,&v2);
    if (dp==0)
        {
        dp=p2d_point_line_distance(line1,&line2->p[0]);
        if (dp==0)
            return(-1);
        return(0);
        }
    x0=line1->p[0].x;
    y0=line1->p[0].y;
    dx0=line1->p[1].x-line1->p[0].x;
    dy0=line1->p[1].y-line1->p[0].y;
    x1=line2->p[0].x;
    y1=line2->p[0].y;
    dx1=line2->p[1].x-line2->p[0].x;
    dy1=line2->p[1].y-line2->p[0].y;
    if (fabs(dx0)>fabs(dy0))
        t1=(y1-y0-dy0*x1/dx0+dy0*x0/dx0)/(dy0*dx1/dx0-dy1);
    else
        t1=(x1-x0-dx0*y1/dy0+dx0*y0/dy0)/(dx0*dy1/dy0-dx1);
    point->x=x1+dx1*t1;
    point->y=y1+dy1*t1;
    return(1);
    }


/*
** Sort triangle points by y-value.
*/
void tri2d_sort_ypoints(TRIANGLE2D *tri)

    {
    if (tri->p[2].y < tri->p[1].y)
        p2d_swap(&tri->p[1],&tri->p[2]);
    if (tri->p[1].y < tri->p[0].y)
        p2d_swap(&tri->p[0],&tri->p[1]);
    if (tri->p[2].y < tri->p[1].y)
        p2d_swap(&tri->p[1],&tri->p[2]);
    }


/*
** Report whether triangle has zero area.
*/
int tri2d_zero_area(TRIANGLE2D *tri)

    {
    /* Are any two points coincident?  If so, area = 0.  */
    if (p2d_same(tri->p[0],tri->p[1])
            || p2d_same(tri->p[0],tri->p[2])
            || p2d_same(tri->p[1],tri->p[2]))
        return(1);
    if (tri->p[0].x == tri->p[1].x)
        return(tri->p[0].x == tri->p[2].x);
    if (tri->p[0].x==tri->p[2].x || tri->p[1].x==tri->p[2].x)
        return(0);
    /* Now we are guaranteed that all x-values are distinct. */
    return(p2d_slope(tri->p[0],tri->p[1])==p2d_slope(tri->p[0],tri->p[2]));
    }


double tri2d_area(TRIANGLE2D *tri)

    {
    double  a,b,c,s;

    a = p2d_dist(tri->p[0],tri->p[1]);
    b = p2d_dist(tri->p[1],tri->p[2]);
    c = p2d_dist(tri->p[2],tri->p[0]);
    s = (a+b+c)/2.;
    return(sqrt(fabs(s*(s-a)*(s-b)*(s-c))));
    }


/*
** MUST BE SORTED!
*/
void p2d_remove_duplicate_xcoords(POINT2D *p,int *n)

    {
    int i;

    for (i=0;i<(*n)-1;i++)
        if (p[i].x==p[i+1].x)
            {
            if ((*n)-(i+2) > 0)
                memmove(&p[i+1],&p[i+2],sizeof(POINT2D)*((*n)-(i+2)));
            (*n)=(*n)-1;
            i--;
            }
    }


void p2d_sort_by_xcoord(POINT2D *x,int n)

    {
    int     top,n1;
    POINT2D x0;

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
            if (child<n1 && x[child].x<x[child+1].x)
                child++;
            if (x0.x<x[child].x)
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


void p2d_sort_by_theta(POINT2D *x,int n)

    {
    int     top,n1;
    POINT2D x0;

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
            if (child<n1 && atan2(x[child].y,x[child].x) < atan2(x[child+1].y,x[child+1].x))
                child++;
            if (atan2(x0.y,x0.x) < atan2(x[child].y,x[child].x))
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                child=n1+1;
            }
        x[parent]=x0;
        }
        }
    }
