/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KIllustrator_view_h_
#define KIllustrator_view_h_

#include <CommandHistory.h>
#include <MainView.h>
#include <koView.h>

const int ID_TOOL_SELECT=1001;
const int ID_TOOL_EDITPOINT=1002;
const int ID_TOOL_FREEHAND=1003;
const int ID_TOOL_LINE=1004;
const int ID_TOOL_BEZIER=1005;
const int ID_TOOL_RECTANGLE=1006;
const int ID_TOOL_POLYGON=1007;
const int ID_TOOL_ELLIPSE=1008;
const int ID_TOOL_TEXT=1009;
const int ID_TOOL_ZOOM=1010;
const int ID_TOOL_PATHTEXT=1011;
const int ID_TOOL_INSERTPART=1012;

class KIllustratorView;
class KIllustratorChild;
class KIllustratorDocument;
class Canvas;
class GDocument;
class LayerDialog;
class ToolController;
class Canvas;
class Ruler;
class EditPointTool;
class InsertPartTool;
class GPart;
class FilterManager;
class CanvasView;
class QWidget;

class KAction;
class KToggleAction;
class KColorBarAction;
class KSelectAction;
class KStatusBar;
class ZoomTool;

class KIllustratorView : public KoView, public MainView
{
    Q_OBJECT
public:
    KIllustratorView (QWidget* parent, const char* name = 0, KIllustratorDocument* doc = 0 );
    ~KIllustratorView ();

    void createMyGUI ();

    /**
     * Overloaded @ref MainView::activeDocument
     */
    GDocument* activeDocument ();
    /**
     * Overloaded @ref MainView::getCanvas
     */
    Canvas* getCanvas () { return canvas; }

    bool printDlg();

/*
    void editInsertObject ();

    void setPenColor (long int id);
    void setFillColor (long int id);

    void configPolygonTool ();
    void configEllipseTool ();

    void zoomSizeSelected (const QString & s);
*/

protected:
    void readConfig();
    void writeConfig();

    virtual void guiActivateEvent( KParts::GUIActivateEvent *ev );
    void showTransformationDialog (int id);

    void setupCanvas ();
    void setupPopups ();
    void resizeEvent (QResizeEvent*);

    virtual void updateReadWrite( bool readwrite );

    virtual void setupPrinter( QPrinter &printer );
    virtual void print( QPrinter &printer );

protected slots:
    void showNodesToolbar(bool show);
    void setUndoStatus( bool undoPossible, bool redoPossible );
    void popupForSelection (int x, int y);
    void resetTools();
    QString getExportFileName (FilterManager *filterMgr);

    void showCurrentMode (const QString &msg);
/*
       void insertPartSlot (KIllustratorChild *child, GPart *part);
       void changeChildGeometrySlot (KIllustratorChild *child); */

private slots:
    /**
     * Actions
     */
    void slotImport();
    void slotExport();
    void slotInsertBitmap();
    void slotInsertClipart();
    void slotCopy();
    void slotPaste();
    void slotCut();
    void slotUndo();
    void slotRedo();
    void slotDuplicate();
    void slotDelete();
    void slotSelectAll();
    void slotProperties();
    void slotOutline( bool );
    void slotNormal( bool );
    void slotShowRuler( bool );
    void slotShowGrid( bool );
    void slotShowHelplines( bool );
    void slotPage();
    void slotGrid();
    void slotHelplines();
    void slotAlignToGrid( bool );
    void slotAlignToHelplines( bool );
    void slotTransformPosition();
    void slotTransformDimension();
    void slotTransformRotation();
    void slotTransformMirror();
    void slotDistribute();
    void slotToFront();
    void slotToBack();
    void slotForwardOne();
    void slotBackOne();
    void slotGroup();
    void slotUngroup();
    void slotTextAlongPath();
    void slotConvertToCurve();
    void slotBlend();
    void slotOptions();
    void slotBrushChosen( const QColor & );
    void slotPenChosen( const QColor & );
    void slotSelectTool( bool );
    void slotPointTool( bool );
    void slotFreehandTool( bool );
    void slotLineTool( bool );
    void slotBezierTool( bool );
    void slotRectTool( bool );
    void slotPolygonTool( bool );
    void slotEllipseTool( bool );
    void slotTextTool( bool );
    void slotZoomTool( bool );
    void slotMoveNode( bool );
    void slotNewNode( bool );
    void slotDeleteNode( bool );
    void slotSplitLine( bool );
    void slotLayers();
    void slotViewZoom (const QString&);
    void slotLoadPalette ();
    void slotConfigurePolygon();
    void slotConfigureEllipse();
    void slotAddHelpline(int x, int y, bool d);
    void slotZoomFactorChanged(float factor, int xpos, int ypos);
    void slotSettingsChanged();
    void slotZoomIn();
    void slotZoomOut();
    void slotViewResize();
    
protected:
    KIllustratorDocument *m_pDoc;
    EditPointTool *editPointTool;
    InsertPartTool *insertPartTool;
    QPopupMenu *objMenu;
    QWidget *mParent;

    bool m_bShowGUI;
    bool m_bShowRulers;

    ToolController *tcontroller;
    CanvasView *scrollview;
    Canvas *canvas;
    Ruler *hRuler, *vRuler;
    KStatusBar *statusbar;
    LayerDialog *layerDialog;
    CommandHistory cmdHistory;

    ZoomTool *mZoomTool;

    KAction* m_copy;
    KAction* m_cut;
    KAction *m_delete;
    KAction* m_undo;
    KAction* m_redo;
    KAction* m_properties;
    KAction* m_distribute;
    KAction* m_toFront;
    KAction* m_toBack;
    KAction* m_forwardOne;
    KAction* m_backOne;
    KToggleAction *m_showHelplines;
    KToggleAction* m_selectTool;
    KToggleAction* m_moveNode;
    KToggleAction* m_newNode;
    KToggleAction* m_deleteNode;
    KToggleAction* m_splitLine;
    KSelectAction *m_viewZoom;
    QString lastOpenDir, lastSaveDir, lastBitmapDir, lastClipartDir,
            lastExportDir, lastImportDir, lastPaletteDir;
    QString lastExport;
};

#endif
