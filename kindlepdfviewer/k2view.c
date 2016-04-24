/*
** k2view.c      Demonstration of minimal k2pdfopt interface.
**               (Can be compiled with no third-party lib dependencies.)
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

int main(int argc,char *argv[])

    {
    static K2PDFOPT_SETTINGS _k2settings, *k2settings;
    static MASTERINFO _masterinfo, *masterinfo;
    WILLUSBITMAP _srcgrey, *srcgrey;
    WILLUSBITMAP _src, *src;
    BMPREGION region;
    int status,pages_done;

    if (argc<2)
        {
        printf("usage:  kview <infile.bmp>\n");
        return(0);
        }
    src=&_src;
    bmp_init(src);
    srcgrey=&_srcgrey;
    bmp_init(srcgrey);
    if ((status=bmp_read(src,argv[1],NULL))<0)
        {
        printf("Error %d reading bitmap file %s.\n",status,argv[1]);
        return(10);
        }
    printf("Bitmap %s is %d x %d x %d\n",argv[1],src->width,src->height,src->bpp);

    /* Initialize settings */
    k2settings=&_k2settings;
    k2pdfopt_settings_init(k2settings);
    k2settings->use_crop_boxes=0;
    k2settings->src_rot=0;
    k2settings->erase_vertical_lines=0;
    k2settings->src_autostraighten=0;
    k2pdfopt_settings_quick_sanity_check(k2settings);
    k2settings->user_src_dpi=(int)(src->width/8.5+.5);
    k2settings->dst_dpi=k2settings->src_dpi/2;
    k2settings->dst_userwidth=3.;
    k2settings->dst_userwidth_units=UNITS_INCHES;
    k2settings->dst_userheight=4.;
    k2settings->dst_userheight_units=UNITS_INCHES;
    /* Init for new source doc */
    k2pdfopt_settings_new_source_document_init(k2settings);

    /* Init master output structure */
    masterinfo=&_masterinfo;
    masterinfo_init(masterinfo,k2settings);

    /* Init new source bitmap */
    bmpregion_init(&region);
    masterinfo_new_source_page_init(masterinfo,k2settings,src,srcgrey,NULL,&region,0.,NULL,NULL,1,-1,NULL);
    /* Set output size */
    k2pdfopt_settings_set_margins_and_devsize(k2settings,&region,masterinfo,-1.,0);
    /* Process single source page */
    pages_done=0;
    bmpregion_source_page_add(&region,k2settings,masterinfo,1,pages_done++);
    bmp_free(srcgrey);
    bmp_free(src);

    /*
    ** Get output pages
    */ 
    {
    WILLUSBITMAP *bmp,_bmp;
    int pn,rows,size_reduction;
    double bmpdpi;

    bmp=&_bmp;
    bmp_init(bmp);
    pn=0;
    while ((rows=masterinfo_get_next_output_page(masterinfo,k2settings,1,bmp,
                                                 &bmpdpi,&size_reduction,NULL))>0)
        {
        char filename[256];
        pn++;
        sprintf(filename,"outpage%02d.bmp",pn);
        bmp_write(bmp,filename,stdout,0);
        wfile_written_info(filename,stdout);
        }
    bmp_free(bmp);
    }
    masterinfo_free(masterinfo,k2settings);
    return(0);
    }
