////////////////////////////////////////////
//         ___ ____ _________________     //
//        / _/_  _// _______________/     //
//       / _/ / / / /  ___ ___ ____       //
//      /__/ /_/ / / /   // _/_  _/       //
//     _________/ / / / // _/ / /         //
//  __(c) 1998___/ /___//_/  /_/          //
//                                        //
////////////////////////////////////////////
//          all rights reserved           //
////////////////////////////////////////////

#ifndef __diagramm_h__
#define __diagramm_h__

#include <map>
#include <list>
#include <string>
#include <vector>
#include <qstrlist.h>

typedef vector<double> line_t;
typedef vector<line_t> data_t;

struct table_t
{
  QStrList xDesc;
  QStrList yDesc;
  data_t data;
};

#include <qcolor.h>
#include <qrect.h>
#include <qpoint.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>

class KoDiagramm
{
public:
  enum dia_type { DT_KREIS, DT_SAEULEN,	DT_KREIS_ODER_SAEULEN, DT_LINIEN, DT_AREA };
  enum data_type { DAT_NUMMER, DAT_GEB,	DAT_DAUER };

  void setData( table_t& data, const char* szTitle, data_type datType = DAT_NUMMER,
		dia_type diaType = DT_KREIS_ODER_SAEULEN);
  void clearData();

  void paint( QPainter& painter, int width, int height );
  
protected:  
  void drawDiagrammKreis( QPainter& painter, int _width, int _height );
  void drawDiagrammLinien( QPainter& painter, int _width, int _height );
  void drawDiagrammSaeulen( QPainter& painter, int _width, int _height );

  /* void FormatAnzahl ( double val, QString& out );
  void FormatGebuehr ( double val, QString& out );
  void FormatDauer ( double val, QString& out );
  */
  void KatDouble ( double val, double& out );
  void KatTime ( double val, double& out );

  table_t m_table;
  QString m_curTitle;
  dia_type m_diaType;
  data_type m_dataType;
};

class KoDiagrammView : public QWidget
{
  Q_OBJECT
public:
  KoDiagrammView( QWidget* _parent );
  virtual ~KoDiagrammView();

  KoDiagramm& diagramm() { return m_diagramm; }

  void paintEvent( QPaintEvent *_ev );
  void resizeEvent( QResizeEvent *_ve );
  
protected:
  KoDiagramm m_diagramm;
};

#endif
