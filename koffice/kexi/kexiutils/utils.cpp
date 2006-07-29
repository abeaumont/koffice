/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "utils.h"
#include "utils_p.h"

#include <qregexp.h>
#include <qpainter.h>
#include <qimage.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <kcursor.h>
#include <kapplication.h>

using namespace KexiUtils;

DelayedCursorHandler::DelayedCursorHandler() {
	connect(&timer, SIGNAL(timeout()), this, SLOT(show()));
}
void DelayedCursorHandler::start(bool noDelay) {
	timer.start(noDelay ? 0 : 1000, true);
}
void DelayedCursorHandler::stop() {
	timer.stop();
	QApplication::restoreOverrideCursor();
}
void DelayedCursorHandler::show() {
	QApplication::setOverrideCursor( KCursor::waitCursor() );
}

DelayedCursorHandler _delayedCursorHandler;

void KexiUtils::setWaitCursor(bool noDelay) {
	if (kapp->guiEnabled())
		_delayedCursorHandler.start(noDelay);
}
void KexiUtils::removeWaitCursor() {
	if (kapp->guiEnabled())
		_delayedCursorHandler.stop();
}

WaitCursor::WaitCursor(bool noDelay)
{
	setWaitCursor(noDelay);
}

WaitCursor::~WaitCursor()
{
	removeWaitCursor();
}

//--------------------------------------------------------------------------------

QString KexiUtils::fileDialogFilterString(const KMimeType::Ptr& mime, bool kdeFormat)
{
	if (mime==0)
		return QString::null;

	QString str;
	if (kdeFormat) {
		if (mime->patterns().isEmpty())
			str = "*";
		else
			str = mime->patterns().join(" ");
		str += "|";
	}
	str += mime->comment();
	if (!mime->patterns().isEmpty() || !kdeFormat) {
		str += " (";
		if (mime->patterns().isEmpty())
			str += "*";
		else
			str += mime->patterns().join("; ");
		str += ")";
	}
	if (kdeFormat)
		str += "\n";
	else
		str += ";;";
	return str;
}

QString KexiUtils::fileDialogFilterString(const QString& mimeString, bool kdeFormat)
{
	KMimeType::Ptr ptr = KMimeType::mimeType(mimeString);
	return fileDialogFilterString( ptr, kdeFormat );
}

QString KexiUtils::fileDialogFilterStrings(const QStringList& mimeStrings, bool kdeFormat)
{
	QString ret;
	QStringList::ConstIterator endIt = mimeStrings.constEnd();
	for(QStringList::ConstIterator it = mimeStrings.constBegin(); it != endIt; ++it)
		ret += fileDialogFilterString(*it, kdeFormat);
	return ret;
}

QColor KexiUtils::blendedColors(const QColor& c1, const QColor& c2, int factor1, int factor2)
{
	return QColor(
		int( (c1.red()*factor1+c2.red()*factor2)/(factor1+factor2) ),
		int( (c1.green()*factor1+c2.green()*factor2)/(factor1+factor2) ),
		int( (c1.blue()*factor1+c2.blue()*factor2)/(factor1+factor2) ) );
}

QColor KexiUtils::contrastColor(const QColor& c)
{
	int g = qGray( c.rgb() );
	if (g>110)
		return c.dark(200);
	else if (g>80)
		return c.light(150);
	else if (g>20)
		return c.light(300);
	return Qt::gray;
}

QColor KexiUtils::bleachedColor(const QColor& c, int factor)
{
	int h, s, v;
	c.getHsv( &h, &s, &v );
	QColor c2;
	if (factor < 100)
		factor = 100;
	if (s>=250 && v>=250) //for colors like cyan or red, make the result more white
		s = QMAX(0, s - factor - 50);
	else if (s<=5 && s<=5)
		v += factor-50;
	c2.setHsv(h, s, QMIN(255,v + factor-100));
	return c2;
}

void KexiUtils::serializeMap(const QMap<QString,QString>& map, const QByteArray& array)
{
	QDataStream ds(array, IO_WriteOnly);
	ds << map;
}

void KexiUtils::serializeMap(const QMap<QString,QString>& map, QString& string)
{
	QByteArray array;
	QDataStream ds(array, IO_WriteOnly);
	ds << map;
	kdDebug() << array[3] << " " << array[4] << " " << array[5] << endl;
	const uint size = array.size();
	string = QString::null;
	string.reserve(size);
	for (uint i=0; i<size; i++) {
		string[i]=QChar(ushort(array[i]+1));
	}
}

QMap<QString,QString> KexiUtils::deserializeMap(const QByteArray& array)
{
	QMap<QString,QString> map;
	QDataStream ds(array, IO_ReadOnly);
	ds >> map;
	return map;
}

QMap<QString,QString> KexiUtils::deserializeMap(const QString& string)
{
	const uint size = string.length();
	QCString cstr(string.latin1());
	QByteArray array( size );
	for (uint i=0; i<size; i++) {
		array[i] = char(string[i].unicode()-1);
	}
	QMap<QString,QString> map;
	QDataStream ds(array, IO_ReadOnly);
	ds >> map;
	return map;
}

QString KexiUtils::stringToFileName(const QString& string)
{
	QString _string(string);
	_string.replace(QRegExp("[\\\\/:\\*?\"<>|]"), " ");
	return _string.simplifyWhiteSpace();
}

void KexiUtils::simpleCrypt(QString& string)
{
	for (uint i=0; i<string.length(); i++)
		string[i] = QChar( string[i].unicode() + 47 + i );
}

void KexiUtils::simpleDecrypt(QString& string)
{
	for (uint i=0; i<string.length(); i++)
		string[i] = QChar( string[i].unicode() - 47 - i );
}

void KexiUtils::drawPixmap( QPainter& p, int lineWidth, const QRect& rect, 
	const QPixmap& pixmap, int alignment, bool scaledContents, bool keepAspectRatio)
{
	if (pixmap.isNull())
		return;

	const bool fast = pixmap.width()>1000 && pixmap.height()>800; //fast drawing needed
	const int w = rect.width()-lineWidth-lineWidth;
	const int h = rect.height()-lineWidth-lineWidth;
//! @todo we can optimize drawing by drawing rescaled pixmap here 
//! and performing detailed painting later (using QTimer)
	QPixmap pixmapBuffer;
	QPainter p2;
	QPainter *target;
	if (fast) {
		target = &p;
	}
	else {
//moved		pixmapBuffer.resize(rect.size()-QSize(lineWidth, lineWidth));
//moved		p2.begin(&pm, p.device());
		target = &p2;
	}
//! @todo only create buffered pixmap of the minimum size and then do not fillRect()
//	target->fillRect(0,0,rect.width(),rect.height(), backgroundColor);

	QPoint pos;
	if (scaledContents) {
		if (keepAspectRatio) {
			QImage img(pixmap.convertToImage());
			img = img.smoothScale(w, h, QImage::ScaleMin);
			pos = rect.topLeft(); //0, 0);
			if (img.width() < w) {
				int hAlign = QApplication::horizontalAlignment( alignment );
				if ( hAlign & Qt::AlignRight )
					pos.setX(pos.x() + w-img.width());
				else if ( hAlign & Qt::AlignHCenter )
					pos.setX(pos.x() + w/2-img.width()/2);
			}
			else if (img.height() < h) {
				if ( alignment & Qt::AlignBottom )
					pos.setY(pos.y() + h-img.height());
				else if ( alignment & Qt::AlignVCenter )
					pos.setY(pos.y() + h/2-img.height()/2);
			}
			pixmapBuffer.convertFromImage(img);
			if (!fast) {
				p2.begin(&pixmapBuffer, p.device());
			}
			else
				target->drawPixmap(pos, pixmapBuffer);
		}
		else {
			if (!fast) {
				pixmapBuffer.resize(rect.size()-QSize(lineWidth, lineWidth));
				p2.begin(&pixmapBuffer, p.device());
				p2.drawPixmap(QRect(rect.x(), rect.y(), w, h), pixmap);
			}
			else
				target->drawPixmap(QRect(rect.x() + lineWidth, rect.y() + lineWidth, w, h), pixmap);
		}
	}
	else {
		int hAlign = QApplication::horizontalAlignment( alignment );
		if ( hAlign & Qt::AlignRight )
			pos.setX(pos.x() + w-pixmap.width());
		else if ( hAlign & Qt::AlignHCenter )
			pos.setX(pos.x() + w/2-pixmap.width()/2);
		else //left, etc.
			pos.setX(pos.x());

		if ( alignment & Qt::AlignBottom )
			pos.setY(pos.y() + h-pixmap.height());
		else if ( alignment & Qt::AlignVCenter )
			pos.setY(pos.y() + h/2-pixmap.height()/2);
		else //top, etc. 
			pos.setY(pos.y());
//		target->drawPixmap(pos, pixmap);
//		if (!fast)
//			p2.begin(&pixmapBuffer, p.device());
		p.drawPixmap(lineWidth, lineWidth, pixmap);
	}
	if (scaledContents && !fast && p.isActive()) {
		p2.end();
		bitBlt( p.device(), 
//			pos.x(), 
//			pos.y(), 
			(int)p.worldMatrix().dx() + rect.x() + lineWidth + pos.x(), 
			(int)p.worldMatrix().dy() + rect.y() + lineWidth + pos.y(), 
			&pixmapBuffer);
	}
}

#include "utils_p.moc"
