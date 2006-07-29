/* This file is part of the KDE project
   Copyright (C) 2004-2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIALTERTABLEDIALOG_P_H
#define KEXIALTERTABLEDIALOG_P_H

#include "kexitabledesignerview.h"
#include <kcommand.h>

class KexiDataAwarePropertySet;

/*! @internal
 Command group, reimplemented to get access to commands().
 We need it to iterate through commands so we can perform a set of ALTER TABLE atomic actions. */
class CommandGroup : public KMacroCommand
{
	public:
		CommandGroup( const QString & name )
			: KMacroCommand(name)
		{}
		virtual ~CommandGroup() {} 
		const QPtrList<KCommand>& commands() const { return m_commands; }
};

/*! @internal
 Command history, reimplemented to get access to commands().
 We need it to iterate through commands so we can perform a set of ALTER TABLE atomic actions. */
class CommandHistory : public KCommandHistory
{
	Q_OBJECT
	public:
		CommandHistory(KActionCollection *actionCollection, bool withMenus = true);

		const QPtrList<KCommand>& commands() const { return m_commandsToUndo; }

		void addCommand(KCommand *command, bool execute = true);

	public slots:
		virtual void undo();
		virtual void redo();

	protected:
		QPtrList<KCommand> m_commandsToUndo, m_commandsToRedo;
};

//----------------------------------------------

//! @internal
class KexiTableDesignerViewPrivate
{
	public:
		KexiTableDesignerViewPrivate(KexiTableDesignerView* aDesignerView);
		~KexiTableDesignerViewPrivate();

		int generateUniqueId();

		/*! @internal
		 Sets property \a propertyName in property set \a set to \a newValue.
		 If \a commandGroup is not 0, a new ChangeFieldPropertyCommand object is added there as well. 
		 While setting the new value, addHistoryCommand_in_slotPropertyChanged_enabled is set to false,
		 so addHistoryCommand() wont be executed in slotPropertyChanged() as an answer to setting 
		 the property. 

		 If \a forceAddCommand is false (the default) and \a newValue does not differ from curent property value
		 (set[propertyName].value()), ChangeFieldPropertyCommand command is not added to the \a commandGroup.
		 Otherwise, command is always added.

		 \a rememberOldValue argument is passed to Property::setValue()

		 If \a slist and \a nlist if not NULL and not empty, these are passed to Property::setListData().
		 If \a slist and \a nlist if not NULL but empty, Property::setListData(0) is called.

		 addHistoryCommand_in_slotPropertyChanged_enabled is then set back to the original state.
		 */
		void setPropertyValueIfNeeded( const KoProperty::Set& set, const QCString& propertyName, 
			const QVariant& newValue, CommandGroup* commandGroup, 
			bool forceAddCommand = false, bool rememberOldValue = true,
			QStringList* const slist = 0, QStringList* const nlist = 0);

		/*! @internal
		 Used in updatePropertiesVisibility().
		 Does nothing if visibility should not be changed, i.e. when prop->isVisible()==visible,
		 otherwise sets changed to true and sets visibility of property \a prop to \a visible.
		*/
		void setVisibilityIfNeeded( const KoProperty::Set& set, KoProperty::Property* prop, 
			bool visible, bool &changed, CommandGroup *commandGroup );

		bool updatePropertiesVisibility(KexiDB::Field::Type fieldType, KoProperty::Set &set,
			CommandGroup *commandGroup = 0);

		QString messageForSavingChanges(bool &emptyTable);

		KexiTableDesignerView* designerView;

		KexiTableView *view; //!< helper

		KexiTableViewData *data;

		KexiDataAwarePropertySet *sets;

		int row; //!< used to know if a new row is selected in slotCellSelected()

		KToggleAction *action_toggle_pkey;

		KPopupTitle *contextMenuTitle;

		int uniqueIdCounter;

		//! internal
		int maxTypeNameTextWidth;
		//! Set to true in beforeSwitchTo() to avoid asking again in storeData()
		bool dontAskOnStoreData : 1;

		bool slotTogglePrimaryKeyCalled : 1;

		bool primaryKeyExists : 1;
		//! Used in slotPropertyChanged() to avoid infinite recursion
		bool slotPropertyChanged_primaryKey_enabled : 1;
		//! Used in slotPropertyChanged() to avoid infinite recursion
		bool slotPropertyChanged_subType_enabled : 1;
		//! used in slotPropertyChanged() to disable addHistoryCommand()
		bool addHistoryCommand_in_slotPropertyChanged_enabled : 1;
		//! used in slotRowUpdated() to disable addHistoryCommand()
		bool addHistoryCommand_in_slotRowUpdated_enabled : 1;
		//! used in slotAboutToDeleteRow() to disable addHistoryCommand()
		bool addHistoryCommand_in_slotAboutToDeleteRow_enabled : 1;
		//! used in slotRowInserted() to disable addHistoryCommand()
		bool addHistoryCommand_in_slotRowInserted_enabled : 1;

		//! used to disable slotBeforeCellChanged()
		bool slotBeforeCellChanged_enabled : 1;

//! @tood temp; remove this:
		//! Temporary flag, used for testingu the Alter Table machinery. Affects storeData()
		//! Used in slotExecuteRealAlterTable() to switch on real alter table for a while.
		bool tempStoreDataUsingRealAlterTable;

		KActionCollection* historyActionCollection;
		CommandHistory* history;
};

#endif
