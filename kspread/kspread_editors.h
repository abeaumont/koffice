#ifndef __kspread_editors_h__
#define __kspread_editors_h__

#include <qwidget.h>
#include <qstring.h>

#include <kformulaedit.h>

class KSpreadCell;

class KFormulaEdit;

class QLineEdit;

class KSpreadCellEditor : public QWidget
{
  Q_OBJECT
public:
  KSpreadCellEditor( KSpreadCell*, QWidget* _parent = 0, const char* _name = 0 );
  ~KSpreadCellEditor();

  KSpreadCell* cell() { return m_pCell; }

  virtual void handleKeyPressEvent( QKeyEvent* _ev ) = 0;
  virtual QString text() = 0;
  virtual void setText(QString text) = 0;
private:
  KSpreadCell* m_pCell;
};

class KSpreadTextEditor : public KSpreadCellEditor
{
  Q_OBJECT
public:
  KSpreadTextEditor( KSpreadCell*, QWidget* _parent = 0, const char* _name = 0 );
  ~KSpreadTextEditor();

  virtual void handleKeyPressEvent( QKeyEvent* _ev );
  virtual QString text();
  virtual void setText(QString text);

protected:
  void resizeEvent( QResizeEvent* );
  /**
   * Steals some key events from the QLineEdit and sends
   * it to the @ref KSpreadCancvas ( its parent ) instead.
   */
  bool eventFilter( QObject* o, QEvent* e );

private:
  QLineEdit* m_pEdit;
};

class KSpreadFormulaEditor : public KSpreadCellEditor
{
  Q_OBJECT
public:
  KSpreadFormulaEditor( KSpreadCell*, QWidget* _parent = 0, const char* _name = 0 );
  ~KSpreadFormulaEditor();

  virtual void handleKeyPressEvent( QKeyEvent* _ev );
  virtual QString text();
  virtual void setText(QString text);

protected slots:
  void slotSizeHint( QSize );

protected:
  void resizeEvent( QResizeEvent* );
  /**
   * Steals some key events from the KFormulaEdit and sends
   * it to the @ref KSpreadCancvas ( its parent ) instead.
   */
  bool eventFilter( QObject* o, QEvent* e );

private:
  KFormulaEdit* m_pEdit;
};

#endif
