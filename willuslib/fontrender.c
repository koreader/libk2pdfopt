/*
** fontrender.c      Render fonts onto bitmaps from pre-made templates.
**                   Note that without libpng, fonts won't render--the
**                   font data is stored in PNG format.
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
#include <ctype.h>
#include <math.h>
#include "willus.h"
#include <time.h>

extern char *willus_bmp_fontdata[];

typedef struct
    {
    double x,y;   // rel position @ lower left corner
    double size;  // font size
    int c;        // character
    int *w;       // font pointer
    } RCHAR;

#define MAXLETTERS 1024

typedef struct
    {
    WILLUSBITMAP bmp;
    double size;
    int c;
    int font_index;
    int rot;
    int count;
    time_t last_used;
    } FONTLETTER;

typedef struct
    {
    FONTLETTER fontletter[MAXLETTERS];
    int n;
    int sorted;
    } FONTCACHE;

static FONTCACHE *fontcache=NULL;
static FONTCACHE _fontcache;

static int fontrender_font_index=0;
static double fontrender_size=20.;
static int fontrender_bgcolor[3]={0,0,0};
static int fontrender_fgcolor[3]={255,255,255};
static int fontrender_justification;  /* 1-9 */
static double fontrender_pixels_per_point=1.0;
static int fontrender_or=0;

static void break_string(char *s,int *nlines);
static int fontrender_get_font_index(char *name);
static void fontrender_clearing_box(WILLUSBITMAP *bmp,double x1,double y1,
                                    double x2,double y2,
                                    double x3,double y3,
                                    double x4,double y4);
static void fontrender_renderchar(WILLUSBITMAP *bmp,int *w,
                                  WILLUSBITMAP *fontbmp,
                                  double x,double y_from_bottom,
                                  int c,int rot);
/*
static int dark(void);
*/
static void colorit(WILLUSBITMAP *bmp);
static void rchar_get_one_line(char *s,RCHAR *rchar,int *nc,double *y1,
                               double *y2,double *width,int translate);
static int style_index(int bold,int italic,int underline);
static int symbol_index(int bold,int italic,int underline);

/*
** Font letter caching to speed up rendering
*/
static void fontcache_init(void);
static void fontcache_free(void);
static WILLUSBITMAP *fontcache_cached_fontletter(FONTLETTER *fontletter);
static void fontcache_add_fontletter(FONTLETTER *fontletter);
static void fontcache_sort(void);
static int fontletter_compare(FONTLETTER *x1,FONTLETTER *x2);


void fontrender_set_typeface(char *name)


    {
    fontrender_font_index = fontrender_get_font_index(name);
    }


static int fontrender_get_font_index(char *name)

    {
    /* Default font_index (0,1) = Times Roman */
    return(0);
    }


void fontrender_set_fgcolor(int r,int g,int b)

    {
    fontrender_fgcolor[0]=r;
    fontrender_fgcolor[1]=g;
    fontrender_fgcolor[2]=b;
    }

/*
** Pass 1 to OR the characters onto the dest. bitmap
*/
void fontrender_set_or(int status)

    {
    fontrender_or=status;
    }


void fontrender_set_bgcolor(int r,int g,int b)

    {
    fontrender_bgcolor[0]=r;
    fontrender_bgcolor[1]=g;
    fontrender_bgcolor[2]=b;
    }


void fontrender_set_pixel_size_ex(double size,double points)

    {
    fontrender_set_pixel_size(size);
    fontrender_pixels_per_point = size/points;
    }


void fontrender_set_pixel_size(double size)

    {
    fontrender_size=size;
    }

/*
** 1 - 9
*/
void fontrender_set_justification(int just)

    {
    fontrender_justification=just;
    }


void fontrender_caption(WILLUSBITMAP *bmp,char *s,int justify,FILE *out)

    {
    int i,j,nl,jx,jy;
    double y0,x0,dp;

    fontrender_tweak_string(s,(int)(bmp->width*.85),&nl);
    fontrender_set_fgcolor(0,0,0);
    fontrender_set_bgcolor(255,255,255);
    fontrender_set_justification(justify);
    jx=(justify-1)%3;
    jy=(justify-1)/3;
    dp=fontrender_size*.4;
    x0=dp+(bmp->width-dp*2.)*jx/2.;
    y0=dp+(bmp->height-dp*2.)*(2-jy)/2.;
    for (i=-1;i<=1;i++)
        for (j=-1;j<=1;j++)
            {
            if (i==0 && j==0)
                continue;
            fontrender_render(bmp,x0+i,y0+j,s,0,out);
            }
    fontrender_set_bgcolor(0,0,0);
    fontrender_set_fgcolor(255,255,255);
    fontrender_render(bmp,x0,y0,s,0,out);
    }


void fontrender_render(WILLUSBITMAP *bmp,double x,double y_from_bottom,
                       char *string,int rot,FILE *out)

    {
    fontrender_render_ex(bmp,x,y_from_bottom,string,rot,0,0,0,out);
    }


void fontrender_render_ex(WILLUSBITMAP *bmp,double x,double y_from_bottom,
                          char *string,int rot,
                          int ep_translate,int justify_per_line,
                          int clear_behind_chars,FILE *out)

    {
    WILLUSBITMAP _fontbmp,*fontbmp;
    int *w,i,j,yj,xj,nl,nct;
    double dw,dh,th,sth,cth,frsize,sz;
    double hbox,wbox,lheight,cwidth;
    RCHAR *rchar;
    unsigned char *s;
    static char *funcname="fontrender_render_ex";
// printf("fontrender_render_ex:  x=%g, y=%g, string='%s'\n",x,y_from_bottom,string);

/*
if (fabs(rot-90.)<1.)
dprintf(NULL,"@fontrender_render_ex(x=%g,y=%g,'%s')\n",x,y_from_bottom,string);
*/
    willus_mem_alloc_warn((void **)&rchar,sizeof(RCHAR)*strlen(string),
                         funcname,10);
    s=(unsigned char *)string;
    // w=(int *)fontrender_font;
    frsize=fontrender_size;
    fontbmp=&_fontbmp;
    bmp_init(fontbmp);
    // bmp_read_png_stream(fontbmp,&fontrender_font[1036],1,NULL);
/*
bmp_promote_to_24(fontbmp);
bmp_write(fontbmp,"font.jpg",stdout,90);
wfile_written_info("font.jpg",stdout);
*/
    yj=(fontrender_justification-1)/3;
    xj=(fontrender_justification-1)%3;
    /* Get height and width of text box */
    fontrender_size=frsize;
    cwidth=0.;
    nct=0;
    nl=0;
    for (hbox=0.,wbox=0.,i=0;s[i]!='\0';i++)
        {
        int nc;
        double y1,y2,width;

        rchar_get_one_line((char *)&s[i],rchar,&nc,&y1,&y2,&width,ep_translate);
        hbox += (y2-y1);
        nl++;
        cwidth += width;
        nct+=nc;
        if (width > wbox)
            wbox=width;
        for (;s[i]!='\n' && s[i]!='\0';i++);
        if (s[i]=='\0')
            break;
        }
    if (nl>0)
        lheight=hbox/nl;
    else
        lheight=0.;
    if (nct>0)
        cwidth/=nct;
    else
        cwidth=0.;
 
    /* 
    ** Apply offsets due to justification so that x,y_from_bottom is
    ** positioned at the "top left" corner of the first char on
    ** the top line.
    */
    dh = (yj/2.)*hbox;
    dw = (-xj/2.)*wbox;
    th = rot*PI/180.;
    sth = sin(th);
    cth = cos(th);
/*
render_partial_circle_pts(x/fontrender_pixels_per_point,y_from_bottom/fontrender_pixels_per_point,5.,0.,2.*PI,-1);
*/
    y_from_bottom += dh*cth + dw*sth;
    x += dw*cth - (dh+lheight)*sth;
/*
if (fabs(rot-90.)<1.)
dprintf(NULL,"dh=%g, hbox=%g, (x',y')=(%g,%g)\n",dh,hbox,x,y_from_bottom);
*/
    if (clear_behind_chars==2)
        {
        double wpad,hpad;
        wpad = wbox*0.05;
        if (wpad > cwidth/2.)
            wpad = cwidth/2.;
        hpad = lheight*0.2;
        fontrender_clearing_box(bmp,
                                x-wpad*cth-hpad*sth,
                                y_from_bottom+hpad*cth-wpad*sth,
                                x-wpad*cth+(hbox+hpad)*sth,
                                y_from_bottom-(hbox+hpad)*cth-wpad*sth,
                                x+(wbox+wpad)*cth+(hbox+hpad)*sth,
                                y_from_bottom-(hbox+hpad)*cth+(wbox+wpad)*sth,
                                x+(wbox+wpad)*cth-hpad*sth,
                                y_from_bottom+hpad*cth+(wbox+wpad)*sth);
        }
    else if (clear_behind_chars==1)
        {
        /* Clear behind all chars */
        sz=fontrender_size=frsize;
        for (j=0,dh=0.;1;j++)
            {
            double y1,y2,width;
            int nc,k;

            fontrender_size=sz;
            rchar_get_one_line((char *)&s[j],rchar,&nc,&y1,&y2,&width,ep_translate);
            sz=fontrender_size;
            dh -= y2;
            if (justify_per_line)
                dw=(wbox-width)/2.;
            else
                dw=0.;
            for (k=0;k<nc;k++)
                {
                double dx,dy,cw,ch,wp,hp;
                dx=dw+rchar[k].x;
                dy=dh+rchar[k].y;
                ch=rchar[k].size;
                cw=rchar[k].w[rchar[k].c+3]*ch/rchar[k].w[0];
                wp=cw*.3;
                cw+=wp;
                hp=ch*.12;
                ch*=1.2;
                fontrender_clearing_box(bmp,x+(dx-wp)*cth-(dy-hp)*sth,
                                        y_from_bottom+(dy-hp)*cth+(dx-wp)*sth,
                                        x+(dx-wp)*cth-(dy+ch)*sth,
                                        y_from_bottom+(dy+ch)*cth+(dx-wp)*sth,
                                        x+(dx+cw)*cth-(dy+ch)*sth,
                                        y_from_bottom+(dy+ch)*cth+(dx+cw)*sth,
                                        x+(dx+cw)*cth-(dy-hp)*sth,
                                        y_from_bottom+(dy-hp)*cth+(dx+cw)*sth);
                }
            dh += y1;
            for (;s[j]!='\n' && s[j]!='\0';j++);
            if (s[j]=='\0')
                break;
            }
        }
    sz=fontrender_size=frsize;
    for (w=NULL,j=0,dh=0.;1;j++)
        {
        double y1,y2,width;
        int nc,k;

        fontrender_size=sz;
        rchar_get_one_line((char *)&s[j],rchar,&nc,&y1,&y2,&width,ep_translate);
        sz=fontrender_size;
        dh -= y2;
        if (justify_per_line)
            dw=(wbox-width)/2.;
        else
            dw=0.;
/*
render_partial_circle_pts((x+dw*cth-dh*sth)/fontrender_pixels_per_point,(y_from_bottom+dh*cth+dw*sth)/fontrender_pixels_per_point,3.,0.,2.*PI,-1);
*/
        for (k=0;k<nc;k++)
            {
            double dx,dy;
            nprintf(out,"%c",rchar[k].c);
            if (out!=NULL)
                fflush(out);
            if (w!=rchar[k].w)
                {
/*
** Nothing will be rendered without libpng--fontrender is essentially useless.
*/
#ifdef HAVE_PNG_LIB
                char *ptr;
                w=rchar[k].w;
                ptr=(char *)w;
                bmp_read_png_stream(fontbmp,&ptr[1036],1,NULL);
#else
                /* create blank bitmap */
                fontbmp->width=24;
                fontbmp->height=32;
                fontbmp->bpp=8;
                {
                int i;
                for (i=0;i<256;i++)
                    fontbmp->red[i]=fontbmp->green[i]=fontbmp->blue[i]=i;
                }
                bmp_alloc(fontbmp);
                bmp_fill(fontbmp,255,255,255);
#endif
                }
            dx=rchar[k].x;
            dy=rchar[k].y;
            fontrender_size = rchar[k].size;
            fontrender_renderchar(bmp,w,fontbmp,x+(dw+dx)*cth-(dh+dy)*sth,
                                       y_from_bottom+(dh+dy)*cth+(dw+dx)*sth,
                                       rchar[k].c,rot);
            }
        dh += y1;
        for (;s[j]!='\n' && s[j]!='\0';j++);
        if (s[j]=='\0')
            break;
        }
    nprintf(out,"\n");
    fontrender_size=frsize;
    bmp_free(fontbmp);
    willus_mem_free((double **)&rchar,funcname);
    }


static void fontrender_clearing_box(WILLUSBITMAP *bmp,double x1,double y1,
                                    double x2,double y2,
                                    double x3,double y3,
                                    double x4,double y4)

    {
    RENDER_COLOR bg;
    TRIANGLE2D tri;

    bg=render_color(fontrender_bgcolor[0]/255.,
                    fontrender_bgcolor[1]/255.,
                    fontrender_bgcolor[2]/255.);
    tri.p[0].x=x1/bmp->width;
    tri.p[0].y=y1/bmp->height;
    tri.p[1].x=x2/bmp->width;
    tri.p[1].y=y2/bmp->height;
    tri.p[2].x=x3/bmp->width;
    tri.p[2].y=y3/bmp->height;
    render_triangle(bmp,&tri,&bg,&bg,RENDER_TYPE_SET);
    tri.p[1].x=x3/bmp->width;
    tri.p[1].y=y3/bmp->height;
    tri.p[2].x=x4/bmp->width;
    tri.p[2].y=y4/bmp->height;
    render_triangle(bmp,&tri,&bg,&bg,RENDER_TYPE_SET);
    }


void fontrender_tweak_string(char *s,int maxwidth,int *nlines)

    {
    int i,w,nl;

    for (i=0,nl=1;i<10;i++,nl++)
        {
        break_string(s,&nl);
        w=fontrender_pixwidth(s);
        if (w<maxwidth)
            break;
        }
    (*nlines)=nl;
    }


static void break_string(char *s,int *nlines)

    {
    int i,j,j0,c,nl,len;

    nl=(*nlines);
    for (i=c=0;s[i]!='\0';i++)
        {
        if (s[i]=='\n' || s[i]=='\t')
            s[i]=' ';
        if (s[i]==' ')
            c++;
        }
    len=strlen(s);
    if (nl>c)
        {
        for (i=0;s[i]!='\0';i++)
            if (s[i]==' ')
                s[i]='\n';
        (*nlines)=c+1;
        return;
        }
    for (i=c=1;i<nl;i++)
        {
        j0=len*i/nl;
        for (j=0;j0-j>=0 || j0+j<len;j++)
            {
            if (j0-j>=0 && s[j0-j]==' ')
                {
                s[j0-j]='\n';
                c++;
                break;
                }
            if (j0+j<len && s[j0+j]==' ')
                {
                s[j0+j]='\n';
                c++;
                break;
                }
            }
        if (c>=nl)
            break;
        }
    }


int fontrender_pixwidth(char *string)

    {
    int *w,i;
    unsigned char *s;
    double x,xmax;

    s=(unsigned char *)string;
    w=(int *)willus_bmp_fontdata[fontrender_font_index];
    for (x=xmax=0.,i=0;string[i]!='\0';i++)
        {
        if (string[i]=='\n')
            {
            x = 0.;
            continue;
            }
        x += w[s[i]+3]*fontrender_size/w[0];
        if (x>xmax)
            xmax=x;
        }
    return((int)(xmax+.5));
    }


void fontrender_close(void)

    {
    fontcache_free();
    }


/*
** Put character so that lower left corner of character is
** at (x,y_from_bottom) pixels.
**
** Needs work on rotated chars--bitmap needs more space.
*/
static void fontrender_renderchar(WILLUSBITMAP *bmp,int *w,
                                  WILLUSBITMAP *fontbmp,
                                  double x,double y_from_bottom,
                                  int c,int rot)

    {
    int i;
    int sx,sdx,nw,nh,bx0,by0;
    double x1,x2,y1,y2,dy0;
    /* double th,cth,sth,dw,dh; */
    /* double dx,dy; */
    FONTLETTER *fontletter,_fontletter;
    WILLUSBITMAP _charbmp,*charbmp;
    static int black[3]={0,0,0};
    static int white[3]={255,255,255};
//static int count=0;

    fontletter=&_fontletter;
    fontletter->c=c;
    fontletter->rot=rot;
    fontletter->size=fontrender_size;
    fontletter->font_index=fontrender_font_index;
    bmp_init(&fontletter->bmp);
    charbmp=&_charbmp;
    bmp_init(charbmp);
    if (c<=32)
        return;
    for (sx=w[2],sdx=w[36],i=33;i<c;sx+=w[i+3]+2*w[2],i++,sdx=w[i+3]);
    x1=sx - (x-floor(x))*w[0]/fontrender_size;
    bx0=floor(x);
    x2=sx+sdx+w[2];
    nw=(x2-x1)*fontrender_size/w[0]+.5;
    x2=x1+nw*w[0]/fontrender_size;
    // dy0 = y_from_bottom - w[1]*fontrender_size/w[0];
    dy0 = y_from_bottom;
    y1=0 - (dy0-floor(dy0))*w[0]/fontrender_size;
    by0=floor(dy0);
    while (y1<0)
        {
        y1 += w[0]/fontrender_size;
        by0++;
        }
    y2=fontbmp->height;
    nh=(y2-y1)*fontrender_size/w[0]+.5;
    y2=y1+nh*w[0]/fontrender_size;
    while (y2>fontbmp->height)
        {
        nh--;
        y2-=w[0]/fontrender_size;
        }
    /*
    dw=nw;
    dh=nh;
    */
    if (fontcache_cached_fontletter(fontletter)==NULL)
        {
// printf("Creating bitmap for letter '%c'...\n",fontletter->c);
        bmp_resample(charbmp,fontbmp,x1,fontbmp->height-y1,x2,fontbmp->height-y2,
                     nw,nh);
        if (rot)
            {
            while (rot<0)
                rot += 360;
            rot=rot%360;
            if (rot==90 || rot==180 || rot==270)
                bmp_rotate_right_angle(charbmp,rot);
            else
                bmp_rotate_fast(charbmp,(double)rot,1);
            }
        bmp_convert_to_greyscale(charbmp);
        bmp_copy(&fontletter->bmp,charbmp);
        fontcache_add_fontletter(fontletter);
        }
    else
        bmp_copy(charbmp,&fontletter->bmp);
    bmp_promote_to_24(charbmp);
    if (fontrender_or)
        {
        colorit(charbmp);
        bmp_overlay(bmp,charbmp,bx0,bmp->height-1-by0-charbmp->height,
                    NULL,NULL,fontrender_bgcolor,NULL);
        }
    else
        bmp_overlay(bmp,charbmp,bx0,bmp->height-1-by0-charbmp->height,
                    fontrender_bgcolor,fontrender_fgcolor,white,black);
    bmp_free(charbmp);
    }



/*
** Must be 24 bit
*/
static void colorit(WILLUSBITMAP *bmp)

    {
    unsigned char *sp;
    int i,j,k;

    for (i=0;i<bmp->height;i++)
        {
        sp=bmp_rowptr_from_top(bmp,i);
        for (j=0;j<bmp->width;j++,sp+=3)
            for (k=0;k<3;k++)
                {
                double f;
                f=sp[k];
                f=(255.-f)/255.;
                f=fontrender_bgcolor[k]+f*(fontrender_fgcolor[k]-fontrender_bgcolor[k]);
                sp[k]=f;
                }
        }
    }


static void rchar_get_one_line(char *s,RCHAR *rchar,int *nc,double *y1,
                               double *y2,double *width,int translate)

    {
    double oldsize[16];
    double oldy[16];
    int nl,symbolexit;
    int i,bold,italic,underline,fc,autoleave;
    double x,y,sf;
    int *w;

    w=(int *)willus_bmp_fontdata[fontrender_font_index];
    (*nc)=0;
    x=y=0.;
    (*width)=0.;
    (*y1)=0.;
    (*y2)=0.;
    fc=0;
    nl=0;
    autoleave=0;
    symbolexit=0;
    bold=italic=underline=0;
    sf=1.00;
    for (i=0;s[i]!='\0' && s[i]!='\n';i++)
        {
        if (translate && s[i]=='}' && nl>0 && !autoleave)
            {
            nl--;
            y=oldy[nl];
            fontrender_size=oldsize[nl];
            continue;
            }
        if (!translate || fc || (s[i]!='\\' && s[i]!='_' && s[i]!='^'))
            {
            double dx;
            fc=0;
            rchar[(*nc)].x = x;
            rchar[(*nc)].y = y;
            rchar[(*nc)].size = fontrender_size;
            rchar[(*nc)].w = w;
            rchar[(*nc)].c = s[i];
            (*nc)=(*nc)+1;
            dx = w[s[i]+3]*fontrender_size/w[0];
// printf("    %c (%5.1f)\n",s[i],dx);
            x += dx;
            (*width) += dx;
            if ((*y1) > y)
                (*y1) = y;
            if ((*y2) < (y+fontrender_size*sf))
                (*y2) = (y+fontrender_size*sf);
            if (autoleave && nl>0)
                {
                autoleave=0;
                nl--;
                y=oldy[nl];
                fontrender_size=oldsize[nl];
                }
            if (symbolexit)
                {
                w=(int *)willus_bmp_fontdata[style_index(bold,italic,underline)];
                symbolexit=0;
                }
            continue;
            }
        if (s[i+1]=='\0' || s[i+1]=='\n')
            break;
        if (s[i]=='_' || s[i]=='^')
            {
            int cc;
            cc=s[i];
            if (s[i]==s[i+1])
                {
                fc=1;
                continue;
                }
            if (s[i+1]=='{')
                {
                i++;
                autoleave=0;
                }
            else
                autoleave=1;
            if (nl<16)
                {
                oldy[nl]=y;
                oldsize[nl]=fontrender_size;
                nl++;
                y = y + ((cc=='_') ? (-fontrender_size*.25) : (fontrender_size*.5));
                fontrender_size *= .65;
                }
            continue;
            }
        if (s[i]!='\\' || s[i+1]=='\\' || s[i+1]=='_' || s[i+1]=='^')
            {
            fc=1;
            continue;
            }
        if (tolower(s[i+1])=='p' && tolower(s[i+2])=='t')
            {
            int j;
            for (j=i+3;s[j]>='0' && s[j]<='9';j++);
            if (s[j]=='/' && j>i+3)
                {
                fontrender_size=atoi(&s[i+3])*fontrender_pixels_per_point;
                i=j;
                continue;
                }
            }
        if (s[i+3]=='/' && ((tolower(s[i+1])=='b' && s[i+2]=='o')
                             || (tolower(s[i+1])=='i' && s[i+2]=='t')
                             || (tolower(s[i+1])=='u' && s[i+2]=='l')))
            {
            if (s[i+1]=='b')
                bold=0;
            else if (s[i+1]=='B')
                bold=1;
            else if (s[i+1]=='i')
                italic=0;
            else if (s[i+1]=='I')
                italic=1;
            else if (s[i+1]=='u')
                underline=0;
            else
                underline=1;
            w=(int *)willus_bmp_fontdata[style_index(bold,italic,underline)];
            i+=3;
            continue;
            }
        w=(int *)willus_bmp_fontdata[symbol_index(bold,italic,underline)];
        symbolexit=1;
        }
    }


static int style_index(int bold,int italic,int underline)

    {
    /* Italic and underline ignored for now */
    if (bold)
        return(fontrender_font_index|1);
    return(fontrender_font_index&(~1));
    }


static int symbol_index(int bold,int italic,int underline)

    {
    char buf[32];

    sprintf(buf,"symbol%s",bold?"-bold":"");
    return(fontrender_get_font_index(buf));
    }


static void fontcache_init(void)

    {
    if (fontcache==NULL)
        {
        fontcache=&_fontcache;
        fontcache->n=0;
        }
    }


static void fontcache_free(void)

    {
    int i;

    fontcache_init();
    for (i=0;i<fontcache->n;i++)
        {
        bmp_free(&fontcache->fontletter[i].bmp);
        }
    fontcache->n=0;
    }


static WILLUSBITMAP *fontcache_cached_fontletter(FONTLETTER *fontletter)

    {
    int i;

    fontcache_init();
    for (i=0;i<fontcache->n;i++)
        {
        if (fontletter->c == fontcache->fontletter[i].c
             && fontletter->font_index == fontcache->fontletter[i].font_index
             && fontletter->rot == fontcache->fontletter[i].rot
             && fontletter->size == fontcache->fontletter[i].size)
            {
            fontcache->fontletter[i].count++;
            time(&fontcache->fontletter[i].last_used);
            fontletter->bmp = fontcache->fontletter[i].bmp;
            fontcache->sorted=0;
            return(&fontletter->bmp);
            }
        }
    return(NULL);
    }



static void fontcache_add_fontletter(FONTLETTER *fontletter)

    {
    int i;

    fontcache_init();
    fontcache_sort();
    if (fontcache->n < MAXLETTERS)
        {
        i=fontcache->n;
        fontcache->n++;
        }
    else
        {
        i=fontcache->n-1;
        bmp_free(&fontcache->fontletter[i].bmp);
        }
    fontcache->fontletter[i]=(*fontletter);
    fontcache->fontletter[i].count=1;
    time(&fontcache->fontletter[i].last_used);
    fontcache->sorted=0;
    }


/*
** Put most used (highest count) and most recently used first.
*/
static void fontcache_sort(void)

    {
    int     n,top,n1;
    FONTLETTER *x;
    FONTLETTER x0;

    if (fontcache->sorted)
        return;
    x=fontcache->fontletter;
    n=fontcache->n;
    if (n<2)
        {
        fontcache->sorted=1;
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
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                fontcache->sorted=1;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && fontletter_compare(&x[child],&x[child+1])>0)
                child++;
            if (fontletter_compare(&x0,&x[child])>0)
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
    fontcache->sorted=1;
    }


static int fontletter_compare(FONTLETTER *x1,FONTLETTER *x2)

    {
    if (x1->count > x2->count)
        return(1);
    if (x1->count < x2->count)
        return(-1);
    if (x1->last_used > x2->last_used)
        return(1);
    if (x1->last_used < x2->last_used)
        return(-1);
    return(0);
    }

