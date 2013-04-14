/*
 ** setting.h   K2pdfopt optimizes PDF/DJVU files for mobile e-readers
 **              (e.g. the Kindle) and smartphones. It works well on
 **              multi-column PDF/DJVU files. K2pdfopt is freeware.
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

#ifndef _KOPTSETTING_H
#define _KOPTSETTING_H

#include "k2pdfopt.h"
#include "context.h"

void k2pdfopt_settings_init_from_koptcontext(K2PDFOPT_SETTINGS *k2settings, KOPTContext *kctx);

#endif

