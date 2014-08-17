/*
** wincomdlg.c     Functions that call common Windows dialogs from comdlg32.dll
**                 (E.g. open file dialog window).
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

#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
** Calls GetOpenFileName() from COMDLG32.DLL.
**
** "filter" input needs to be null-terminated pairs of strings, e.g.
**     "PDF files\0*.pdf\0DJVU files\0*.djvu\0All files\0*\0\0\0";
** "filter" can be NULL.
**
** "title" is the string shown in the title field of the dialog box.
**
** The "defext" string is appended to the file name if no extension is typed
** by the user (only the first three chars are used).  Don't put a "." in
** the extension.  If "defext" is NULL, no extension is appended.
**
** If multiselect!=0, multiple files can be selected.  They are returned
** like so:
**
**     filename = "c:\mypath\mysubdir\file1\0file2\0file3\0\0";
**
** If must_exist!=0, the file must already exist.
**
** If for_writing is non-zero, calls GetSaveFileName().
**
** Returns 1 for success, 0 for cancel.
**
*/
int wincomdlg_get_filename(char *filename,int maxlen,char *filter,char *title,char *defext,
                           int multiselect,int must_exist,int for_writing)

    {
    OPENFILENAME *fn,_fn;

    fn=&_fn;
    fn->lStructSize=sizeof(OPENFILENAME);
    fn->hwndOwner=NULL;
    fn->hInstance=0;
    fn->lpstrFilter=filter;
    fn->lpstrCustomFilter=NULL;
    fn->nMaxCustFilter=0;
    fn->nFilterIndex=1;
    fn->lpstrFile=filename;
    fn->lpstrFile[0]='\0';
    fn->nMaxFile=maxlen;
    fn->lpstrFileTitle=NULL;
    fn->nMaxFileTitle=0;
    fn->lpstrInitialDir=NULL;
    fn->lpstrTitle=title;
    fn->Flags = 0;
    if (!for_writing && multiselect)
        fn->Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    if (must_exist)
        fn->Flags |= OFN_FILEMUSTEXIST;
    fn->nFileOffset=0;
    fn->nFileExtension=0;
    fn->lpstrDefExt=defext;
    fn->lCustData=0;
    fn->lpfnHook=NULL;
    fn->lpTemplateName=NULL;
    fn->pvReserved=NULL;
    fn->dwReserved=0;
    fn->FlagsEx=0;
    return(for_writing ? GetSaveFileName(fn) : GetOpenFileName(fn));
    }


#endif /* WIN32 */
