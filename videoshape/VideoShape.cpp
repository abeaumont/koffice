/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QPainter>

#include <kdebug.h>
#include "VideoShape.h"



VideoShape::VideoShape(const KUrl&url)
 : m_currentUrl(url)
{
  m_widget = new QWidget(0);
  m_videowidget = new VideoWidget(m_widget );
  m_videopath = new VideoPath( m_widget );
  m_audiooutput = new AudioOutput( Phonon::VideoCategory, m_widget );
  m_audiopath = new AudioPath( m_widget );
  m_mediaobject = new MediaObject( m_widget );
  m_mediaobject->addVideoPath( m_videopath );
  m_videopath->addOutput( m_videowidget );
  m_mediaobject->addAudioPath( m_audiopath );
  m_audiopath->addOutput( m_audiooutput );

  m_mediaobject->setTickInterval( 350 );
}

VideoShape::~VideoShape()
{
}

void VideoShape::paint( QPainter& painter, const KoViewConverter& converter )
{
  kDebug()<<" VideoShape::paint( QPainter& painter, const KoViewConverter& converter )\n";
  m_widget->repaint();
}

void VideoShape::play()
{
  kDebug()<<" VideoShape::play\n";
  m_mediaobject->play();
}

void VideoShape::start()
{ 
  kDebug()<<"VideoShape::start\n";
  m_mediaobject->play();
}

void VideoShape::stop()
{ 
  kDebug()<<"VideoShape::stop \n";
  m_mediaobject->stop();
}

void VideoShape::changeUrl()
{
  kDebug()<<"VideoShape::changeUrl \n";
  //TODO

}

void VideoShape::next()
{
  kDebug()<<"VideoShape::next\n";
  //TODO
}

void VideoShape::previous()
{
  kDebug()<<"VideoShape::previous \n";
  //TODO
}

void VideoShape::pause()
{
 kDebug()<<" VideoShape::pause \n";
 m_mediaobject->pause();
}
