/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TextTool.h"
#include "TextEditingPluginContainer.h"
#include "dialogs/CreateBookmark.h"
#include "dialogs/JumpOverview.h"
#include "dialogs/SimpleStyleWidget.h"
#include "dialogs/StylesWidget.h"
#include "dialogs/ParagraphSettingsDialog.h"
#include "dialogs/StyleManagerDialog.h"
#include "dialogs/InsertCharacter.h"
#include "dialogs/FontDia.h"
#include "dialogs/TableDialog.h"
#include "dialogs/ChangeConfigureDialog.h"
#include "dialogs/ChangeTrackingOptionsWidget.h"
#include "dialogs/SimpleTableWidget.h"
#include "commands/TextCutCommand.h"
#include "commands/TextPasteCommand.h"
#include "commands/ChangeListCommand.h"
#include "commands/ChangeListLevelCommand.h"
#include "commands/ListItemNumberingCommand.h"
#include "commands/ShowChangesCommand.h"
#include "commands/ChangeTrackedDeleteCommand.h"
#include "commands/DeleteCommand.h"

#include <KCanvasBase.h>
#include <KShapeController.h>
#include <KoColor.h>
#include <KShapeSelection.h>
#include <KShapeManager.h>
#include <KPointerEvent.h>
#include <KVariable.h>
#include <KColorBackground.h>
#include <KOdf.h>
#include <KoColorPopupAction.h>
#include <KTextDocumentLayout.h>
#include <KParagraphStyle.h>
#include <KTextEditingPlugin.h>
#include <KTextEditingRegistry.h>
#include <KInlineTextObjectManager.h>
#include <KStyleManager.h>
#include <KTextOdfSaveHelper.h>
#include <KTextDrag.h>
#include <KTextDocument.h>
#include <KoTextEditor.h>
#include <KoGlobal.h>
#include <KChangeTracker.h>
#include <KChangeTrackerElement.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>

#include <KDE/KDebug>
#include <KDE/KRun>
#include <KDE/KPageDialog>
#include <KDE/KStandardShortcut>
#include <KDE/KFontSizeAction>
#include <KDE/KFontChooser>
#include <KDE/KFontAction>
#include <KDE/KAction>
#include <KDE/KLocale>
#include <KDE/KStandardAction>
#include <KDE/KMimeType>
#include <KDE/KMessageBox>
#include <KDE/KUser>
#include <KDE/KTabWidget>

#include <KDocumentRdfBase.h>

#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QtGui/QMenu>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QToolTip>

static bool hit(const QKeySequence &input, KStandardShortcut::StandardShortcut shortcut)
{
    foreach (const QKeySequence & ks, KStandardShortcut::shortcut(shortcut).toList()) {
        if (input == ks)
            return true;
    }
    return false;
}

TextTool::TextTool(KCanvasBase *canvas)
        : KToolBase(canvas),
        m_textShape(0),
        m_textShapeData(0),
        m_changeTracker(0),
        m_allowActions(true),
        m_allowAddUndoCommand(true),
        m_trackChanges(false),
        m_allowResourceManagerUpdates(true),
        m_prevCursorPosition(-1),
        m_caretTimer(this),
        m_caretTimerState(true),
        m_currentCommand(0),
        m_currentCommandHasChildren(false),
        m_specialCharacterDocker(0),
        m_textTyping(false),
        m_textDeleting(false),
        m_changeTipTimer(this),
        m_changeTipCursorPos(0)
{
    setFlags(ToolHandleKeyEvents | ToolHandleShortcutOverride | ToolMouseTracking);

    m_actionFormatBold  = new KAction(KIcon("format-text-bold"), i18n("Bold"), this);
    addAction("format_bold", m_actionFormatBold);
    m_actionFormatBold->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionFormatBold->setCheckable(true);
    connect(m_actionFormatBold, SIGNAL(triggered(bool)), this, SLOT(bold(bool)));

    m_actionFormatItalic  = new KAction(KIcon("format-text-italic"), i18n("Italic"), this);
    addAction("format_italic", m_actionFormatItalic);
    m_actionFormatItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    m_actionFormatItalic->setCheckable(true);
    connect(m_actionFormatItalic, SIGNAL(triggered(bool)), this, SLOT(italic(bool)));

    m_actionFormatUnderline  = new KAction(KIcon("format-text-underline"), i18nc("Text formatting", "Underline"), this);
    addAction("format_underline", m_actionFormatUnderline);
    m_actionFormatUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    m_actionFormatUnderline->setCheckable(true);
    connect(m_actionFormatUnderline, SIGNAL(triggered(bool)), this, SLOT(underline(bool)));

    m_actionFormatStrikeOut  = new KAction(KIcon("format-text-strikethrough"), i18n("Strike Out"), this);
    addAction("format_strike", m_actionFormatStrikeOut);
    m_actionFormatStrikeOut->setCheckable(true);
    connect(m_actionFormatStrikeOut, SIGNAL(triggered(bool)), this, SLOT(strikeOut(bool)));

    QActionGroup *alignmentGroup = new QActionGroup(this);
    m_actionAlignLeft  = new KAction(KIcon("format-justify-left"), i18n("Align Left"), this);
    addAction("format_alignleft", m_actionAlignLeft);
    m_actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionAlignLeft->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignLeft);
    connect(m_actionAlignLeft, SIGNAL(triggered(bool)), this, SLOT(alignLeft()));

    m_actionAlignRight  = new KAction(KIcon("format-justify-right"), i18n("Align Right"), this);
    addAction("format_alignright", m_actionAlignRight);
    m_actionAlignRight->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignRight->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignRight);
    connect(m_actionAlignRight, SIGNAL(triggered(bool)), this, SLOT(alignRight()));

    m_actionAlignCenter  = new KAction(KIcon("format-justify-center"), i18n("Align Center"), this);
    addAction("format_aligncenter", m_actionAlignCenter);
    m_actionAlignCenter->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
    m_actionAlignCenter->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignCenter);
    connect(m_actionAlignCenter, SIGNAL(triggered(bool)), this, SLOT(alignCenter()));

    m_actionAlignBlock  = new KAction(KIcon("format-justify-fill"), i18n("Align Block"), this);
    addAction("format_alignblock", m_actionAlignBlock);
    m_actionAlignBlock->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignBlock->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignBlock);
    connect(m_actionAlignBlock, SIGNAL(triggered(bool)), this, SLOT(alignBlock()));

    m_actionFormatSuper = new KAction(KIcon("format-text-superscript"), i18n("Superscript"), this);
    addAction("format_super", m_actionFormatSuper);
    m_actionFormatSuper->setCheckable(true);
    connect(m_actionFormatSuper, SIGNAL(triggered(bool)), this, SLOT(superScript(bool)));

    m_actionFormatSub = new KAction(KIcon("format-text-subscript"), i18n("Subscript"), this);
    addAction("format_sub", m_actionFormatSub);
    m_actionFormatSub->setCheckable(true);
    connect(m_actionFormatSub, SIGNAL(triggered(bool)), this, SLOT(subScript(bool)));

    KAction *action = new KAction(
        KIcon(QApplication::isRightToLeft() ? "format-indent-less" : "format-indent-more"),
        i18n("Increase Indent"), this);
    addAction("format_increaseindent", action);
    connect(action, SIGNAL(triggered()), this, SLOT(increaseIndent()));

    m_actionFormatDecreaseIndent = new KAction(
        KIcon(QApplication::isRightToLeft() ? "format-indent-more" : "format-indent-less"),
        i18n("Decrease Indent"), this);
    addAction("format_decreaseindent", m_actionFormatDecreaseIndent);
    connect(m_actionFormatDecreaseIndent, SIGNAL(triggered()), this, SLOT(decreaseIndent()));

    action = new KAction(i18n("Increase Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Greater);
    addAction("fontsizeup", action);
    connect(action, SIGNAL(triggered()), this, SLOT(increaseFontSize()));

    action = new KAction(i18n("Decrease Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Less);
    addAction("fontsizedown", action);
    connect(action, SIGNAL(triggered()), this, SLOT(decreaseFontSize()));

    m_actionFormatFontFamily = new KFontAction(KFontChooser::SmoothScalableFonts, this);
    addAction("format_fontfamily", m_actionFormatFontFamily);
    connect(m_actionFormatFontFamily, SIGNAL(triggered(const QString &)),
            this, SLOT(setFontFamily(const QString &)));

    /*
        m_actionFormatStyleMenu  = new KActionMenu(i18n("Style"), this);
        addAction("format_stylemenu", m_actionFormatStyleMenu);
        m_actionFormatStyle  = new KSelectAction(i18n("Style"), this);
        addAction("format_style", m_actionFormatStyle);
        connect(m_actionFormatStyle, SIGNAL(activated(int)),
                this, SLOT(textStyleSelected(int)));
        updateStyleList();

        // ----------------------- More format actions, for the toolbar only
        QActionGroup* spacingActionGroup = new QActionGroup(this);
        spacingActionGroup->setExclusive(true);
        m_actionFormatSpacingSingle = new KToggleAction(i18n("Line Spacing 1"), "format-line-spacing-simple", Qt::CTRL + Qt::Key_1,
                this, SLOT(textSpacingSingle()),
                actionCollection(), "format_spacingsingle");
        m_actionFormatSpacingSingle->setActionGroup(spacingActionGroup);
        m_actionFormatSpacingOneAndHalf = new KToggleAction(i18n("Line Spacing 1.5"), "format-line-spacing-double", Qt::CTRL + Qt::Key_5,
                this, SLOT(textSpacingOneAndHalf()),
                actionCollection(), "format_spacing15");
        m_actionFormatSpacingOneAndHalf->setActionGroup(spacingActionGroup);
        m_actionFormatSpacingDouble = new KToggleAction(i18n("Line Spacing 2"), "format-line-spacing-triple", Qt::CTRL + Qt::Key_2,
                this, SLOT(textSpacingDouble()),
                actionCollection(), "format_spacingdouble");
        m_actionFormatSpacingDouble->setActionGroup(spacingActionGroup);

        m_actionFormatColor = new TKSelectColorAction(i18n("Text Color..."), TKSelectColorAction::TextColor,
                this, SLOT(textColor()),
                actionCollection(), "format_color", true);
        m_actionFormatColor->setDefaultColor(QColor());


        m_actionFormatNumber  = new KActionMenu(KIcon("format-list-ordered"), i18n("Number"), this);
        addAction("format_number", m_actionFormatNumber);
        m_actionFormatNumber->setDelayed(false);
        m_actionFormatBullet  = new KActionMenu(KIcon("format-list-unordered"), i18n("Bullet"), this);
        addAction("format_bullet", m_actionFormatBullet);
        m_actionFormatBullet->setDelayed(false);
        QActionGroup* counterStyleActionGroup = new QActionGroup(this);
        counterStyleActionGroup->setExclusive(true);
        QList<KoCounterStyleWidget::StyleRepresenter*> stylesList;
        KoCounterStyleWidget::makeCounterRepresenterList(stylesList);
        foreach (KoCounterStyleWidget::StyleRepresenter* styleRepresenter, stylesList) {
            // Dynamically create toggle-actions for each list style.
            // This approach allows to edit toolbars and extract separate actions from this menu
            KToggleAction* act = new KToggleAction(styleRepresenter->name(), // TODO icon
                    actionCollection(),
                    QString("counterstyle_%1").arg(styleRepresenter->style()));
            connect(act, SIGNAL(triggered(bool)), this, SLOT(slotCounterStyleSelected()));
            act->setActionGroup(counterStyleActionGroup);
            // Add to the right menu: both for "none", bullet for bullets, numbers otherwise
            if (styleRepresenter->style() == KoParagCounter::STYLE_NONE) {
                m_actionFormatBullet->insert(act);
                m_actionFormatNumber->insert(act);
            } else if (styleRepresenter->isBullet())
                m_actionFormatBullet->insert(act);
            else
                m_actionFormatNumber->insert(act);
        }
    */


    // ------------------- Actions with a key binding and no GUI item
    action  = new KAction(i18n("Insert Non-Breaking Space"), this);
    addAction("nonbreaking_space", action);
    action->setShortcut(Qt::CTRL + Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT(nonbreakingSpace()));

    action  = new KAction(i18n("Insert Non-Breaking Hyphen"), this);
    addAction("nonbreaking_hyphen", action);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Minus);
    connect(action, SIGNAL(triggered()), this, SLOT(nonbreakingHyphen()));

    action  = new KAction(i18n("Insert Index"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_T);
    addAction("insert_index", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertIndexMarker()));

    action  = new KAction(i18n("Insert Bookmark..."), this);
    addAction("insert_bookmark", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertBookmark()));

    action  = new KAction(i18n("Jump To..."), this);
    addAction("jump_to_text", action);
    connect(action, SIGNAL(triggered()), this, SLOT(jumpToText()));

    action  = new KAction(i18n("Insert Soft Hyphen"), this);
    addAction("soft_hyphen", action);
    //action->setShortcut(Qt::CTRL+Qt::Key_Minus); // TODO this one is also used for the kde-global zoom-out :(
    connect(action, SIGNAL(triggered()), this, SLOT(softHyphen()));

    action  = new KAction(i18n("Line Break"), this);
    addAction("line_break", action);
    action->setShortcut(Qt::SHIFT + Qt::Key_Return);
    connect(action, SIGNAL(triggered()), this, SLOT(lineBreak()));

    action  = new KAction(i18n("Font..."), this);
    addAction("format_font", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_F);
    action->setToolTip(i18n("Change character size, font, boldface, italics etc."));
    action->setWhatsThis(i18n("Change the attributes of the currently selected characters."));
    connect(action, SIGNAL(triggered()), this, SLOT(selectFont()));

    m_actionFormatFontSize = new KFontSizeAction(i18n("Font Size"), this);
    addAction("format_fontsize", m_actionFormatFontSize);
    connect(m_actionFormatFontSize, SIGNAL(fontSizeChanged(int)), this, SLOT(setFontSize(int)));

    m_actionFormatTextColor = new KoColorPopupAction(this);
    m_actionFormatTextColor->setIcon(KIcon("format-text-color"));
    m_actionFormatTextColor->setToolTip(i18n("Text Color..."));
    addAction("format_textcolor", m_actionFormatTextColor);
    connect(m_actionFormatTextColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setTextColor(const KoColor &)));

    m_actionFormatBackgroundColor = new KoColorPopupAction(this);
    m_actionFormatBackgroundColor->setIcon(KIcon("format-fill-color"));
    m_actionFormatBackgroundColor->setToolTip(i18n("Background Color..."));
    m_actionFormatBackgroundColor->setText(i18n("Background"));
    addAction("format_backgroundcolor", m_actionFormatBackgroundColor);
    connect(m_actionFormatBackgroundColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setBackgroundColor(const KoColor &)));

    action = new KAction(i18n("Default Format"), this);
    addAction("text_default", action);
    action->setToolTip(i18n("Change text attributes to their default values"));
    connect(action, SIGNAL(triggered()), this, SLOT(setDefaultFormat()));

    m_textEditingPlugins = canvas->shapeController()->resourceManager()->
        resource(TextEditingPluginContainer::ResourceId).value<TextEditingPluginContainer*>();
    Q_ASSERT(m_textEditingPlugins);

    for (QHash<QString, KTextEditingPlugin*>::const_iterator it = m_textEditingPlugins->constBegin(); it != m_textEditingPlugins->constEnd(); ++it) {
        KTextEditingPlugin *plugin = it.value();
        connect(plugin, SIGNAL(startMacro(const QString &)),
                this, SLOT(startMacro(const QString &)));
        connect(plugin, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
        QHash<QString, KAction*> actions = plugin->actions();
        QHash<QString, KAction*>::iterator i = actions.begin();
        while (i != actions.end()) {
            addAction(i.key(), i.value());
            ++i;
        }
    }

    // setup the context list.
    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(startTextEditingPlugin(QString)));
    QList<QAction*> list;
    list.append(this->action("text_default"));
    list.append(this->action("format_font"));
    for (KGenericRegistry<KTextEditingFactory*>::const_iterator it = KTextEditingRegistry::instance()->constBegin();
		    it != KTextEditingRegistry::instance()->constEnd(); ++it) {
        KTextEditingFactory *factory = it.value();
        if (factory->showInMenu()) {
            KAction *a = new KAction(i18n("Apply %1", factory->title()), this);
            connect(a, SIGNAL(triggered()), signalMapper, SLOT(map()));
            signalMapper->setMapping(a, factory->id());
            list.append(a);
            addAction(QString("apply_%1").arg(factory->id()), a);
        }
    }
    setPopupActionList(list);

    action = new KAction(i18n("Table..."), this);
    addAction("insert_table", action);
    action->setToolTip(i18n("Insert a table into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertTable()));
    action  = new KAction(KIcon("edit-table-insert-row-above"), i18n("Row Above"), this);
    action->setToolTip(i18n("Insert Row Above"));
    addAction("insert_tablerow_above", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowAbove()));

    action  = new KAction(KIcon("edit-table-insert-row-below"), i18n("Row Below"), this);
    action->setToolTip(i18n("Insert Row Below"));
    addAction("insert_tablerow_below", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowBelow()));

    action  = new KAction(KIcon("edit-table-insert-column-left"), i18n("Column Left"), this);
    action->setToolTip(i18n("Insert Column Left"));
    addAction("insert_tablecolumn_left", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnLeft()));

    action  = new KAction(KIcon("edit-table-insert-column-right"), i18n("Column Right"), this);
    action->setToolTip(i18n("Insert Column Right"));
    addAction("insert_tablecolumn_right", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnRight()));
    action  = new KAction(KIcon("edit-table-delete-column"), i18n("Column"), this);
    action->setToolTip(i18n("Delete Column"));
    addAction("delete_tablecolumn", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableColumn()));

    action  = new KAction(KIcon("edit-table-delete-row"), i18n("Row"), this);
    action->setToolTip(i18n("Delete Row"));
    addAction("delete_tablerow", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableRow()));

    action  = new KAction(KIcon("merge"), i18n("Merge Cells"), this);
    addAction("merge_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(mergeTableCells()));

    action  = new KAction(KIcon("split"), i18n("Split Cells"), this);
    addAction("split_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(splitTableCells()));

    action = new KAction(i18n("Paragraph..."), this);
    addAction("format_paragraph", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    action->setToolTip(i18n("Change paragraph margins, text flow, borders, bullets, numbering etc."));
    action->setWhatsThis(i18n("Change paragraph margins, text flow, borders, bullets, numbering etc.<p>Select text in multiple paragraphs to change the formatting of all selected paragraphs.<p>If no text is selected, the paragraph where the cursor is located will be changed.</p>"));
    connect(action, SIGNAL(triggered()), this, SLOT(formatParagraph()));

    m_actionShowChanges = new KAction(i18n("Show Changes"), this);
    m_actionShowChanges->setCheckable(true);
    addAction("edit_show_changes", m_actionShowChanges, KToolBase::ReadOnlyAction);
    connect(m_actionShowChanges, SIGNAL(triggered(bool)), this, SLOT(toggleShowChanges(bool)));

    m_actionRecordChanges = new KAction(i18n("Record Changes"), this);
    m_actionRecordChanges->setCheckable(true);
    addAction("edit_record_changes", m_actionRecordChanges);
    connect(m_actionRecordChanges, SIGNAL(triggered(bool)), this, SLOT(toggleRecordChanges(bool)));

    m_configureChangeTracking = new KAction(i18n("Configure Change Tracking..."), this);
    addAction("configure_change_tracking", m_configureChangeTracking);
    connect(m_configureChangeTracking, SIGNAL(triggered()), this, SLOT(configureChangeTracking()));

    action = new KAction(i18n("Style Manager"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_S);
    action->setToolTip(i18n("Change attributes of styles"));
    action->setWhatsThis(i18n("Change font and paragraph attributes of styles.<p>Multiple styles can be changed using the dialog box."));
    addAction("format_stylist", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showStyleManager()));

    action = new KAction(i18n("Special Character..."), this);
    action->setIcon(KIcon("character-set"));
    action->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_C);
    addAction("insert_specialchar", action);
    action->setToolTip(i18n("Insert one or more symbols or characters not found on the keyboard"));
    action->setWhatsThis(i18n("Insert one or more symbols or characters not found on the keyboard."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertSpecialCharacter()));

#ifndef NDEBUG
    action = new KAction("Paragraph Debug", this); // do NOT add i18n!
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
    addAction("detailed_debug_paragraphs", action, KToolBase::ReadOnlyAction);
    connect(action, SIGNAL(triggered()), this, SLOT(debugTextDocument()));
    action = new KAction("Styles Debug", this); // do NOT add i18n!
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_S);
    addAction("detailed_debug_styles", action, KToolBase::ReadOnlyAction);
    connect(action, SIGNAL(triggered()), this, SLOT(debugTextStyles()));
#endif

    connect(canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(shapeAddedToCanvas()));

    m_caretTimer.setInterval(500);
    connect(&m_caretTimer, SIGNAL(timeout()), this, SLOT(blinkCaret()));

    m_changeTipTimer.setInterval(500);
    m_changeTipTimer.setSingleShot(true);
    connect(&m_changeTipTimer, SIGNAL(timeout()), this, SLOT(showChangeTip()));

    QStringList mimes;
    mimes << "text/plain" << "text/html" << "application/vnd.oasis.opendocument.text";
    setSupportedPasteMimeTypes(mimes);
}

#ifndef NDEBUG
#include "tests/MockShapes.h"
#include <KUndoStack>

TextTool::TextTool(MockCanvas *canvas)  // constructor for our unit tests;
    : KToolBase(canvas),
    m_textShape(0),
    m_textShapeData(0),
    m_changeTracker(0),
    m_allowActions(true),
    m_allowAddUndoCommand(true),
    m_trackChanges(false),
    m_allowResourceManagerUpdates(true),
    m_prevCursorPosition(-1),
    m_caretTimer(this),
    m_caretTimerState(true),
    m_currentCommand(0),
    m_currentCommandHasChildren(false),
    m_specialCharacterDocker(0),
    m_textEditingPlugins(0),
    m_changeTipTimer(this),
    m_changeTipCursorPos(0)
{
    m_textEditingPlugins = TextEditingPluginContainer::create(0, TextEditingPluginContainer::TestSetup);
    m_textEditingPlugins->setParent(this); // don't leak memory
    // we could init some vars here, but we probably don't have to
    KGlobal::setLocale(new KLocale("en"));
    QTextDocument *document = new QTextDocument();
    KTextDocumentLayout *layout = new KTextDocumentLayout(document);
    KInlineTextObjectManager *inlineManager = new KInlineTextObjectManager();
    layout->setInlineTextObjectManager(inlineManager);
    document->setDocumentLayout(layout);
    m_textEditor = new KoTextEditor(document);
    m_changeTracker = new KChangeTracker();
    KTextDocument(document).setChangeTracker(m_changeTracker);
    KTextDocument(document).setUndoStack(new KUndoStack());
    KTextDocument(document).setTextEditor(m_textEditor.data());
}
#endif

TextTool::~TextTool()
{
}

void TextTool::showChangeTip()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor || !m_changeTipCursorPos || !m_changeTracker->displayChanges())
        return;
    QTextCursor c(textEditor->document());
    c.setPosition(m_changeTipCursorPos);
    if (m_changeTracker && m_changeTracker->containsInlineChanges(c.charFormat())) {
        KChangeTrackerElement *element = m_changeTracker->elementById(c.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt());
        if (element->isEnabled()) {
            QString changeType;
            if (element->changeType() == KOdfGenericChange::InsertChange)
                changeType = i18n("Insertion");
            else if (element->changeType() == KOdfGenericChange::DeleteChange)
                changeType = i18n("Deletion");
            else
                changeType = i18n("Formatting");

            QString change = "<p align=center style=\'white-space:pre\' ><b>" + changeType + "</b><br/>";

            QString date = element->date();
            //Remove the T which separates the Data and Time.
            date[10] = ' ';
            change += element->creator() + ' ' + date + "</p>";

            int toolTipWidth = QFontMetrics(QToolTip::font()).boundingRect(element->date() + ' ' + element->creator()).width();
            m_changeTipPos.setX(m_changeTipPos.x() - toolTipWidth/2);

            QToolTip::showText(m_changeTipPos,change,canvas()->canvasWidget());

        }
    }
}

void TextTool::blinkCaret()
{
    if (! canvas()->canvasWidget()->hasFocus()) {
        m_caretTimer.stop();
        m_caretTimerState = false; // not visible.
    }
    else {
        m_caretTimerState = !m_caretTimerState;
    }
    repaintCaret();
}

void TextTool::paint(QPainter &painter, const KViewConverter &converter)
{
    if (m_textEditor.isNull())
        return;
    if (canvas()->canvasWidget()->hasFocus() && !m_caretTimer.isActive()) // make sure we blink
        m_caretTimer.start();
    QTextBlock block = m_textEditor.data()->block();
    if (! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;

    int selectStart = m_textEditor.data()->position();
    int selectEnd = m_textEditor.data()->anchor();
    if (selectEnd < selectStart)
        qSwap(selectStart, selectEnd);
    QList<TextShape *> shapesToPaint;
    KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(textEditor->document()->documentLayout());
    if (lay) {
        foreach (KShape *shape, lay->shapes()) {
            TextShape *ts = dynamic_cast<TextShape*>(shape);
            if (! ts)
                continue;
            KTextShapeData *data = ts->textShapeData();
            // check if shape contains some of the selection, if not, skip
            if (!((data->endPosition() >= selectStart && data->position() <= selectEnd)
                    || (data->position() <= selectStart && data->endPosition() >= selectEnd)))
                continue;
            if (painter.hasClipping()) {
                QRect rect = converter.documentToView(ts->boundingRect()).toRect();
                if (painter.clipRegion().intersect(QRegion(rect)).isEmpty())
                    continue;
            }
            shapesToPaint << ts;
        }
    }
    if (shapesToPaint.isEmpty()) // quite unlikely, though ;)
        return;

    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);

    QAbstractTextDocumentLayout::PaintContext pc;
    QAbstractTextDocumentLayout::Selection selection;
    selection.cursor = *(m_textEditor.data()->cursor());
    selection.format.setBackground(canvas()->canvasWidget()->palette().brush(QPalette::Highlight));
    selection.format.setForeground(canvas()->canvasWidget()->palette().brush(QPalette::HighlightedText));
    pc.selections.append(selection);
    foreach (TextShape *ts, shapesToPaint) {
        KTextShapeData *data = ts->textShapeData();
        Q_ASSERT(data);
        if (data->endPosition() == -1)
            continue;

        painter.save();
        QTransform shapeMatrix = ts->absoluteTransformation(&converter);
        shapeMatrix.scale(zoomX, zoomY);
        painter.setTransform(shapeMatrix * painter.transform());
        painter.setClipRect(QRectF(QPointF(), ts->size()), Qt::IntersectClip);
        painter.translate(0, -data->documentOffset());
        if ((data->endPosition() >= selectStart && data->position() <= selectEnd)
                || (data->position() <= selectStart && data->endPosition() >= selectEnd)) {
            QRectF clip = textRect(qMax(data->position(), selectStart), qMin(data->endPosition(), selectEnd));
            painter.save();
            painter.setClipRect(clip, Qt::IntersectClip);
            data->document()->documentLayout()->draw(&painter, pc);
            painter.restore();
        }
        if ((data == m_textShapeData) && m_caretTimerState) {
            // paint caret
            QPen caretPen(Qt::black);
            if (! m_textShape->hasTransparency()) {
                KColorBackground * fill = dynamic_cast<KColorBackground*>(m_textShape->background());
                if (fill) {
                    QColor bg = fill->color();
                    QColor invert = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
                    caretPen.setColor(invert);
                }
            }
            painter.setPen(caretPen);
            int posInParag = m_textEditor.data()->position() - block.position();
            if (posInParag <= block.layout()->preeditAreaPosition())
                posInParag += block.layout()->preeditAreaText().length();
            block.layout()->drawCursor(&painter, QPointF(), posInParag);
        }

        painter.restore();
    }
}

void TextTool::updateSelectedShape(const QPointF &point)
{
    if (!(m_textShape && m_textShape->boundingRect().contains(point))) {
        KoTextEditor *textEditor = m_textEditor.data();
        Q_ASSERT(textEditor);
        QRectF area(point, QSizeF(1, 1));
        if (textEditor->hasSelection())
            repaintSelection();
        else
            repaintCaret();
        foreach (KShape *shape, canvas()->shapeManager()->shapesAt(area, true)) {
            if (shape->isContentProtected())
                continue;
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KTextShapeData *d = static_cast<KTextShapeData*>(textShape->userData());
                const bool sameDocument = d->document() == textEditor->document();
                if (sameDocument && d->position() < 0)
                    continue; // don't change to a shape that has no text
                m_textShape = textShape;
                if (sameDocument)
                    break; // stop looking.
            }
        }
        if (m_textShape)
            setShapeData(static_cast<KTextShapeData*>(m_textShape->userData()));
    }
}

const QTextCursor TextTool::cursor()
{
    return *(m_textEditor.data()->cursor());
}

void TextTool::setShapeData(KTextShapeData *data)
{
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
    if (m_textShapeData) {
        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay)
            disconnect(lay, SIGNAL(shapeAdded(KShape*)), this, SLOT(shapeAddedToDoc(KShape*)));
    }
    m_textShapeData = data;
    if (m_textShapeData == 0)
        return;
    connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    if (docChanged) {
        if (!m_textEditor.isNull())
            disconnect(m_textEditor.data(), SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));
        m_textEditor = KTextDocument(m_textShapeData->document()).textEditor();
        Q_ASSERT(m_textEditor.data());
        connect(m_textEditor.data(), SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));

        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
            connect(lay, SIGNAL(shapeAdded(KShape*)), this, SLOT(shapeAddedToDoc(KShape*)));

             // check and remove the demo text.
            bool demoTextOn = true;
            foreach (KShape *shape, lay->shapes()) {
                TextShape *ts = dynamic_cast<TextShape*>(shape);
                if (ts && !ts->demoText()) { // if any shape in the series has it turned off, we don't have it anymore.
                    demoTextOn = false;
                    break;
                }
            }

            if (demoTextOn) {
                QTextDocument *doc = m_textShapeData->document();
                doc->setUndoRedoEnabled(false); // removes undo history
                KTextDocument document(doc);
                document.clearText();
                KStyleManager *styleManager = document.styleManager();
                if (styleManager) {
                    QTextBlock block = doc->begin();
                    styleManager->defaultParagraphStyle()->applyStyle(block);
                }
                m_textShapeData->document()->setUndoRedoEnabled(true); // allow undo history
                foreach (KShape *shape, lay->shapes()) {
                    TextShape *ts = dynamic_cast<TextShape*>(shape);
                    if (ts) ts->setDemoText(false);
                }
            }
        }

	for (QHash<QString, KTextEditingPlugin*>::const_iterator it = m_textEditingPlugins->constBegin(); it != m_textEditingPlugins->constEnd(); ++it) {
            KTextEditingPlugin *plugin = it.value();
            plugin->setCurrentCursorPosition(m_textEditor.data()->document(),
                    m_textEditor.data()->position());
        }
    }
    m_textEditor.data()->updateDefaultTextDirection(m_textShapeData->pageDirection());
}

void TextTool::updateSelectionHandler()
{
    if (m_textEditor) {
        emit selectionChanged(m_textEditor.data()->hasSelection());
        if (m_textEditor.data()->hasSelection()) {
            QClipboard *clipboard = QApplication::clipboard();
            if (clipboard->supportsSelection())
                clipboard->setText(m_textEditor.data()->selectedText(), QClipboard::Selection);
        }
    }

    KResourceManager *p = canvas()->resourceManager();
    m_allowResourceManagerUpdates = false;
    if (m_textEditor) {
        p->setResource(KOdfText::CurrentTextPosition, m_textEditor.data()->position());
        p->setResource(KOdfText::CurrentTextAnchor, m_textEditor.data()->anchor());
        QVariant variant;
        variant.setValue<void*>(m_textEditor.data()->document());
        p->setResource(KOdfText::CurrentTextDocument, variant);
    } else {
        p->clearResource(KOdfText::CurrentTextPosition);
        p->clearResource(KOdfText::CurrentTextAnchor);
        p->clearResource(KOdfText::CurrentTextDocument);
    }
    m_allowResourceManagerUpdates = true;
}

void TextTool::copy() const
{
    if (m_textShapeData == 0 || m_textEditor.isNull() || !m_textEditor.data()->hasSelection())
        return;
    int from = m_textEditor.data()->position();
    int to = m_textEditor.data()->anchor();
    KTextOdfSaveHelper saveHelper(m_textShapeData, from, to);
    KTextDrag drag;

    if (KDocumentRdfBase *rdf = KDocumentRdfBase::fromResourceManager(canvas())) {
        saveHelper.setRdfModel(rdf->model());
    }
    drag.setOdf(KOdf::mimeType(KOdf::TextDocument), saveHelper);
    QTextDocumentFragment fragment = m_textEditor.data()->selection();
    drag.setData("text/html", fragment.toHtml("utf-8").toUtf8());
    drag.setData("text/plain", fragment.toPlainText().toUtf8());
    drag.addToClipboard();
}

void TextTool::deleteSelection()
{
    if (m_actionRecordChanges->isChecked())
      m_textEditor.data()->addCommand(new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, this));
    else
      m_textEditor.data()->addCommand(new DeleteCommand(DeleteCommand::NextChar, this));
    editingPluginEvents();
}

bool TextTool::paste()
{
    const QMimeData *data = QApplication::clipboard()->mimeData(QClipboard::Clipboard);

    // on windows we do not have data if we try to paste the selection
    if (!data)
        return false;

    m_prevCursorPosition = m_textEditor.data()->position();
    m_textEditor.data()->addCommand(new TextPasteCommand(QClipboard::Clipboard, this));
    editingPluginEvents();
    return true;
}

void TextTool::cut()
{
    m_textEditor.data()->addCommand(new TextCutCommand(this));
}

int TextTool::pointToPosition(const QPointF &point) const
{
    KTextShapeData *textShapeData = m_textShapeData;
    if (textShapeData == 0)
        return -1;
    if (textShapeData->endPosition() == -1) {
        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(textShapeData->document()->documentLayout());
        if (lay) {
            foreach (KShape *shape, lay->shapes()) {
                KTextShapeData *sd = dynamic_cast<KTextShapeData*>(shape->userData());
                if (sd && sd->endPosition() >= 0)
                    textShapeData = sd;
                if (shape->boundingRect().contains(point))
                    break;
                if (shape == m_textShape)
                    break;
            }
        }
        if (textShapeData == 0) // we have no textShapeData and probably no KTextDocumentLayout
            return -1;
        if (textShapeData->endPosition() == -1) // never been layed-out before   
            return 0;
    }
    Q_ASSERT(m_textShapeData);
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_textEditor.data()->document()->documentLayout()->hitTest(p, Qt::ExactHit);
    caretPos = qMax(caretPos, m_textShapeData->position());
    if (m_textShapeData->endPosition() == -1) { // not fully laid-out textframe
        return -1;
    }
    caretPos = qMin(caretPos, m_textShapeData->endPosition());
    return caretPos;
}

void TextTool::shortcutOverride(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ShiftModifier || event->modifiers() == Qt::NoModifier) {
        // match all simple chars too to allow apps to have single-key shortcuts when I'm not active.
        event->accept(); // its mine!
    }
    if (event->matches(QKeySequence::SelectAll)) {
        // match select all to avoid conflicting with apps having that too.
        selectAll();
        event->accept(); // its mine!
        return;
    }
}

void TextTool::mousePressEvent(KPointerEvent *event)
{
    if (m_textEditor.isNull())
        return;
    if (event->button() != Qt::RightButton)
        updateSelectedShape(event->point);
    KShapeSelection *selection = canvas()->shapeManager()->selection();
    if (!selection->isSelected(m_textShape) && m_textShape->isSelectable()) {
        selection->deselectAll();
        selection->select(m_textShape);
    }

    const bool canMoveCaret = !m_textEditor.data()->hasSelection() || event->button() !=  Qt::RightButton;
    if (canMoveCaret) {
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if (m_textEditor.data()->hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if (! m_textEditor.data()->hasSelection())
            repaintCaret();
        int prevPosition = m_textEditor.data()->position();
        int position = pointToPosition(event->point);
        if (position >= 0)
            m_textEditor.data()->setPosition(position,
                    shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if (shiftPressed) // altered selection.
            repaintSelection(prevPosition, m_textEditor.data()->position());
        else
            repaintCaret();

        updateSelectionHandler();
        updateStyleManager();
    }
    updateActions();

    if (event->button() == Qt::RightButton) {
        // activate context-menu for spelling-suggestions
        KTextEditingPlugin *plugin = m_textEditingPlugins->spellcheck();
        if (plugin)
            plugin->setCurrentCursorPosition(m_textEditor.data()->document(), m_textEditor.data()->position());

        // Is there a KVariable here?
        KInlineTextObjectManager *inlineTextObjectManager = KTextDocument(m_textEditor.data()->document()).inlineTextObjectManager();
        KVariable *variable = 0;
        if (inlineTextObjectManager) {
            const int position = pointToPosition(event->point);
            QTextCursor cursor(m_textEditor.data()->document());
            cursor.setPosition(position);
            KInlineObject *obj = inlineTextObjectManager->inlineTextObject(cursor.charFormat());
            variable = dynamic_cast<KVariable*>(obj);
        }

        if (variable) {
            QMenu menu(canvas()->canvasWidget());
            QAction *action = menu.addAction(i18n("Edit Variable..."), this, SLOT(showEditVariableDialog()));
            QVariant v;
            v.setValue<void*>(variable);
            action->setData(v);
            menu.exec(canvas()->canvasWidget()->mapToGlobal(canvas()->viewConverter()->
                        documentToView(event->point).toPoint()), 0);
            return;
        }

        event->ignore();
    }

    if (event->button() ==  Qt::MidButton) // Paste
        paste();
}

void TextTool::mouseDoubleClickEvent(KPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_textShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    m_textEditor.data()->clearSelection();
    int pos = m_textEditor.data()->position();
    m_textEditor.data()->movePosition(QTextCursor::WordLeft);
    m_textEditor.data()->movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
    if (qAbs(pos - m_textEditor.data()->position()) <= 1) // clicked between two words
        m_textEditor.data()->movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);

    repaintSelection();
    updateSelectionHandler();
}

void TextTool::mouseMoveEvent(KPointerEvent *event)
{
    if (m_textEditor.isNull())
        return;
    m_changeTipPos = event->globalPos();

    setCursor(Qt::IBeamCursor);
    if (event->buttons()) {
        updateSelectedShape(event->point);
    }

    m_changeTipTimer.stop();

    if (QToolTip::isVisible())
        QToolTip::hideText();

    int position = pointToPosition(event->point);

    if (position > 0 && event->buttons() == Qt::NoButton) {
        QTextCursor mouseOver(m_textEditor.data()->document());
        mouseOver.setPosition(position);
        QTextCharFormat fmt = mouseOver.charFormat();

        if (fmt.isAnchor())
            setCursor(Qt::PointingHandCursor);
        else
            setCursor(Qt::IBeamCursor);

        if (m_changeTracker && m_changeTracker->containsInlineChanges(fmt)) {
            m_changeTipTimer.start();
            m_changeTipCursorPos = position;
        }
        return;
    }
    if (event->buttons() == Qt::NoButton)
        return;
    if (position == m_textEditor.data()->position())
        return;
    if (position >= 0) {
        repaintCaret();
        int prevPos = m_textEditor.data()->position();
        m_textEditor.data()->setPosition(position, QTextCursor::KeepAnchor);
        repaintSelection(prevPos, m_textEditor.data()->position());
    }

    updateSelectionHandler();
}

void TextTool::mouseReleaseEvent(KPointerEvent *event)
{
    event->ignore();
    editingPluginEvents();
    Q_ASSERT(m_textEditor.data());
    const QTextCharFormat cfm = m_textEditor.data()->charFormat();

    // Is there an anchor here ?
    if (cfm.isAnchor() && !m_textEditor.data()->hasSelection()) {
        QString anchor = cfm.anchorHref();
        if (!anchor.isEmpty()) {
            KTextDocument document(m_textEditor.data()->document());
            KInlineTextObjectManager *inlineManager = document.inlineTextObjectManager();
            if (inlineManager) {
                QList<QString> bookmarks = inlineManager->bookmarkManager()->bookmarkNames();
                // Which are the bookmarks we have ?
                foreach(const QString& s, bookmarks) {
                    // Is this bookmark the good one ?
                    if (s == anchor) {
                        // if Yes, let's jump to it
                        KoBookmark *bookmark = inlineManager->bookmarkManager()->bookmark(s);
                        m_textEditor.data()->setPosition(bookmark->textPosition());
                        ensureCursorVisible();
                        event->accept();
                        return;
                    }
                }
            }

            bool isLocalLink = (anchor.indexOf("file:") == 0);
            QString type = KMimeType::findByUrl(anchor, 0, isLocalLink)->name();

            if (KRun::isExecutableFile(anchor, type)) {
                QString question = i18n("This link points to the program or script '%1'.\n"
                                        "Malicious programs can harm your computer. "
                                        "Are you sure that you want to run this program?", anchor);
                // this will also start local programs, so adding a "don't warn again"
                // checkbox will probably be too dangerous
                const int choice = KMessageBox::warningYesNo(canvas()->canvasWidget(), question,
                        i18n("Open Link?"));
                if (choice != KMessageBox::Yes)
                    return;
            }

            event->accept();
            new KRun(cfm.anchorHref(), 0);
            m_textEditor.data()->setPosition(0);
            ensureCursorVisible();
            return;
        } else {
            QStringList anchorList = cfm.anchorNames();
            QString anchorName;
            if (!anchorList.isEmpty()) {
                anchorName = anchorList.takeFirst();
            }
            KTextDocument document(m_textEditor.data()->document());
            KoBookmark *bookmark = document.inlineTextObjectManager()->bookmarkManager()->bookmark(anchorName);
            if (bookmark) {
                m_textEditor.data()->setPosition(bookmark->textPosition());
                ensureCursorVisible();
            } else {
                kDebug(32500) << "A bookmark should exist but has not been found";
            }
        }
    }
}

void TextTool::keyPressEvent(QKeyEvent *event)
{
    int destinationPosition = -1; // for those cases where the moveOperation is not relevant;
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    const bool rw = isReadWrite();
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    if (rw && event->key() == Qt::Key_Backspace) {
        if (!textEditor->hasSelection() && textEditor->block().textList()
            && (textEditor->position() == textEditor->block().position())
            && !(m_actionRecordChanges->isChecked())) {
            if (!textEditor->blockFormat().boolProperty(KParagraphStyle::UnnumberedListItem)) {
                // backspace at beginning of numbered list item, makes it unnumbered
                ListItemNumberingCommand *lin = new ListItemNumberingCommand(textEditor->block(), false);
                addCommand(lin);
            } else {
                // backspace on numbered, empty parag, removes numbering.
                ChangeListCommand *clc = new ChangeListCommand(*textEditor->cursor(), KListStyle::None, 0 /* level */);
                addCommand(clc);
            }
        } else if (textEditor->position() > 0 || textEditor->hasSelection()) {
            if (!textEditor->hasSelection() && event->modifiers() & Qt::ControlModifier) // delete prev word.
                textEditor->movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            if (m_actionRecordChanges->isChecked())
                textEditor->addCommand(new ChangeTrackedDeleteCommand(
                            ChangeTrackedDeleteCommand::PreviousChar, this));
            else
                textEditor->addCommand(new DeleteCommand(DeleteCommand::PreviousChar, this));
            editingPluginEvents();
        }
    } else if (rw && (event->key() == Qt::Key_Tab)
        && ((!textEditor->hasSelection() && (textEditor->position() == textEditor->block().position())) || (textEditor->block().document()->findBlock(textEditor->anchor()) != textEditor->block().document()->findBlock(textEditor->position()))) && textEditor->block().textList()) {
        ChangeListLevelCommand::CommandType type = ChangeListLevelCommand::IncreaseLevel;
        ChangeListLevelCommand *cll = new ChangeListLevelCommand(*textEditor->cursor(), type, 1);
        addCommand(cll);
        editingPluginEvents();
    } else if (rw && (event->key() == Qt::Key_Backtab)
        && ((!textEditor->hasSelection() && (textEditor->position() == textEditor->block().position())) || (textEditor->block().document()->findBlock(textEditor->anchor()) != textEditor->block().document()->findBlock(textEditor->position()))) && textEditor->block().textList() && !(m_actionRecordChanges->isChecked())) {
        ChangeListLevelCommand::CommandType type = ChangeListLevelCommand::DecreaseLevel;
        ChangeListLevelCommand *cll = new ChangeListLevelCommand(*textEditor->cursor(), type, 1);
        addCommand(cll);
        editingPluginEvents();
    } else if (rw && event->key() == Qt::Key_Delete) {
        if (!textEditor->hasSelection() && event->modifiers() & Qt::ControlModifier) // delete next word.
            textEditor->movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        // the event only gets through when the Del is not used in the app
        // if the app forwards Del then deleteSelection is used
        if (m_actionRecordChanges->isChecked())
          textEditor->addCommand(new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, this));
        else
          textEditor->addCommand(new DeleteCommand(DeleteCommand::NextChar, this));
        editingPluginEvents();
    } else if ((event->key() == Qt::Key_Left) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Left;
    } else if ((event->key() == Qt::Key_Right) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Right;
    } else if ((event->key() == Qt::Key_Up) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Up;
    } else if ((event->key() == Qt::Key_Down) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Down;
    } else {
        // check for shortcuts.
        QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));
        if (hit(item, KStandardShortcut::Begin)) {
            // Goto beginning of the document. Default: Ctrl-Home
            destinationPosition = 0;
        } else if (hit(item, KStandardShortcut::End)) {
            // Goto end of the document. Default: Ctrl-End
            QTextBlock last = textEditor->document()->end().previous();
            destinationPosition = last.position() + last.length() - 1;
        } else if (hit(item, KStandardShortcut::Prior)) { // page up
            // Scroll up one page. Default: Prior
            moveOperation = QTextCursor::StartOfLine; // TODO
        } else if (hit(item, KStandardShortcut::Next)) {
            // Scroll down one page. Default: Next
            moveOperation = QTextCursor::StartOfLine; // TODO
        } else if (hit(item, KStandardShortcut::BeginningOfLine)) {
            // Goto beginning of current line. Default: Home
            moveOperation = QTextCursor::StartOfLine;
        } else if (hit(item, KStandardShortcut::EndOfLine)) {
            // Goto end of current line. Default: End
            moveOperation = QTextCursor::EndOfLine;
        } else if (hit(item, KStandardShortcut::BackwardWord)) {
            moveOperation = QTextCursor::WordLeft;
        } else if (hit(item, KStandardShortcut::ForwardWord)) {
            moveOperation = QTextCursor::WordRight;
#ifndef NDEBUG
        } else if (event->key() == Qt::Key_F12) {
            textEditor->insertTable(3, 2);
            QTextTable *table = textEditor->cursor()->currentTable();
            QTextCursor c = table->cellAt(1,1).firstCursorPosition();
            c.insertText("foo bar baz");
            textEditor->setPosition(c.position());
#endif
#ifdef Q_WS_MAC
        // Don't reject "alt" key, it may be used for typing text on Mac OS
        } else if ((event->modifiers() & Qt::ControlModifier) || event->text().length() == 0) {
#else
        } else if ((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) || event->text().length() == 0) {
#endif
            event->ignore();
            return;
        } else if (rw && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
            textEditor->newLine();
            updateActions();
            editingPluginEvents();
            ensureCursorVisible();
        } else if (rw && (event->key() == Qt::Key_Tab || !(event->text().length() == 1 && !event->text().at(0).isPrint()))) { // insert the text
            m_prevCursorPosition = textEditor->position();
            textEditor->insertText(event->text());
            ensureCursorVisible();
            editingPluginEvents();
        }
    }
    if (moveOperation != QTextCursor::NoMove || destinationPosition != -1) {
        setCursor(Qt::BlankCursor);
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if (textEditor->hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if (! textEditor->hasSelection())
            repaintCaret();
        QTextBlockFormat format = textEditor->blockFormat();

        KOdfText::Direction dir = static_cast<KOdfText::Direction>(format.intProperty(KParagraphStyle::TextProgressionDirection));
        bool isRtl;
        if (dir == KOdfText::AutoDirection)
            isRtl = textEditor->block().text().isRightToLeft();
        else
            isRtl =  dir == KOdfText::RightLeftTopBottom;

        if (isRtl) { // if RTL toggle direction of cursor movement.
            switch (moveOperation) {
            case QTextCursor::Left: moveOperation = QTextCursor::Right; break;
            case QTextCursor::Right: moveOperation = QTextCursor::Left; break;
            case QTextCursor::WordRight: moveOperation = QTextCursor::WordLeft; break;
            case QTextCursor::WordLeft: moveOperation = QTextCursor::WordRight; break;
            default: break;
            }
        }
        int prevPosition = textEditor->position();
        if (moveOperation != QTextCursor::NoMove)
            textEditor->movePosition(moveOperation,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        else
            textEditor->setPosition(destinationPosition,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if (moveOperation == QTextCursor::Down && prevPosition == textEditor->position()) {
            // change behavior a little bit from Qt; at the bottom of the doc we go to the end of the doc
            textEditor->movePosition(QTextCursor::End,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        }
        if (shiftPressed) // altered selection.
            repaintSelection(prevPosition, textEditor->position());
        else
            repaintCaret();
        updateActions();
        if (rw)
            editingPluginEvents();
        ensureCursorVisible();
    }
    if (m_caretTimer.isActive()) { // make the caret not blink but decide on the action if its visible or not.
        m_caretTimer.stop();
        m_caretTimer.start();
        m_caretTimerState = moveOperation != QTextCursor::NoMove; // turn caret off while typing
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return
                || event->key() == Qt::Key_Backspace) // except the enter/backspace key
            m_caretTimerState = true;
    }

    updateSelectionHandler();
}

QVariant TextTool::inputMethodQuery(Qt::InputMethodQuery query, const KViewConverter &converter) const
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0 || m_textShapeData == 0)
        return QVariant();
    switch (query) {
    case Qt::ImMicroFocus: {
        // The rectangle covering the area of the input cursor in widget coordinates.
        QRectF rect = textRect(textEditor->position(), textEditor->position());
        rect.moveTop(rect.top() - m_textShapeData->documentOffset());
        QTransform shapeMatrix = m_textShape->absoluteTransformation(&converter);
        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        shapeMatrix.scale(zoomX, zoomY);
        rect = shapeMatrix.mapRect(rect);

        return rect.toRect();
    }
    case Qt::ImFont:
        // The currently used font for text input.
        return textEditor->charFormat().font();
    case Qt::ImCursorPosition:
        // The logical position of the cursor within the text surrounding the input area (see ImSurroundingText).
        return textEditor->position() - textEditor->block().position();
    case Qt::ImSurroundingText:
        // The plain text around the input area, for example the current paragraph.
        return textEditor->block().text();
    case Qt::ImCurrentSelection:
        // The currently selected text.
        return textEditor->selectedText();
    default:
        ; // Qt 4.6 adds ImMaximumTextLength and ImAnchorPosition
    }
    return QVariant();
}

void TextTool::inputMethodEvent(QInputMethodEvent *event)
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    if (event->replacementLength() > 0) {
        textEditor->setPosition(textEditor->position() + event->replacementStart());
        for (int i = event->replacementLength(); i > 0; --i) {
            if (m_actionRecordChanges->isChecked())
              textEditor->addCommand(new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, this));
            else
              textEditor->addCommand(new DeleteCommand(DeleteCommand::NextChar, this));
        }
    }
    QTextBlock block = textEditor->block();
    QTextLayout *layout = block.layout();
    Q_ASSERT(layout);
    if (!event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
        layout->setPreeditArea(-1, QString());
    } else {
        layout->setPreeditArea(textEditor->position() - block.position(),
                event->preeditString());
        textEditor->document()->markContentsDirty(textEditor->position(), 1);
    }
    event->accept();
}

void TextTool::ensureCursorVisible()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0 || m_textShapeData == 0)
        return;
    if (m_textShapeData->endPosition() < textEditor->position() || m_textShapeData->position() > textEditor->position()) {
        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        foreach (KShape* shape, lay->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            Q_ASSERT(textShape);
            KTextShapeData *d = static_cast<KTextShapeData*>(textShape->userData());
            if (textEditor->position() >= d->position() && textEditor->position() <= d->endPosition()) {
                if (m_textShapeData)
                    disconnect(m_textShapeData, SIGNAL(destroyed(QObject*)), this, SLOT(shapeDataRemoved()));
                m_textShapeData = d;
                if (m_textShapeData)
                    connect(m_textShapeData, SIGNAL(destroyed(QObject*)), this, SLOT(shapeDataRemoved()));
                m_textShape = textShape;
                break;
            }
        }
    }

    QRectF cursorPos = textRect(textEditor->position(), textEditor->position());
    if (! cursorPos.isValid()) { // paragraph is not yet layouted.
        // The number one usecase for this is when the user pressed enter.
        // So take bottom of last paragraph.
        QTextBlock block = textEditor->block().previous();
        if (block.isValid()) {
            qreal y = block.layout()->boundingRect().bottom();
            cursorPos = QRectF(0, y, 1, 10);
        }
    }
    cursorPos.moveTop(cursorPos.top() - m_textShapeData->documentOffset());
    canvas()->ensureVisible(m_textShape->absoluteTransformation(0).mapRect(cursorPos));
}

void TextTool::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();
}

void TextTool::updateActions()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    m_allowActions = false;
    QTextCharFormat cf = textEditor->charFormat();
    m_actionFormatBold->setChecked(cf.fontWeight() > QFont::Normal);
    m_actionFormatItalic->setChecked(cf.fontItalic());
    m_actionFormatUnderline->setChecked(cf.intProperty(KCharacterStyle::UnderlineType) != KCharacterStyle::NoLineType);
    m_actionFormatStrikeOut->setChecked(cf.intProperty(KCharacterStyle::StrikeOutType) != KCharacterStyle::NoLineType);
    bool super = false, sub = false;
    switch (cf.verticalAlignment()) {
    case QTextCharFormat::AlignSuperScript:
        super = true;
        break;
    case QTextCharFormat::AlignSubScript:
        sub = true;
        break;
    default:;
    }
    m_actionFormatSuper->setChecked(super);
    m_actionFormatSub->setChecked(sub);
    m_actionFormatFontSize->setFontSize(qRound(cf.fontPointSize()));
    m_actionFormatFontFamily->setFont(cf.font().family());

    QTextBlockFormat bf = textEditor->blockFormat();
    if (bf.alignment() == Qt::AlignLeading || bf.alignment() == Qt::AlignTrailing) {
        bool revert = (textEditor->block().layout()->textOption().textDirection() == Qt::LeftToRight) != QApplication::isLeftToRight();
        if (bf.alignment() == (Qt::AlignLeading ^ revert))
            m_actionAlignLeft->setChecked(true);
        else
            m_actionAlignRight->setChecked(true);
    } else if (bf.alignment() == Qt::AlignHCenter) {
        m_actionAlignCenter->setChecked(true);
    }
    if (bf.alignment() == Qt::AlignJustify)
        m_actionAlignBlock->setChecked(true);
    else if (bf.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        m_actionAlignLeft->setChecked(true);
    else if (bf.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        m_actionAlignRight->setChecked(true);

    m_actionFormatDecreaseIndent->setEnabled(textEditor->blockFormat().leftMargin() > 0.);

    if (m_changeTracker && m_changeTracker->displayChanges())
        m_actionShowChanges->setChecked(true);
    if (m_changeTracker && m_changeTracker->recordChanges())
        m_actionRecordChanges->setChecked(true);

    m_allowActions = true;

    emit charFormatChanged(cf);
    emit blockFormatChanged(bf);
    emit blockChanged(textEditor->block());
}

void TextTool::updateStyleManager()
{
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    KStyleManager *styleManager = KTextDocument(textEditor->document()).styleManager();
    emit styleManagerChanged(styleManager);
    m_changeTracker = KTextDocument(textEditor->document()).changeTracker();
}

void TextTool::activate(ToolActivation toolActivation, const QSet<KShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    m_caretTimer.start();
    foreach (KShape *shape, shapes) {
        m_textShape = dynamic_cast<TextShape*>(shape);
        if (m_textShape)
            break;
    }
    if (m_textShape == 0) { // none found
        emit done();
        return;
    }
    setShapeData(static_cast<KTextShapeData*>(m_textShape->userData()));
    setCursor(Qt::IBeamCursor);

    // restore the selection from a previous time we edited this document.
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    for (int i = 0; i < m_previousSelections.count(); i++) {
        TextSelection selection = m_previousSelections.at(i);
        if (selection.document == textEditor->document()) {
            textEditor->setPosition(selection.anchor);
            textEditor->setPosition(selection.position, QTextCursor::KeepAnchor);
            m_previousSelections.removeAt(i);
            break;
        }
    }

    repaintSelection();
    updateSelectionHandler();
    updateActions();
    updateStyleManager();
    if (m_specialCharacterDocker)
        m_specialCharacterDocker->setEnabled(true);
    readConfig();
}

void TextTool::deactivate()
{
    m_caretTimer.stop();
    m_caretTimerState = false;
    repaintCaret();
    m_textShape = 0;
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    if (textEditor) {
        TextSelection selection;
        selection.document = textEditor->document();
        selection.position = m_textEditor.data()->position();
        selection.anchor = m_textEditor.data()->anchor();
        m_previousSelections.append(selection);
    }
    setShapeData(0);
    if (m_previousSelections.count() > 20) // don't let it grow indefinitely
        m_previousSelections.removeAt(0);

    updateSelectionHandler();
    if (m_specialCharacterDocker) {
        m_specialCharacterDocker->setEnabled(false);
        m_specialCharacterDocker->setVisible(false);
    }
}

void TextTool::repaintDecorations()
{
    repaintSelection();
}

void TextTool::repaintCaret()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0 || m_textShapeData == 0)
        return;
    QTextBlock block = textEditor->block();
    if (block.isValid()) {
        QTextLine tl = block.layout()->lineForTextPosition(textEditor->position() - block.position());
        QRectF repaintRect;
        if (tl.isValid()) {
            repaintRect = tl.rect();
            const int posInParag = textEditor->position() - block.position();
            repaintRect.setX(tl.cursorToX(posInParag) - 2);
            if (posInParag != 0 || block.length() != 1)
                repaintRect.setWidth(6);
        }
        repaintRect.moveTop(repaintRect.y() - m_textShapeData->documentOffset());
        repaintRect = m_textShape->absoluteTransformation(0).mapRect(repaintRect);
        canvas()->updateCanvas(repaintRect);
    }
}

void TextTool::repaintSelection()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    repaintSelection(textEditor->position(), textEditor->anchor());
}

void TextTool::repaintSelection(int startPosition, int endPosition)
{
    if (startPosition == endPosition)
        return;
    if (startPosition > endPosition)
        qSwap(startPosition, endPosition);
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    QList<TextShape *> shapes;
    KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(textEditor->document()->documentLayout());
    Q_ASSERT(lay);
    foreach (KShape* shape, lay->shapes()) {
        TextShape *textShape = dynamic_cast<TextShape*>(shape);
        if (textShape == 0) // when the shape is being deleted its no longer a TextShape but a KShape
            continue;

        const int from = textShape->textShapeData()->position();
        const int end = textShape->textShapeData()->endPosition();
        if ((from <= startPosition && end >= startPosition && end <= endPosition)
            || (from >= startPosition && end <= endPosition) // shape totally included
            || (from <= endPosition && end >= endPosition)
           )
            shapes.append(textShape);
    }

    // loop over all shapes that contain the text and update per shape.
    QRectF repaintRect = textRect(startPosition, endPosition);
    foreach (TextShape *ts, shapes) {
        QRectF rect = repaintRect;
        rect.moveTop(rect.y() - ts->textShapeData()->documentOffset());
        rect = ts->absoluteTransformation(0).mapRect(rect);
        canvas()->updateCanvas(ts->boundingRect().intersected(rect));
    }
}

QRectF TextTool::textRect(int startPosition, int endPosition) const
{
    Q_ASSERT(startPosition >= 0);
    Q_ASSERT(endPosition >= 0);
    KoTextEditor *textEditor = m_textEditor.data();
    Q_ASSERT(textEditor);
    if (startPosition > endPosition)
        qSwap(startPosition, endPosition);
    QTextBlock block = textEditor->document()->findBlock(startPosition);
    if (!block.isValid())
        return QRectF();
    QTextLine line1 = block.layout()->lineForTextPosition(startPosition - block.position());
    if (!line1.isValid())
        return QRectF();
    qreal startX = line1.cursorToX(startPosition - block.position());
    if (startPosition == endPosition)
        return QRectF(startX, line1.y(), 1, line1.height());

    QTextBlock block2 = textEditor->document()->findBlock(endPosition);
    if (!block2.isValid())
        return QRectF();
    QTextLine line2 = block2.layout()->lineForTextPosition(endPosition - block2.position());
    if (! line2.isValid())
        return QRectF();
    qreal endX = line2.cursorToX(endPosition - block2.position());

    if (line1.textStart() + block.position() == line2.textStart() + block2.position())
        return QRectF(qMin(startX, endX), line1.y(), qAbs(startX - endX), line1.height());
    return QRectF(0, line1.y(), 10E6, line2.y() + line2.height() - line1.y());
}

KToolSelection* TextTool::selection()
{
    return m_textEditor.data();
}

QWidget *TextTool::createOptionWidget()
{
    KTabWidget *widget = new KTabWidget();
    SimpleStyleWidget *ssw = new SimpleStyleWidget(this, widget);
    widget->addTab(ssw, i18n("Abc"));
    StylesWidget *styles = new StylesWidget(widget);
    styles->setCanvas(canvas());
    widget->addTab(styles, i18n("Styles"));
    ChangeTrackingOptionsWidget *changeTrackingOptions = new ChangeTrackingOptionsWidget(this, widget);
    widget->addTab(changeTrackingOptions, i18n("Change Tracking"));
    SimpleTableWidget *stw = new SimpleTableWidget(this, 0);
    widget->addTab(stw, i18n("Table"));

    connect(this, SIGNAL(styleManagerChanged(KStyleManager *)), ssw, SLOT(setStyleManager(KStyleManager *)));
    connect(this, SIGNAL(blockChanged(const QTextBlock&)), ssw, SLOT(setCurrentBlock(const QTextBlock&)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)), ssw, SLOT(setCurrentFormat(const QTextCharFormat &)));

    connect(ssw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    connect(this, SIGNAL(styleManagerChanged(KStyleManager *)), styles, SLOT(setStyleManager(KStyleManager *)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)),
            styles, SLOT(setCurrentFormat(const QTextCharFormat &)));
    connect(this, SIGNAL(blockFormatChanged(const QTextBlockFormat &)),
            styles, SLOT(setCurrentFormat(const QTextBlockFormat &)));

    connect(styles, SIGNAL(paragraphStyleSelected(KParagraphStyle *)),
            this, SLOT(setStyle(KParagraphStyle*)));
    connect(styles, SIGNAL(characterStyleSelected(KCharacterStyle *)),
            this, SLOT(setStyle(KCharacterStyle*)));
    connect(styles, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(changeTrackingOptions, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(m_actionShowChanges, SIGNAL(triggered(bool)), changeTrackingOptions, SLOT(toggleShowChanges(bool)));
    connect(m_actionRecordChanges, SIGNAL(triggered(bool)), changeTrackingOptions, SLOT(toggleRecordChanges(bool)));

    updateStyleManager();
    if (m_textShape)
        updateActions();
    return widget;
}

void TextTool::returnFocusToCanvas()
{
    canvas()->canvasWidget()->setFocus();
}

void TextTool::addUndoCommand()
{
    return;
/*    if (! m_allowAddUndoCommand) return;
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, TextTool *tool, QUndoCommand *parent = 0)
                : QUndoCommand(i18n("Text"), parent),
                m_document(document),
                m_tool(tool) {
        }

        void undo() {
            if (m_document.isNull())
                return;
            if (!(m_tool.isNull()) && (m_tool->m_textShapeData) && (m_tool->m_textShapeData->document() == m_document)) {
                m_tool->stopMacro();
                m_tool->m_allowAddUndoCommand = false;


                m_document->undo(&m_tool->m_caret);
            } else
                m_document->undo();
            if (! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        void redo() {
            if (m_document.isNull())
                return;

            if (!(m_tool.isNull()) && (m_tool->m_textShapeData) && (m_tool->m_textShapeData->document() == m_document)) {
                m_tool->m_allowAddUndoCommand = false;
                m_document->redo(&m_tool->m_caret);
            } else
                m_document->redo();
            if (! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        QPointer<QTextDocument> m_document;
        QPointer<TextTool> m_tool;
    };
    kDebug() << "in TextTool addCommand";
    if (m_currentCommand) {
        new UndoTextCommand(m_textShapeData->document(), this, m_currentCommand);
        if (! m_currentCommandHasChildren)
            canvas()->addCommand(m_currentCommand);
        m_currentCommandHasChildren = true;
    } else
        canvas()->addCommand(new UndoTextCommand(m_textShapeData->document(), this));
*/}

void TextTool::addCommand(QUndoCommand *command)
{
/*    m_currentCommand = command;
    TextCommandBase *cmd = dynamic_cast<TextCommandBase*>(command);
    if (cmd)
        cmd->setTool(this);
    m_currentCommandHasChildren = true; //to avoid adding it again on the first child UndoTextCommand (infinite loop)
    canvas()->addCommand(command); // will execute it.
    m_currentCommand = 0;
    m_currentCommandHasChildren = false;
*/
    Q_ASSERT(!m_textEditor.isNull());
    m_textEditor.data()->addCommand(command);
}

void TextTool::startEditing(QUndoCommand* command)
{
    m_currentCommand = command;
    m_currentCommandHasChildren = true;
}

void TextTool::stopEditing()
{
    m_currentCommand = 0;
    m_currentCommandHasChildren = false;
}

void TextTool::bold(bool bold)
{
    m_textEditor.data()->bold(bold);
}

void TextTool::italic(bool italic)
{
    m_textEditor.data()->italic(italic);
}

void TextTool::underline(bool underline)
{
    m_textEditor.data()->underline(underline);
}

void TextTool::strikeOut(bool strikeOut)
{
    m_textEditor.data()->strikeOut(strikeOut);
}

void TextTool::nonbreakingSpace()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(Qt::Key_nobreakspace)));
}

void TextTool::nonbreakingHyphen()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(0x2013)));
}

void TextTool::softHyphen()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(Qt::Key_hyphen)));
}

void TextTool::lineBreak()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(0x2028)));
}

void TextTool::alignLeft()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    Qt::Alignment align = Qt::AlignLeading;
    if (m_textEditor.data()->block().layout()->textOption().textDirection() != Qt::LeftToRight)
        align |= Qt::AlignTrailing;
    m_textEditor.data()->setHorizontalTextAlignment(align);
}

void TextTool::alignRight()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    Qt::Alignment align = Qt::AlignTrailing;
    if (m_textEditor.data()->block().layout()->textOption().textDirection() == Qt::RightToLeft)
        align = Qt::AlignLeading;
    m_textEditor.data()->setHorizontalTextAlignment(align);
}

void TextTool::alignCenter()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignHCenter);
}

void TextTool::alignBlock()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignJustify);
}

void TextTool::superScript(bool on)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    if (on)
        m_actionFormatSub->setChecked(false);
    m_textEditor.data()->setVerticalTextAlignment(on ? Qt::AlignTop : Qt::AlignVCenter);
}

void TextTool::subScript(bool on)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    if (on)
        m_actionFormatSuper->setChecked(false);
    m_textEditor.data()->setVerticalTextAlignment(on ? Qt::AlignBottom : Qt::AlignVCenter);
}

void TextTool::increaseIndent()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->increaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textEditor.data()->blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseIndent()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->decreaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textEditor.data()->blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseFontSize()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->decreaseFontSize();
}

void TextTool::increaseFontSize()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->increaseFontSize();
}

void TextTool::setFontFamily(const QString &font)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setFontFamily(font);
}

void TextTool::setFontSize (int size)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setFontSize(size);
}

void TextTool::setDefaultFormat()
{
    m_textEditor.data()->setDefaultFormat();
}

void TextTool::insertIndexMarker()
{
    if (!m_textEditor.data()->insertIndexMarker()) {
        KMessageBox::sorry(canvas()->canvasWidget(), i18n("Failed to mark word for inclusion in index.\nPlease reposition cursor and try again"));
    }
}

void TextTool::setStyle(KCharacterStyle *style)
{
    m_textEditor.data()->setStyle(style);
    emit charFormatChanged(m_textEditor.data()->charFormat());
}

void TextTool::setStyle(KParagraphStyle *style)
{
    m_textEditor.data()->setStyle(style);
    emit blockFormatChanged(m_textEditor.data()->blockFormat());
    emit charFormatChanged(m_textEditor.data()->charFormat());
}

void TextTool::insertTable()
{
    QPointer<TableDialog> dia = new TableDialog(0);
    if (dia->exec() == TableDialog::Accepted)
	if (dia)
            m_textEditor.data()->insertTable(dia->rows(), dia->columns());
    delete dia;
}

void TextTool::insertTableRowAbove()
{
    m_textEditor.data()->insertTableRowAbove();
}

void TextTool::insertTableRowBelow()
{
    m_textEditor.data()->insertTableRowBelow();
}

void TextTool::insertTableColumnLeft()
{
    m_textEditor.data()->insertTableColumnLeft();
}

void TextTool::insertTableColumnRight()
{
    m_textEditor.data()->insertTableColumnRight();
}

void TextTool::deleteTableColumn()
{
    m_textEditor.data()->deleteTableColumn();
}

void TextTool::deleteTableRow()
{
    m_textEditor.data()->deleteTableRow();
}

void TextTool::mergeTableCells()
{
    m_textEditor.data()->mergeTableCells();
}

void TextTool::splitTableCells()
{
    m_textEditor.data()->splitTableCells();
}

void TextTool::formatParagraph()
{
    QPointer<ParagraphSettingsDialog> dia = new ParagraphSettingsDialog(this, m_textEditor.data()->cursor());//TODO  check this with KoTextEditor
    dia->setUnit(canvas()->unit());
    connect(dia, SIGNAL(startMacro(const QString&)), this, SLOT(startMacro(const QString&)));//TODO
    connect(dia, SIGNAL(stopMacro()), this, SLOT(stopMacro()));

    dia->exec();
    delete dia;
}

void TextTool::toggleShowChanges(bool on)//TODO transfer this in KoTextEditor
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    m_actionShowChanges->setChecked(on);
    ShowChangesCommand *command = new ShowChangesCommand(on, textEditor->document(), canvas());
    connect(command, SIGNAL(toggledShowChange(bool)), m_actionShowChanges, SLOT(setChecked(bool)));
    m_textEditor.data()->addCommand(command);
}

void TextTool::toggleRecordChanges(bool on)
{
    m_actionRecordChanges->setChecked(on);
    if (m_changeTracker)
        m_changeTracker->setRecordChanges(on);
}

void TextTool::configureChangeTracking()
{
    if (m_changeTracker) {
        QColor insertionBgColor, deletionBgColor, formatChangeBgColor;
        insertionBgColor = m_changeTracker->insertionBgColor();
        deletionBgColor = m_changeTracker->deletionBgColor();
        formatChangeBgColor = m_changeTracker->formatChangeBgColor();
        QString authorName = m_changeTracker->authorName();
        KChangeTracker::ChangeSaveFormat changeSaveFormat = m_changeTracker->saveFormat();

        QPointer<ChangeConfigureDialog> changeDialog = new ChangeConfigureDialog(insertionBgColor, 
			deletionBgColor, formatChangeBgColor, authorName, changeSaveFormat, canvas()->canvasWidget());

        if (changeDialog->exec()) {
	    if (changeDialog){
                m_changeTracker->setInsertionBgColor(changeDialog->insertionBgColor());
                m_changeTracker->setDeletionBgColor(changeDialog->deletionBgColor());
                m_changeTracker->setFormatChangeBgColor(changeDialog->formatChangeBgColor());
                m_changeTracker->setAuthorName(changeDialog->authorName());
                m_changeTracker->setSaveFormat(changeDialog->saveFormat());
                writeConfig();
	    }
        }
	delete changeDialog;
    }
}

void TextTool::insertBookmark()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    QPointer<CreateBookmark> dia = new CreateBookmark(textEditor, canvas()->canvasWidget());
    dia->exec();
    delete dia;
}

void TextTool::selectAll()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    const int selectionLength = qAbs(textEditor->position() - textEditor->anchor());
    textEditor->setPosition(textEditor->document()->characterCount() - 1);
    textEditor->setPosition(0, QTextCursor::KeepAnchor);
    repaintSelection(0, textEditor->anchor());
    if (selectionLength != qAbs(textEditor->position() - textEditor->anchor())) // it actually changed
        emit selectionChanged(true);
}

void TextTool::startMacro(const QString &title)
{
    if (title != i18n("Key Press") && title !=i18n("Autocorrection")) //dirty hack while waiting for refactor of text editing
        m_textTyping = false;
    else
        m_textTyping = true;

    if (title != i18n("Delete") && title != i18n("Autocorrection")) //same dirty hack as above
        m_textDeleting = false;
    else
        m_textDeleting = true;

    if (m_currentCommand) return;

    class MacroCommand : public QUndoCommand
    {
    public:
        MacroCommand(const QString &title) : QUndoCommand(title), m_first(true) {}
        virtual void redo() {
            if (! m_first)
                QUndoCommand::redo();
            m_first = false;
        }
        virtual bool mergeWith(const QUndoCommand *) {
            return false;
        }
        bool m_first;
    };

    m_currentCommand = new MacroCommand(title);
    m_currentCommandHasChildren = false;
}

void TextTool::stopMacro()
{
    if (m_currentCommand == 0) return;
    if (! m_currentCommandHasChildren)
        delete m_currentCommand;
    m_currentCommand = 0;
}

void TextTool::showStyleManager()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    KStyleManager *styleManager = KTextDocument(textEditor->document()).styleManager();
    Q_ASSERT(styleManager);
    if (!styleManager)
        return;  //don't crash
    StyleManagerDialog *dia = new StyleManagerDialog(canvas()->canvasWidget());
    dia->setStyleManager(styleManager);
    dia->setUnit(canvas()->unit());
    dia->show();
}

void TextTool::startTextEditingPlugin(const QString &pluginId)
{
    KTextEditingPlugin *plugin = m_textEditingPlugins->plugin(pluginId);
    if (plugin) {
        KoTextEditor *textEditor = m_textEditor.data();
        if (textEditor == 0)
            return;
        if (m_textEditor.data()->hasSelection()) {
            int from = m_textEditor.data()->position();
            int to = m_textEditor.data()->anchor();
            if (from > to) // make sure we call the plugin consistently
                qSwap(from, to);
            plugin->checkSection(textEditor->document(), from, to);
        } else
            plugin->finishedWord(textEditor->document(), m_textEditor.data()->position());
    }
}

void TextTool::showEditVariableDialog()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        KVariable *variable = static_cast<KVariable*>(action->data().value<void*>());
        if (variable) {
            QWidget *optionsWidget = variable->createOptionsWidget();
            QPointer<KPageDialog> dialog = new KPageDialog(canvas()->canvasWidget());
            dialog->setCaption(i18n("Variable Options"));
            if (optionsWidget)
                dialog->addPage(optionsWidget, QString());
    // TODO make this qundocommand based ...
            dialog->exec();
            delete dialog;
        }
    }
}

bool TextTool::isBidiDocument() const
{
    if (m_textEditor)
        return m_textEditor.data()->isBidiDocument();
    return false;
}

void TextTool::resourceChanged(int key, const QVariant &var)
{
    if (m_allowResourceManagerUpdates == false)
        return;
    if (key == KOdfText::CurrentTextPosition) {
        repaintSelection();
        m_textEditor.data()->setPosition(var.toInt());
        ensureCursorVisible();
    } else if (key == KOdfText::CurrentTextAnchor) {
        repaintSelection();
        int pos = m_textEditor.data()->position();
        m_textEditor.data()->setPosition(var.toInt());
        m_textEditor.data()->setPosition(pos, QTextCursor::KeepAnchor);
    } else return;

    repaintSelection();
}

void TextTool::isBidiUpdated()
{
    emit blockChanged(m_textEditor.data()->block()); // make sure that the dialogs follow this change
}

void TextTool::insertSpecialCharacter()
{
    if (m_specialCharacterDocker == 0) {
        m_specialCharacterDocker = new InsertCharacter(canvas()->canvasWidget());
        connect(m_specialCharacterDocker, SIGNAL(insertCharacter(const QString&)),
                this, SLOT(insertString(const QString&)));
    }

    m_specialCharacterDocker->show();
}

void TextTool::insertString(const QString& string)
{
    m_textEditor.data()->insertText(string);
}

void TextTool::selectFont()
{
    QPointer<FontDia> fontDlg = new FontDia(m_textEditor.data()->cursor());//TODO check this with KoTextEditor
    connect(fontDlg, SIGNAL(startMacro(const QString &)), this, SLOT(startMacro(const QString &)));
    connect(fontDlg, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
    fontDlg->exec();
    delete fontDlg;
}

void TextTool::jumpToText()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    const int oldPos = textEditor->position();
    QPointer<JumpOverview> dia = new JumpOverview(textEditor->document(), canvas()->canvasWidget());
    connect (dia, SIGNAL(cursorPositionSelected(int)), textEditor, SLOT(setPosition(int)));
    connect (dia, SIGNAL(cursorPositionSelected(int)), this, SLOT(ensureCursorVisible()));
    if (dia->exec() == QDialog::Rejected) {
        textEditor->setPosition(oldPos);
        ensureCursorVisible();
    }
    delete dia;
}

void TextTool::shapeAddedToCanvas()
{
    if (m_textShape) {
        KShapeSelection *selection = canvas()->shapeManager()->selection();
        KShape *shape = selection->firstSelectedShape();
        if (shape != m_textShape && canvas()->shapeManager()->shapes().contains(m_textShape)) {
            // this situation applies when someone, not us, changed the selection by selecting another
            // text shape. Possibly by adding one.
            // Deselect the new shape again, so we can keep editing what we were already editing
            selection->select(m_textShape);
            selection->deselect(shape);
        }
    }
}

void TextTool::shapeDataRemoved()
{
    m_textShapeData = 0;
    m_textShape = 0;
    if (!m_textEditor.isNull() && m_textEditor.data()->cursor()->isNull()) {
        const QTextDocument *doc = m_textEditor.data()->document();
        Q_ASSERT(doc);
        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(doc->documentLayout());
        if (lay == 0 || lay->shapes().isEmpty()) {
            emit done();
            return;
        }

        m_textShape = static_cast<TextShape*>(lay->shapes().first());
        m_textShapeData = static_cast<KTextShapeData*>(m_textShape->userData());
        Q_ASSERT(m_textShapeData);
        connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    }
}

// ---------- editing plugins methods.
void TextTool::editingPluginEvents()
{
    if (m_prevCursorPosition == -1 || m_prevCursorPosition == m_textEditor.data()->position())
        return;

    QTextBlock block = m_textEditor.data()->block();
    if (! block.contains(m_prevCursorPosition)) {
        finishedWord();
        finishedParagraph();
        m_prevCursorPosition = -1;
    } else {
        int from = m_prevCursorPosition;
        int to = m_textEditor.data()->position();
        if (from > to)
            qSwap(from, to);
        QString section = block.text().mid(from - block.position(), to - from);
        if (section.contains(' ')) {
            finishedWord();
            m_prevCursorPosition = -1;
        }
    }
}

void TextTool::finishedWord()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    for (QHash<QString, KTextEditingPlugin*>::const_iterator it = m_textEditingPlugins->constBegin(); it != m_textEditingPlugins->constEnd(); ++it) {
        KTextEditingPlugin* plugin = it.value();
        plugin->finishedWord(textEditor->document(), m_prevCursorPosition);
    }
}

void TextTool::finishedParagraph()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    
    for (QHash<QString, KTextEditingPlugin*>::const_iterator it = m_textEditingPlugins->constBegin(); it != m_textEditingPlugins->constEnd(); ++it) {
        KTextEditingPlugin* plugin = it.value();
        plugin->finishedParagraph(textEditor->document(), m_prevCursorPosition);
    }
}

void TextTool::setTextColor(const KoColor &color)
{
    m_textEditor.data()->setTextColor(color.toQColor());
}

void TextTool::setBackgroundColor(const KoColor &color)
{
    m_textEditor.data()->setTextBackgroundColor(color.toQColor());
}

void TextTool::shapeAddedToDoc(KShape *shape)
{
    // calling ensureCursorVisible below is a rather intrusive thing to do for the user
    // so make doube sure we need it!
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    TextShape *ts = dynamic_cast<TextShape*>(shape);
    if (!ts)
        return;
    KTextShapeData *data = qobject_cast<KTextShapeData*>(ts->userData());
    if (!data)
        return;
    if (data->document() != textEditor->document())
        return;
    KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(textEditor->document()->documentLayout());
    Q_ASSERT(lay);
    const QList<KShape*> shapes = lay->shapes();
    // only when the new one is directly after our current one should we do the move
    if (shapes.indexOf(ts) - shapes.indexOf(m_textShape) > 1)
        return;
    // in case the new frame added is a freshly appended frame
    // allow the layouter to do some work and then optionally move the view to follow the cursor
    QTimer::singleShot(0, this, SLOT(ensureCursorVisible()));
}


void TextTool::readConfig()
{
    if (m_changeTracker) {
        QColor bgColor, defaultColor;
        QString changeAuthor;
        int changeSaveFormat = KChangeTracker::DELTAXML;
        KConfigGroup interface = KoGlobal::kofficeConfig()->group("Change-Tracking");
        if (interface.exists()) {
            bgColor = interface.readEntry("insertionBgColor", defaultColor);
            m_changeTracker->setInsertionBgColor(bgColor);
            bgColor = interface.readEntry("deletionBgColor", defaultColor);
            m_changeTracker->setDeletionBgColor(bgColor);
            bgColor = interface.readEntry("formatChangeBgColor", defaultColor);
            m_changeTracker->setFormatChangeBgColor(bgColor);
            changeAuthor = interface.readEntry("changeAuthor", changeAuthor);
            if (changeAuthor.isEmpty()) {
                KUser user(KUser::UseRealUserID);
                m_changeTracker->setAuthorName(user.property(KUser::FullName).toString());
            } else {
                m_changeTracker->setAuthorName(changeAuthor);
            }
            changeSaveFormat = interface.readEntry("changeSaveFormat", changeSaveFormat);
            m_changeTracker->setSaveFormat((KChangeTracker::ChangeSaveFormat)(changeSaveFormat));
        }
    }
}

void TextTool::writeConfig()
{
    if (m_changeTracker) {
        KConfigGroup interface = KoGlobal::kofficeConfig()->group("Change-Tracking");
        interface.writeEntry("insertionBgColor", m_changeTracker->insertionBgColor());
        interface.writeEntry("deletionBgColor", m_changeTracker->deletionBgColor());
        interface.writeEntry("formatChangeBgColor", m_changeTracker->formatChangeBgColor());
        KUser user(KUser::UseRealUserID);
        QString changeAuthor = m_changeTracker->authorName();
        if (changeAuthor != user.property(KUser::FullName).toString()) {
            interface.writeEntry("changeAuthor", changeAuthor);
        }
        interface.writeEntry("changeSaveFormat", (int)(m_changeTracker->saveFormat()));
    }
}

void TextTool::debugTextDocument()
{
#ifndef NDEBUG
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    int CHARSPERLINE = QString::fromLatin1(getenv("KOFFICE_DEBUG_COLUMNS")).toInt();
    if (CHARSPERLINE < 5)
        CHARSPERLINE = 80;
    const int CHARPOSITION = 278301935;
    KTextDocument document(textEditor->document());
    KStyleManager *styleManager = document.styleManager();
    KInlineTextObjectManager *inlineManager = document.inlineTextObjectManager();

    QTextBlock block = textEditor->document()->begin();
    for (;block.isValid(); block = block.next()) {
        QVariant var = block.blockFormat().property(KParagraphStyle::StyleId);
        if (!var.isNull()) {
            KParagraphStyle *ps = styleManager->paragraphStyle(var.toInt());
            kDebug(32500) << "--- Paragraph Style:" << (ps ? ps->name() : QString()) << var.toInt();
        }
        var = block.charFormat().property(KCharacterStyle::StyleId);
        if (!var.isNull()) {
            KCharacterStyle *cs = styleManager->characterStyle(var.toInt());
            kDebug(32500) << "--- Character Style:" << (cs ? cs->name() : QString()) << var.toInt();
        }
        int lastPrintedChar = -1;
        QTextBlock::iterator it;
        QString fragmentText;
        QList<QTextCharFormat> inlineCharacters;
        for (it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (!fragment.isValid())
                continue;
            QTextCharFormat fmt = fragment.charFormat();
            kDebug(32500) << "changeId: " << fmt.property(KCharacterStyle::ChangeTrackerId);
            const int fragmentStart = fragment.position() - block.position();
            for (int i = fragmentStart; i < fragmentStart + fragment.length(); i += CHARSPERLINE) {
                if (lastPrintedChar == fragmentStart-1)
                    fragmentText += '|';
                if (lastPrintedChar < fragmentStart || i > fragmentStart) {
                    QString debug = block.text().mid(lastPrintedChar, CHARSPERLINE);
                    lastPrintedChar += CHARSPERLINE;
                    if (lastPrintedChar > block.length())
                        debug += "\\n";
                    kDebug(32500) << debug;
                }
                var = fmt.property(KCharacterStyle::StyleId);
                QString charStyleLong, charStyleShort;
                if (! var.isNull()) { // named style
                    charStyleShort = QString::number(var.toInt());
                    KCharacterStyle *cs = styleManager->characterStyle(var.toInt());
                    if (cs)
                        charStyleLong = cs->name();
                }
                if (inlineManager && fmt.hasProperty(KCharacterStyle::InlineInstanceId)) {
                    QTextCharFormat inlineFmt = fmt;
                    inlineFmt.setProperty(CHARPOSITION, fragmentStart);
                    inlineCharacters << inlineFmt;
                }

                if (fragment.length() > charStyleLong.length())
                    fragmentText += charStyleLong;
                else if (fragment.length() > charStyleShort.length())
                    fragmentText += charStyleShort;
                else if (fragment.length() >= 2)
                    fragmentText += QChar(8230); // elipses



                int rest =  fragmentStart - (lastPrintedChar-CHARSPERLINE) + fragment.length() - fragmentText.length();
                rest = qMin(rest, CHARSPERLINE - fragmentText.length());
                if (rest >= 2)
                    fragmentText = QString("%1%2").arg(fragmentText).arg(' ', rest);
                if (rest >= 0)
                    fragmentText += '|';
                if (fragmentText.length() >= CHARSPERLINE) {
                    kDebug(32500) << fragmentText;
                    fragmentText.clear();
                }
            }
        }
        if (!fragmentText.isEmpty()) {
            kDebug(32500) << fragmentText;
        }
        else if (block.length() == 1) { // no actual tet
            kDebug(32500) << "\\n";
        }
        foreach (const QTextCharFormat &cf, inlineCharacters) {
            KInlineObject *object= inlineManager->inlineTextObject(cf);
            kDebug(32500) << "At pos:" << cf.intProperty(CHARPOSITION) << object;
            // kDebug(32500) << "-> id:" << cf.intProperty(577297549);
        }
        QTextList *list = block.textList();
        if (list) {
            if (list->format().hasProperty(KListStyle::StyleId)) {
                KListStyle *ls = styleManager->listStyle(list->format().intProperty(KListStyle::StyleId));
                kDebug(32500) << "   List style applied:" << ls->styleId() << ls->name();
            }
            else
                kDebug(32500) << " +- is a list..." << list;
        }
    }
#endif
}

void TextTool::debugTextStyles()
{
#ifndef NDEBUG
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    KTextDocument document(textEditor->document());
    KStyleManager *styleManager = document.styleManager();

    QSet<int> seenStyles;

    foreach (KParagraphStyle *style, styleManager->paragraphStyles()) {
        kDebug(32500) << style->styleId() << style->name() << (styleManager->defaultParagraphStyle() == style ? "[Default]" : "");
        if (style->parentStyle())
            kDebug(32500) << "  +- With parent style; " << style->parentStyle()->styleId();
        KCharacterStyle *cs = style->characterStyle();
        seenStyles << style->styleId();
        if (cs) {
            kDebug(32500) << "  +- CharStyle: " << cs->styleId() << cs->name();
        if (cs->hasProperty(QTextFormat::ForegroundBrush))
            kDebug(32500) << "  |  " << cs->foreground();
        if (cs->hasProperty(QTextFormat::FontPointSize))
            kDebug(32500) << "  |  " << cs->font();
            seenStyles << cs->styleId();
        } else {
            kDebug(32500) << "  +- ERROR; no char style found!" << endl;
        }
        KListStyle *ls = style->listStyle();
        if (ls) { // optional ;)
            kDebug(32500) << "  +- ListStyle: " << ls->styleId() << ls->name()
                << (ls == styleManager->defaultListStyle() ? "[Default]":"");
            foreach (int level, ls->listLevels()) {
                KListLevelProperties llp = ls->levelProperties(level);
                kDebug(32500) << "  |  level" << llp.level() << " style (enum):" << llp.style();
                if (llp.bulletCharacter().unicode() != 0) {
                    kDebug(32500) << "  |  bullet" << llp.bulletCharacter();
                }
            }
            seenStyles << ls->styleId();
        }
    }

    bool first = true;
    foreach (KCharacterStyle *style, styleManager->characterStyles()) {
        if (seenStyles.contains(style->styleId()))
            continue;
        if (first) {
            kDebug(32500) << "--- Character styles ---";
            first = false;
        }
        kDebug(32500) << style->styleId() << style->name();
        kDebug(32500) << style->font();
    }

    first = true;
    foreach (KListStyle *style, styleManager->listStyles()) {
        if (seenStyles.contains(style->styleId()))
            continue;
        if (first) {
            kDebug(32500) << "--- List styles ---";
            first = false;
        }
        kDebug(32500) << style->styleId() << style->name()
                << (style == styleManager->defaultListStyle() ? "[Default]":"");
    }
#endif
}

#include <TextTool.moc>
