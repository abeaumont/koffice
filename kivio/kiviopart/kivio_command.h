/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kiviocommand_h
#define kiviocommand_h

#include <kcommand.h>
#include <qfont.h>
#include <qcolor.h>
#include "kivio_rect.h"
#include "tkpagelayout.h"
class KivioPage;
class KivioLayer;
class KivioPage;
class KivioLayer;
class KivioStencil;

class KivioChangePageNameCommand : public KNamedCommand
{
public:
    KivioChangePageNameCommand( const QString &_name,  const QString & _oldPageName, const QString & _newPageName, KivioPage *_page );
    ~KivioChangePageNameCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    QString oldPageName;
    QString newPageName;
    KivioPage * m_page;
};

class KivioHidePageCommand : public KNamedCommand
{
public:
    KivioHidePageCommand( const QString &_name, KivioPage *_page );
    ~KivioHidePageCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage * m_page;
};

class KivioShowPageCommand : public KivioHidePageCommand
{
public:
    KivioShowPageCommand( const QString &_name, KivioPage *_page );
    ~KivioShowPageCommand() {}

    void execute() { KivioHidePageCommand::unexecute(); }
    void unexecute() { KivioHidePageCommand::execute(); }
};

class KivioAddPageCommand : public KNamedCommand
{
public:
    KivioAddPageCommand( const QString &_name, KivioPage *_page );
    ~KivioAddPageCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage * m_page;
};

class KivioRemovePageCommand : public KNamedCommand
{
public:
    KivioRemovePageCommand( const QString &_name, KivioPage *_page );
    ~KivioRemovePageCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage * m_page;
};

class KivioAddStencilCommand : public KNamedCommand
{
public:
    KivioAddStencilCommand( const QString &_name, KivioPage *_page,  KivioLayer * _layer, KivioStencil *_stencil  );
    ~KivioAddStencilCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage * m_page;
    KivioLayer * m_layer;
    KivioStencil *m_stencil;
};


class KivioRemoveStencilCommand : public KivioAddStencilCommand
{
public:
    KivioRemoveStencilCommand(const QString &_name, KivioPage *_page,  KivioLayer * _layer, KivioStencil *_stencil );
    ~KivioRemoveStencilCommand() {}

    void execute() { KivioAddStencilCommand::unexecute(); }
    void unexecute() { KivioAddStencilCommand::execute(); }
};

class KivioChangeStencilTextCommand : public KNamedCommand
{
public:
    KivioChangeStencilTextCommand( const QString &_name, KivioStencil *_stencil, const QString & _oldText, const QString & _newText, KivioPage *_page);
    ~KivioChangeStencilTextCommand();
    virtual void execute();
    virtual void unexecute();
protected:
    KivioStencil *m_stencil;
    QString oldText;
    QString newText;
    KivioPage *m_page;
};

class KivioAddLayerCommand : public KNamedCommand
{
public:
    KivioAddLayerCommand( const QString &_name, KivioPage *_page, KivioLayer * _layer,int _pos );
    ~KivioAddLayerCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage * m_page;
    KivioLayer *m_layer;
    int layerPos;
};

class KivioRemoveLayerCommand : public KivioAddLayerCommand
{
public:
    KivioRemoveLayerCommand( const QString &_name, KivioPage *_page, KivioLayer * _layer, int _pos );
    ~KivioRemoveLayerCommand() {}

    void execute() { KivioAddLayerCommand::unexecute(); }
    void unexecute() { KivioAddLayerCommand::execute(); }
};

class KivioRenameLayerCommand : public KNamedCommand
{
public:
    KivioRenameLayerCommand( const QString &_name, KivioLayer * _layer, const QString & _oldName, const QString & _newName);
    ~KivioRenameLayerCommand();
    virtual void execute();
    virtual void unexecute();

protected:
    KivioLayer *m_layer;
    QString oldName;
    QString newName;

};

class KivioResizeStencilCommand : public KNamedCommand
{
public:
    KivioResizeStencilCommand(const QString &_name, KivioStencil *_stencil, KivioRect _initSize, KivioRect _endSize, KivioPage *_page );
    ~KivioResizeStencilCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioStencil *m_stencil;
    KivioRect initSize;
    KivioRect endSize;
    KivioPage *m_page;
};

class KivioMoveStencilCommand : public KNamedCommand
{
public:
    KivioMoveStencilCommand(const QString &_name, KivioStencil *_stencil, KivioRect _initSize, KivioRect _endSize, KivioPage *_page );
    ~KivioMoveStencilCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioStencil *m_stencil;
    KivioRect initSize;
    KivioRect endSize;
    KivioPage *m_page;
};

class KivioChangeLayoutCommand : public KNamedCommand
{
public:
    KivioChangeLayoutCommand(const QString &_name, KivioPage *_page, TKPageLayout _oldLayout, TKPageLayout _newLayout);
    ~KivioChangeLayoutCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    TKPageLayout oldLayout;
    TKPageLayout newLayout;
};


class KivioChangeStencilHAlignmentCommand : public KNamedCommand
{
public:
    KivioChangeStencilHAlignmentCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, int _oldAlign,  int _newAlign);
    ~KivioChangeStencilHAlignmentCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    int oldAlign;
    int newAlign;
};

class KivioChangeStencilVAlignmentCommand : public KNamedCommand
{
public:
    KivioChangeStencilVAlignmentCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, int _oldAlign,  int _newAlign);
    ~KivioChangeStencilVAlignmentCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    int oldAlign;
    int newAlign;
};


class KivioChangeStencilFontCommand : public KNamedCommand
{
public:
    KivioChangeStencilFontCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, const QFont & _oldFont,  const QFont & _newFont);
    ~KivioChangeStencilFontCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    QFont oldFont;
    QFont newFont;
};

class KivioChangeStencilColorCommand : public KNamedCommand
{
public:
    enum ColorType { CT_TEXTCOLOR, CT_FGCOLOR, CT_BGCOLOR };
    KivioChangeStencilColorCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, const QColor & _oldColor,  const QColor & _newColor, ColorType _type);
    ~KivioChangeStencilColorCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    QColor oldColor;
    QColor newColor;
    ColorType type;
};

class KivioChangeLineWidthCommand : public KNamedCommand
{
public:
    KivioChangeLineWidthCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, int _oldValue,  int _newValue);
    ~KivioChangeLineWidthCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    int oldValue;
    int newValue;
};

class KivioChangeBeginEndArrowCommand : public KNamedCommand
{
public:
    KivioChangeBeginEndArrowCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, int _oldArrow,  int _newArrow, bool _beginArrow);

    ~KivioChangeBeginEndArrowCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    int oldArrow;
    int newArrow;
    bool beginArrow;
};

class KivioChangeBeginEndSizeArrowCommand : public KNamedCommand
{
public:
    KivioChangeBeginEndSizeArrowCommand( const QString &_name, KivioPage *_page, KivioStencil * _stencil, float _oldH,float _oldW, float _newH,float _newW, bool _beginArrow);

    ~KivioChangeBeginEndSizeArrowCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    KivioPage *m_page;
    KivioStencil *m_stencil;
    float oldWidth;
    float oldLength;
    float newWidth;
    float newLength;

    bool beginArrow;
};

#endif

