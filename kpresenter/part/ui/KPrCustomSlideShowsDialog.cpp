/* This file is part of the KDE project
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
//Qt includes
#include <QDebug>
#include <QStringList>
#include <QListWidgetItem>
#include <QIcon>
#include <QListWidgetItem>
#include <QList>

//KDE includes
#include <KMessageBox>

//KOffice includes
#include <KoPAPageBase.h>
#include <KPrDocument.h>

//KPresenter includes
#include <KPrCustomSlideShows.h>

#include "KPrCustomSlideShowsDialog.h"

//so to be able to use it in a QVariant
Q_DECLARE_METATYPE(KoPAPageBase*)

KPrCustomSlideShowsDialog::KPrCustomSlideShowsDialog( QWidget *parent, KPrCustomSlideShows *slideShows,
                                                      KPrDocument *doc, KPrCustomSlideShows *&newSlideShows )
: QDialog(parent)
, m_slideShows( new KPrCustomSlideShows(*slideShows) )
, m_oldSlideShows(slideShows)
, m_doc(doc)
, m_firstTime( true )
{
    m_uiWidget.setupUi( this );
    //Conections
    connect( m_uiWidget.addButton, SIGNAL( clicked() ), this, SLOT( addCustomSlideShow() ) );
    connect( m_uiWidget.okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_uiWidget.deleteButton, SIGNAL( clicked() ), this, SLOT( deleteCustomSlideShow() ) );
    connect( m_uiWidget.undoButton, SIGNAL( clicked() ), this, SLOT( loadCustomSlideShowsData() ) );
    connect( m_uiWidget.addSlideButton, SIGNAL( clicked() ), this, SLOT( addSlidesToCurrentSlideShow() ) );
    connect( m_uiWidget.customSlideShowsList, SIGNAL( currentItemChanged(QListWidgetItem*, QListWidgetItem*) ),
             this, SLOT( changedSelectedSlideshow(QListWidgetItem*, QListWidgetItem*) ) );

    //Make the newSlideShows be the same as m_slideShows
    newSlideShows = m_slideShows;

    //Load the CustomSlideShows already created
    loadCustomSlideShowsData();

    //Load the available slides
    QListWidgetItem * item;
    int currentPage = 1;
    foreach( KoPAPageBase* page, doc->pages() )
    {
        item = new QListWidgetItem( QIcon(), i18n("Slide %1", currentPage++), m_uiWidget.availableSlidesList );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

        item->setData( SlideData, QVariant::fromValue(page) );
    }
}

KPrCustomSlideShowsDialog::~KPrCustomSlideShowsDialog()
{
}

void KPrCustomSlideShowsDialog::addCustomSlideShow()
{
    //We create a different default name for every SlideShow:
    static int newSlideShowsCount = 0;
    QListWidgetItem * item = new QListWidgetItem( i18n("New Slide Show %1", ++newSlideShowsCount) );
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    item->setData( SlideShowNameData, QVariant(i18n("New Slide Show %1", newSlideShowsCount)) );

    //we add it to the display and the slideShows
    m_uiWidget.customSlideShowsList->addItem( item );
    m_slideShows->insert( i18n("New Slide Show %1", newSlideShowsCount), QList<KoPAPageBase*>() );

    //Let the user choose the name
    m_uiWidget.customSlideShowsList->editItem( item );
}

void KPrCustomSlideShowsDialog::renameCustomSlideShow( QListWidgetItem *item )
{
    //We disconect the signal so not to call this function over and over:
    disconnect( m_uiWidget.customSlideShowsList, SIGNAL( itemChanged( QListWidgetItem*) ),
        this, SLOT( renameCustomSlideShow(QListWidgetItem*) ) );

    //If the name is not already in use, use it
    if( !m_slideShows->names().contains( item->data( Qt::DisplayRole ).toString() ) )
    {
        m_slideShows->rename( item->data( SlideShowNameData ).toString(), item->data( Qt::DisplayRole ).toString() );
        item->setData( SlideShowNameData, item->data( Qt::DisplayRole ) );
    }
    //otherwise let the user know
    else
    {
        KMessageBox Message;
        Message.sorry(this, i18n("There can't be 2 SlideShows with the same name"), i18n("Error"), KMessageBox::Notify );

        item->setText( item->data( SlideShowNameData ).toString() );
        m_uiWidget.customSlideShowsList->editItem( item );
    }

    //Reconect the rename function.
    connect( m_uiWidget.customSlideShowsList, SIGNAL( itemChanged( QListWidgetItem*) ),
                this, SLOT( renameCustomSlideShow(QListWidgetItem*) ) );
}

void KPrCustomSlideShowsDialog::deleteCustomSlideShow()
{
    QListWidgetItem* item = m_uiWidget.customSlideShowsList->currentItem();
    //Use item, which points to current item, to calculate the row to
    //take it from the widget:
    m_uiWidget.customSlideShowsList->takeItem( m_uiWidget.customSlideShowsList->row(item) );
    if( item ){
        m_slideShows->remove( item->data(SlideShowNameData).toString() );
        delete item;
    }
}

void KPrCustomSlideShowsDialog::loadCustomSlideShowsData()
{
    //We disconect the signal so not to call the rename function over and over:
    disconnect( m_uiWidget.customSlideShowsList, SIGNAL( itemChanged( QListWidgetItem*) ),
                this, SLOT( renameCustomSlideShow(QListWidgetItem*) ) );

    //check if is our first load so to use the work already done
    if( !m_firstTime )
    {
        delete m_slideShows;
        m_slideShows = new KPrCustomSlideShows(*m_oldSlideShows);
        m_uiWidget.customSlideShowsList->clear();
    }
    m_firstTime = false;

    //build, configure and insert every Item:
    QListWidgetItem * item;
    foreach( QString slideShowName, m_oldSlideShows->names() )
    {
        item = new QListWidgetItem( slideShowName, m_uiWidget.customSlideShowsList );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setData( SlideShowNameData, QVariant(slideShowName) );
    }

    //Reconect the rename function.
    connect( m_uiWidget.customSlideShowsList, SIGNAL( itemChanged( QListWidgetItem*) ),
             this, SLOT( renameCustomSlideShow(QListWidgetItem*) ) );
}

void KPrCustomSlideShowsDialog::changedSelectedSlideshow(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous);
    m_uiWidget.currentSlidesList->clear();

    //if we have selected a slideShow we can safelly try to add slides to it
    m_uiWidget.addSlideButton->setEnabled( true );

    //get the slideShow and its pages
    m_selectedSlideShowName = current->data( SlideShowNameData ).toString();
    QList<KoPAPageBase*> pages( m_slideShows->getByName( m_selectedSlideShowName ) );

    //insert them into the current slideShow list
    QListWidgetItem * item;
    foreach( KoPAPageBase* page, pages )
    {
        item = new QListWidgetItem( QIcon(), i18n( "Slide %1", m_doc->pageIndex(page)+1 ), m_uiWidget.currentSlidesList );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        item->setData( SlideData, QVariant( page ) );
    }
}

void KPrCustomSlideShowsDialog::addSlidesToCurrentSlideShow()
{
    //get the selected items and slideshow
    QList<QListWidgetItem*> selectedPages( m_uiWidget.availableSlidesList->selectedItems() );
    QList<KoPAPageBase*> selectedSlideShow( m_slideShows->getByName( m_selectedSlideShowName ) );

    //insert the slides at the end and update the Widget
    foreach( QListWidgetItem* item, selectedPages )
    {
        KoPAPageBase* page( (item->data( SlideData ).value<KoPAPageBase*>()));
        selectedSlideShow.append( page );

        item = new QListWidgetItem( QIcon(), i18n("Slide %1", m_doc->pageIndex(page)+1 ), m_uiWidget.currentSlidesList );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        item->setData( SlideData, QVariant( page ) );
    }
    //update the SlidwShow with the resulting list
    m_slideShows->update( m_selectedSlideShowName, selectedSlideShow );
}

void KPrCustomSlideShowsDialog::removeSlidesFromCurrentSlideShow()
{
//     //get the selected items and slideshow
//     QList<QListWidgetItem*> selectedPages( m_uiWidget.currentSlidesList->selectedItems() );
//     QList<KoPAPageBase*> selectedSlideShow( m_slideShows->getByName( m_selectedSlideShowName ) );
//     KoPAPageBase* page;
//     int i;
// 
//     //remove the slides and update the widget
//     foreach( QListWidgetItem* item, selectedPages )
//     {
//         m_uiWidget.customSlideShowsList->takeItem( m_uiWidget.customSlideShowsList->row(item) );
//         i = selectedSlideShow.indexOf();
//         while( i != -1) {
//             selectedSlideShow.
//         }
//         delete item;
//     }
//     //update the selected SlideShow
//     m_slideShows->update( m_selectedSlideShowName, selectedSlideShow );
}

#include "KPrCustomSlideShowsDialog.moc"
