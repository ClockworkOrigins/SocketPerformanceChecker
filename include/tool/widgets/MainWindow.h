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

#include <cstdint>

class QCheckBox;

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

	signals:
		/**
		 * \brief called when one socket completed all tests to trigger GUI update
		 */
		void finishedSocket(QString, std::vector<uint64_t>);

	private slots:
		/**
		 * \brief shuts test down
		 */
		void closeTool();

		/**
		 * \brief starts the configured test
		 */
		void startTest();

		/**
		 * \brief called when a socket was updated in Qt thread, so now GUI can be updated
		 */
		void updateSocketResults(QString pluginName, std::vector<uint64_t> durations);

	private:
		/**
		 * \brief stores all loaded SocketPlugins
		 */
		std::map<QString, std::pair<plugins::SocketPluginInterface *, QCheckBox *>> _socketPlugins;

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

		/**
		 * \brief adds checkboxes for SocketPlugins to MainWindow
		 */
		void addSocketCheckboxes();

		/**
		 * \brief runs the actual test
		 */
		void performTest(QString ip, uint16_t port, QStringList socketList, uint32_t runs, uint32_t messageCount, uint32_t payloadSize);

		/**
		 * \brief called from a non-Qt-thread when a message for the current test arrives
		 */
		void receivedMessage();
	};

} /* namespace widgets */
} /* namespace spc */

#endif /* __SPC_WIDGETS_MAINWINDOW_H__ */
