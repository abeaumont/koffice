/* Style: A base class from which all other styles are inherited, includes
 * a name.
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef STYLE_H
#define STYLE_H
#include <libwpd/libwpd.h>
#include "DocumentElement.hxx"

class TopLevelElementStyle
{
public:
	TopLevelElementStyle() : mpsMasterPageName(NULL) { }
	virtual ~TopLevelElementStyle() { if (mpsMasterPageName) delete mpsMasterPageName; }
	void setMasterPageName(RVNGString &sMasterPageName) { mpsMasterPageName = new RVNGString(sMasterPageName); }
	const RVNGString * getMasterPageName() const { return mpsMasterPageName; }

private:
	RVNGString *mpsMasterPageName;
};

class Style
{
 public:
	Style(const RVNGString &psName) : msName(psName) {}
	virtual ~Style() {}

	virtual void write(DocumentHandler &xHandler) const {};
	const RVNGString &getName() const { return msName; }

 private:
	RVNGString msName;
};
#endif
