#ifndef __kspread_dlg_layout_h__
#define __kspread_dlg_layout_h__

#include <qwidget.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qgrpbox.h>
#include <qtabdlg.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qfont.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qframe.h> 
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlcdnum.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qscrbar.h>
#include <qtooltip.h>
#include <qstring.h>
#include <kcolordlg.h>

class CellLayoutDlg;
class KSpreadTable;

#include "kspread_cell.h"

class KSpreadPatternSelect : public QFrame
{
    Q_OBJECT
public:    
    KSpreadPatternSelect( QWidget *parent, const char *_name );

    void setPenStyle( PenStyle _pat ) { penStyle = _pat; repaint(); }
    PenStyle getPenStyle() { return penStyle; }
    void setColor( const QColor &_col ) { penColor = _col; repaint(); }
    const QColor& getColor() { return penColor; }
    void setPenWidth( int _w ) { penWidth = _w; repaint(); }
    int getPenWidth() { return penWidth; }

    void setPattern( const QColor &_color, int _width, PenStyle _style );
    void setUndefined();
    void setDefined() { undefined = FALSE; repaint(); }
    
    bool isDefined() { return !undefined; }
    
signals:
    void clicked( KSpreadPatternSelect *_this );
    
public slots:
    void slotUnselect();
    void slotSelect();
    
protected:
    virtual void paintEvent( QPaintEvent *_ev );
    virtual void mousePressEvent( QMouseEvent *_ev );

    PenStyle penStyle;
    QColor penColor;
    int penWidth;
    
    bool selected;
    bool undefined;
};

/**
 */
class CellLayoutPageFont : public QWidget
{
    Q_OBJECT
public:
    CellLayoutPageFont( QWidget* parent, CellLayoutDlg *_dlg );

    void apply( KSpreadCell *_cell );
    
signals:
    /**
     * Connect to this to monitor the font as it as selected if you are
     * not running modal.
     */
    void fontSelected( const QFont &font );
    
private slots:

    void      family_chosen_slot(const char* );
    void      size_chosen_slot(const char* );
    void      weight_chosen_slot(const char*);
    void      style_chosen_slot(const char*);
    void      display_example(const QFont &font);


private:
    
    void setCombos();
   
    QGroupBox	 *box1;
    QGroupBox	 *box2;

    QLabel	 *family_label;
    QLabel	 *size_label;
    QLabel       *weight_label;
    QLabel       *style_label;

    QLabel	 *actual_family_label;
    QLabel	 *actual_size_label;
    QLabel       *actual_weight_label;
    QLabel       *actual_style_label;

    QLabel	 *actual_family_label_data;
    QLabel	 *actual_size_label_data;
    QLabel       *actual_weight_label_data;
    QLabel       *actual_style_label_data;

    QComboBox    *family_combo;
    QComboBox    *size_combo;
    QComboBox    *weight_combo;
    QComboBox    *style_combo;
    
    QLabel       *example_label;
    QFont         selFont;

    CellLayoutDlg *dlg;
};

class CellLayoutPageMisc : public QWidget
{
    Q_OBJECT
public:
    CellLayoutPageMisc( QWidget *parent, CellLayoutDlg *_dlg );
    
    void apply( KSpreadCell *_cell );
    
public slots:
    void slotTextColor();
    void slotBackgroundColor();
    
protected:
    void setColor( QPushButton *_button, const QColor &_color );

    QColor textColor;
    bool bTextColorUndefined;
    QColor bgColor;
    bool bBgColorUndefined;
    
    QPushButton *textColorButton;
    QPushButton *bgColorButton;

    CellLayoutDlg *dlg;
};
    
/**
 * KSpreadLayout of numbers.
 * This widget is part of the layout dialog.
 * It allows the user to cinfigure the way numbers are formatted.
 */
class CellLayoutPageFloat : public QWidget
{
public:
    CellLayoutPageFloat( QWidget *parent, CellLayoutDlg *_dlg );
    
    void apply( KSpreadCell *_cell );
    
protected:
    QLineEdit* postfix;
    QLineEdit* precision;
    QLineEdit* prefix;
    QComboBox *format;

    CellLayoutDlg *dlg;
};

/**
 */
class CellLayoutPageBorder : public QWidget
{
    Q_OBJECT
public:
    CellLayoutPageBorder( QWidget *parent, CellLayoutDlg *_dlg );
    
    void apply( KSpreadCell *_cell );
    /**
     * Apply the changes in the outline to the objects.
     */
    void applyOutline( int _left, int _top, int _right, int _bottom );
    
public slots:
    void slotUnselect1( KSpreadPatternSelect *_select );
    void slotUnselect2( KSpreadPatternSelect *_select );

    void slotColorButton();
    void slotSetColorButton( const QColor &_color );

protected:

    KSpreadPatternSelect *selectedPattern;
    KSpreadPatternSelect *selectedBorder;
    
    KSpreadPatternSelect *left;
    KSpreadPatternSelect *right;    
    KSpreadPatternSelect *top;
    KSpreadPatternSelect *bottom;
    KSpreadPatternSelect *outline;
    KSpreadPatternSelect* pattern1;
    KSpreadPatternSelect* pattern2;
    KSpreadPatternSelect* pattern3;
    KSpreadPatternSelect* pattern4;
    KSpreadPatternSelect* pattern5;
    KSpreadPatternSelect* pattern6;
    KSpreadPatternSelect* pattern7;
    KSpreadPatternSelect* pattern8;
    QPushButton* color;

    QColor currentColor;
    
    CellLayoutDlg *dlg;
};

/**
 */
class CellLayoutDlg : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a layout dlg for the rectangular area in '_table'.
     */
    CellLayoutDlg( KSpreadView *_view, KSpreadTable *_table, int _left, int _top, int _right, int _bottom );

    void init();
    /**
     * Run the dialogs event loop and return when closed.
     */
    int exec();

    KSpreadTable* getTable() {	return table; }
    
    // The layout of the selected area
    PenStyle leftBorderStyle;
    int leftBorderWidth;
    bool bLeftBorderStyle;
    QColor leftBorderColor;
    bool bLeftBorderColor;
    PenStyle rightBorderStyle;
    int rightBorderWidth;
    bool bRightBorderStyle;
    QColor rightBorderColor;
    bool bRightBorderColor;
    PenStyle topBorderStyle;
    int topBorderWidth;
    bool bTopBorderStyle;
    QColor topBorderColor;
    bool bTopBorderColor;
    PenStyle bottomBorderStyle;
    int bottomBorderWidth;
    bool bBottomBorderStyle;
    QColor bottomBorderColor;
    bool bBottomBorderColor;
    PenStyle outlineBorderStyle;
    int outlineBorderWidth;
    bool bOutlineBorderStyle;
    QColor outlineBorderColor;
    bool bOutlineBorderColor;
    QString prefix;
    QString postfix;
    int precision;
    KSpreadCell::FloatFormat floatFormat;
    bool bFloatFormat;
    KSpreadCell::FloatColor floatColor;
    bool bFloatColor;
    QColor textColor;
    bool bTextColor;
    bool bTextFontBold;
    bool textFontBold;
    bool bTextFontItalic;
    bool textFontItalic;
    bool bTextFontSize;
    int textFontSize;
    bool bTextFontFamily;
    QString textFontFamily;
    QFont textFont;
    QColor bgColor;
    bool bBgColor;
    
    static QPixmap* formatOnlyNegSignedPixmap;
    static QPixmap* formatRedOnlyNegSignedPixmap;
    static QPixmap* formatRedNeverSignedPixmap;
    static QPixmap* formatAlwaysSignedPixmap;
    static QPixmap* formatRedAlwaysSignedPixmap;

    static QPixmap* undefinedPixmap;
    
public slots:    
    void slotApply();
    
protected:

    /**
     * Used to draw the @ref #redPixmap and friends
     */
    QPixmap* paintColorPixmap( const QColor &color );
    /**
     * Used to draw the @ref border1Pixmap and friends.
     */
    QPixmap* paintBorderPixmap( PenStyle _style, int _width  );
    /**
     * Used to draw the @ref #formatPixmap and friends.
     */
    QPixmap* paintFormatPixmap( const char *_string1, const QColor & _color1,
				const char *_string2, const QColor & _color2 );
    
    CellLayoutPageFloat *floatPage;    
    CellLayoutPageBorder *borderPage;
    CellLayoutPageMisc *miscPage;
    CellLayoutPageFont *fontPage;
    
    QTabDialog *tab;
    
    /**
     * The table that opened this dlg.
     */
    KSpreadTable *table;

    KSpreadView *m_pView;
  
    // The rectangular area for which this dlg has been opened.
    int left;
    int right;
    int top;
    int bottom;
    
};

#endif
