/* This file is part of the KDE project
   Copyright (C) 2005,2006 Jaroslaw Staniek <js@iidea.pl>

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

#include "kexicsvexportwizard.h"
#include "kexicsvwidgets.h"
#include <main/startup/KexiStartupFileDialog.h>
#include <kexidb/cursor.h>
#include <kexidb/utils.h>
#include <core/keximainwindow.h>
#include <core/kexiproject.h>
#include <core/kexipartinfo.h>
#include <core/kexipartmanager.h>
#include <core/kexiguimsghandler.h>
#include <kexiutils/utils.h>
#include <widget/kexicharencodingcombobox.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qclipboard.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kactivelabel.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ksavefile.h>

//-------------------------------------------------------------

KexiCSVExportWizard::Options::Options()
 : mode(File), itemId(0)
{
}

//-------------------------------------------------------------

KexiCSVExportWizard::KexiCSVExportWizard( const Options& options,
	KexiMainWindow* mainWin, QWidget * parent, const char * name )
 : KWizard(parent, name)
 , m_options(options)
// , m_mode(mode)
// , m_itemId(itemId)
 , m_mainWin(mainWin)
 , m_fileSavePage(0)
 , m_defaultsBtn(0)
 , m_rowCountDetermined(false)
 , m_cancelled(false)
{
	if (m_options.mode==Clipboard) {
		finishButton()->setText(i18n("Copy"));
		backButton()->hide();
	}
	else {
		finishButton()->setText(i18n("Export"));
	}
	helpButton()->hide();

	QString infoLblFromText;
	KexiGUIMessageHandler msgh(this);
	m_tableOrQuery = new KexiDB::TableOrQuerySchema(
		m_mainWin->project()->dbConnection(), m_options.itemId);
	if (m_tableOrQuery->table()) {
		if (m_options.mode==Clipboard) {
			setCaption(i18n("Copy Data From Table to Clipboard"));
			infoLblFromText = i18n("Copying data from table:");
		}
		else {
			setCaption(i18n("Export Data From Table to CSV File"));
			infoLblFromText = i18n("Exporting data from table:");
		}
	}
	else if (m_tableOrQuery->query()) {
		if (m_options.mode==Clipboard) {
			setCaption(i18n("Copy Data From Query to Clipboard"));
			infoLblFromText = i18n("Copying data from table:");
		}
		else {
			setCaption(i18n("Export Data From Query to CSV File"));
			infoLblFromText = i18n("Exporting data from query:");
		}
	}
	else {
		msgh.showErrorMessage(m_mainWin->project()->dbConnection(), 
			i18n("Could not open data for exporting."));
		m_cancelled = true;
		return;
	}
	// OK, source data found.

	// Setup pages

	// 1. File Save Page
	if (m_options.mode==File) {
		m_fileSavePage = new KexiStartupFileDialog(
			":CSVImportExport", //startDir
			KexiStartupFileDialog::Custom | KexiStartupFileDialog::SavingFileBasedDB,
			this, "m_fileSavePage");
		m_fileSavePage->setMinimumHeight(kapp->desktop()->height()/2);
		m_fileSavePage->setAdditionalFilters( csvMimeTypes() );
		m_fileSavePage->setDefaultExtension("csv");
		m_fileSavePage->setLocationText( KexiUtils::stringToFileName(m_tableOrQuery->captionOrName()) );
		connect(m_fileSavePage, SIGNAL(rejected()), this, SLOT(reject()));
		addPage(m_fileSavePage, i18n("Enter the Name of the File You Want To Save the Data To"));
	}

	// 2. Export options
	m_exportOptionsPage = new QWidget(this, "m_exportOptionsPage");
	QGridLayout *exportOptionsLyr = new QGridLayout( m_exportOptionsPage, 6, 3, 
		KDialogBase::marginHint(), KDialogBase::spacingHint(), "exportOptionsLyr");
	m_infoLblFrom = new KexiCSVInfoLabel( infoLblFromText, m_exportOptionsPage );
	KexiPart::Info *partInfo = Kexi::partManager().infoForMimeType(
		m_tableOrQuery->table() ? "kexi/table" : "kexi/query");
	if (partInfo)
		m_infoLblFrom->setIcon(partInfo->itemIcon());
	m_infoLblFrom->separator()->hide();
	exportOptionsLyr->addMultiCellWidget(m_infoLblFrom, 0, 0, 0, 2);

	m_infoLblTo = new KexiCSVInfoLabel(
		(m_options.mode==File) ? i18n("To CSV file:") : i18n("To clipboard:"),
		m_exportOptionsPage
	);
	if (m_options.mode==Clipboard)
		m_infoLblTo->setIcon("editpaste");
	exportOptionsLyr->addMultiCellWidget(m_infoLblTo, 1, 1, 0, 2);

	m_showOptionsButton = new KPushButton(KGuiItem(i18n("Show Options >>"), "configure"), 
		m_exportOptionsPage);
	connect(m_showOptionsButton, SIGNAL(clicked()), this, SLOT(slotShowOptionsButtonClicked()));
	exportOptionsLyr->addMultiCellWidget(m_showOptionsButton, 2, 2, 0, 0);
	m_showOptionsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	// -<options section>
	m_exportOptionsSection = new QGroupBox(1, Vertical, i18n("Options"), m_exportOptionsPage, 
		"m_exportOptionsSection");
	m_exportOptionsSection->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	exportOptionsLyr->addMultiCellWidget(m_exportOptionsSection, 3, 3, 0, 1);
	QWidget *exportOptionsSectionWidget 
		= new QWidget(m_exportOptionsSection, "exportOptionsSectionWidget");
	QGridLayout *exportOptionsSectionLyr = new QGridLayout( exportOptionsSectionWidget, 5, 2, 
		0, KDialogBase::spacingHint(), "exportOptionsLyr");

	// -delimiter
	m_delimiterWidget = new KexiCSVDelimiterWidget(false, //!lineEditOnBottom
		exportOptionsSectionWidget);
	m_delimiterWidget->setDelimiter(defaultDelimiter());
	exportOptionsSectionLyr->addWidget( m_delimiterWidget, 0, 1 );
	QLabel *delimiterLabel = new QLabel(m_delimiterWidget, i18n("Delimiter:"), exportOptionsSectionWidget);
	exportOptionsSectionLyr->addWidget( delimiterLabel, 0, 0 );

	// -text quote
	QWidget *textQuoteWidget = new QWidget(exportOptionsSectionWidget);
	QHBoxLayout *textQuoteLyr = new QHBoxLayout(textQuoteWidget);
	exportOptionsSectionLyr->addWidget(textQuoteWidget, 1, 1);
	m_textQuote = new KexiCSVTextQuoteComboBox( textQuoteWidget );
	m_textQuote->setTextQuote(defaultTextQuote());
	textQuoteLyr->addWidget( m_textQuote );
	textQuoteLyr->addStretch(0);
	QLabel *textQuoteLabel = new QLabel(m_textQuote, i18n("Text quote:"), exportOptionsSectionWidget);
	exportOptionsSectionLyr->addWidget( textQuoteLabel, 1, 0 );

	// - character encoding
	m_characterEncodingCombo = new KexiCharacterEncodingComboBox( exportOptionsSectionWidget );
	m_characterEncodingCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	exportOptionsSectionLyr->addWidget( m_characterEncodingCombo, 2, 1 );
	QLabel *characterEncodingLabel = new QLabel(m_characterEncodingCombo, i18n("Text encoding:"), 
		exportOptionsSectionWidget);
	exportOptionsSectionLyr->addWidget( characterEncodingLabel, 2, 0 );

	// - checkboxes
	m_addColumnNamesCheckBox = new QCheckBox(i18n("Add column names as the first row"), 
		exportOptionsSectionWidget);
	m_addColumnNamesCheckBox->setChecked(true);
	exportOptionsSectionLyr->addWidget( m_addColumnNamesCheckBox, 3, 1 );
//! @todo 1.1: for copying use "Always use above options for copying" string
	m_alwaysUseCheckBox = new QCheckBox(i18n("Always use above options for exporting"), 
		m_exportOptionsPage);
	exportOptionsLyr->addMultiCellWidget(m_alwaysUseCheckBox, 4, 4, 0, 1);
//	exportOptionsSectionLyr->addWidget( m_alwaysUseCheckBox, 4, 1 );
	m_exportOptionsSection->hide();
	m_alwaysUseCheckBox->hide();
	// -</options section>

//	exportOptionsLyr->setColStretch(3, 1);
	exportOptionsLyr->addMultiCell( 
		new QSpacerItem( 0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding), 5, 5, 0, 1 );

//	addPage(m_exportOptionsPage, i18n("Set Export Options"));
	addPage(m_exportOptionsPage, m_options.mode==Clipboard ? i18n("Copying") : i18n("Exporting"));
	setFinishEnabled(m_exportOptionsPage, true);

	// load settings
	kapp->config()->setGroup("ImportExport");
	if (m_options.mode!=Clipboard && readBoolEntry("ShowOptionsInCSVExportDialog", false)) {
		show();
		slotShowOptionsButtonClicked();
	}
	if (readBoolEntry("StoreOptionsForCSVExportDialog", false)) {
		// load defaults:
		m_alwaysUseCheckBox->setChecked(true);
		QString s = readEntry("DefaultDelimiterForExportingCSVFiles", defaultDelimiter());
		if (!s.isEmpty())
			m_delimiterWidget->setDelimiter(s);
		s = readEntry("DefaultTextQuoteForExportingCSVFiles", defaultTextQuote());
		m_textQuote->setTextQuote(s); //will be invaliudated here, so not a problem
		s = readEntry("DefaultEncodingForExportingCSVFiles");
		if (!s.isEmpty())
			m_characterEncodingCombo->setSelectedEncoding(s);
		m_addColumnNamesCheckBox->setChecked( 
			readBoolEntry("AddColumnNamesForExportingCSVFiles", true) );
	}

	updateGeometry();

	// -keep widths equal on page #2:
	int width = QMAX( m_infoLblFrom->leftLabel()->sizeHint().width(), 
		m_infoLblTo->leftLabel()->sizeHint().width());
	m_infoLblFrom->leftLabel()->setFixedWidth(width);
	m_infoLblTo->leftLabel()->setFixedWidth(width);
}

KexiCSVExportWizard::~KexiCSVExportWizard()
{
	delete m_tableOrQuery;
}

bool KexiCSVExportWizard::cancelled() const
{
	return m_cancelled;
}

void KexiCSVExportWizard::showPage ( QWidget * page ) 
{
	if (page == m_fileSavePage) {
		m_fileSavePage->setFocus();
	}
	else if (page==m_exportOptionsPage) {
		if (m_options.mode==File)
			m_infoLblTo->setFileName( m_fileSavePage->currentFileName() );
		QString text = m_tableOrQuery->captionOrName();
		if (!m_rowCountDetermined) {
			//do this costly operation only once
			m_rowCount = KexiDB::rowCount(*m_tableOrQuery);
			m_rowCountDetermined = true;
		}
		int columns = KexiDB::fieldCount(*m_tableOrQuery);
		text += "\n";
		if (m_rowCount>0)
			text += i18n("(rows: %1, columns: %2)").arg(m_rowCount).arg(columns);
		else
			text += i18n("(columns: %1)").arg(columns);
		m_infoLblFrom->setLabelText(text);
		QFontMetrics fm(m_infoLblFrom->fileNameLabel()->font());
		m_infoLblFrom->fileNameLabel()->setFixedHeight( fm.height() * 2 + fm.lineSpacing() );
		if (m_defaultsBtn)
			m_defaultsBtn->show();
	}

	if (page!=m_exportOptionsPage) {
		if (m_defaultsBtn)
			m_defaultsBtn->hide();
	}

	KWizard::showPage(page);
}

void KexiCSVExportWizard::next()
{
	if (currentPage() == m_fileSavePage) {
		if (!m_fileSavePage->checkFileName())
			return;
		KWizard::next();
		finishButton()->setFocus();
		return;
	}
	KWizard::next();
}

void KexiCSVExportWizard::done(int result)
{
	if (QDialog::Accepted == result) {
		
		if (!exportData())
			return;
	}
	else if (QDialog::Rejected == result) {
		//nothing to do
	}

	//store options
	kapp->config()->setGroup("ImportExport");
	if (m_options.mode!=Clipboard)
		writeEntry("ShowOptionsInCSVExportDialog", m_exportOptionsSection->isVisible());
	const bool store = m_alwaysUseCheckBox->isChecked();
	writeEntry("StoreOptionsForCSVExportDialog", store);
	// only save if an option differs from default

	if (store && m_delimiterWidget->delimiter()!=defaultDelimiter())
		writeEntry("DefaultDelimiterForExportingCSVFiles", m_delimiterWidget->delimiter());
	else
		deleteEntry("DefaultDelimiterForExportingCSVFiles");
	if (store && m_textQuote->textQuote()!=defaultTextQuote())
		writeEntry("DefaultTextQuoteForExportingCSVFiles", m_textQuote->textQuote());
	else
		deleteEntry("DefaultTextQuoteForExportingCSVFiles");
	if (store && !m_characterEncodingCombo->defaultEncodingSelected())
		writeEntry("DefaultEncodingForExportingCSVFiles", m_characterEncodingCombo->selectedEncoding());
	else
		deleteEntry("DefaultEncodingForExportingCSVFiles");
	if (store && !m_addColumnNamesCheckBox->isChecked())
		writeEntry("AddColumnNamesForExportingCSVFiles", m_addColumnNamesCheckBox->isChecked());
	else
		deleteEntry("AddColumnNamesForExportingCSVFiles");

	KWizard::done(result);
}

void KexiCSVExportWizard::slotShowOptionsButtonClicked()
{
	if (m_exportOptionsSection->isVisible()) {
		m_showOptionsButton->setText(i18n("Show Options >>"));
		m_exportOptionsSection->hide();
		m_alwaysUseCheckBox->hide();
		if (m_defaultsBtn)
			m_defaultsBtn->hide();
	}
	else {
		m_showOptionsButton->setText(i18n("Hide Options <<"));
		m_exportOptionsSection->show();
		m_alwaysUseCheckBox->show();
		if (m_defaultsBtn)
			m_defaultsBtn->show();
	}
}

void KexiCSVExportWizard::layOutButtonRow( QHBoxLayout * layout )
{
	QWizard::layOutButtonRow( layout );

	//find the last sublayout
	QLayout *l = 0;
	for (QLayoutIterator lit( layout->iterator() ); lit.current(); ++lit)
		l = lit.current()->layout();
	if (dynamic_cast<QBoxLayout*>(l)) {
		if (!m_defaultsBtn) {
			m_defaultsBtn = new KPushButton(i18n("Defaults"), this);
			QWidget::setTabOrder(backButton(), m_defaultsBtn);
			connect(m_defaultsBtn, SIGNAL(clicked()), this, SLOT(slotDefaultsButtonClicked()));
		}
		if (!m_exportOptionsSection->isVisible())
			m_defaultsBtn->hide();
		dynamic_cast<QBoxLayout*>(l)->insertWidget(0, m_defaultsBtn);
	}
}

void KexiCSVExportWizard::slotDefaultsButtonClicked()
{
	m_delimiterWidget->setDelimiter(defaultDelimiter());
	m_textQuote->setTextQuote(defaultTextQuote());
	m_addColumnNamesCheckBox->setChecked(true);
	m_characterEncodingCombo->selectDefaultEncoding();
}

bool KexiCSVExportWizard::exportData()
{
	KexiDB::Connection *conn = m_mainWin->project()->dbConnection();

//! @todo move this to non-GUI location so it can be also used via command line
//! @todo add a "finish" page with a progressbar.
//! @todo look at m_rowCount whether the data is really large; 
//!       if so: avoid copying to clipboard (or ask user) because of system memory

//! @todo OPTIMIZATION: use fieldsExpanded(true /*UNIQUE*/)
//! @todo OPTIMIZATION? (avoid multiple data retrieving) look for already fetched data within KexiProject..

	KexiDB::QuerySchema* query = m_tableOrQuery->query();
	if (!query)
		query = m_tableOrQuery->table()->query();

	KexiDB::QueryColumnInfo::Vector fields( query->fieldsExpanded() );
	QString buffer;

	KSaveFile *kSaveFile = 0;
	QTextStream *stream = 0;

	const bool copyToClipboard = m_options.mode==Clipboard;
	if (copyToClipboard) {
//! @todo (during exporting): enlarge bufSize by factor of 2 when it became too small
		uint bufSize = QMIN((m_rowCount<0 ? 10 : m_rowCount) * fields.count() * 20, 128000);
		buffer.reserve( bufSize );
		if (buffer.capacity() < bufSize) {
			kdWarning() << "KexiCSVExportWizard::exportData() cannot allocate memory for " << bufSize 
				<< " characters" << endl;
			return false;
		}
	}
	else {
		QString fname( m_fileSavePage->currentFileName() );
		if (fname.isEmpty()) {//sanity
			kdWarning() << "KexiCSVExportWizard::exportData(): fname is empty" << endl;
			return false;
		}
		kSaveFile = new KSaveFile(m_fileSavePage->currentFileName());
		if (0 == kSaveFile->status())
			stream = kSaveFile->textStream();
		if (0 != kSaveFile->status() || !stream) {//sanity
			kdWarning() << "KexiCSVExportWizard::exportData(): status != 0 or stream == 0" << endl;
			delete kSaveFile;
			return false;
		}
	}

//! @todo escape strings

#define _ERR \
	delete [] isText; \
	if (kSaveFile) { kSaveFile->abort(); delete kSaveFile; } \
	return false

#define APPEND(what) \
		if (copyToClipboard) buffer.append(what); else (*stream) << (what)

// line endings should be as in RFC 4180
#define CSV_EOLN "\r\n"

	// 0. Cache information
	const uint fieldsCount = fields.count();
	const QCString delimiter( m_delimiterWidget->delimiter().left(1).latin1() );
	const bool hasTextQuote = !m_textQuote->textQuote().isEmpty();
	const QCString textQuote( m_textQuote->textQuote().left(1).latin1() );
	const QCString escapedTextQuote( textQuote + textQuote );
	//cache for faster checks
	bool *isText = new bool[fieldsCount]; 
	bool *isDateTime = new bool[fieldsCount]; 
//	bool isInteger[fieldsCount]; //cache for faster checks
//	bool isFloatingPoint[fieldsCount]; //cache for faster checks
	for (uint i=0; i<fieldsCount; i++) {
		isText[i] = fields[i]->field->isTextType();
		isDateTime[i] = fields[i]->field->type()==KexiDB::Field::DateTime;
//		isInteger[i] = fields[i]->field->isIntegerType() 
//			|| fields[i]->field->type()==KexiDB::Field::Boolean;
//		isFloatingPoint[i] = fields[i]->field->isFPNumericType();
	}

	// 1. Output column names
	if (m_addColumnNamesCheckBox->isChecked()) {
		for (uint i=0; i<fieldsCount; i++) {
			if (i>0)
				APPEND( delimiter );
			if (hasTextQuote){
				APPEND( textQuote );
				APPEND( fields[i]->captionOrAliasOrName().replace(textQuote, escapedTextQuote) );
				APPEND( textQuote );
			}
			else {
				APPEND( fields[i]->captionOrAliasOrName() );
			}
		}
		APPEND(CSV_EOLN);
	}
	
	KexiGUIMessageHandler handler;
	KexiDB::Cursor *cursor = conn->executeQuery(*query);
	if (!cursor) {
		handler.showErrorMessage(conn);
		_ERR;
	}
	for (cursor->moveFirst(); !cursor->eof() && !cursor->error(); cursor->moveNext()) {
		const uint realFieldCount = QMIN(cursor->fieldCount(), fieldsCount);
		for (uint i=0; i<realFieldCount; i++) {
			if (i>0)
				APPEND( delimiter );
			if (cursor->value(i).isNull())
				continue;
			if (hasTextQuote && isText[i]) {
				APPEND( textQuote );
				APPEND( QString(cursor->value(i).toString()).replace(textQuote, escapedTextQuote) );
				APPEND( textQuote );
			}
			else if (isDateTime[i]) { //avoid "T" in ISO DateTime
				APPEND( cursor->value(i).toDateTime().date().toString(Qt::ISODate)+" "
					+ cursor->value(i).toDateTime().time().toString(Qt::ISODate) );
			}
			else {
				APPEND( cursor->value(i).toString() );
			}
		}
		APPEND(CSV_EOLN);
	}

	if (copyToClipboard)
		buffer.squeeze();

	if (!conn->deleteCursor(cursor)) {
		handler.showErrorMessage(conn);
		_ERR;
	}

	if (copyToClipboard)
		kapp->clipboard()->setText(buffer, QClipboard::Clipboard);

	delete [] isText;

	if (kSaveFile) {
		if (!kSaveFile->close()) {
			kdWarning() << "KexiCSVExportWizard::exportData(): error close(); status == " 
				<< kSaveFile->status() << endl;
		}
		delete kSaveFile;
	}
	return true;
}

static QString convertKey(const char *key, KexiCSVExportWizard::Mode mode)
{
	QString _key(QString::fromLatin1(key));
	if (mode == KexiCSVExportWizard::Clipboard) {
		_key.replace("Exporting", "Copying");
		_key.replace("Export", "Copy");
		_key.replace("CSVFiles", "CSVToClipboard");
	}
	return _key;
}

bool KexiCSVExportWizard::readBoolEntry(const char *key, bool defaultValue)
{
	return kapp->config()->readBoolEntry(convertKey(key, m_options.mode), defaultValue);
}

QString KexiCSVExportWizard::readEntry(const char *key, const QString& defaultValue)
{
	return kapp->config()->readEntry(convertKey(key, m_options.mode), defaultValue);
}

void KexiCSVExportWizard::writeEntry(const char *key, const QString& value)
{
	kapp->config()->writeEntry(convertKey(key, m_options.mode), value);
}

void KexiCSVExportWizard::writeEntry(const char *key, bool value)
{
	kapp->config()->writeEntry(convertKey(key, m_options.mode), value);
}

void KexiCSVExportWizard::deleteEntry(const char *key)
{
	kapp->config()->deleteEntry(convertKey(key, m_options.mode));
}

QString KexiCSVExportWizard::defaultDelimiter() const
{
	if (m_options.mode==Clipboard) {
		if (!m_options.forceDelimiter.isEmpty())
			return m_options.forceDelimiter;
		else
			return KEXICSV_DEFAULT_CLIPBOARD_DELIMITER;
	}
	return KEXICSV_DEFAULT_FILE_DELIMITER;
}

QString KexiCSVExportWizard::defaultTextQuote() const
{
	if (m_options.mode==Clipboard)
		return QString::null;
	return KEXICSV_DEFAULT_TEXT_QUOTE;
}

#include "kexicsvexportwizard.moc"
