/*
** wininet.c      Windows specific calls (they do nothing in other platforms)
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2020  http://willus.com
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
#include <wininet.h>

#endif

/* #include <process.h> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifdef HAVE_WIN32_API

static int  wininet_timeout_ms=-1;
static int  wininet_retries=-1;
static void wininet_set_timeouts_internal(wfile *wf);



static int wininet_httpget_to_file_simple(char *dstname,char *lurl);
static int wininet_httpstart_simple(wfile *wf,char *host0);
static void *wininet_httpinituri_simple(wfile *wf,char *uri,int *error);

/*
** ret value of 0 indicates error.
*/
int wininet_query_option(wfile *wf,int option,void *buf,int maxlen)

    {
    return(InternetQueryOption((void *)wf->ihandle,option,buf,(void *)&maxlen));
    }


/*
** ret value of 0 indicates error.
*/
int wininet_set_option(wfile *wf,int option,void *value,int nbytes)

    {
    return(InternetSetOption((void *)wf->ihandle,option,value,nbytes));
    }


void wininet_set_timeout(double secs,int retries)

    {
    wininet_timeout_ms = 1000.*secs+0.5;
    wininet_retries = retries;
    }


void wininet_timeout_settings(wfile *wf)

    {
    static char *label[] =
        { "Connect retries","Connect timeout","Send timeout",
          "Receive timeout","Data receive timeout",
          "Data send timeout","Control receive timeout",
          "Control send timeout","" };
    static int units[8]={1,1000,1000,1000,1000,1000,1000,1000};
    static int optno[8]=
        {
        INTERNET_OPTION_CONNECT_RETRIES,
        INTERNET_OPTION_CONNECT_TIMEOUT,
        INTERNET_OPTION_SEND_TIMEOUT,
        INTERNET_OPTION_RECEIVE_TIMEOUT,
        INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
        INTERNET_OPTION_DATA_SEND_TIMEOUT,
        INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,
        INTERNET_OPTION_CONTROL_SEND_TIMEOUT
        };
    int     i,n;

    for (i=0;label[i][0]!='\0';i++)
        {
        if (wininet_query_option(wf,optno[i],&n,4))
            if (units[i]==1)
                printf("%-25s  %d\n",label[i],n);
            else
                printf("%-25s  %.2f s\n",label[i],(double)n/units[i]);
        else
            printf("Query error:  %s\n",win_lasterror());
        }
    }


static void wininet_set_timeouts_internal(wfile *wf)

    {
    static int optno[12] =
        {
        INTERNET_OPTION_CONNECT_RETRIES,
        INTERNET_OPTION_CONNECT_TIMEOUT,
        INTERNET_OPTION_SEND_TIMEOUT,
        INTERNET_OPTION_RECEIVE_TIMEOUT,
        INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
        INTERNET_OPTION_DATA_SEND_TIMEOUT,
        INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,
        INTERNET_OPTION_CONTROL_SEND_TIMEOUT,
        -1,-1,-1,-1
        };
    int     i;

    if (wininet_retries>=0)
        wininet_set_option(wf,optno[0],&wininet_retries,4);
    if (wininet_timeout_ms>=0)
        for (i=1;optno[i]!=-1;i++)
            wininet_set_option(wf,optno[i],&wininet_timeout_ms,4);
    }


static int wininet_httpget_to_file_simple(char *dstname,char *lurl)

    {
    static char url[MAXFILENAMELEN],host[MAXFILENAMELEN],filename[MAXFILENAMELEN];
    wfile wf;
    int   status,i,errcode;
    void *handle;
    static char buffer[4096];
    int    maxsize;
    int    nbytes,tot1,tot2;
    FILE  *out;

    maxsize=4096;
    if (!strnicmp(lurl,"http://",7))
        strcpy(host,&lurl[7]);
    else if (!strnicmp(lurl,"https://",8))
        strcpy(host,&lurl[8]);
    else
        strcpy(host,lurl);
    strcpy(filename,dstname);
    for (i=0;host[i]!='?' && host[i]!='/' && host[i]!='\0';i++);
    if (host[i]=='\0')
        strcpy(url,"/");
    else
        strcpy(url,&host[i]);
    host[i]='\0';
    status=wininet_httpstart_simple(&wf,host);
    if (status<0)
        return(-2);
    errcode=0;
    handle = wininet_httpinituri_simple(&wf,url,&errcode);
    if (handle==NULL)
        {
        wininet_httpend(&wf);
        return(errcode==HTTP_STATUS_DENIED ? -6: -3);
        }
    out=fopen(filename,"wb");
    if (out==NULL)
        {
        wlprintf("Cannot open file %s for writing.\n",filename);
        wininet_httpenduri(handle);
        wininet_httpend(&wf);
        return(-4);
        }
    tot1 = tot2 = 0;
    while ((nbytes=wininet_httpgetdata(handle,buffer,maxsize))>0)
        {
        tot1 += nbytes;
        tot2 += fwrite(buffer,sizeof(char),nbytes,out);
        }
    fclose(out);
    wininet_httpenduri(handle);
    wininet_httpend(&wf);
    if (tot1!=tot2)
        return(-5);
    return(0);
    }


static int wininet_httpstart_simple(wfile *wf,char *host0)

    {
    static char host[MAXFILENAMELEN];
    int i,port;

    port=INTERNET_INVALID_PORT_NUMBER;
    strncpy(host,host0,255);
    host[255]='\0';
    for (i=strlen(host)-1;i>=0;i--)
        if (host[i]==':')
            break;
    if (i>=0 && host[i]==':' && is_an_integer(&host[i+1]))
        {
        port=atoi(&host[i+1]);
        host[i]='\0';
        }
    wf->ihandle=wf->ftphandle=wf->winhandle=NULL;
    wf->ihandle=InternetOpen("whttp",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,0);
    if (wf->ihandle==NULL)
        return(-1);
    wininet_set_timeouts_internal(wf);
    wf->ftphandle=InternetConnect((HANDLE)wf->ihandle,host,port,NULL,NULL,
                                  INTERNET_SERVICE_HTTP,0,0);
    if (wf->ftphandle==NULL)
        return(-2);
    return(0);
    }


static void *wininet_httpinituri_simple(wfile *wf,char *uri,int *error)

    {
    HINTERNET   handle;
    int     tries;
    int     dwStatus;
    int     dwStatusSize = sizeof(dwStatus);

    handle=HttpOpenRequest((HINTERNET)wf->ftphandle,"GET",uri,NULL,NULL,
                            NULL,INTERNET_FLAG_KEEP_CONNECTION
                             | INTERNET_FLAG_PRAGMA_NOCACHE
                             | INTERNET_FLAG_RELOAD,0);
    if (handle==NULL)
        return(handle);
    (*error)=0;
    for (tries=0;tries<10;tries++)
        {
        int status;
        status=HttpSendRequest(handle,NULL,0,NULL,0);
        if (!status)
            {
            (*error)=GetLastError();
            break;
            }
        status=HttpQueryInfo(handle, HTTP_QUERY_FLAG_NUMBER |
                HTTP_QUERY_STATUS_CODE, &dwStatus,(void *)&dwStatusSize, NULL);
        if (!status)
            {
            (*error)=GetLastError();
            break;
            }
        if (dwStatus==HTTP_STATUS_DENIED)
            {
            if (tries>=5)
                {
                (*error)=HTTP_STATUS_DENIED;
                wininet_httpenduri(handle);
                return(NULL);
                }
            }
        if (dwStatus==HTTP_STATUS_OK)
            break;
        }
    return((void *)handle);
    }


int wininet_httpgetdata(void *handle,char *data,int maxlen)

    {
    int     index,status,bytesread;

    index=0;
    while (1)
        {
        bytesread=0;
        status=InternetReadFile((HINTERNET)handle,&data[index],maxlen-index,(void *)&bytesread);
/* printf("IRF index=%d, status=%d, bread=%d\n",index,status,bytesread); */
        if (!status)
            {
            if (index>0)
                return(index);
            return(-1);
            }
        if (bytesread>0)
            {
            if (maxlen-index==bytesread)
                return(maxlen);
            index+=bytesread;
            continue;
            }
        break;
        }
    return(index);
    }


int wininet_httpenduri(void *handle)

    {
    /* TRUE for success */
    return(InternetCloseHandle((HINTERNET)handle));
    }


void wininet_httpend(wfile *wf)

    {
    /*
    if (wf->ftphandle!=0)
        {
        InternetCloseHandle((HANDLE)wf->ftphandle);
        wf->ftphandle=0;
        }
    */
    if (wf->ihandle!=0)
        {
        InternetCloseHandle((HANDLE)wf->ihandle);
        wf->ihandle=0;
        }
    }
#endif /* HAVE_WIN32_API */

int inet_httpget(char *dstname,char *lurl)

    {
    static char tmpbuf[512];
    int status;

#ifdef HAVE_WIN32_API
    status=wininet_httpget_to_file_simple(dstname,lurl);
    if (wfile_size(dstname)<0.5)
        remove(dstname);
    /* Try wget and curl if internal method didn't work */
    if (status || wfile_status(dstname)==0)
        {
#endif /* HAVE_WIN32_API */
    sprintf(tmpbuf,"wget --no-check-certificate --tries=1 -O \"%s\" \"%s\"",dstname,lurl);
// printf("\n\n\ncommand='%s'\n\n\n",tmpbuf);
    remove(dstname);
    status=wsys_shell_command(tmpbuf,NULL,NULL);
    if (status)
        {
        sprintf(tmpbuf,"curl --connect-timeout 15 -o \"%s\" \"%s\"",dstname,lurl);
        remove(dstname);
        status=wsys_shell_command(tmpbuf,NULL,NULL);
        }
    if (status || wfile_status(dstname)!=1)
        {
        /*
        wlprintf("Could not get URL %s to local file %s (ret status=%d).\n",lurl,dstname,status);
        */
        if (!status)
            status=-1;
        }
    else
        status=0;
#ifdef HAVE_WIN32_API
        }
#endif
    return(status);
    }
