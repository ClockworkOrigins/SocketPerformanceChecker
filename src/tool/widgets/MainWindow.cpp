/*
 * SocketPerformanceTester
 * Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
 *
 * This file is part of i6engine; i6engine is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "widgets/MainWindow.h"

#include "spcBuildSettings.h"

#include <QCloseEvent>
#include <QStandardItemModel>

namespace spc {
namespace widgets {

	MainWindow::MainWindow(QMainWindow * par) : QMainWindow(par) {
		setupUi(this);

		setWindowTitle(QString("SocketPerformanceChecker (v ") + QString::number(SPC_VERSION_MAJOR) + QString(".") + QString::number(SPC_VERSION_MINOR) + QString(".") + QString::number(SPC_VERSION_PATCH) + QString(")"));

		QHeaderView * header = new QHeaderView(Qt::Orientation::Horizontal, resultTableView);
		QStandardItemModel * model = new QStandardItemModel(header);
		QStringList horizontalHeader;
		horizontalHeader.append("Socket implementation");
		horizontalHeader.append("Duration in ms");
		model->setHorizontalHeaderLabels(horizontalHeader);
		model->index(0, 0, model->index(1, 1));
		header->setModel(model);
		resultTableView->setHorizontalHeader(header);
		resultTableView->resizeRowsToContents();
		resultTableView->resizeColumnsToContents();
	}

	MainWindow::~MainWindow() {
	}

	void MainWindow::closeTool() {
		qApp->exit();
	}

	void MainWindow::startTest() {
	}

	void MainWindow::closeEvent(QCloseEvent * evt) {
		closeTool();
		evt->ignore();
	}

} /* namespace widgets */
} /* namespace spc */
