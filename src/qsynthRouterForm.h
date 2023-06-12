// qsynthRouterForm.h
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

#ifndef __qsynthRouterForm_h
#define __qsynthRouterForm_h

#include "ui_qsynthRouterForm.h"

#include <fluidsynth.h>

// Forward declarations.
class qsynthOptions;


//----------------------------------------------------------------------------
// qsynthRouterForm -- UI wrapper form.

class qsynthRouterForm : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthRouterForm(QWidget *pParent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());

	// Destructor.
	~qsynthRouterForm();


	void setup(qsynthOptions *pOptions, fluid_synth_t *pSynth, int iChan);

public slots:

	void stabilizeForm();
	void bankChanged();
	void progChanged();
	void previewChanged();

protected slots:

	void accept();
	void reject();

protected:

	void setBankProg(int iBank, int iProg);

	QTreeWidgetItem *findBankItem(int iBank);
	QTreeWidgetItem *findProgItem(int iProg);

	bool validateForm();

private:

	// The Qt-designer UI struct...
	Ui::qsynthRouterForm m_ui;

	// Instance variables.
	qsynthOptions *m_pOptions;
	fluid_synth_t *m_pSynth;

	int m_iChan;
	int m_iBank;
	int m_iProg;

	int m_iDirtySetup;
	int m_iDirtyCount;
};


#endif	// __qsynthRouterForm_h


// end of qsynthRouterForm.h
