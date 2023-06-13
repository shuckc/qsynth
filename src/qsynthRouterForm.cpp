// qsynthRouterForm.cpp
//
/****************************************************************************
   Copyright (C) 2003-2020, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthAbout.h"
#include "qsynthRouterForm.h"

#include "qsynthOptions.h"

#include <QHeaderView>
#include <QPushButton>
#include <QFileInfo>
#include <QItemDelegate>
#include <QLineEdit>
#include <QComboBox>

class CustomDelegate : public QItemDelegate
{
public:
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
							 const QModelIndex & index) const
	{
		if (index.column()==0) {
			// these are 'FLUID_MIDI_ROUTER_RULE_*' constants
			QComboBox *comboEdit = new QComboBox(parent);
			comboEdit->addItem("CC");
			comboEdit->addItem("Note");
			comboEdit->addItem("Prog Change");
			comboEdit->addItem("Pitch Bend");
			comboEdit->addItem("Channel Pressure");
			comboEdit->addItem("Key Pressure");
			comboEdit->addItem("Count");
			// combo.currentIndexChanged.connect(self.currentIndexChanged)
			return comboEdit;
		} else {
			QLineEdit *lineEdit = new QLineEdit(parent);
			// TODO: for match columns, permit a singleton or range of values e.g. '0-16'
			QIntValidator *validator = new QIntValidator(0, 16, lineEdit);
			lineEdit->setValidator(validator);
			return lineEdit;
		}
	 }
};


// Custom list-view item (as for numerical sort purposes...)
class qsynthRouterRuleItem : public QTreeWidgetItem
{
public:

	// Constructor.
	qsynthRouterRuleItem(QTreeWidget *pListView,
		QTreeWidgetItem *pItemAfter)
		: QTreeWidgetItem(pListView, pItemAfter) {}

	// Sort/compare overriden method.
	bool operator< (const QTreeWidgetItem& other) const
	{
		int iColumn = QTreeWidgetItem::treeWidget()->sortColumn();
		const QString& s1 = text(iColumn);
		const QString& s2 = other.text(iColumn);
		//if (iColumn == 0 || iColumn == 2) {
		//	return (s1.toInt() < s2.toInt());
		//} else {
			return (s1 < s2);
		//}
	}
};


//----------------------------------------------------------------------------
// qsynthRouterForm -- UI wrapper form.

// Constructor.
qsynthRouterForm::qsynthRouterForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	m_pSynth = nullptr;
	m_iChan  = 0;
	m_iBank  = 0;
	m_iProg  = 0;

	// To avoid setup jitterness and preview side effects.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Some default sorting, initially.
	// m_ui.RulesListView->setSorting(0);

	// Soundfonts list view...
	QHeaderView *pHeader = m_ui.RulesListView->header();
	pHeader->setDefaultAlignment(Qt::AlignLeft);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	pHeader->setSectionsMovable(false);
#else
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setMovable(false);
#endif
	pHeader->setStretchLastSection(true);

	m_ui.RulesListView->resizeColumnToContents(0);
	m_ui.RulesListView->resizeColumnToContents(1);
	m_ui.RulesListView->resizeColumnToContents(2);
	m_ui.RulesListView->resizeColumnToContents(3);
	m_ui.RulesListView->resizeColumnToContents(4);
	m_ui.RulesListView->resizeColumnToContents(5);
	m_ui.RulesListView->resizeColumnToContents(6);

	// Initial sort order...
	m_ui.RulesListView->sortItems(0, Qt::AscendingOrder);

	printf("construct fired\n");
	// Clear up the program listview.
	m_ui.RulesListView->setUpdatesEnabled(false);
	m_ui.RulesListView->clear();

	// TODO: take strings from settings object and csv parse
	QTreeWidgetItem *pProgItem = nullptr;
	for (int i = 0; i < 5; ++i) {
		printf("adding item\n");
		pProgItem = new qsynthRouterRuleItem(m_ui.RulesListView, pProgItem);
		if (pProgItem) {
			const QString sName = "CC";
			// pPreset->get_name(pPreset);
			pProgItem->setText(0, sName);										// event
			pProgItem->setText(1, QString("%1 - %2").arg(0).arg(1)); // ch match
			pProgItem->setText(2, i==3 ? QString::number(3) : "-");  // ch set
			pProgItem->setText(3, QString::number(i));
			pProgItem->setText(4, i==3 ? QString::number(3) : "-");  // ch set
			pProgItem->setText(5, QString::number(i));
			pProgItem->setText(6, i==3 ? QString::number(3) : "-");  // ch set
			pProgItem->setFlags(pProgItem->flags() | Qt::ItemIsEditable);
		}
	}
	m_ui.RulesListView->setUpdatesEnabled(true);
	m_ui.RulesListView->setItemDelegate(new CustomDelegate);
	printf("construct add rules done\n");

	// UI connections...
	//QObject::connect(m_ui.RulesListView,
	//	SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	//	SLOT(progChanged()));
	QObject::connect(m_ui.PreviewCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(previewChanged()));
	//QObject::connect(m_ui.RulesListView,
	//	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
	//	SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));
}


// Destructor.
qsynthRouterForm::~qsynthRouterForm (void)
{
}


// Dialog setup loader.
void qsynthRouterForm::setup ( qsynthOptions *pOptions, fluid_synth_t *pSynth, int iChan )
{
	// Set our internal stuff...
	m_pOptions = pOptions;
	m_pSynth = pSynth;
	m_iChan  = iChan;

	printf("setup fired\n");

	// We'll goinfg to changes the whole thing...
	m_iDirtySetup++;

	// And the preview state...
	m_ui.PreviewCheckBox->setChecked(m_pOptions->bPresetPreview);

	// Done with setup...
	m_iDirtySetup--;
}


// Stabilize current state form.
void qsynthRouterForm::stabilizeForm()
{
	m_ui.DialogButtonBox->button(
		QDialogButtonBox::Ok)->setEnabled(validateForm());
}


// Validate form fields.
bool qsynthRouterForm::validateForm()
{
	bool bValid = true;

	bValid = bValid && (m_ui.RulesListView->currentItem() != nullptr);

	return bValid;
}


// Validate form fields and accept it valid.
void qsynthRouterForm::accept()
{

}


// Reject settings (Cancel button slot).
void qsynthRouterForm::reject (void)
{
	// Reset selection to initial selection, if applicable...
	if (m_iDirtyCount > 0) {
		// setBankProg(m_iBank, m_iProg);
	}
	// Done (hopefully nothing).
	// QDialog::reject();
}

// Preview change slot.
void qsynthRouterForm::previewChanged (void)
{
	// Just like a program change, if not on setup...
	// if (m_iDirtySetup == 0)
		// progChanged();
}


// end of qsynthRouterForm.cpp
