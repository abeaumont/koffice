/* This file is part of the KOffice libraries
    Copyright (c) 2003 Kexi Team

    Version information based on kofficeversion.h,
    Copyright (c) 2003 KOffice Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kexi_version.h"

using namespace Kexi;

KEXICORE_EXPORT unsigned int version()
{
    return KEXI_VERSION;
}

KEXICORE_EXPORT unsigned int versionMajor()
{
    return KEXI_VERSION_MAJOR;
}

KEXICORE_EXPORT unsigned int versionMinor()
{
    return KEXI_VERSION_MINOR;
}

KEXICORE_EXPORT unsigned int versionRelease()
{
    return KEXI_VERSION_RELEASE;
}

KEXICORE_EXPORT const char *versionString()
{
    return KEXI_VERSION_STRING;
}

