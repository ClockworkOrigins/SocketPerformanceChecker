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

#ifndef __SPC_WIDGETS_MAINWINDOW_H__
#define __SPC_WIDGETS_MAINWINDOW_H__

#include "ui_wndMainWindow.h"

namespace spc {
namespace plugins {
	class SocketPluginInterface;
} /* namespace plugins */
namespace widgets {

	class MainWindow : public QMainWindow, public Ui::wndMainWindow {
		Q_OBJECT

	public:
		MainWindow(QMainWindow * par = nullptr);
		~MainWindow();

	private slots:
		/**
		 * \brief shuts test down
		 */
		void closeTool();

		/**
		 * \brief starts the configured test
		 */
		void startTest();

	private:
		/**
		 * \brief stores all loaded SocketPlugins
		 */
		std::map<std::string, plugins::SocketPluginInterface *> _socketPlugins;

		/**
		 * \brief used to capture pressing 'x' to close Window or close Window using ALT + F4
		 * calls closeTool()
		 */
		void closeEvent(QCloseEvent * evt) override;

		/**
		 * \brief loads all possible plugin types, actually only SocketPlugins
		 */
		void loadPlugins();

		/**
		 * \brief loads all SocketPlugins
		 */
		void loadSocketPlugins();
	};

} /* namespace widgets */
} /* namespace spc */

#endif /* __SPC_WIDGETS_MAINWINDOW_H__ */
