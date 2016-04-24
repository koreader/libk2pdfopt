/*
** pdfwrite.c   Routines to help write a PDF file.
**
** Part of willus.com general purpose C code library.
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

/*
** IMPORTANT!!
**
** NEEDS SPECIAL VERSION OF ZLIB WITH CUSTOM MODES--SEE gzflags BELOW!
** SEE gzwrite.c and gzlib.c in zlib_mod FOLDER.
** (SEARCH FOR "WILLUS MOD" IN FILES.)
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

#define MAXPDFPAGES 10000

typedef struct
    {
    int cid;    /* cid = 0xAAABB where 0xAAA is the font # and 0xBB is the CID */
    int unicode;
    } WILLUSCHARMAP;

/*
** Array of mapping from character ID's to unicode values.
** Populated for each new page.
*/
typedef struct
    {
    WILLUSCHARMAP *cmap;
    int n;
    int na;
    } WILLUSCHARMAPLIST;

typedef struct
    {
    double abovebase;
    double belowbase;
    double x0;
    double width;
    double nextchar;
    } WILLUSCHARINFO;




static WILLUSCHARINFO Helvetica[224] =
    {
    /*    */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* !  */ { 0.72944,-0.00080, 0.12352, 0.08496, 0.27792},
    /* "  */ { 0.70928,-0.46448, 0.05152, 0.25344, 0.35568},
    /* #  */ { 0.69776, 0.01936, 0.01264, 0.52992, 0.55584},
    /* $  */ { 0.77120, 0.12448, 0.03136, 0.48672, 0.55584},
    /* %  */ { 0.70928, 0.01936, 0.02848, 0.83088, 0.88992},
    /* &  */ { 0.70928, 0.02224, 0.05152, 0.58608, 0.66672},
    /* '  */ { 0.72944,-0.49760, 0.06448, 0.09360, 0.22176},
    /* (  */ { 0.72944, 0.21088, 0.07168, 0.22032, 0.33264},
    /* )  */ { 0.72944, 0.21088, 0.03712, 0.21888, 0.33264},
    /* *  */ { 0.72944,-0.44144, 0.03856, 0.30528, 0.38880},
    /* +  */ { 0.47456, 0.00928, 0.04864, 0.48528, 0.58464},
    /* ,  */ { 0.10448, 0.14608, 0.08608, 0.10656, 0.27792},
    /* -  */ { 0.31328,-0.24128, 0.04576, 0.23904, 0.33264},
    /* .  */ { 0.10448,-0.00080, 0.08608, 0.10512, 0.27792},
    /* /  */ { 0.72944, 0.01936,-0.00896, 0.29376, 0.27792},
    /* 0  */ { 0.70928, 0.02224, 0.04288, 0.46512, 0.55584},
    /* 1  */ { 0.70928,-0.00080, 0.10192, 0.24624, 0.55584},
    /* 2  */ { 0.70928,-0.00080, 0.03280, 0.47952, 0.55584},
    /* 3  */ { 0.70928, 0.02224, 0.03136, 0.47520, 0.55584},
    /* 4  */ { 0.70928,-0.00080, 0.02704, 0.49392, 0.55584},
    /* 5  */ { 0.70928, 0.02224, 0.03424, 0.47952, 0.55584},
    /* 6  */ { 0.70928, 0.02224, 0.04288, 0.47088, 0.55584},
    /* 7  */ { 0.70928,-0.00080, 0.04576, 0.47520, 0.55584},
    /* 8  */ { 0.70928, 0.02224, 0.03568, 0.47808, 0.55584},
    /* 9  */ { 0.70928, 0.02224, 0.03712, 0.47232, 0.55584},
    /* :  */ { 0.52496,-0.00080, 0.10912, 0.10512, 0.27792},
    /* ;  */ { 0.52496, 0.14608, 0.10912, 0.10656, 0.27792},
    /* <  */ { 0.47456, 0.00784, 0.04432, 0.48960, 0.58464},
    /* =  */ { 0.35360,-0.11168, 0.04864, 0.48528, 0.58464},
    /* >  */ { 0.47456, 0.00784, 0.04864, 0.49104, 0.58464},
    /* ?  */ { 0.74240,-0.00080, 0.07600, 0.43344, 0.55584},
    /* @  */ { 0.74240, 0.14176, 0.03280, 0.91872, 1.01520},
    /* A  */ { 0.72944,-0.00080, 0.01696, 0.63648, 0.66672},
    /* B  */ { 0.72944,-0.00080, 0.07888, 0.54432, 0.66672},
    /* C  */ { 0.74240, 0.02224, 0.04720, 0.63072, 0.72288},
    /* D  */ { 0.72944,-0.00080, 0.08896, 0.57888, 0.72288},
    /* E  */ { 0.72944,-0.00080, 0.08896, 0.52416, 0.66672},
    /* F  */ { 0.72944,-0.00080, 0.08896, 0.49104, 0.61056},
    /* G  */ { 0.74240, 0.02224, 0.04288, 0.66672, 0.77760},
    /* H  */ { 0.72944,-0.00080, 0.08176, 0.56304, 0.72288},
    /* I  */ { 0.72944,-0.00080, 0.09904, 0.09504, 0.27792},
    /* J  */ { 0.72944, 0.02224, 0.01696, 0.40896, 0.49968},
    /* K  */ { 0.72944,-0.00080, 0.07888, 0.58032, 0.66672},
    /* L  */ { 0.72944,-0.00080, 0.07888, 0.45504, 0.55584},
    /* M  */ { 0.72944,-0.00080, 0.07456, 0.68688, 0.83376},
    /* N  */ { 0.72944,-0.00080, 0.07600, 0.57024, 0.72288},
    /* O  */ { 0.74240, 0.02224, 0.03712, 0.70560, 0.77760},
    /* P  */ { 0.72944,-0.00080, 0.09040, 0.52704, 0.66672},
    /* Q  */ { 0.74240, 0.05824, 0.03712, 0.70560, 0.77760},
    /* R  */ { 0.72944,-0.00080, 0.09184, 0.58752, 0.72288},
    /* S  */ { 0.74240, 0.02224, 0.04720, 0.57456, 0.66672},
    /* T  */ { 0.72944,-0.00080, 0.01984, 0.57312, 0.61056},
    /* U  */ { 0.72944, 0.02224, 0.08464, 0.56160, 0.72288},
    /* V  */ { 0.72944,-0.00080, 0.02992, 0.61632, 0.66672},
    /* W  */ { 0.72944,-0.00080, 0.02128, 0.90864, 0.94464},
    /* X  */ { 0.72944,-0.00080, 0.02128, 0.62784, 0.66672},
    /* Y  */ { 0.72944,-0.00080, 0.01264, 0.64944, 0.66672},
    /* Z  */ { 0.72944,-0.00080, 0.02704, 0.55728, 0.61056},
    /* [  */ { 0.72944, 0.21088, 0.06304, 0.18720, 0.27792},
    /* \  */ { 0.72944, 0.01936,-0.00896, 0.29376, 0.27792},
    /* ]  */ { 0.72944, 0.21088, 0.02272, 0.18720, 0.27792},
    /* ^  */ { 0.70928,-0.32912, 0.04288, 0.38304, 0.46944},
    /* _  */ {-0.12592, 0.17488,-0.02336, 0.60192, 0.55584},
    /* `  */ { 0.70928,-0.47744, 0.06448, 0.09360, 0.22176},
    /* a  */ { 0.53936, 0.02224, 0.04144, 0.49392, 0.55584},
    /* b  */ { 0.72944, 0.02224, 0.05296, 0.47088, 0.55584},
    /* c  */ { 0.53936, 0.02224, 0.02992, 0.44784, 0.49968},
    /* d  */ { 0.72944, 0.02224, 0.02560, 0.46944, 0.55584},
    /* e  */ { 0.53936, 0.02224, 0.03856, 0.47520, 0.55584},
    /* f  */ { 0.73232,-0.00080, 0.01696, 0.24192, 0.27792},
    /* g  */ { 0.53936, 0.21664, 0.02848, 0.46080, 0.55584},
    /* h  */ { 0.72944,-0.00080, 0.06880, 0.41760, 0.55584},
    /* i  */ { 0.72944,-0.00080, 0.06592, 0.08496, 0.22176},
    /* j  */ { 0.72944, 0.21664,-0.01904, 0.17280, 0.22176},
    /* k  */ { 0.72944,-0.00080, 0.05728, 0.44496, 0.49968},
    /* l  */ { 0.72944,-0.00080, 0.06736, 0.08496, 0.22176},
    /* m  */ { 0.53936,-0.00080, 0.06880, 0.69408, 0.83376},
    /* n  */ { 0.53936,-0.00080, 0.06880, 0.41904, 0.55584},
    /* o  */ { 0.53936, 0.02224, 0.03568, 0.47520, 0.55584},
    /* p  */ { 0.53936, 0.21664, 0.05296, 0.47088, 0.55584},
    /* q  */ { 0.53936, 0.21664, 0.02560, 0.46944, 0.55584},
    /* r  */ { 0.53936,-0.00080, 0.06880, 0.25344, 0.33264},
    /* s  */ { 0.53936, 0.02224, 0.03280, 0.42624, 0.49968},
    /* t  */ { 0.66896, 0.02224, 0.01264, 0.24192, 0.27792},
    /* u  */ { 0.52496, 0.02224, 0.06448, 0.41760, 0.55584},
    /* v  */ { 0.52496,-0.00080, 0.00976, 0.47664, 0.49968},
    /* w  */ { 0.52496,-0.00080, 0.00544, 0.70272, 0.72288},
    /* x  */ { 0.52496,-0.00080, 0.01696, 0.45648, 0.49968},
    /* y  */ { 0.52496, 0.21664, 0.01984, 0.45936, 0.49968},
    /* z  */ { 0.52496,-0.00080, 0.02992, 0.42768, 0.49968},
    /* {  */ { 0.72944, 0.21088, 0.04288, 0.23328, 0.33408},
    /* |  */ { 0.72944, 0.21088, 0.09904, 0.06192, 0.26064},
    /* }  */ { 0.72944, 0.21088, 0.02848, 0.23472, 0.33408},
    /* ~  */ { 0.43856,-0.26864, 0.07456, 0.43344, 0.58464},
    /* 7F */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 80 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 81 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 82 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 83 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 84 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 85 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 86 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 87 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 88 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 89 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8A */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8B */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8C */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8D */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8E */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 8F */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 90 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 91 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 92 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 93 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 94 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 95 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 96 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 97 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 98 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 99 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9A */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9B */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9C */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9D */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9E */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* 9F */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* A0 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* A1 */ { 0.52496, 0.20368, 0.12064, 0.08496, 0.33264},
    /* A2 */ { 0.62864, 0.11872, 0.05152, 0.45936, 0.55584},
    /* A3 */ { 0.72944, 0.02224, 0.02560, 0.50976, 0.55584},
    /* A4 */ { 0.67904, 0.01936,-0.17456, 0.43344, 0.16704},
    /* A5 */ { 0.70928,-0.00080, 0.00976, 0.53568, 0.55584},
    /* A6 */ { 0.73808, 0.21088, 0.00976, 0.53280, 0.55584},
    /* A7 */ { 0.72944, 0.21232, 0.04288, 0.46368, 0.55584},
    /* A8 */ { 0.55232,-0.13328, 0.06592, 0.42336, 0.55584},
    /* A9 */ { 0.70928,-0.46448, 0.04720, 0.09504, 0.19152},
    /* AA */ { 0.70928,-0.47744, 0.04720, 0.25200, 0.33264},
    /* AB */ { 0.43856,-0.10736, 0.09760, 0.35856, 0.55584},
    /* AC */ { 0.43856,-0.10736, 0.09040, 0.15264, 0.33264},
    /* AD */ { 0.43856,-0.10736, 0.08464, 0.15552, 0.33264},
    /* AE */ { 0.73232,-0.00080, 0.01120, 0.42480, 0.49968},
    /* AF */ { 0.73232,-0.00080, 0.01696, 0.41328, 0.49968},
    /* B0 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* B1 */ { 0.31328,-0.24128,-0.00608, 0.56736, 0.55584},
    /* B2 */ { 0.70928, 0.17632, 0.03712, 0.47664, 0.55584},
    /* B3 */ { 0.70928, 0.17632, 0.03712, 0.47664, 0.55584},
    /* B4 */ { 0.42704,-0.30320, 0.08608, 0.12528, 0.27792},
    /* B5 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* B6 */ { 0.72944, 0.17632, 0.04720, 0.47520, 0.53712},
    /* B7 */ { 0.47024,-0.22112, 0.04864, 0.25200, 0.34992},
    /* B8 */ { 0.10448, 0.12736, 0.06304, 0.09504, 0.22176},
    /* B9 */ { 0.10448, 0.12736, 0.04576, 0.25488, 0.33264},
    /* BA */ { 0.70928,-0.47744, 0.04864, 0.25344, 0.33264},
    /* BB */ { 0.43856,-0.10736, 0.09760, 0.35424, 0.55584},
    /* BC */ { 0.10448,-0.00080, 0.11488, 0.77040, 1.00080},
    /* BD */ { 0.73808, 0.02080, 0.00832, 0.98496, 1.00080},
    /* BE */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* BF */ { 0.52496, 0.21664, 0.09472, 0.43344, 0.61056},
    /* C0 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* C1 */ { 0.74096,-0.59264, 0.02128, 0.21024, 0.33264},
    /* C2 */ { 0.74096,-0.59264, 0.09184, 0.21024, 0.33264},
    /* C3 */ { 0.74240,-0.59120, 0.01984, 0.28800, 0.33264},
    /* C4 */ { 0.71792,-0.61424, 0.00400, 0.31536, 0.33264},
    /* C5 */ { 0.70208,-0.63152, 0.02704, 0.27504, 0.33264},
    /* C6 */ { 0.73232,-0.59840, 0.01408, 0.30240, 0.33264},
    /* C7 */ { 0.71648,-0.61280, 0.11488, 0.10512, 0.33264},
    /* C8 */ { 0.71504,-0.61280, 0.02992, 0.26640, 0.33264},
    /* C9 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* CA */ { 0.75536,-0.57968, 0.07888, 0.17712, 0.33264},
    /* CB */ { 0.00080, 0.21376, 0.03856, 0.24912, 0.33264},
    /* CC */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* CD */ { 0.74096,-0.59120,-0.03632, 0.38448, 0.33264},
    /* CE */ { 0.00080, 0.20368, 0.05584, 0.21024, 0.33264},
    /* CF */ { 0.74240,-0.59120, 0.01840, 0.28800, 0.33264},
    /* D0 */ { 0.31328,-0.24128,-0.01040, 1.01232, 1.00080},
    /* D1 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D2 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D3 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D4 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D5 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D6 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D7 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D8 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* D9 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DA */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DB */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DC */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DD */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DE */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* DF */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E0 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E1 */ { 0.72944,-0.00080, 0.00976, 0.94032, 1.00080},
    /* E2 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E3 */ { 0.74240,-0.30320, 0.03568, 0.29808, 0.37008},
    /* E4 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E5 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E6 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E7 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* E8 */ { 0.72944,-0.00080,-0.00032, 0.55296, 0.55584},
    /* E9 */ { 0.75536, 0.02224, 0.02992, 0.71424, 0.77760},
    /* EA */ { 0.74240, 0.01936, 0.04288, 0.91728, 1.00080},
    /* EB */ { 0.74240,-0.30320, 0.03856, 0.28656, 0.36576},
    /* EC */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* ED */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* EE */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* EF */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F0 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F1 */ { 0.53936, 0.02224, 0.03280, 0.81216, 0.88992},
    /* F2 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F3 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F4 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F5 */ { 0.52496,-0.00080, 0.09328, 0.08496, 0.27792},
    /* F6 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F7 */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* F8 */ { 0.72944,-0.00080,-0.00032, 0.21312, 0.22176},
    /* F9 */ { 0.53936, 0.02944, 0.01696, 0.51264, 0.61056},
    /* FA */ { 0.53936, 0.02224, 0.03856, 0.86112, 0.94464},
    /* FB */ { 0.72944, 0.01936, 0.12496, 0.44208, 0.61056},
    /* FC */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* FD */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* FE */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792},
    /* FF */ {-1.00000,-1.00000,-1.00000,-1.00000, 0.27792}
    };

static int lastfont=-1;
static double lastfontsize=-1;

static void pdffile_start(PDFFILE *pdf,int pages_at_end);
static int pdffile_page_reference(PDFFILE *pdf,int pageno);
static void pdf_utf8_out(FILE *out,char *s);
static void pdffile_unicode_map(PDFFILE *pdf,WILLUSCHARMAPLIST *cmaplist,int nf);
static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp);
static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *bmp,int quality,int halfsize,int thumb);
static void bmp_flate_decode(WILLUSBITMAP *bmp,FILE *f,compress_handle handle,int halfsize);
static void pdffile_new_object(PDFFILE *pdf,int flags);
static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object);
#ifdef HAVE_Z_LIB
static int pdf_numpages_1(void *ptr,int bufsize);
static int decodecheck(FILE *f,int np);
static int wpdf_getline(char *buf,int maxlen,FILE *f);
static int wpdf_getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize);
#endif
static void insert_length(FILE *f,long pos,int len);
static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode,
                                   WILLUSCHARMAPLIST *cmaplist,int use_spaces,int ocr_flags);
static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi,WILLUSCHARMAPLIST *cmaplist);
static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight,
                                        WILLUSCHARMAPLIST *cmaplist,double *charpos);
static double size_round_off(double size,double median_size,double log_size_increment);
static void ocrwords_optimize_spaces(OCRWORD *sentence,OCRWORD *word,int n,double dpi,
                                     WILLUSCHARMAPLIST *cmaplist,int optimize);
static void ocrwords_sentence_construct(OCRWORD *sentence,OCRWORD *word,int n,int *nspaces);
static void sentence_check_alignment(OCRWORD *word,int n,int *nspaces,double *pos,
                                     WILLUSCHARMAPLIST *cmaplist);
static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts,
                                  WILLUSCHARMAPLIST *cmaplist,int ocr_flags);
static void willuscharmaplist_init(WILLUSCHARMAPLIST *list);
static void willuscharmaplist_free(WILLUSCHARMAPLIST *list);
static void willuscharmaplist_add_charmap(WILLUSCHARMAPLIST *list,int unichar);
static int  willuscharmaplist_maxcid(WILLUSCHARMAPLIST *list);
static int  willuscharmaplist_cid_index(WILLUSCHARMAPLIST *list,int unichar);
static void willuscharmaplist_populate(WILLUSCHARMAPLIST *cmaplist,OCRWORDS *ocrwords);
static void willuscharmaplist_populate_string(WILLUSCHARMAPLIST *cmaplist,char *s);

/* Outline support functions */
static int wpdfoutline_num_anchors_on_level(WPDFOUTLINE *outline,int *rc);
static int wpdfoutline_within(WPDFOUTLINE *outline,WPDFOUTLINE *outline1);
static int wpdfoutline_index(WPDFOUTLINE *outline,WPDFOUTLINE *outline1);
static int wpdfoutline_num_anchors_recursive(WPDFOUTLINE *outline);
static WPDFOUTLINE *wpdfoutline_by_index(WPDFOUTLINE *outline,int n);
static WPDFOUTLINE *wpdfoutline_previous(WPDFOUTLINE *outline,WPDFOUTLINE *outline1);
static WPDFOUTLINE *wpdfoutline_parent(WPDFOUTLINE *outline,WPDFOUTLINE *outline1);
static int legal_pdf_encoded_byte(int c);


FILE *pdffile_init(PDFFILE *pdf,char *filename,int pages_at_end)

    {
    pdf->n=pdf->na=0;
    pdf->object=NULL;
    pdf->pae=0;
    pdf->imc=0;
    strncpy(pdf->filename,filename,511);
    pdf->filename[511]='\0';
    pdf->f = wfile_fopen_utf8(filename,"wb");
    if (pdf->f!=NULL)
        fclose(pdf->f);
    pdf->f = wfile_fopen_utf8(filename,"rb+");
    if (pdf->f!=NULL)
        pdffile_start(pdf,pages_at_end);
    return(pdf->f);
    }

void pdffile_close(PDFFILE *pdf)

    {
    if (pdf->f!=NULL)
        {
        fclose(pdf->f);
        pdf->f=NULL;
        }
    willus_mem_free((double **)&pdf->object,"pdffile_close");
    pdf->n=pdf->na=pdf->imc=0;
    }


static void pdffile_start(PDFFILE *pdf,int pages_at_end)

    {
    fprintf(pdf->f,"%%PDF-1.3 \n");
    pdffile_new_object(pdf,2);
    fprintf(pdf->f,"<<\n"
                   "/Pages ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    if (pages_at_end)
        fprintf(pdf->f,"      ");
    else
        fprintf(pdf->f,"2");
    fprintf(pdf->f," 0 R\n"
                   "/Outlines ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[2]=ftell(pdf->f);
    fprintf(pdf->f,"       0 R\n"
                   "/Type /Catalog\n"
                   ">>\n"
                   "endobj\n");
    if (!pages_at_end)
        {
        int i;
        char cline[73];
        pdffile_new_object(pdf,4);
        fprintf(pdf->f,"<<\n"
                       "/Type /Pages\n"
                       "/Kids [");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        pdf->pae=ftell(pdf->f);
        cline[0]='%';
        cline[1]='%';
        for (i=2;i<71;i++)
            cline[i]=' ';
        cline[71]='\n';
        cline[72]='\0';
        for (i=0;i<120;i++)
            fprintf(pdf->f,"%s",cline);
        }
    else
        pdf->pae=0;
    }


int pdffile_page_count(PDFFILE *pdf)

    {
    int i,np;

    for (np=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            np++;
    return(np);
    }


static int pdffile_page_reference(PDFFILE *pdf,int pageno)

    {
    int i,np;

    for (np=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            {
            np++;
            if (np==pageno)
                return(i+1);
            }
    return(-1);
    }


/*
** Must be called after all pages are added
*/
void pdffile_add_outline(PDFFILE *pdf,WPDFOUTLINE *outline)

    {
    int i,n0,n,nl,np,rcount;

    if (outline==NULL)
        return;
    np=pdffile_page_count(pdf);
    wpdfoutline_fill_in_blank_dstpages(outline,np);
    n=wpdfoutline_num_anchors_recursive(outline);
    if (n==0)
         return;
    nl=wpdfoutline_num_anchors_on_level(outline,&rcount);
    /* Outline head */
    pdffile_new_object(pdf,4);
    fprintf(pdf->f,"<<\n"
                   "  /Count %d\n"
                   "  /First %d 0 R\n"
                   "  /Last %d 0 R\n"
                   "  /Type /Outlines\n"
                   ">>\n"
                   "endobj\n\n",
                   nl,pdf->n+1,pdf->n+1+rcount*2);
    n0=pdf->n+1;
    for (i=0;i<n;i++)
        {
        WPDFOUTLINE *local,*next;

        pdffile_new_object(pdf,8);
        local=wpdfoutline_by_index(outline,i);
        fprintf(pdf->f,"<<\n"
                       "  /A %d 0 R\n",pdf->n+1);
        if (local->down!=NULL)
            {
            int nl2,rc2;

            nl2=wpdfoutline_num_anchors_on_level(local->down,&rc2);
            fprintf(pdf->f,"  /Count %d\n"
                           "  /First %d 0 R\n"
                           "  /Last %d 0 R\n",
                           nl2,
                           pdf->n+2,
                           pdf->n+2+rc2*2);
            }
        if (local->next!=NULL)
            {
            int rc2;
            rc2=wpdfoutline_num_anchors_recursive(local->down);
            fprintf(pdf->f,"  /Next %d 0 R\n",pdf->n+2+rc2*2);
            }
        next=wpdfoutline_previous(outline,local);
        if (next!=NULL)
            fprintf(pdf->f,"  /Prev %d 0 R\n",n0+wpdfoutline_index(outline,next)*2);
        next=wpdfoutline_parent(outline,local);
        if (next!=NULL)
            fprintf(pdf->f,"  /Parent %d 0 R\n",n0+wpdfoutline_index(outline,next)*2);
        else
            fprintf(pdf->f,"  /Parent %d 0 R\n",n0-1);
        fprintf(pdf->f,"  /Title ");
        pdf_utf8_out(pdf->f,local->title);
        fprintf(pdf->f,"\n"
                       ">>\n"
                       "endobj\n\n");
        pdffile_new_object(pdf,16);
        fprintf(pdf->f,"<<\n"
                       "  /D [ %d 0 R /Fit ]\n"
                       "  /S /GoTo\n"
                       ">>\n"
                       "endobj\n\n",pdffile_page_reference(pdf,local->dstpage+1));
        }
    }


static void pdf_utf8_out(FILE *out,char *s)

    {
    static char *funcname="pdf_utf8_out";
    int i,len,docenc;
    int *d;
    unsigned char *de;

    willus_mem_alloc_warn((void **)&de,strlen(s)+2,funcname,10);
    docenc=wpdf_docenc_from_utf8((char *)de,s,strlen(s)+1);
    if (docenc)
        {
        fprintf(out,"(");
        for (i=0;de[i]!='\0';i++)
            if (de[i]>=32 && de[i]<=127)
                fputc(de[i],out);
            else
                fprintf(out,"\\%03o",de[i]);
        fprintf(out,")");
        }
    willus_mem_free((double **)&de,funcname);
    if (docenc)
        return;
    /* If UTF-8 text can't be PDF document-encoded, convert to Unicode-16 */
    len=strlen(s)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*len,funcname,10);
    len=utf8_to_unicode(d,s,len-1);
    fprintf(out,"<FEFF");
    for (i=0;i<len;i++)
        fprintf(out,"%04X",d[i]);
    fprintf(out,">");
    willus_mem_free((double **)&d,funcname);
    }


void pdffile_add_bitmap(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,int quality,int halfsize)

    {
    pdffile_add_bitmap_with_ocrwords(pdf,bmp,dpi,quality,halfsize,NULL,1);
    }


/*
** Use quality=-1 for PNG
**
** NEEDS SPECIAL VERSION OF ZLIB WITH CUSTOM MOD--SEE gzflags BELOW!
** SEE gzwrite.c and gzlib.c in zlib_mod FOLDER.
** (SEARCH FOR "WILLUS MOD" IN FILES.)
**
** If quality < 0, the deflate (PNG-style) method is used.
**
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
**
** ocr_render_flags
**     Bit 1 (1):  1=Show source bitmap
**     Bit 2 (2):  1=Show OCR text
**     Bit 3 (4):  1=Box around text
**     Bit 4 (8):  1=Use spaces, 1 space per word
**     Bit 5 (16): 1=Use spaces, optimize number of spaces per word
**     Bit 6 (32): 1=Do not round off font sizes
**
*/
void pdffile_add_bitmap_with_ocrwords(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,
                                      int quality,int halfsize,OCRWORDS *ocrwords,
                                      int ocr_render_flags)

    {
    double pw,ph;
    int ptr1,ptr2,ptrlen,showbitmap,nf;
    WILLUSCHARMAPLIST *cmaplist,_cmaplist;
/*
{
int i;
printf("ADDING BITMAP.\n");
printf("    Words=%d\n",ocrwords->n);
printf("    dpi=%g\n",dpi);
printf("    bmp=%dx%d\n",bmp->width,bmp->height);
for (i=0;i<ocrwords->n;i++)
printf("    OCR %s: (c=%d,r=%d) (%dx%d)\n",
ocrwords->word[i].text,
ocrwords->word[i].c,
ocrwords->word[i].r,
ocrwords->word[i].w,
ocrwords->word[i].h);
}
*/
    lastfont=-1;
    lastfontsize=-1;
    showbitmap = (ocr_render_flags&1);

    pw=bmp->width*72./dpi;
    ph=bmp->height*72./dpi;

    /* New page object */
    pdffile_new_object(pdf,3);
    pdf->imc++;
    fprintf(pdf->f,"<<\n"
                   "/Type /Page\n"
                   "/Parent ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    fprintf(pdf->f,"%s 0 R\n"
                   "/Resources\n    <<\n",
                   pdf->pae>0 ? "2" : "      ");
    if (ocrwords!=NULL)
        {
        int maxid,ifont;
        cmaplist=&_cmaplist;
        willuscharmaplist_init(cmaplist);
        /*
        ** Map all unicode chars to fonts
        */
        willuscharmaplist_populate(cmaplist,ocrwords);
        maxid=willuscharmaplist_maxcid(cmaplist);
        nf=(maxid>>8)&0xfff;
// nf++;
        /*
        ** Declare the fonts (all Helvetica, but w/different unicode mappings)
        */
        fprintf(pdf->f,"    /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>");
        for (ifont=1;ifont<=nf;ifont++)
            fprintf(pdf->f,"\n             /F%d << /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding /ToUnicode %d 0 R >>",ifont+1,pdf->n+ifont);
        fprintf(pdf->f," >>\n");
        }
    else
        nf=0;
    if (showbitmap)
        fprintf(pdf->f,"    /XObject << /Im%d %d 0 R >>\n"
                   "    /ProcSet [ /PDF /Text /ImageC ]\n",
                   pdf->imc,pdf->n+nf+2);
    fprintf(pdf->f,"    >>\n"
                   "/MediaBox [0 0 %.1f %.1f]\n"
                   "/CropBox [0 0 %.1f %.1f]\n"
                   "/Contents %d 0 R\n",
                   pw,ph,pw,ph,
                   pdf->n+nf+1); /* Contents stream */
    if (showbitmap)
        fprintf(pdf->f,"/Thumb %d 0 R\n",pdf->n+nf+3);
    fprintf(pdf->f,">>\n"
                   "endobj\n");

    /*
    ** Write the unicode mappings for each font to the PDF file
    */
    if (ocrwords!=NULL)
        {
        int i;
        for (i=0;i<nf;i++)
            pdffile_unicode_map(pdf,cmaplist,i+1);
        }

    /* Execution stream:  draw bitmap and OCR words */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<< /Length ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=ftell(pdf->f);
    fprintf(pdf->f,"         >>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=ftell(pdf->f);
    if (showbitmap)
        fprintf(pdf->f,"q\n%.1f 0 0 %.1f 0 0 cm\n/Im%d Do\nQ\n",pw,ph,pdf->imc);
    if (ocrwords!=NULL)
        {
        int use_spaces;

        if (ocr_render_flags&16)
            use_spaces=2;
        else if (ocr_render_flags&8)
            use_spaces=1;
        else
            use_spaces=0;
        ocrwords_to_pdf_stream(ocrwords,pdf->f,dpi,ph,(ocr_render_flags&2)?0:3,cmaplist,use_spaces,
                               ocr_render_flags);
        /* 2-1-14: Fix memory leak */
        willuscharmaplist_free(cmaplist);
        }
    if (ocr_render_flags&4)
        ocrwords_box(ocrwords,bmp);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=ftell(pdf->f);
    fprintf(pdf->f,"endstream\n"
                   "endobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (showbitmap)
        {
        /* Stream the bitmap */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,0);
        /* Stream the thumbnail */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,1);
        }
    }


static void pdffile_unicode_map(PDFFILE *pdf,WILLUSCHARMAPLIST *cmaplist,int nf)

    {
    int i,c;
    int *uni;
    static char *funcname="pdffile_unicode_map";

// if(nf>1)nf--;
    willus_mem_alloc_warn((void **)&uni,sizeof(int)*256,funcname,10);
    for (i=0;i<256;i++)
        uni[i]=-1;
    pdffile_new_object(pdf,0);
    for (i=c=0;i<cmaplist->n;i++)
        {
        if (((cmaplist->cmap[i].cid>>8)&0xfff)==nf)
            {
            uni[cmaplist->cmap[i].cid&0xff]=cmaplist->cmap[i].unicode;
            c++;
            }
        }
    if (c>0)
        {
        int ptr1,ptr2,ptrlen;

        fprintf(pdf->f,"<< /Length ");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptrlen=ftell(pdf->f);
        fprintf(pdf->f,"         >>\n"
                       "stream\n");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr1=ftell(pdf->f);
        fprintf(pdf->f,"/CIDInit /ProcSet findresource begin\n"
                       "12 dict begin\n"
                       "begincmap\n"
                       "/CIDSystemInfo\n"
                       "<< /Registry (UC%03d)\n"
                       "/Ordering (T42UV)\n"
                       "/Supplement 0\n"
                       ">> def\n"
                       "/CMapName /UC%03d def\n"
                       "/CMapType 2 def\n"
                       "1 begincodespacerange\n"
                       "<00> <FF>\n"
                       "endcodespacerange\n"
                       "%d beginbfchar\n",nf,nf,c);
        for (i=0;i<256;i++)
            if (uni[i]>=0)
                fprintf(pdf->f,"<%02x> <%04x>\n",i,uni[i]);
        fprintf(pdf->f,"endbfchar\n"
                       "endcmap\n"
                       "CMapName currentdict /CMap defineresource pop\n"
                       "end\n"
                       "end\n"
                       "endstream\n");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr2=ftell(pdf->f);
        fprintf(pdf->f,"endstream\n"
                       "endobj\n");
        insert_length(pdf->f,ptrlen,ptr2-ptr1);
        }
    else
        fprintf(pdf->f,"endobj\n");
    willus_mem_free((double **)&uni,funcname);
    }


static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp)

    {
    if (bmp->width > bmp->height)
        {
        thumb->width = bmp->width<106 ? bmp->width : 106;
        thumb->height = (int)(((double)bmp->height/bmp->width)*thumb->width+.5);
        if (thumb->height<1)
            thumb->height=1;
        }
    else
        {
        thumb->height = bmp->height<106 ? bmp->height : 106;
        thumb->width = (int)(((double)bmp->width/bmp->height)*thumb->height+.5);
        if (thumb->width<1)
            thumb->width=1;
        }
    bmp_resample(thumb,bmp,0.,0.,(double)bmp->width,(double)bmp->height,
                 thumb->width,thumb->height);
    if (bmp->bpp==8)
        bmp_convert_to_greyscale(thumb);
    }


static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *src,int quality,int halfsize,int thumb)

    {
    int ptrlen,ptr1,ptr2,bpc;
    WILLUSBITMAP *bmp,_bmp;

    if (thumb)
        {
        bmp=&_bmp;
        bmp_init(bmp);
        thumbnail_create(bmp,src);
        }
    else
        bmp=src;
    if (quality<0 && halfsize>0 && halfsize<4)
        bpc=8>>halfsize;
    else
        bpc=8;
    /* The bitmap */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<<\n");
    if (!thumb)
        fprintf(pdf->f,"/Type /XObject\n"
                       "/Subtype /Image\n");
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        fprintf(pdf->f,"/Filter %s/DCTDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
#if (defined(HAVE_JPEG_LIB) && defined(HAVE_Z_LIB))
    else
#endif
#ifdef HAVE_Z_LIB
        fprintf(pdf->f,"/Filter %s/FlateDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
    fprintf(pdf->f,"/Width %d\n"
                   "/Height %d\n"
                   "/ColorSpace /Device%s\n"
                   "/BitsPerComponent %d\n"
                   "/Length ",
                   bmp->width,bmp->height,
                   bmp->bpp==8?"Gray":"RGB",
                   bpc);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=(int)ftell(pdf->f);
    fprintf(pdf->f,"         \n"
                   ">>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=(int)ftell(pdf->f);
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        {
        bmp_write_jpeg_stream(bmp,pdf->f,quality,NULL);
        fprintf(pdf->f,"\n");
        }
    else
#endif
        {
        compress_handle h;
        h=compress_start(pdf->f,7); /* compression level = 7 */
        bmp_flate_decode(bmp,pdf->f,h,halfsize);
        compress_done(pdf->f,&h);
        fprintf(pdf->f,"\n");
        }
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=(int)ftell(pdf->f)-1;
    fprintf(pdf->f,"endstream\nendobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (thumb)
        bmp_free(bmp);
    }



/*
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
**
** To do:  Check for errors when writing
*/
static void bmp_flate_decode(WILLUSBITMAP *bmp,FILE *f,compress_handle handle,int halfsize)

    {
    int row;
    static char *funcname="bmp_flate_decode";

    if (halfsize==1)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+1)/2;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=2)
                data[i]=(p[0] & 0xf0) | (p[1] >> 4);
            if (nb&1)
                data[i]=p[0]&0xf0;
            else
                data[i]=(p[0]&0xf0) | (p[1] >> 4);
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==2)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+3)/4;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=4)
                data[i]=(p[0] & 0xc0) | ((p[1] >> 2)&0x30) | ((p[2]>>4)&0xc) | (p[3]>>6);
            data[i]=0;
            j=(nb&3);
            if (j==0)
                j=4;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0xc0)>>(k*2));
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==3)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+7)/8;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=8)
                data[i]=(p[0] & 0x80) | ((p[1]&0x80) >> 1)
                                      | ((p[2]&0x80) >> 2)
                                      | ((p[3]&0x80) >> 3)
                                      | ((p[4]&0x80) >> 4)
                                      | ((p[5]&0x80) >> 5)
                                      | ((p[6]&0x80) >> 6)
                                      | ((p[7]&0x80) >> 7);
            data[i]=0;
            j=(nb&7);
            if (j==0)
                j=8;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0x80)>>k);
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else
        {
        int nb;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            compress_write(f,handle,p,nb);
            }
        }
    }


void pdffile_finish(PDFFILE *pdf,char *title,char *author,char *producer,char *cdate)

    {
    int icat,i,pagecount;
    time_t now;
    struct tm today;
    size_t ptr;
    char nbuf[10];
    char buf[128];
    char mdate[128];
    char basename[256];

    time(&now);
    today=(*localtime(&now));

    /* Insert outline reference if available */
    for (i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&4)
            break;
    if (i<pdf->n)
        {
        fflush(pdf->f);
        fseek(pdf->f,pdf->object[0].ptr[2],0);
        sprintf(nbuf,"%6d",i+1);
        fwrite(nbuf,1,6,pdf->f);
        }
    else
        {
        fflush(pdf->f);
        fseek(pdf->f,pdf->object[0].ptr[2]-10,0);
        strcpy(nbuf,"%% ");
        fwrite(nbuf,1,3,pdf->f);
        }
        
    ptr=0; /* Avoid compiler warning */
    fseek(pdf->f,0L,2);
    if (pdf->pae==0)
        {
        pdffile_new_object(pdf,0);
        icat=pdf->n;
        fprintf(pdf->f,"<<\n"
                   "/Type /Pages\n"
                   "/Kids [");
        }
    else
        {
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr=ftell(pdf->f);
        icat=pdf->n;
        fseek(pdf->f,pdf->pae,0);
        }
    for (pagecount=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            {
            pagecount++;
            if (pagecount>MAXPDFPAGES && pdf->pae>0)
                {
                printf("WILLUS lib %s:  PDF page counts > %d not supported!\n",
                       willuslibversion(),MAXPDFPAGES);
                exit(10);
                }
            fprintf(pdf->f," %d 0 R",i+1);
            }
    fprintf(pdf->f," ]\n"
                   "/Count %d\n"
                   ">>\n"
                   "endobj\n",pagecount);
    if (pdf->pae > 0)
        {
        fseek(pdf->f,ptr,0);
        }

    pdffile_new_object(pdf,0);
    if (producer==NULL)
        sprintf(buf,"WILLUS lib %s",willuslibversion());
    else
        buf[0]='\0';
    for (i=0;buf[i]!='\0';i++)
        if (buf[i]=='(' || buf[i]==')')
            buf[i]=' ';
    sprintf(mdate,"D:%04d%02d%02d%02d%02d%02d%s",
                   today.tm_year+1900,today.tm_mon+1,today.tm_mday,
                   today.tm_hour,today.tm_min,today.tm_sec,
                   wsys_utc_string());
    fprintf(pdf->f,"<<\n");
    if (author!=NULL && author[0]!='\0')
        fprintf(pdf->f,"/Author (%s)\n",author);
    if (title==NULL || title[0]=='\0')
        wfile_basespec(basename,pdf->filename);
    fprintf(pdf->f,"/Title (%s)\n"
                   "/CreationDate (%s)\n"
                   "/ModDate (%s)\n"
                   "/Producer (%s)\n"
                   ">>\n"
                   "endobj\n",
                   title!=NULL && title[0]!='\0' ? title : basename,
                   cdate!=NULL && cdate[0]!='\0' ? cdate : mdate,
                   mdate,
                   producer==NULL ? buf : producer);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr=ftell(pdf->f);
    /* Kindles require the space after the 'f' and 'n' in the lines below. */
    fprintf(pdf->f,"xref\n"
                   "0 %d\n"
                   "0000000000 65535 f \n",pdf->n+1);
    for (i=0;i<pdf->n;i++)
        fprintf(pdf->f,"%010d 00000 n \n",(int)pdf->object[i].ptr[0]);
    fprintf(pdf->f,"trailer\n"
                   "<<\n"
                   "/Size %d\n"
                   "/Info %d 0 R\n"
                   "/Root 1 0 R\n"
                   ">>\n"
                   "startxref\n"
                   "%d\n"
                   "%%%%EOF\n",pdf->n+1,pdf->n,(int)ptr);
    /*
    ** Go back and put in catalog block references
    */
    if (pdf->pae==0)
        {
        sprintf(nbuf,"%6d",icat);
        for (i=0;i<pdf->n;i++)
            if (pdf->object[i].flags&2)
                {
                fseek(pdf->f,pdf->object[i].ptr[1],0);
                fwrite(nbuf,1,6,pdf->f);
                }
        }
    fclose(pdf->f);
    pdf->f=wfile_fopen_utf8(pdf->filename,"ab");
    }


static void pdffile_new_object(PDFFILE *pdf,int flags)

    {
    PDFOBJECT obj;

    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    obj.ptr[0]=obj.ptr[1]=ftell(pdf->f);
    obj.flags=flags;
    pdffile_add_object(pdf,&obj);
    fprintf(pdf->f,"%d 0 obj\n",pdf->n);
    }


static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object)

    {
    static char *funcname="pdffile_add_object";

    if (pdf->n>=pdf->na)
        {
        int newsize;
        newsize = pdf->na < 512 ? 1024 : pdf->na*2;
        if (pdf->na==0)
            willus_mem_alloc_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),funcname,10);
        else
            willus_mem_realloc_robust_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),
                                        pdf->na*sizeof(PDFOBJECT),funcname,10);
        pdf->na=newsize;
        }
    pdf->object[pdf->n++]=(*object);
    }


#ifdef HAVE_Z_LIB
int pdf_numpages(char *filename)

    {
    FILE *f;
    int np;

    f=wfile_fopen_utf8(filename,"rb");
    if (f==NULL)
        return(-1);
    np=pdf_numpages_1((void *)f,0);
    fclose(f);
    return(np);
    }


static int pdf_numpages_1(void *ptr,int bufsize)

    {
    char buf[256];
    FILE *f;
    char *opbuf;
    int i,i0,status,np,gls;
    static char *kwords[]={"/Type","/Pages","/Kids","/Count",
                           "/Filter","/FlateDecode","/Length",
                           "/ObjStm","stream",""};

    f=NULL; /* Avoid compiler warning */
    opbuf=NULL; /* Avoid compiler warning */
    if (bufsize==0)
        f=(FILE *)ptr;
    else
        opbuf=(char *)ptr;
    status=0;
    i0=0;
    np=-1;
    while (1)
        {
        if (bufsize==0)
            gls=wpdf_getline(buf,254,f);
        else
            gls=wpdf_getbufline(buf,254,opbuf,&i0,bufsize);
        for (i=0;kwords[i][0]!='\0';i++)
            {
            int ip;

            ip=in_string(buf,kwords[i]);
            if (ip>=0)
                {
                status |= (1<<i);
                if (i==3 || i==6)
                    np=atoi(&buf[ip+strlen(kwords[i])]);
/*
printf("    '%s' %x np=%d\n",kwords[i],status,np);
*/
                if (status==15 && np>0)
                    break;
                if (bufsize==0 && (status&0x1f1)==0x1f1 && np>0)
                    {
                    np=decodecheck(f,np);
                    if (np>0)
                        {
                        status=15;
                        break;
                        }
                    }
                }
            }
        if (status==15 && np>0)
            break;
        if (in_string(buf,"endobj")>=0)
            {
            status=0;
            np=-1;
            }
        if (!gls)
            break;
        }
    if (np>0)
        return(np);
    return(-2);
    }


static int decodecheck(FILE *f,int np)

    {
    char *inbuf,*outbuf;
    z_stream zstrm;
    int i0,status,obsize,extra;
    static char *funcname="decodecheck";

// printf("@decodecheck(np=%d)\n",np);    
    extra=4;
    willus_mem_alloc_warn((void **)&inbuf,np+extra,funcname,10);
    obsize=np*10;
    if (obsize<1024)
        obsize=1024;
    willus_mem_alloc_warn((void **)&outbuf,obsize,funcname,10);
    fread(inbuf,1,np+extra,f);
    i0=0;
    if (inbuf[i0]=='\n' || inbuf[i0]=='\r')
        i0++;
    memset(&zstrm,0,sizeof(zstrm));
    zstrm.avail_in=np+extra-i0;
    zstrm.avail_out=obsize;
    zstrm.next_in=(Bytef*)&inbuf[i0];
    zstrm.next_out=(Bytef*)outbuf;
    status=inflateInit(&zstrm);
    if (status!=Z_OK)
        {
        willus_mem_free((double **)&outbuf,funcname);
        willus_mem_free((double **)&inbuf,funcname);
        return(0);
        }
    status=inflate(&zstrm,Z_FINISH);
/*
printf("    Total output bytes = %d, status = %d\n",(int)zstrm.total_out,status);
printf("    ");
fwrite(outbuf,1,zstrm.total_out>2048 ? 2048:zstrm.total_out,stdout);
*/
    if (zstrm.total_out>0)
        np=pdf_numpages_1(outbuf,(int)zstrm.total_out);
    else
        np=0;
    willus_mem_free((double **)&outbuf,funcname);
    willus_mem_free((double **)&inbuf,funcname);
    return(np);
    }


static int wpdf_getline(char *buf,int maxlen,FILE *f)

    {
    int i,c;

    i=0;
    while ((c=fgetc(f))!=EOF)
        {
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return(c!=EOF);
    }


static int wpdf_getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize)

    {
    int i,c;

    i=0;
    while ((*i0) < bufsize)
        {
        c=opbuf[(*i0)];
        (*i0)=(*i0)+1;
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return((*i0)<bufsize);
    }
#endif /* HAVE_Z_LIB */


static void insert_length(FILE *f,long pos,int len)

    {
    long ptr;
    int i;
    char nbuf[64];

    fflush(f);
    fseek(f,0L,1);
    ptr=ftell(f);
    fseek(f,pos,0);
    sprintf(nbuf,"%d",len);
    for (i=0;i<8 && nbuf[i]!='\0';i++)
        fputc(nbuf[i],f);
    fseek(f,ptr,0);
    }


void ocrwords_box(OCRWORDS *ocrwords,WILLUSBITMAP *bmp)

    {
    int i,bpp;

    if (ocrwords==NULL)
        return;
    bpp=bmp->bpp==24 ? 3 : 1;
    for (i=0;i<ocrwords->n;i++)
        {
        int j;
        unsigned char *p;
        OCRWORD *word;
        word=&ocrwords->word[i];
        p=bmp_rowptr_from_top(bmp,word->r)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        p=bmp_rowptr_from_top(bmp,word->r-word->maxheight)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        for (j=0;j<word->maxheight;j++)
            {
            p=bmp_rowptr_from_top(bmp,word->r-j)+word->c*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            p=bmp_rowptr_from_top(bmp,word->r-j)+(word->c+word->w-1)*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        }
    }


static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode,
                                   WILLUSCHARMAPLIST *cmaplist,int use_spaces,int ocr_flags)

    {
    int i;
    double median_size;

    fprintf(f,"BT\n%d Tr\n",text_render_mode);
    median_size=ocrwords_median_size(ocrwords,dpi,cmaplist);
    if (use_spaces)
        {
        int i1;

        for (i1=i=0;i<ocrwords->n;i++)
            {
            if (i==ocrwords->n-1 || ocrwords->word[i+1].r!=ocrwords->word[i].r)
                {
                OCRWORD word;
                ocrword_init(&word);
                ocrwords_optimize_spaces(&word,&ocrwords->word[i1],i-i1+1,dpi,cmaplist,
                                         use_spaces==2 ? 1 : 0);
                ocrword_to_pdf_stream(&word,f,dpi,page_height_pts,median_size,cmaplist,ocr_flags);
                ocrword_free(&word);
                i1=i+1;
                }
            }
        }
    else
        for (i=0;i<ocrwords->n;i++)
            ocrword_to_pdf_stream(&ocrwords->word[i],f,dpi,page_height_pts,median_size,cmaplist,
                                  ocr_flags);
    fprintf(f,"ET\n");
    }


static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi,WILLUSCHARMAPLIST *cmaplist)

    {
    static char *funcname="ocrwords_to_histogram";
    static double *fontsize_hist;
    double msize;
    int i;

    if (ocrwords->n<=0)
        return(1.);
    willus_mem_alloc_warn((void **)&fontsize_hist,sizeof(double)*ocrwords->n,funcname,10);
    for (i=0;i<ocrwords->n;i++)
        {
        double w,h;
        ocrword_width_and_maxheight(&ocrwords->word[i],&w,&h,cmaplist,NULL);
        fontsize_hist[i] = (72.*ocrwords->word[i].maxheight/dpi) / h;
        }
    sortd(fontsize_hist,ocrwords->n);
    msize=fontsize_hist[ocrwords->n/2];
    if (msize < 0.5)
        msize = 0.5;
    willus_mem_free(&fontsize_hist,funcname);
    return(msize);
    }


/*
** If charpos[] is not NULL, it gets the position of the right side of each character.
** Must be dimensioned from [0..n-1] where n is the number of chars in the word.
**
** Returned values are in points for a 1-point-sized font (points per point).
**
*/
static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight,
                                        WILLUSCHARMAPLIST *cmaplist,double *charpos)

    {
    int i,n;
    int *d;
    static char *funcname="ocrword_width_and_maxheight";

    n=strlen(word->text)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,word->text,n-1);
    (*width)=0.;
    (*maxheight)=0.;
    for (i=0;i<n;i++)
        {
        int c,cid,index;

        if (d[i]<256)
            cid=d[i];
        else
            {
            index=willuscharmaplist_cid_index(cmaplist,d[i]);
            if (index<0 || index>=cmaplist->n || cmaplist->cmap[index].unicode!=d[i])
                cid=32;
            else
                cid=cmaplist->cmap[index].cid&0xff;
            }
        c=cid-32;
        if (c<0 || c>=224)
            c=0;
        if (word->text[i+1]=='\0')
            (*width) += Helvetica[c].width;
        else
            (*width) += Helvetica[c].nextchar;
        if (charpos!=NULL)
            charpos[i]=(*width);
        if (Helvetica[c].abovebase > (*maxheight))
            (*maxheight)=Helvetica[c].abovebase;
        }
    /* Limit checks -- 7-21-2013 */
    if ((*width) < .01)
        (*width) = .01;
    if ((*maxheight) < .01)
        (*maxheight) = .01;
    willus_mem_free((double **)&d,funcname);
    }


static double size_round_off(double size,double median_size,double log_size_increment)

    {
    double rat,lograt;

    if (size < .5)
        size = .5;
    rat=size / median_size;
    /* limit to prevent overflow / infinity -- 7-21-2013  */
    if (rat < 1e-3)
        rat = 1e-3;
    if (rat > 1e5)
        rat = 1e5;
    lograt = floor(log10(rat)/log_size_increment+.5);
    return(median_size*pow(10.,lograt*log_size_increment));
    }


/*
** In "sentence", put all of the words in "words" separated by an optimum
** number of spaces between each word so that the words best line up with
** where their bitmaps are on the page.
*/
static void ocrwords_optimize_spaces(OCRWORD *sentence,OCRWORD *word,int n,double dpi,
                                     WILLUSCHARMAPLIST *cmaplist,int optimize)
                              

    {
    int i,pixwidth;
    int *nspaces;
    double *pos; /* Left position of each word rel to sentence length */
    static char *funcname="ocrwords_optimize_spaces";

    pixwidth=word[n-1].c+word[n-1].w-word[0].c;
/*
printf("Text row: ");
for (i=0;i<n;i++)
printf(" %s (%5.3f)",word[i].text,(double)word[i].w/pixwidth);
printf("\n");
printf("pixwidth=%d\n",pixwidth);
*/
    willus_mem_alloc_warn((void **)&pos,sizeof(double)*2*n,funcname,10);
    willus_mem_alloc_warn((void **)&nspaces,sizeof(int)*n,funcname,10);
    for (i=0;i<n-1;i++)
        nspaces[i]=1;
    nspaces[n-1]=0;
    if (optimize)
        {
        /* First find number of spaces on end that makes all pos[] values negative */
        for (i=0;i<n;i++)
            nspaces[i]=1;
        for (i=0;i<2000;i++)
            {
            int j;

            nspaces[n-1]=i;
            sentence_check_alignment(word,n,nspaces,pos,cmaplist);
/*
{
int k;
printf("  i=%3d ",i);
for (k=0;k<n;k++)
printf(" %5.3f",pos[k*2]-(k==0?0.:pos[k*2-1]));
printf("\n");
}
*/
            /* Are any words still too long? */
            for (j=0;j<n;j++)
                if (pos[j*2]-(j==0 ? 0. : pos[j*2-1]) > (double)word[j].w/pixwidth)
                    break;
            if (j>=n)
                break;
            }
        for (i=0;i<n-1;i++)
            {
            int j,ns1;
            double err;
            double wordpos;

            ns1=nspaces[n-1];
            wordpos=(double)(word[i+1].c-word[0].c)/pixwidth;
            err=0.;
            for (j=1;j<ns1;j++)
                {
                nspaces[i]=j;
                nspaces[n-1]=ns1+1-j;
                sentence_check_alignment(word,n,nspaces,pos,cmaplist);
                if (pos[2*i+1] > wordpos)
                    {
                    if (j>1 && pos[2*i+1]-wordpos > err)
                        {
                        nspaces[i]=j-1;
                        nspaces[n-1]=ns1+1-(j-1);
                        sentence_check_alignment(word,n,nspaces,pos,cmaplist);
                        }
                    break;
                    }
                err=wordpos-pos[2*i+1];
                }
            }
/*
{
int k;
printf("    Pass %d: ",i);
for (k=0;k<n-1;k++)
printf(" %d (%5.3f, %5.3f)",nspaces[k],pos[2*k],pos[2*k+1]);
printf("\n");
}
*/
        }
    ocrwords_sentence_construct(sentence,word,n,nspaces);
    willus_mem_free((double **)&nspaces,funcname);
    willus_mem_free(&pos,funcname);
    }


/*
** Construct single OCRWORD "sentence" from array of OCRWORDs and nspaces[] array
*/
static void ocrwords_sentence_construct(OCRWORD *sentence,OCRWORD *word,int n,int *nspaces)

    {
    int i,len;
    static char *funcname="ocrwords_sentence_construct";

    for (len=i=0;i<n;i++)
        len += strlen(word[i].text)+nspaces[i];
    ocrword_free(sentence);
    willus_mem_alloc_warn((void **)&sentence->text,len+1,funcname,10);
    sentence->text[0]='\0';
    sentence->r=word[0].r;
    sentence->c=word[0].c;
    sentence->w=word[n-1].c+word[n-1].w-word[0].c;
    sentence->maxheight=0;
    sentence->rot=word[0].rot;
    for (i=0;i<n;i++)
        {
        int j;

        strcat(sentence->text,word[i].text);
        if (word[i].maxheight > sentence->maxheight)
            sentence->maxheight = word[i].maxheight;
        len=strlen(sentence->text);
        for (j=0;j<nspaces[i];j++)
            sentence->text[len+j]=' ';
        sentence->text[len+j]='\0';
        }
    }


/*
** Inputs:  word[0..n-1] = words in sentence / row of text.
**          nspaces[0..n-1] = number of spaces after word[0..n-1]
**
** Output:  pos[0..2*n-1] = positional error of right side of word[i/2] or space[i/2].
**              pos[0] = right side of first word
**              pos[1] = right side of space after first word (left side of next word)
**              pos[2] = right side of 2nd word
**              pos[3] = right side of space after second word
**              ...
**          positive means the OCR layer word starts to the right of the bitmap word.
**
*/
static void sentence_check_alignment(OCRWORD *word,int n,int *nspaces,double *pos,
                                     WILLUSCHARMAPLIST *cmaplist)

    {
    int i,index,len;
    double *charpos;
    static char *funcname="sentence_check_alignment";
    double ptwidth,ptheight;
    OCRWORD _sentence,*sentence;

    sentence=&_sentence;
    ocrword_init(sentence);
    ocrwords_sentence_construct(sentence,word,n,nspaces);
    len=strlen(sentence->text);
    willus_mem_alloc_warn((void **)&charpos,sizeof(double)*(len+2),funcname,10);
    ocrword_width_and_maxheight(sentence,&ptwidth,&ptheight,cmaplist,charpos);
/*
{
int wordlen;
printf("S:");
wordlen=utf8_to_unicode(NULL,sentence->text,len+2);
for (i=0;i<wordlen;i++)
printf(" %.3f",charpos[i]/ptwidth);
printf("\n");
}
*/
    ocrword_free(sentence);
    for (i=index=0;i<n;i++)
        {
        int wordlen;
        double xpt;

        wordlen=utf8_to_unicode(NULL,word[i].text,len+2);
        xpt=charpos[index+wordlen-1];
        pos[2*i]=xpt/ptwidth;
        if (i<n-1)
            {
            xpt=charpos[index+wordlen+nspaces[i]-1];
            pos[2*i+1]=xpt/ptwidth;
            }
        index += wordlen+nspaces[i];
        }
    willus_mem_free((double **)&charpos,funcname);
    }


static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts,
                                  WILLUSCHARMAPLIST *cmaplist,int ocr_flags)

    {
    int cc,i,n,wordw;
    double fontsize_width,fontsize_height,ybase,x0,y0;
    double width_per_point,height_per_point,arat;
    char rotbuf[48];
    int *d;
    static char *funcname="ocrword_to_pdf_stream";

/*
printf("word->text='%s'\n",word->text);
printf("    wxh = %dx%d\n",word->w,word->h);
*/
    n=strlen(word->text)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,word->text,n-1);
    ocrword_width_and_maxheight(word,&width_per_point,&height_per_point,cmaplist,NULL);
/*
printf("    word->maxheight=%g\n",word->maxheight);
*/
    if (word->w/10. < word->lcheight)
        wordw = 0.9*word->w;
    else
        wordw = word->w-word->lcheight;
    fontsize_width = 72.*wordw/dpi / width_per_point;
    fontsize_height = (72.*word->maxheight/dpi) / height_per_point;
    if (!(ocr_flags & 0x20))
        fontsize_height = size_round_off(fontsize_height,median_size_pts,.25);
/*
printf("    fontsize_height=%g\n",fontsize_height);
*/
    /*
    ** Keeping the height small generally improves the ability to graphically
    ** select the text in PDF readers, so multiply by 0.5.
    */
    fontsize_height = 0.5*fontsize_height;
    arat = fontsize_width / fontsize_height;
    ybase = page_height_pts - 72.*word->r/dpi;
    if (word->rot==0)
        sprintf(rotbuf,"%.4f 0 0 1",arat);
    else if (word->rot==90)
        sprintf(rotbuf,"0 %.4f -1 0",arat);
    else
        {
        double theta,sinth,costh;

        theta=word->rot*PI/180.;
        sinth=sin(theta);
        costh=cos(theta);
        sprintf(rotbuf,"%.3f %.3f %.3f %.3f",costh*arat,sinth*arat,-sinth,costh);
        }
    cc=0;
    x0=72.*word->c/dpi;
    y0=ybase;
    /*
    ** Go through word letter by letter and select correct font for each
    ** letter so that unicode copy / paste works.
    */
    for (i=0;i<n;i++)
        {
        int cid,index,fn;

        if (d[i]<256)
            {
            fn=1;
            cid=d[i];
            }
        else
            {
            index=willuscharmaplist_cid_index(cmaplist,d[i]);
            if (index<0 || index>=cmaplist->n || cmaplist->cmap[index].unicode!=d[i])
                {
                cid=32;
                fn=1;
                }
            else
                {
                cid=cmaplist->cmap[index].cid&0xff;
                fn=1+((cmaplist->cmap[index].cid>>8)&0xfff);
                }
            }
        if (cid<32 || cid>255)
            cid=32;
        if (fn!=lastfont || fabs(fontsize_height-lastfontsize)>.01)
            {
            if (cc>0)
                {
                fprintf(f,"> Tj\n");
                cc=0;
                }
            fprintf(f,"/F%d %.2f Tf\n",fn,fontsize_height);
            lastfontsize=fontsize_height;
            lastfont=fn;
            }
        if (i==0)
            fprintf(f,"%s %.2f %.2f Tm\n",rotbuf,x0,y0);
        fprintf(f,"%s%02X",cc==0?"<":"",cid);
        cc++;
        x0 += fontsize_height*arat*Helvetica[cid-32].nextchar;
        }
    /* 2-1-14: Memory leak fixed */
    willus_mem_free((double **)&d,funcname);
    if (cc>0)
        fprintf(f,"> Tj\n");
    }


static void willuscharmaplist_init(WILLUSCHARMAPLIST *list)

    {
    list->n=list->na=0;
    list->cmap=NULL;
    }


static void willuscharmaplist_free(WILLUSCHARMAPLIST *list)

    {
    willus_mem_free((double **)&list->cmap,"willuscharmaplist_free");
    list->n=list->na=0;
    }


static void willuscharmaplist_add_charmap(WILLUSCHARMAPLIST *list,int unichar)

    {
    static char *funcname="willuscharmaplist_add_charmap";
    int i,cid;

    i=willuscharmaplist_cid_index(list,unichar);
    if (i>=0 && i<list->n && list->cmap[i].unicode==unichar)
        return;
    if (list->n >= list->na)
        {
        int newsize;
        newsize = list->na < 512 ? 1024 : list->na*2;
        willus_mem_realloc_robust_warn((void **)&list->cmap,sizeof(WILLUSCHARMAP)*newsize,
                                   sizeof(WILLUSCHARMAP)*list->na,funcname,10);
        list->na=newsize;
        }
    if (i<list->n)
        memmove(&list->cmap[i+1],&list->cmap[i],(list->n-i)*sizeof(WILLUSCHARMAP));
    cid=willuscharmaplist_maxcid(list)+1;
    if (cid<=0x120)
        cid=0x121;
    /* Issue with Adobe copy/paste--only do one dummy char in first font then skip */
    /* to next font. */
    if (list->n==1)
        cid=0x221;
    while (1)
        {
        if ((cid&0xff)<0x21)
            cid=(cid&0xfff00)|0x21;
/*
if ((cid&0xff)<0x41)
cid=(cid&0xfff00)|0x41;
if ((cid&0xff)>0x57)
cid=((cid+0x100)&0xfff00)|0x41;
*/
        /* Choose a letter that isn't too thin or too far above/below the baseline */
        if (Helvetica[(cid&0xff)-32].abovebase < .47
             || Helvetica[(cid&0xff)-32].abovebase > 1.0
             || Helvetica[(cid&0xff)-32].belowbase < -.001
             || Helvetica[(cid&0xff)-32].belowbase > 0.2
             || Helvetica[(cid&0xff)-32].width < 0.4)
            {
            cid++;
            continue;
            }
        break;
        }
    list->cmap[i].cid = cid;
    list->cmap[i].unicode=unichar;
    list->n++;
    }


static int willuscharmaplist_maxcid(WILLUSCHARMAPLIST *list)

    {
    int i,max;

    for (i=max=0;i<list->n;i++)
        if (list->cmap[i].cid > max)
            max=list->cmap[i].cid;
    return(max);
    }


/*
** Must be sorted by unicode value!
*/
static int willuscharmaplist_cid_index(WILLUSCHARMAPLIST *list,int unichar)

    {
    int i1,i2;

    if (list->n<=0)
        return(0);
    if (unichar <= list->cmap[0].unicode)
        return(0);
    if (unichar > list->cmap[list->n-1].unicode)
        return(list->n);
    if (unichar == list->cmap[list->n-1].unicode)
        return(list->n-1);
    i1=0;
    i2=list->n-1;
    while (i2-i1>1)
        {
        int i;
        i=(i1+i2)/2;
        if (unichar==list->cmap[i].unicode)
            return(i);
        if (unichar>list->cmap[i].unicode)
            i1=i;
        else
            i2=i;
        }
    return(i2);
    }


/*
** Map all unicode characters on the page to character ID's (cids).
** Use multiple fonts (multiple copies of Helvetica with different
** unicode mappings) if necessary.
*/
static void willuscharmaplist_populate(WILLUSCHARMAPLIST *cmaplist,OCRWORDS *ocrwords)

    {
    int i;

    /* Add one dummy char--issue with Adobe */
    willuscharmaplist_add_charmap(cmaplist,0xffff);
    for (i=0;i<ocrwords->n;i++)
        willuscharmaplist_populate_string(cmaplist,ocrwords->word[i].text);
    }


static void willuscharmaplist_populate_string(WILLUSCHARMAPLIST *cmaplist,char *s)

    {
    int *d;
    int i,n;
    static char *funcname="willuscharmaplist_populate_string";

    n=strlen(s)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,s,n-1);
    for (i=0;i<n;i++)
        if (d[i]>=256)
            willuscharmaplist_add_charmap(cmaplist,d[i]);
    willus_mem_free((double **)&d,funcname);
    }


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


static int wpdfoutline_num_anchors_on_level(WPDFOUTLINE *outline,int *rc)

    {
    int i,na;

    for (na=i=0;outline!=NULL;i++,outline=outline->next)
        if (outline->next!=NULL)
            na += wpdfoutline_num_anchors_recursive(outline->down)+1;
    if (rc!=NULL)
        (*rc)=na;
    return(i);
    }


static int wpdfoutline_num_anchors_recursive(WPDFOUTLINE *outline)

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


static int wpdfoutline_index(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    int na;

    if (outline==NULL || outline==outline1)
        return(0);
    if (wpdfoutline_within(outline->down,outline1))
        return(1+wpdfoutline_index(outline->down,outline1));
    na=wpdfoutline_num_anchors_recursive(outline->down);
    return(1+na+wpdfoutline_index(outline->next,outline1));
    }


static WPDFOUTLINE *wpdfoutline_by_index(WPDFOUTLINE *outline,int n)

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


static WPDFOUTLINE *wpdfoutline_previous(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

    {
    if (outline==NULL)
        return(NULL);
    if (outline->next==outline1)
        return(outline);
    if (wpdfoutline_within(outline->down,outline1))
        return(wpdfoutline_previous(outline->down,outline1));
    return(wpdfoutline_previous(outline->next,outline1));
    }


static WPDFOUTLINE *wpdfoutline_parent(WPDFOUTLINE *outline,WPDFOUTLINE *outline1)

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


int wpdf_docenc_from_utf8(char *dst,char *src_utf8,int maxlen)

    {
    static char *funcname="wpdf_docenc_from_utf8";
	int i,j,n,legal;
    int *utf16;

    legal=1;
    n=strlen(src_utf8)+2;
    willus_mem_alloc_warn((void **)&utf16,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(utf16,src_utf8,n-1);
	for (i=j=0;i<n;i++)
        {
        int c;
        if ((c=legal_pdf_encoded_byte(utf16[i]))!=0)
            {
            if (dst!=NULL && j<maxlen)
                dst[j++] = c;
            }
        else
            legal=0;
        }
    if (dst!=NULL)
        dst[j]='\0';
    willus_mem_free((double **)&utf16,funcname);
    return(legal);
    }


static int legal_pdf_encoded_byte(int c)

    {
    int k;
    const unsigned short wpdfdoc_encoding_table[256] =
        {
        /* 0x0 to 0x17 except \t, \n and \r are really undefined */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x02d8, 0x02c7, 0x02c6, 0x02d9, 0x02dd, 0x02db, 0x02da, 0x02dc,
        0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
        0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
        0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
        0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
        0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
        0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
        0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
        0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
        0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
        0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
        0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
        0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000,
        0x2022, 0x2020, 0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044,
        0x2039, 0x203a, 0x2212, 0x2030, 0x201e, 0x201c, 0x201d, 0x2018,
        0x2019, 0x201a, 0x2122, 0xfb01, 0xfb02, 0x0141, 0x0152, 0x0160,
        0x0178, 0x017d, 0x0131, 0x0142, 0x0153, 0x0161, 0x017e, 0x0000,
        0x20ac, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
        0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x0000, 0x00ae, 0x00af,
        0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
        0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
        0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
        0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
        0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
        0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
        0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
        0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
        0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
        0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
        };

    for (k=1;k<256;k++)
        if (c==wpdfdoc_encoding_table[k])
            return(k);
    return(0);
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
