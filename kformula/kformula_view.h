#ifndef __kformula_view_h__
#define __kformula_view_h__

class KFormulaView;
class KFormulaDocument;

#include <view_impl.h>
#include <document_impl.h>
#include <part_frame_impl.h>
#include <menu_impl.h>
#include <toolbar_impl.h>

#include "kformula.h"

#include <qwidget.h>
#include <qlist.h>

class KFormulaView : public QWidget,
		   virtual public View_impl,
		   virtual public KFormula::View_skel
{
  Q_OBJECT
public:
  // C++
  KFormulaView( QWidget *_parent = 0L, const char *_name = 0L );
  virtual ~KFormulaView();

  // IDL
  virtual void newView();
  
  // C++
  virtual void setDocument( KFormulaDocument *_doc );
  virtual void createGUI();

public slots:
  // Document signals
  void slotModified();
  
protected:
  virtual void cleanUp();
  
  void paintEvent( QPaintEvent *_ev );
  void resizeEvent( QResizeEvent *_ev );

  KFormulaDocument *m_pDoc;
  
  OPParts::MenuBarFactory_var m_vMenuBarFactory;
  MenuBar_ref m_rMenuBar;
  CORBA::Long m_idMenuView;
  CORBA::Long m_idMenuView_NewView;
  
  OPParts::ToolBarFactory_var m_vToolBarFactory;
  ToolBar_ref m_rToolBarFormula;
  ToolBar_ref m_rToolBarFont;
  ToolBar_ref m_rToolBarType;
  CORBA::Long m_idButtonFormula_0;
  CORBA::Long m_idButtonFormula_1;
  CORBA::Long m_idButtonFormula_2;
  CORBA::Long m_idButtonFormula_3;
  CORBA::Long m_idButtonFormula_4;
  CORBA::Long m_idButtonFormula_5;
  CORBA::Long m_idButtonFormula_6;
  CORBA::Long m_idButtonFont_0;
  CORBA::Long m_idButtonFont_1;
  CORBA::Long m_idButtonFont_2;
  CORBA::Long m_idButtonFont_3;
  CORBA::Long m_idButtonFont_4;
  CORBA::Long m_idButtonFont_5;
  CORBA::Long m_idButtonType_0;
  CORBA::Long m_idButtonType_1;
  CORBA::Long m_idButtonType_2;
  CORBA::Long m_idButtonType_3;
};

#endif
