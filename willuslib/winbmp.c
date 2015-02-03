/*
** winbmp.c  Windows specific calls related to bitmaps.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2014  http://willus.com
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

#include "willus.h"

#ifdef HAVE_WIN32_API

#include <windows.h>
/* #include <process.h> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>

static int bmp_from_icon(WILLUSBITMAP *bmp,HICON hIcon);
static HICON bmp_to_icon(WILLUSBITMAP *bmp,HICON template);
static HBITMAP bmp_to_from_winbmp(WILLUSBITMAP *bmp,HBITMAP hBitmap_src);


int win_clipboard_to_bmp(WILLUSBITMAP *bmp,FILE *out)

    {
    static char *funcname="win_clipboard_to_bmp";
    HBITMAP hbitmap;
    HWND    dtwin;
    HDC     hDC,hMemDC;

    dtwin=GetDesktopWindow();
    if (!OpenClipboard(dtwin))
        {
        nprintf(out,"win_clipboard_to_bmp:  Error opening clipboard.\n");
        return(-1);
        }
    hbitmap=(HBITMAP)GetClipboardData(CF_BITMAP);
    if (hbitmap==NULL)
        {
        nprintf(out,"win_clipboard_to_bmp:  Clipboard does not have a bitmap.\n");
        CloseClipboard();
        return(-2);
        }
    // This is kludgy, but works for 24-bit desktops
    hDC=GetDC(dtwin);
    if (hDC==NULL)
        {
        nprintf(out,"win_clipboard_to_bmp:  Could not get DC.\n");
        CloseClipboard();
        return(-3);
        }
    // Create a device context for the windows bitmap
    hMemDC=CreateCompatibleDC(hDC);
    if (hMemDC==NULL)
        {
        nprintf(out,"win_clipboard_to_bmp:  Could not create hMemDC.\n");
        // DeleteObject(hbitmap);
        ReleaseDC(dtwin,hDC);
        return(-4);
        }
    // Select the windows bitmap into the new device context
    SelectObject(hMemDC,hbitmap);
    /* Kludgey way to get BMP dims, but GetBitmapDimensionsEx() doesn't work. */
    {
    int step;
    for (step=2048,bmp->width=0;1;bmp->width+=step)
        {
        if (GetPixel(hMemDC,bmp->width,0)==CLR_INVALID)
            {
            if (step==1 || bmp->width==0)
                break;
            bmp->width -= step;
            step >>= 1;
            }
        }
    for (step=1024,bmp->height=0;1;bmp->height+=step)
        {
        if (GetPixel(hMemDC,0,bmp->height)==CLR_INVALID)
            {
            if (step==1 || bmp->height==0)
                break;
            bmp->height -= step;
            step >>= 1;
            }
        }
    }
    if (bmp->height==0 || bmp->width==0)
        {
        nprintf(out,"win_clipboard_to_bmp:  Zero sized bitmap.\n");
        DeleteDC(hMemDC);
        ReleaseDC(dtwin,hDC);
        return(-5);
        }
    bmp->bpp=24;
    bmp->type=WILLUSBITMAP_TYPE_WIN32;
    bmp_alloc(bmp);

    // Set up the device independent bitmap
    {
    BITMAPINFO         *bmi;
    willus_mem_alloc_warn((void **)&bmi,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD),funcname,10);
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = bmp->width;
    bmi->bmiHeader.biHeight = bmp->height;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = 24;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = 0;
    bmi->bmiHeader.biXPelsPerMeter = (int)(72.0*100./2.54);
    bmi->bmiHeader.biYPelsPerMeter = (int)(72.0*100./2.54);
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;
    GetDIBits(hMemDC,hbitmap,0,bmp->height,bmp->data,bmi,DIB_RGB_COLORS);
/* Old way--pretty slow */
/*
    for (row=0;row<bmp->height;row++)
        {
        unsigned char *p;
        int col;

        p=bmp_rowptr_from_top(bmp,row);
        for (col=0;col<bmp->width;col++,p+=3)
            {
            int x;

            x=(int)GetPixel(hMemDC,col,row);
            p[0]=x&0xff;
            p[1]=(x>>8)&0xff;
            p[2]=(x>>16)&0xff;
            }
        }
*/
    willus_mem_free((double **)&bmi,funcname);
    }
    DeleteDC(hMemDC);
    ReleaseDC(dtwin,hDC);
    return(0);
    }


int win_emf_dims(char *filename,double *width_in,double *height_in)

    {
    wmetafile *wmf;
    HENHMETAFILE hemf;
    ENHMETAHEADER   header;

    (*width_in) = -1.0;
    (*height_in) = -1.0;
    wmf=win_emf_from_file(filename);
    if (wmf==NULL)
        return(-1);
    hemf=(HENHMETAFILE)wmf;
    GetEnhMetaFileHeader(hemf,sizeof(ENHMETAHEADER),&header);
    /* Set up bitmap dimensions */
    (*width_in) = (header.rclFrame.right-header.rclFrame.left)/2540.;
    (*height_in) = (header.rclFrame.bottom-header.rclFrame.top)/2540.;
    win_emf_close(wmf,0);
    return(0);
    }


/*
** 24-bit only
*/
int win_emf_to_bmp(wmetafile *wmf,int dpi,WILLUSBITMAP *bmp,FILE *out)

    {
    HENHMETAFILE hemf;
    HWND    h;
    ENHMETAHEADER   header;
    double  width_in,height_in,dpi_adjust;
    int     width_pix,height_pix;
    int     width_bytes;
    HDC     hDC,hMemDC;
    RECT    lprect;
    BITMAPINFO         *bmi;
    static  HBITMAP hBitmap=NULL;
    int planes,bpp;
    static char *funcname="win_emf_to_bmp";

    hemf=(HENHMETAFILE)wmf;
    h=GetDesktopWindow();
    GetEnhMetaFileHeader(hemf,sizeof(ENHMETAHEADER),&header);

    /* Set up bitmap dimensions */
    width_in = (header.rclFrame.right-header.rclFrame.left)/2540.;
    height_in = (header.rclFrame.bottom-header.rclFrame.top)/2540.;
    dpi_adjust = header.rclFrame.left==0 ? 1.0 : (1.0/5.8);
/*
printf("h.rclframe.left=%g\n",header.rclFrame.left/2540.);
printf("width_in = %g\n",width_in);
printf("szlmm = %d\n",header.szlMillimeters);
printf("szlpix = %d\n",header.szlDevice);
printf("iType = %d\n",header.iType);
printf("dSig = %d\n",header.dSignature);
printf("nDesc = %d\n",header.nDescription);
printf("nVer = %d\n",header.nVersion);
printf("h.rclbounds.left=%d\n",header.rclBounds.left);
printf("h.rclbounds.width=%d\n",header.rclBounds.right-header.rclBounds.left);
*/
    width_pix = (int)(width_in*dpi*dpi_adjust);
    height_pix = (int)(height_in*dpi*dpi_adjust);
    width_bytes=width_pix;
    while (width_bytes&3)
        width_bytes++;
    lprect.left=0;
    lprect.right=width_pix-1;
    lprect.top=0;
    lprect.bottom=height_pix-1;

    /*
    ** Allocate bit map info structure (has to be allocated due to
    ** arbitrarily sized color palette ==> cannot be statically declared)
    */
    if (!willus_mem_alloc((double **)&bmi,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD),funcname))
        return(2);
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    // Get device context
    hDC=GetDC(h);
    if (hDC==NULL)
        {
        willus_mem_free((double **)&bmi,funcname);
        nprintf(out,"win_emf_to_bmp:  Could not get DC.\n");
        return(4);
        }

    planes=GetDeviceCaps(hDC,PLANES);
    bpp=GetDeviceCaps(hDC,BITSPIXEL);
    // Create a windows bitmap to play the metafile into
    hBitmap=CreateBitmap(width_pix,height_pix,planes,bpp,NULL);
    if (hBitmap==NULL)
        {
        ReleaseDC(h,hDC);
        willus_mem_free((double **)&bmi,funcname);
        return(5);
        }

    // Create a device context for the windows bitmap
    hMemDC=CreateCompatibleDC(hDC);
    if (hMemDC==NULL)
        {
        nprintf(out,"win_emf_to_bmp:  Could not create hMemDC.\n");
        DeleteObject(hBitmap);
        ReleaseDC(h,hDC);
        willus_mem_free((double **)&bmi,funcname);
        return(6);
        }

    // Select the windows bitmap into the new device context
    SelectObject(hMemDC,hBitmap);

    // Clear the windows bitmap
    PatBlt(hMemDC,0,0,width_pix,height_pix,WHITENESS); // White background

    // Play the metafile into it
    PlayEnhMetaFile(hMemDC,hemf,&lprect);

    bmp->bpp=24;
    bmp->width=width_pix;
    bmp->height=height_pix;
    bmp->type=WILLUSBITMAP_TYPE_WIN32;
    bmp_alloc(bmp);

    // Set up the device independent bitmap
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = width_pix;
    bmi->bmiHeader.biHeight = height_pix;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = 24;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = 0;
    bmi->bmiHeader.biXPelsPerMeter = (int)(dpi*100./2.54);
    bmi->bmiHeader.biYPelsPerMeter = (int)(dpi*100./2.54);
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;

    // Get the bits from the windows bitmap into the DI bitmap
    GetDIBits(hMemDC,hBitmap,0,height_pix,bmp->data,bmi,DIB_RGB_COLORS);
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);
    ReleaseDC(h,hDC);
    willus_mem_free((double **)&bmi,funcname);
    return(0);
    }


static LOGPALETTE *bmp8_pal_data=NULL;
static HPALETTE    bmp8_palette=NULL;

void bmp8_win32_palette_init(WILLUSBITMAP *bmp)

    {
    static char *funcname="bmp8_win32_palette_init";

    if (!willus_mem_alloc((double **)&bmp8_pal_data,sizeof(LOGPALETTE)+256*sizeof(PALETTEENTRY),funcname))
        return;
    bmp8_pal_data->palVersion = 0x300;
    bmp8_pal_data->palNumEntries = 256;
    bmp8_palette = NULL;
    }


void bmp8_win32_palette_set(WILLUSBITMAP *bmp)

    {
    int     i;

    if (bmp8_pal_data==NULL)
        bmp8_win32_palette_init(bmp);
    if (bmp8_pal_data==NULL)
        return;
    for (i=0;i<256;i++)
        {
        bmp8_pal_data->palPalEntry[i].peRed   = bmp->red[i];
        bmp8_pal_data->palPalEntry[i].peGreen = bmp->green[i];
        bmp8_pal_data->palPalEntry[i].peBlue  = bmp->blue[i];
        bmp8_pal_data->palPalEntry[i].peFlags = PC_RESERVED;
        }
    if (bmp8_palette!=NULL)
        {
        DeleteObject(bmp8_palette);
        bmp8_palette=NULL;
        }
    bmp8_palette=CreatePalette(bmp8_pal_data);
    }


void bmp8_win32_palette_free(WILLUSBITMAP *bmp)

    {
    if (bmp8_palette!=NULL)
        {
        DeleteObject(bmp8_palette);
        bmp8_palette=NULL;
        }
    willus_mem_free((double **)&bmp8_pal_data,"bmp8_win32_palette_free");
    }



int bmp_show_bmp(WILLUSBITMAP *bmp,void *handle,int x0,int y0)

    {
    return(bmp_show_bmp_ex(bmp,handle,x0,y0,1));
    }


int bmp_show_bmp_ex(WILLUSBITMAP *bmp,void *handle,int x0,int y0,int update)

    {
    HWND h;
    HDC hDC;

    h=(HWND)handle;
    hDC=GetDC(h);
    if (hDC==NULL)
        return(4);
    bmp_blit_to_hdc(bmp,hDC,x0,y0);
    if (update)
        UpdateWindow(h);
    /*
    UpdateWindow(h);
    SelectPalette(hDC,bmp8_palette,FALSE);
    RealizePalette(hDC);
    */
    ReleaseDC(h,hDC);
    return(0);
    }


void bmp_blit_to_hdc(WILLUSBITMAP *bmp,void *hdc,int x0,int y0)

    {
    bmp_blit_to_hdc_ex(bmp,hdc,x0,y0,bmp->width,bmp->height,0,0);
    }


/*
** Display bitmap <bmp> into HDC.
**
** x0,y0 = top left corner of destination rectangle in HDC
** width,height = width and height of destination rectangle in HDC
** xs,ys = top left corner of where the source pixels come from in the
**         bitmap.  E.g. xs,ys = 0,0 means the top left corner of the
**         source bitmap is placed at x0,y0 in the destination device.
*/
void bmp_blit_to_hdc_ex(WILLUSBITMAP *bmp,void *hdc,int x0,int y0,int width,int height,
                        int xs,int ys)

    {
    HDC hDC;
    BITMAPINFO *bmi;
    int i,bytewidth,w,bpp;
    /* Worst case size (8-bit w/palette) */
    static char _bmih[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    hDC=(HDC)hdc;
    /*
    ** Allocate bit map info structure (has to be allocated due to
    ** arbitrarily sized color palette ==> cannot be statically declared)
    */
    bmi = (BITMAPINFO *)_bmih;
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = bmp->width;
    bmi->bmiHeader.biHeight = bmp->height;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = bmp->bpp;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = bmp->width*bmp->height*(bmp->bpp==8 ? 1 : 3);
    bmi->bmiHeader.biXPelsPerMeter = 72;
    bmi->bmiHeader.biYPelsPerMeter = 72;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;
    /*
    ** Set up GIF palette
    */
    if (bmp->bpp==8)
        {
        RGBQUAD *rgb;
        rgb=&bmi->bmiColors[0];
        for (i=0;i<256;i++)
            {
            rgb[i].rgbRed   = bmp->red[i];
            rgb[i].rgbGreen = bmp->green[i];
            rgb[i].rgbBlue  = bmp->blue[i];
            rgb[i].rgbReserved = 0;
            }
        bmp8_win32_palette_set(bmp);
        if (bmp8_palette!=NULL)
            {
            SelectPalette(hDC,bmp8_palette,FALSE);
            RealizePalette(hDC);
            }
        }

    if (bmp->type==WILLUSBITMAP_TYPE_WIN32)
        {
        if (xs==0 && ys==0 && width==bmp->width && height==bmp->height)
            {
            SetDIBitsToDevice(hDC,x0,y0,bmp->width,bmp->height,0,0,0,bmp->height,
                              bmp->data,bmi,DIB_RGB_COLORS);
            return;
            }
        }
    if (bmp->type!=WILLUSBITMAP_TYPE_WIN32 && bmp->bpp==24)
        bmp24_flip_rgb(bmp);
    bpp = bmp->bpp==24 ? 3 : 1;
    w = width < bmp->width-xs ? width : bmp->width-xs;
    bytewidth = bmp_bytewidth(bmp);
    for (i=ys;i<bmp->height && i<ys+height;i++)
        SetDIBitsToDevice(hDC,x0,i+y0-ys,w,1,0,0,0,1,
                      &bmp->data[i*bytewidth]+bpp*xs,bmi,DIB_RGB_COLORS);
    if (bmp->type!=WILLUSBITMAP_TYPE_WIN32 && bmp->bpp==24)
        bmp24_flip_rgb(bmp);
    /*
    else
        {
        w = width < bmp->width-xs ? width : bmp->width-xs;
        bytewidth = bmp_bytewidth(bmp);
        for (i=ys;i<bmp->height && i<ys+height;i++)
            SetDIBitsToDevice(hDC,x0,i+y0,w,1,0,0,0,1,
                          &bmp->data[i*bytewidth],bmi,DIB_RGB_COLORS);
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        }
    */
    }


/*
** Get the contents of the specified window handle and put
** them into the specified bitmap.  x0,y0 = upper left coordinates
** of source rectangle.
*/
int bmp_get_bmp(WILLUSBITMAP *bmp,void *handle,int x0,int y0)

    {
    HWND    h;
    HDC     hDC,memDC;
    BITMAPINFO         *bmi;
    static  HBITMAP hBitmap=NULL;
    int     i,bytewidth;
    int     planes,bpp;
    /* Worst case size (8-bit w/palette) */
    static char _bmih[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
    /*
    RECT    winsize;
    */

    h=(HWND)handle;
    /*
    h=GetDesktopWindow();
    GetWindowRect(h,&winsize);
    winwidth = winsize.right-winsize.left;
    winheight = winsize.bottom-winsize.top;
    */
    /* Get device context */
    hDC=GetDC(h);
    if (hDC==NULL)
        return(4);

    /*
    ** Allocate bit map info structure (has to be allocated due to
    ** arbitrarily sized color palette ==> cannot be statically declared)
    */
    bmi=(BITMAPINFO *)_bmih;
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = bmp->width;
    bmi->bmiHeader.biHeight = bmp->height;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = bmp->bpp;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = bmp->width*bmp->height*(bmp->bpp==8 ? 1 : 3);
    bmi->bmiHeader.biXPelsPerMeter = 72;
    bmi->bmiHeader.biYPelsPerMeter = 72;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;

    planes=GetDeviceCaps(hDC,PLANES);
    bpp=GetDeviceCaps(hDC,BITSPIXEL);
    /* Create bitmap buffer */
    hBitmap=CreateBitmap(bmp->width,
                         bmp->type==WILLUSBITMAP_TYPE_WIN32 ? bmp->height : 1,
                         planes,bpp,NULL);
    if (hBitmap==NULL)
        {
        ReleaseDC(h,hDC);
        return(5);
        }
    memDC = CreateCompatibleDC(hDC);
    SelectObject(memDC,hBitmap);
    if (bmp->type == WILLUSBITMAP_TYPE_WIN32)
        {
        /* Get the whole enchilida */
        BitBlt(memDC,0,0,bmp->width,bmp->height,hDC,x0,y0,SRCCOPY);
        GetDIBits(memDC,hBitmap,0,bmp->height,bmp->data,bmi,DIB_RGB_COLORS);
        }
    else
        {
        bytewidth = bmp_bytewidth(bmp);

        /* Just one scan line at a time */
        bmi->bmiHeader.biHeight = 1;
        bmi->bmiHeader.biSizeImage = bmp->width*(bmp->bpp==8 ? 1 : 3);

        /* Get the bitmap data */
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            /* int     status; */

            p=&bmp->data[i*bytewidth];
            /* status= */ BitBlt(memDC,0,0,bmp->width,1,hDC,x0,y0+i,SRCCOPY);
            /* status= */ GetDIBits(memDC,hBitmap,0,1,p,bmi,DIB_RGB_COLORS);
            }
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        }
    /* Extract the palette (seems to work) */
    if (bmp->bpp==8)
        {
        RGBQUAD *rgb;
        rgb=&bmi->bmiColors[0];
        for (i=0;i<256;i++)
            {
            bmp->red[i]   = rgb[i].rgbRed;
            bmp->green[i] = rgb[i].rgbGreen;
            bmp->blue[i]  = rgb[i].rgbBlue;
            }
        }

    /* Clean up */
    DeleteDC(memDC);
    DeleteObject(hBitmap);
    ReleaseDC(h,hDC);
    return(0);
    }


void win_icon_free(void *icon)

    {
    DestroyIcon((HICON)icon);
    }


/*
** Create antialiased icons since Windows fails to do this.
**
** This works for the standard way that I compile exe's, which is to include
** a single 72 x 72 icon with them.
**
*/
void win_icons_from_exe(void **iconr,void **smalliconr)

    {
    char  fullname[512];
    char  basename[256];
    char  appname[256];
    HICON icon,smallicon,icon72;
    int   small_dx,small_dy,icon_dx,icon_dy;
    HINSTANCE hinst;

    hinst=(HINSTANCE)win_hinstance();
    win_full_exe_name(fullname);
    wfile_basespec(basename,fullname);
    wfile_newext(appname,basename,"");
    icon_dx = GetSystemMetrics(SM_CXICON);
    icon_dy = GetSystemMetrics(SM_CYICON);
    icon = LoadImage(hinst,appname,IMAGE_ICON,icon_dx,icon_dy,0);
    small_dx = GetSystemMetrics(SM_CXSMICON);
    small_dy = GetSystemMetrics(SM_CYSMICON);
    smallicon = LoadImage(hinst,appname,IMAGE_ICON,small_dx,small_dy,0);
    icon72 = LoadImage(hinst,appname,IMAGE_ICON,72,72,0);
    /* Create anti-aliased icons since Windows doesn't(!) */
    if (icon72)
        {
        WILLUSBITMAP *bmp,_bmp;
        WILLUSBITMAP *bmpsmall,_bmpsmall;
        WILLUSBITMAP *bmplarge,_bmplarge;

        bmp=&_bmp;
        bmp_init(bmp);
        bmp->width=72;
        bmp->height=72;
        bmp->bpp=24;
        bmp_alloc(bmp);
        bmp_from_icon(bmp,icon72);
        bmplarge=&_bmplarge;
        bmp_init(bmplarge);
        bmp_resample(bmplarge,bmp,0.,0.,72.,72.,icon_dx,icon_dy);
        (*iconr)=(void *)bmp_to_icon(bmplarge,icon);
        DestroyIcon(icon);
        bmpsmall=&_bmpsmall;
        bmp_init(bmpsmall);
        bmp_resample(bmpsmall,bmp,0.,0.,72.,72.,small_dx,small_dy);
        (*smalliconr)=(void *)bmp_to_icon(bmpsmall,smallicon);
        DestroyIcon(smallicon);
        bmp_free(bmpsmall);
        bmp_free(bmplarge);
        bmp_free(bmp);
        }
    else
        {
        (*iconr)=(void *)icon;
        (*smalliconr)=(void *)smallicon;
        }
    DestroyIcon(icon72);
    }


/*
** Get the contents of the specified window handle and put
** them into the specified bitmap.  x0,y0 = upper left coordinates
** of source rectangle.
*/
static int bmp_from_icon(WILLUSBITMAP *bmp,HICON hIcon)

    {
    ICONINFO iinfo;

    if (!GetIconInfo(hIcon,&iinfo))
        return(0);
    return(bmp_to_from_winbmp(bmp,iinfo.hbmColor)!=NULL);
    }


static HICON bmp_to_icon(WILLUSBITMAP *bmp,HICON template)

    {
    HICON hIcon;
    ICONINFO iinfo;

    if (!GetIconInfo(template,&iinfo))
        return(0);
    iinfo.hbmColor = bmp_to_from_winbmp(bmp,NULL);
    if (iinfo.hbmColor==NULL)
        return(NULL);
    hIcon=CreateIconIndirect(&iinfo);
    return(hIcon);
    }


/*
** If hBitmap==NULL, converts bmp to HBITMAP and returns handle.
** If not, hBitmap is put into bmp.
*/
static HBITMAP bmp_to_from_winbmp(WILLUSBITMAP *bmp,HBITMAP hBitmap_src)

    {
    HDC     hDC,memDC;
    BITMAPINFO         *bmi;
    HBITMAP hBitmap;
    int     i,bytewidth;
    int     planes,bpp;
    /* Worst case size (8-bit w/palette) */
    static char _bmih[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    hDC=GetDC(GetDesktopWindow());
    if (hDC==NULL)
        return(NULL);
    /*
    ** Allocate bit map info structure (has to be allocated due to
    ** arbitrarily sized color palette ==> cannot be statically declared)
    */
    bmi=(BITMAPINFO *)_bmih;
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = bmp->width;
    bmi->bmiHeader.biHeight = bmp->height;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = bmp->bpp;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = bmp->width*bmp->height*(bmp->bpp==8 ? 1 : 3);
    bmi->bmiHeader.biXPelsPerMeter = 72;
    bmi->bmiHeader.biYPelsPerMeter = 72;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;
    planes=GetDeviceCaps(hDC,PLANES);
    bpp=GetDeviceCaps(hDC,BITSPIXEL);
    if (hBitmap_src==NULL)
        {
        hBitmap=CreateBitmap(bmp->width,bmp->height,planes,bpp,NULL);
        if (hBitmap==NULL)
            {
            ReleaseDC(GetDesktopWindow(),hDC);
            return(NULL);
            }
        }
    else
        hBitmap=hBitmap_src;
    memDC = CreateCompatibleDC(hDC);
    SelectObject(memDC,hBitmap);
    if (bmp->type == WILLUSBITMAP_TYPE_WIN32)
        {
        /* Get the whole enchilida */
        if (hBitmap_src==NULL)
            SetDIBits(memDC,hBitmap,0,bmp->height,bmp->data,bmi,DIB_RGB_COLORS);
        else
            GetDIBits(memDC,hBitmap,0,bmp->height,bmp->data,bmi,DIB_RGB_COLORS);
        }
    else
        {
        bytewidth = bmp_bytewidth(bmp);
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        /* Get the bitmap data */
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            /* int     status; */

            p=&bmp->data[i*bytewidth];
            if (hBitmap_src==NULL)
                SetDIBits(memDC,hBitmap,bmp->height-1-i,1,p,bmi,DIB_RGB_COLORS);
            else
                GetDIBits(memDC,hBitmap,bmp->height-1-i,1,p,bmi,DIB_RGB_COLORS);
            }
        if (bmp->bpp==24)
            bmp24_flip_rgb(bmp);
        }
    /* Extract the palette (seems to work) */
    if (bmp->bpp==8)
        {
        RGBQUAD *colors;

        colors=&bmi->bmiColors[0]; /* Avoid compiler warning */
        for (i=0;i<256;i++)
            {
            bmp->red[i]   = colors[i].rgbRed;
            bmp->green[i] = colors[i].rgbGreen;
            bmp->blue[i]  = colors[i].rgbBlue;
            }
        }

    /* Clean up */
    DeleteDC(memDC);
    ReleaseDC(GetDesktopWindow(),hDC);
    return(hBitmap);
    }


#endif
/* HAVE_WIN32_API */
