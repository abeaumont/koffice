/*
   $Id$
   This file is part of the KDE project
   Copyright (C) 2001 Daniel Naber <daniel.naber@t-online.de>
   This is a thesaurus based on a subset of WordNet. It also offers a 
   mostly-complete WordNet 1.7 frontend (WordNet is a powerful lexical 
   database/thesaurus)
*/
/***************************************************************************
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

/*
FIXME:
-Fix "no mimesource" warning of QTextBrowser
-click in thesaurus doesn't always select immediately
-See the fixme's in the source below

TODO:
-Add back/forwar buttons
-Don't start WordNet before its tab is activated
-Move the edit field outside the tabs and make it work for both
-Help link about WordNet
-Also "Replace" with text selection in m_resultbox?
-Make dialog non-modal???
-Be more verbose if the result is empty
-Better table layout (no empty space on top)
-Function words (like "if" etc) are not part of WordNet. There  are not *that*
 many of them, so maybe we should add them. See search tools' stop word list for
 function words.
-Don't forget to insert comments for the translators where necessary
 (because WordNet is English language only)
*/

#include "main.h"

/***************************************************
 *
 * Factory
 *
 ***************************************************/

K_EXPORT_COMPONENT_FACTORY( libthesaurustool, KGenericFactory<Thesaurus> );

/***************************************************
 *
 * Thesaurus *
 ***************************************************/

Thesaurus::Thesaurus(QObject* parent, const char* name, const QStringList &)
    : KDataTool(parent, name)
{
    
    m_tab = new QTabDialog();
    m_tab->setOkButton(i18n("Replace"));
    m_tab->setCancelButton();
    m_tab->resize(500, 350);
    m_tab->setCaption(i18n("KWord Thesaurus & WordNet"));

    //
    // Thesaurus Tab
    //

    vbox = new QVBox(m_tab);
    vbox->setMargin(5);        // fixme: use global value?
    vbox->setSpacing(5);
    
    m_thes_edit = new KHistoryCombo(vbox);
    
    QHBox *hbox = new QHBox(vbox);
    hbox->setSpacing(5);

    vbox_syn = new QVBox(hbox);
    (void) new QLabel(i18n("Synonyms"), vbox_syn);
    m_thes_syn = new QListBox(vbox_syn);
    
    vbox_hyper = new QVBox(hbox);
    (void) new QLabel(i18n("More general words"), vbox_hyper);
    m_thes_hyper = new QListBox(vbox_hyper);
    
    vbox_hypo = new QVBox(hbox);
    (void) new QLabel(i18n("More specific words"), vbox_hypo);
    m_thes_hypo = new QListBox(vbox_hypo);

    // double click:
    connect(m_thes_syn, SIGNAL(selected(const QString &)),
        this, SLOT(slotFindTerm(const QString &)));
    connect(m_thes_hyper, SIGNAL(selected(const QString &)),
        this, SLOT(slotFindTerm(const QString &)));
    connect(m_thes_hypo, SIGNAL(selected(const QString &)),
        this, SLOT(slotFindTerm(const QString &)));

    // we need to keep the "Replace" button unambiguous, so only select one item at one:
    /* we currently use the m_thes_edit anyway to get the replacement
    connect(m_thes_syn, SIGNAL(selectionChanged()), m_thes_hyper, SLOT(clearSelection()));
    connect(m_thes_syn, SIGNAL(selectionChanged()), m_thes_hypo, SLOT(clearSelection()));
    connect(m_thes_hyper, SIGNAL(selectionChanged()), m_thes_syn, SLOT(clearSelection()));
    connect(m_thes_hyper, SIGNAL(selectionChanged()), m_thes_hypo, SLOT(clearSelection()));
    connect(m_thes_hypo, SIGNAL(selectionChanged()), m_thes_syn, SLOT(clearSelection()));
    connect(m_thes_hypo, SIGNAL(selectionChanged()), m_thes_hyper, SLOT(clearSelection()));
    */
    
    m_tab->insertTab(vbox, i18n("Thesaurus"));

    //
    // WordNet Tab
    //
    
    vbox2 = new QVBox(m_tab);
    vbox2->setMargin(5);        // fixme: use global values?
    vbox2->setSpacing(5);
    
    m_edit = new KHistoryCombo(vbox2);

    m_combobox = new QComboBox(vbox2);
    m_combobox->setEditable(false);
    connect(m_combobox, SIGNAL(activated(int)), this, SLOT(slotFindTerm()));

    m_resultbox = new QTextBrowser(vbox2);
    m_resultbox->setTextFormat(Qt::RichText);
    // fixme?: m_resultbox->setMimeSourceFactory(...);
    connect(m_resultbox, SIGNAL(linkClicked(const QString &)),
        this, SLOT(slotFindTerm(const QString &)));

    m_tab->insertTab(vbox2, i18n("WordNet"));

    //
    // Two history boxes. TODO: move out of tab, so we only need one.
    //
    // Do not use Return as default key...
    m_thes_edit->setTrapReturnKey(true);
    // ...but search term when return is pressed or old term is selected:
    connect(m_thes_edit, SIGNAL(returnPressed(const QString&)), this, SLOT(slotFindTerm(const QString&)));
    connect(m_thes_edit, SIGNAL(activated(const QString &)), this, SLOT(slotFindTerm(const QString &)));
    // remember input:
    connect(m_thes_edit, SIGNAL(returnPressed(const QString&)), m_edit, SLOT(addToHistory(const QString&)));
    connect(m_thes_edit, SIGNAL(returnPressed(const QString&)), m_thes_edit, SLOT(addToHistory(const QString&)));

    // same for the other combo box:
    m_edit->setTrapReturnKey(true);
    connect(m_edit, SIGNAL(returnPressed(const QString&)), this, SLOT(slotFindTerm(const QString&)));
    connect(m_edit, SIGNAL(activated(const QString &)), this, SLOT(slotFindTerm(const QString &)));
    connect(m_edit, SIGNAL(returnPressed(const QString&)), m_edit, SLOT(addToHistory(const QString&)));
    connect(m_edit, SIGNAL(returnPressed(const QString&)), m_thes_edit, SLOT(addToHistory(const QString&)));

    //
    // The external command stuff
    //
    m_wnproc = new KProcess;
    connect(m_wnproc, SIGNAL(processExited(KProcess*)),
        this, SLOT(wnExited(KProcess*)));
    connect(m_wnproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
        this, SLOT(receivedWnStdout(KProcess*, char*, int)));
    connect(m_wnproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
        this, SLOT(receivedWnStderr(KProcess*, char*, int)));

    m_thesproc = new KProcess;
    connect(m_thesproc, SIGNAL(processExited(KProcess*)),
        this, SLOT(thesExited(KProcess*)));
    connect(m_thesproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
        this, SLOT(receivedThesStdout(KProcess*, char*, int)));
    connect(m_thesproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
        this, SLOT(receivedThesStderr(KProcess*, char*, int)));

}


Thesaurus::~Thesaurus()
{
    if( m_tab ) {
        delete m_tab;
    }
    if( m_thesproc ) {
        delete m_thesproc;
    }
    if( m_wnproc ) {
        delete m_wnproc;
    }
    // TODO: can we ignore the others because they are children of m_tab etc?
}


bool Thesaurus::run(const QString& command, void* data, const QString& datatype, const QString& mimetype)
{

    if ( command != "thesaurus" )
    {
        kdDebug(31000) << "Thesaurus does only accept the command 'thesaurus'" << endl;
        kdDebug(31000) << "   The command " << command << " is not accepted" << endl;
        return FALSE;
    }
    // Check whether we can accept the data
    if ( datatype != "QString" )
    {
        kdDebug(31000) << "Thesaurus only accepts datatype QString" << endl;
        return FALSE;
    }
    if ( mimetype != "text/plain" )
    {
        kdDebug(31000) << "Thesaurus only accepts mimetype text/plain" << endl;
        return FALSE;
    }

    // Get data and clean it up:
    QString buffer = *((QString *)data);
    buffer = buffer.stripWhiteSpace();
    QRegExp re("[.,;!?\"'()\\[\\]]");
    buffer.replace(re, "");
    m_edit->insertItem(buffer, 0);
    m_thes_edit->insertItem(buffer, 0);

    m_wnproc_stdout = "";
    m_wnproc_stderr = "";

    m_thesproc_stdout = "";
    m_thesproc_stderr = "";

    slotFindTerm(buffer);

    if( m_tab->exec() == QDialog::Accepted ) {    // Replace
        QString replace_text;
        replace_text = m_edit->currentText();
        *((QString*)data) = replace_text;
    }

    return TRUE;
}


// Triggered when Return is pressed.
void Thesaurus::slotFindTerm()
{
    findTerm(m_edit->currentText());
}


// Triggered when a link is clicked.
void Thesaurus::slotFindTerm(const QString &term)
{
    if( term.startsWith("http://") ) {
        (void) new KRun(KURL(term));
    } else {
        m_edit->insertItem(term, 0);
        m_edit->setCurrentItem(0);
        m_thes_edit->insertItem(term, 0);
        m_thes_edit->setCurrentItem(0);
        findTerm(term);
    }
}

void Thesaurus::findTerm(const QString &term)
{
    findTermThesaurus(term);
    findTermWordnet(term);
}


//
// Thesaurus
//
void Thesaurus::findTermThesaurus(const QString &term)
{
    QApplication::setOverrideCursor(KCursor::waitCursor());

    m_thesproc_stdout = "";
    m_thesproc_stderr = "";
    
    // Find oly whole words. Looks clumsy, but this way we don't have to rely on
    // features that might only be in certain versions of grep:
    QString term_tmp = ";" + term + ";";
    m_thesproc->clearArguments();
    *m_thesproc << "grep" << "-i" << term_tmp;
    *m_thesproc << KGlobal::dirs()->findResourceDir("data", "thesaurus/")
        + "thesaurus/thesaurus.txt";

    if( !m_thesproc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) {
        KMessageBox::error(0, i18n("<b>Error:</b> Failed to execute grep."));
        m_thes_edit->setEnabled(false);
        QApplication::restoreOverrideCursor();
        return;
    }
}

void Thesaurus::thesExited(KProcess *)
{

    if( !m_thesproc_stderr.isEmpty() ) {
        KMessageBox::error(0, i18n("<b>Error:</b> Failed to execute grep. "
          "Output:<br>%1").arg(m_thesproc_stderr));
        QApplication::restoreOverrideCursor();
        return;
    }

    QString search_term = m_thes_edit->currentText();
    
    QStringList syn;
    QStringList hyper;
    QStringList hypo;

    QStringList lines = lines.split(QRegExp("\n"), m_thesproc_stdout, false);
    for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
        QString line = (*it);
        int sep_pos = line.find("#");
        QString syn_part = line.left(sep_pos);
        QString hyper_part = line.right(line.length()-sep_pos-1);
        QStringList syn_tmp = QStringList::split(QRegExp(";"), syn_part);
        QStringList hyper_tmp = QStringList::split(QRegExp(";"), hyper_part);
        if( syn_tmp.grep(search_term, false).size() > 0 ) {
            // match on the left side of the '#' -- synonyms
            for ( QStringList::Iterator it2 = syn_tmp.begin(); it2 != syn_tmp.end(); ++it2 ) {
                // add if it's not the term itself and if it's not yet in the list
                QString term = (*it2);
                if( term.lower() != search_term.lower() && syn.contains(term) == 0 ) {
                    syn.append(term);
                }
            }
            for ( QStringList::Iterator it2 = hyper_tmp.begin(); it2 != hyper_tmp.end(); ++it2 ) {
                QString term = (*it2);
                if( term.lower() != search_term.lower() && hyper.contains(term) == 0 ) {
                    hyper.append(term);
                }
            }
        }
        if( hyper_tmp.grep(search_term, false).size() > 0 ) {
            // match on the right side of the '#' -- hypernyms
            for ( QStringList::Iterator it2 = syn_tmp.begin(); it2 != syn_tmp.end(); ++it2 ) {
                QString term = (*it2);
                if( term.lower() != search_term && hypo.contains(term) == 0 ) {
                    hypo.append(term);
                }
            }
        }
    }

    m_thes_syn->clear();
    if( syn.size() > 0 ) {
        m_thes_syn->insertStringList(syn);
    } else {
        m_thes_syn->insertItem(i18n("No match"));
    }
    m_thes_syn->sort();
    
    m_thes_hyper->clear();
    if( hyper.size() > 0 ) {
        m_thes_hyper->insertStringList(hyper);
    } else {
        m_thes_hyper->insertItem(i18n("No match"));
    }
    m_thes_hyper->sort();

    m_thes_hypo->clear();
    if( hypo.size() > 0 ) {
        m_thes_hypo->insertStringList(hypo);
    } else {
        m_thes_hypo->insertItem(i18n("No match"));
    }
    m_thes_hypo->sort();

    QApplication::restoreOverrideCursor();
}

void Thesaurus::receivedThesStdout(KProcess *, char *result, int len)
{
    m_thesproc_stdout += QString::fromLocal8Bit( QCString(result, len+1) );
}

void Thesaurus::receivedThesStderr(KProcess *, char *result, int len)
{
    m_thesproc_stderr += QString::fromLocal8Bit( QCString(result, len+1) );
}


//
// WordNet
//
void Thesaurus::findTermWordnet(const QString &term)
{
    QApplication::setOverrideCursor(KCursor::waitCursor());

    m_wnproc_stdout = "";
    m_wnproc_stderr = "";
    
    m_wnproc->clearArguments();
    *m_wnproc << "wn";
    *m_wnproc << term;

    // get all results: nouns, verbs, adjectives, adverbs (see below for order):
    if( m_combobox->currentItem() == 0 ) {
        *m_wnproc << "-synsn" << "-synsv" << "-synsa" << "-synsr";
        m_mode = other;
    } else if( m_combobox->currentItem() == 1 ) {
        *m_wnproc << "-simsv";
        m_mode = other;
    } else if( m_combobox->currentItem() == 2 ) {
        *m_wnproc << "-antsn" << "-antsv" << "-antsa" << "-antsr";
        m_mode = other;
    } else if( m_combobox->currentItem() == 3 ) {
        *m_wnproc << "-hypon" << "-hypov";
        m_mode = other;
    } else if( m_combobox->currentItem() == 4 ) {
        *m_wnproc << "-meron";
        m_mode = other;
    } else if( m_combobox->currentItem() == 5 ) {
        *m_wnproc << "-holon";
        m_mode = other;
    } else if( m_combobox->currentItem() == 6 ) {
        // e.g. "kill -> die"
        *m_wnproc << "-causv";
        m_mode = other;
    } else if( m_combobox->currentItem() == 7) {
        // e.g. "walk -> step"
        *m_wnproc << "-entav";
        m_mode = other;
    } else if( m_combobox->currentItem() == 8) {
        *m_wnproc << "-famln" << "-famlv" << "-famla" << "-famlr";
        m_mode = other;
    } else if( m_combobox->currentItem() == 9) {
        *m_wnproc << "-framv";
        m_mode = other;
    } else if( m_combobox->currentItem() == 10 ) {
        *m_wnproc << "-grepn" << "-grepv" << "-grepa" << "-grepr";
        m_mode = grep;
    } else if( m_combobox->currentItem() == 11 ) {
        *m_wnproc << "-over";
        m_mode = other;
    }
    *m_wnproc << "-g";    // "Display gloss"

    int current = m_combobox->currentItem();    // remember current position
    m_combobox->clear();
    // 0:    
    m_combobox->insertItem(i18n("Synonyms/Hypernyms - ordered by frequency"));
    m_combobox->insertItem(i18n("Synonyms - ordered by similariy of meaning (verbs only)"));
    m_combobox->insertItem(i18n("Antonyms - words with opposite meanings"));
    m_combobox->insertItem(i18n("Hyponyms - ...is a (kind of) %1").arg(m_edit->currentText()));
    m_combobox->insertItem(i18n("Meroyms - %1 has a...").arg(m_edit->currentText()));
    // 5:
    m_combobox->insertItem(i18n("Holonyms - ...has a %1").arg(m_edit->currentText()));
    m_combobox->insertItem(i18n("Cause to (for some verbs only)"));
    m_combobox->insertItem(i18n("Verb Entailment (for some verbs only)"));
    m_combobox->insertItem(i18n("Familiarity & Polysemy count"));
    m_combobox->insertItem(i18n("Verb Frames (examples of use)"));
    // 10:
    m_combobox->insertItem(i18n("List of Compound Words"));
    m_combobox->insertItem(i18n("Overview of senses"));

    /** NOT todo:
      * -Hypernym tree: layout is difficult, you can get the same information
      *  by following links
      * -Coordinate terms (sisters): just go to synset and then use hyponyms
      * -Has Part Meronyms, Has Substance Meronyms, Has Member Meronyms,
      *  Member of Holonyms, Substance of Holonyms, Part of Holonyms:
      *  these are just subsets of Meronyms/Holonyms
      * -hmern, hholn: these are just compact versions, you can get the
      *  same information by following some links
      */

    /** TODO?:
      * -attr
      * -pert
      * -nomn(n|v), e.g. deny -> denial, but this doesn't seem to work?
      */

    m_combobox->setCurrentItem(current);    // reset previous position

    if( m_wnproc->isRunning() ) {
        // should never happen
        kdDebug(31000) << "Warning: findTerm(): process is already running?!" << endl;
        QApplication::restoreOverrideCursor();
        return;
    }

    if( !m_wnproc->start(KProcess::NotifyOnExit, KProcess::AllOutput) ) {
        m_resultbox->setText(i18n("<b>Error:</b> Failed to execute WordNet program 'wn'. "
            "WordNet has to be installed on your computer if you want to use it, "
            "and 'wn' has to be in your PATH. "
            "You can get WordNet at <a href=\"http://www.cogsci.princeton.edu/~wn/\">"
            "http://www.cogsci.princeton.edu/~wn/</a>. Note that WordNet only supports "
            "the English language."));
        m_edit->setEnabled(false);
        m_combobox->setEnabled(false);
        QApplication::restoreOverrideCursor();
        return;
    }

}

void Thesaurus::wnExited(KProcess *)
{
    
    if( !m_wnproc_stderr.isEmpty() ) {
        m_resultbox->setText(i18n("<b>Error:</b> Failed to execute WordNet program 'wn'. "
          "Output:<br>%1").arg(m_wnproc_stderr));
        QApplication::restoreOverrideCursor();
        return;
    }

    if( m_wnproc_stdout.isEmpty() ) {
        m_resultbox->setText(i18n("No match for '%1'.").arg(m_edit->currentText()));
    } else {
        // render in a table, each line one row:
        QStringList lines = lines.split(QRegExp("\n"), m_wnproc_stdout, false);
        QString result = "<qt><table>\n";
        // TODO: try without the following line (it's necessary to ensure the
        // first column is really always quite small):
        result += "<tr><td width=\"10%\"></td><td width=\"90%\"></td></tr>\n";
        uint ct = 0;
        for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
            QString l = (*it);
            // Remove some lines:
            QRegExp re("^\\d+( of \\d+)? senses? of \\w+");
            if( re.search(l) != -1 ) {
                continue;
            }
            // Escape XML:
            l = l.replace(QRegExp("<"), "&lt;");
            l = l.replace(QRegExp(">"), "&gt;");
            // TODO: 
            // move "=>" in own column?
            l = formatLine(l);
            // Table layout:
            result += "<tr>";
            if( l.startsWith(" ") ) {
                result += "\t<td width=\"15\"></td>";
                l = l.stripWhiteSpace();
                result += "\t<td>" + l + "</td>";
            } else {
                l = l.stripWhiteSpace();
                result += "<td colspan=\"2\">" + l + "</td>";
            }
            result += "</tr>\n";
            ct++;
        }
        result += "\n</table></qt>\n";
        m_resultbox->setText(result);
        m_resultbox->setContentsPos(0,0);
        //kdDebug() << result << endl;
    }
    
    QApplication::restoreOverrideCursor();
}

void Thesaurus::receivedWnStdout(KProcess *, char *result, int len)
{
    m_wnproc_stdout += QString::fromLocal8Bit( QCString(result, len+1) );
}

void Thesaurus::receivedWnStderr(KProcess *, char *result, int len)
{
    m_wnproc_stderr += QString::fromLocal8Bit( QCString(result, len+1) );
}


//
// Tools
//

/** Format lines using Qt's simple richtext. */
QString Thesaurus::formatLine(QString l)
{
    
    if( l == "--------------" ) {
        return QString("<hr>");
    }
    
    QRegExp re;

    re.setPattern("^(\\d+\\.)(.*)$");
    if( re.search(l) != -1 ) {
        l = "<b>" +re.cap(1)+ "</b>" +re.cap(2);
        return l;
    } 

    re.setPattern("^.* of (noun|verb|adj|adv) .*");
    if( re.search(l) != -1 ) {
        l = "<font size=\"5\">" +re.cap()+ "</font>\n\n";
        return l;
    } 

    if( m_mode == grep ) {
        l = l.stripWhiteSpace();
        return QString("<a href=\"" +l+ "\">" +l+ "</a>");
    }

    re.setPattern("^(Sense \\d+)");
    if( re.search(l) != -1 ) {
        l = "<b>" +re.cap()+ "</b>\n";
        return l;
    }
    
    re.setPattern("(.*)(Also See-&gt;)(.*)");
    // Example: first sense of verb "keep"
    if( re.search(l) != -1 ) {
        l = re.cap(1);
        l += re.cap(2);
        QStringList links = links.split(QRegExp(";"), re.cap(3), false);
        for ( QStringList::Iterator it = links.begin(); it != links.end(); ++it ) {
            QString link = (*it);
            if( it != links.begin() ) {
                l += ", ";
            }
            link = link.stripWhiteSpace();
            link = link.replace(QRegExp("#\\d+"), "");
            l += "<a href=\"" +link+ "\">" +link+ "</a>";
        }
        l = " " + l;        // indent in table
    }

    re.setPattern("(.*)(=&gt;|HAS \\w+:|PART OF:)(.*) --");
    re.setMinimal(true);    // non-greedy
    if( re.search(l) != -1 ) {
        int dash_pos = l.find("--");
        QString line_end = l.mid(dash_pos+2, l.length()-dash_pos);
        l = re.cap(1);
        l += re.cap(2) + " ";
        QStringList links = links.split(QRegExp(","), re.cap(3), false);
        for ( QStringList::Iterator it = links.begin(); it != links.end(); ++it ) {
            QString link = (*it);
            if( it != links.begin() ) {
                l += ", ";
            }
            link = link.stripWhiteSpace();
            l += "<a href=\"" +link+ "\">" +link+ "</a>";
        }
        l += "<font color=\"#777777\">" +line_end+ "</font>";
        l = " " + l;        // indent in table
        return l;
    }
    re.setMinimal(false);    // greedy again

    return l;
}

#include "main.moc"
