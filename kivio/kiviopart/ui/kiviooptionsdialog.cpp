#include "kiviooptionsdialog.h"

#include "kivio_view.h"

#include "guides1page.h"
#include "guides2page.h"
#include "guidessetupdialog.h"
#include "gridsetupdialog.h"
#include "gridsetupdialog.h"

#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qwidgetstack.h>

KivioOptionsDialog::KivioOptionsDialog(KivioView* view, QWidget* parent, const char* name)
: KivioOptionsDialogBase(parent, name, true), m_pView(view)
{
  list->header()->hide();
  list->header()->removeLabel(1);
  list->header()->removeLabel(1);
}

KivioOptionsDialog::~KivioOptionsDialog()
{
}

void KivioOptionsDialog::apply()
{
}

void KivioOptionsDialog::slotCurrentChanged(QListViewItem* i)
{
  QCString pname = i->text(1).latin1(); // ### latin1() breaks unicode!!! (Simon)
  QWidget* page = 0;

  page = (QWidget*)stack->child(pname);
  if (page) {
    stack->raiseWidget(page);
  } else {

    if (pname == "_page_") {
      page = new GuidesSetupDialog(m_pView, page, pname);
    }

    if (pname == "_pagesize_") {
      page = new GuidesSetupDialog(m_pView, page, pname);
    }

    if (pname == "_pagegrid_") {
      page = new GridSetupDialog(page, pname);
//      page = new GridSetupDialog(m_pView, page, pname);
    }

    if (pname == "_guides_") {
      page = new GuidesSetupDialog(m_pView, page, pname);
    }

    if (pname == "_allguides_") {
      page = new GuidesTwoPositionPage(m_pView, page, pname);
    }

    if (pname == "_guidehorizontal_") {
      page = new GuidesOnePositionPage(Horizontal, m_pView, page, pname);
    }

    if (pname == "_guidevertical_") {
      page = new GuidesOnePositionPage(Vertical, m_pView, page, pname);
    }

    if (page)
      stack->addWidget(page,0);
  }

  if (page) {
    stack->raiseWidget(page);
    label->setText(i->text(2));
  }
}

void KivioOptionsDialog::slotOk()
{
  apply();
  accept();
}

