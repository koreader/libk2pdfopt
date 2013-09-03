/*
** winshell.c    Windows specific calls related to Windows Shell
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2013  http://willus.com
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

#ifdef WIN32
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
*/
int win_resolve_shortcut(char *shortcut,char *target,int maxlen)

    {
	IShellLink *psl = NULL;

	if (target == NULL)
		return(0);
	target[0]='\0';
    /*
	** Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	** has already been called.
    */
	HRESULT hres = CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,
                                    &IID_IShellLink,(LPVOID*)&psl);
	if (SUCCEEDED(hres))
        {
		/* Get a pointer to the IPersistFile interface. */
		IPersistFile *ppf = NULL;

		hres = psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(void**)&ppf);
		if (SUCCEEDED(hres))
            {
			/* Add code here to check return value from MultiByteWideChar for success. */
			/* Load the shortcut. */
#ifdef _UNICODE
			hres = ppf->lpVtbl->Load(ppf,shortcut,STGM_READ);
#else
			WCHAR widestr[512];

            widestr[0]='\0';
			/* Ensure that the string is Unicode. */
			MultiByteToWideChar(CP_ACP,0,shortcut,-1, widestr,511);
			hres = ppf->lpVtbl->Load(ppf,widestr,STGM_READ);
#endif
			if (SUCCEEDED(hres))
                {
				/* Resolve the link. */
				hres = psl->lpVtbl->Resolve(psl,GetDesktopWindow(),0);
				if (SUCCEEDED(hres))
                    hres = psl->lpVtbl->GetPath(psl,target,maxlen,NULL,SLGP_RAWPATH);
			    }
			ppf->lpVtbl->Release(ppf);
		    }
		psl->lpVtbl->Release(psl);
	    }
	return(SUCCEEDED(hres) ? 1 : 0);
    }


#endif /* WIN32 */
