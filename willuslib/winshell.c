/*
** winshell.c    Windows specific calls related to Windows Shell
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2015  http://willus.com
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

#if (!defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#include "willus.h"

#ifdef HAVE_WIN32_API
#include <windows.h>
#include <shlobj.h>
/* #include <process.h> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>

/*
** Resolve a windows shortcut into it's full target name.
** If wide, shortcut and target are treated as wide ptrs, otherwise char.
*/
int win_resolve_shortcut(void *shortcut0,void *target0,int maxlen,int wide)

    {
    WCHAR *shortcutw,*targetw;
    char *shortcut,*target;
	IShellLinkW *psl = NULL;
    static char *funcname="win_resolve_shortcut";

	if (target0 == NULL)
		return(0);
    shortcutw=(WCHAR *)shortcut0;
    shortcut=(char *)shortcut0;
    targetw=(WCHAR *)target0;
    target=(char *)target0;
    if (wide)
    	target[0]='\0';
    else
        targetw[0]=0;
    /*
	** Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	** has already been called.
    */
	HRESULT hres = CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,
                                    &IID_IShellLinkW,(LPVOID*)&psl);
	if (SUCCEEDED(hres))
        {
		/* Get a pointer to the IPersistFile interface. */
		IPersistFile *ppf = NULL;

		hres = psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(void**)&ppf);
		if (SUCCEEDED(hres))
            {
			/* Add code here to check return value from MultiByteWideChar for success. */
			/* Load the shortcut. */
/*
** MinGW doesn't appear to define this...
#ifdef _UNICODE
			hres = ppf->lpVtbl->Load(ppf,shortcut,STGM_READ);
#else
*/
            if (!wide)
                {
    			WCHAR widestr[512];

                widestr[0]='\0';
	    		/* Ensure that the string is Unicode. */
	    		MultiByteToWideChar(CP_ACP,0,shortcut,-1, widestr,511);
	    		hres = ppf->lpVtbl->Load(ppf,widestr,STGM_READ);
                }
            else
                hres = ppf->lpVtbl->Load(ppf,shortcutw,STGM_READ);
/*
#endif
*/
			if (SUCCEEDED(hres))
                {
				/* Resolve the link. */
				hres = psl->lpVtbl->Resolve(psl,GetDesktopWindow(),0);
				if (SUCCEEDED(hres))
                    {
                    if (!wide)
                        {
                        short *buf;

                        willus_mem_alloc_warn((void **)&buf,maxlen*sizeof(short),funcname,10);
                        hres = psl->lpVtbl->GetPath(psl,(WCHAR *)buf,maxlen,NULL,SLGP_RAWPATH);
                        wide_to_char(target,buf);
                        }
                    else
                        hres = psl->lpVtbl->GetPath(psl,targetw,maxlen,NULL,SLGP_RAWPATH);
                    }
			    }
			ppf->lpVtbl->Release(ppf);
		    }
		psl->lpVtbl->Release(psl);
	    }
	return(SUCCEEDED(hres) ? 1 : 0);
    }


/*
** Returns 0 if user cancels, 1 otherwise
** foldername[] must have MAX_PATH (260) characters allocated
*/
int winshell_get_foldername(char *foldername,char *title)

    {
    BROWSEINFO *bi,_bi;
    PCIDLIST_ABSOLUTE pidl;

    bi=&_bi;
    bi->hwndOwner = NULL;
    bi->pidlRoot = NULL;  /* Start folder */
    bi->pszDisplayName = foldername;
    bi->lpszTitle = title;
    bi->ulFlags = BIF_EDITBOX | BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi->lpfn = NULL;
    bi->lParam = 0;
    bi->iImage = 0;
    pidl=SHBrowseForFolder(bi);
    if (pidl==NULL)
        return(0);
    if (SHGetPathFromIDList(pidl,foldername))
        return(1);
    return(0);
    }

/*
** Returns 0 if user cancels, 1 otherwise
** foldername[] must have MAX_PATH (260) characters allocated
*/
int winshell_get_foldernamew(short *foldername,char *title)

    {
    BROWSEINFOW *bi,_bi;
    PCIDLIST_ABSOLUTE pidl;
    short *wtitle;

    wtitle=char_to_wide(NULL,title);
    bi=&_bi;
    bi->hwndOwner = NULL;
    bi->pidlRoot = NULL;  /* Start folder */
    bi->pszDisplayName = (LPWSTR)foldername;
    bi->lpszTitle = (LPWSTR)wtitle;
    bi->ulFlags = BIF_EDITBOX | BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi->lpfn = NULL;
    bi->lParam = 0;
    bi->iImage = 0;
    pidl=SHBrowseForFolderW(bi);
    if (pidl==NULL)
        return(0);
    if (SHGetPathFromIDListW(pidl,(LPWSTR)foldername))
        return(1);
    return(0);
    }

#endif /* HAVE_WIN32_API */
#endif /* GNU C version test */
