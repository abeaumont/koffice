/*
 * $Id$
 *
 * Copyright 1999 by Kalle Dalheimer, released under Artistic License.
 */

#ifndef __KODIAGRAMMCOLORCONFIGPAGE_H__
#define __KODIAGRAMMCOLORCONFIGPAGE_H__

#include <qwidget.h>
#include <kcolorbtn.h>

// PENDING(kalle) Make this dynamic.
#define NUMDATACOLORS 6

class KoDiagrammColorConfigPage : public QWidget
{
	Q_OBJECT

public:
	KoDiagrammColorConfigPage( QWidget* parent );

	void setAccentColor( QColor color );
	QColor accentColor() const;
	void setAxisLabelColor( QColor color );
	QColor axisLabelColor() const;
	void setForegroundColor( QColor color );
	QColor foregroundColor() const;
	void setBackgroundColor( QColor color );
	QColor backgroundColor() const;
	void setTextColor( QColor color );
	QColor textColor() const;
	void setLabelColor( QColor color );
	QColor labelColor() const;
	void setDataColor( uint dataset, QColor color );
	QColor dataColor( uint dataset ) const;

private:
	KColorButton* _accentCB;
	KColorButton* _axislabelCB;
	KColorButton* _foregroundCB;
	KColorButton* _backgroundCB;
	KColorButton* _textCB;
	KColorButton* _labelCB;
	KColorButton* _dataCB[NUMDATACOLORS];
};


inline void KoDiagrammColorConfigPage::setAccentColor( QColor color )
{
	_accentCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::accentColor() const
{
	return _accentCB->color();
}


inline void KoDiagrammColorConfigPage::setAxisLabelColor( QColor color )
{
	_axislabelCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::axisLabelColor() const
{
	return _axislabelCB->color();
}


inline void KoDiagrammColorConfigPage::setForegroundColor( QColor color )
{
	_foregroundCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::foregroundColor() const
{
	return _foregroundCB->color();
}


inline void KoDiagrammColorConfigPage::setBackgroundColor( QColor color )
{
	_backgroundCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::backgroundColor() const
{
	return _backgroundCB->color();
}


inline void KoDiagrammColorConfigPage::setTextColor( QColor color )
{
	_textCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::textColor() const
{
	return _textCB->color();
}


inline void KoDiagrammColorConfigPage::setLabelColor( QColor color )
{
	_labelCB->setColor( color );
}

inline QColor KoDiagrammColorConfigPage::labelColor() const
{
	return _labelCB->color();
}


inline void KoDiagrammColorConfigPage::setDataColor( uint dataset,
													 QColor color )
{
	if( dataset >= NUMDATACOLORS )
		return;
	else
		_dataCB[ dataset ]->setColor( color );
}


inline QColor KoDiagrammColorConfigPage::dataColor( uint dataset ) const
{
	if( dataset >= NUMDATACOLORS )
		return _dataCB[0]->color();
	else
		return _dataCB[ dataset ]->color();
}


#endif
