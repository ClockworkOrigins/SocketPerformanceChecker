/*
 * SocketPerformanceTester
 * Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
 *
 * This file is part of SocketPerformanceTester; SocketPerformanceTester is free software; you can redistribute it and/or
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

#include <atomic>
#include <cstdint>

class QCheckBox;
class QGraphicsScene;
class QGraphicsView;

namespace clockUtils {
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spc {
namespace plugins {
	class SocketPluginInterface;
} /* namespace plugins */
namespace widgets {

	class AboutDialog;
	class LineChartWidget;

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

		/**
		 * \brief trigger update progress when either a test finished or a message arrived
		 */
		void updateProgress();

		/**
		 * \brief triggers reenabling of the GUI after finishing all tests
		 */
		void finishedTest();

		/**
		 * \brief adds a message box printing an error message
		 */
		void addErrorMessageBox(QString, QString);

	private slots:
		/**
		 * \brief shuts test down
		 */
		void closeTool();

		/**
		 * \brief opens about dialog
		 */
		void showAboutDialog();

		/**
		 * \brief starts the configured test
		 */
		void startTest();

		/**
		 * \brief aborts the currently running test
		 */
		void abortTest();

		/**
		 * \brief called when a socket was updated in Qt thread, so now GUI can be updated
		 */
		void updateSocketResults(QString pluginName, std::vector<uint64_t> durations);

		/**
		 * \brief updates the progress bar with current amount of processed message
		 */
		void updateProgressBar();

		/**
		 * \brief called when the test has finished
		 * prints a message box and reenables the GUI
		 */
		void testFinished();

		/**
		 * \brief adds a message box printing an error message
		 */
		void showErrorMessageBox(QString title, QString message) const;

		/**
		 * \brief called when a row in result table is selected
		 */
		void selectedRow(const QModelIndex & index);

		/**
		 * \brief called when header is of table is clicked to sort by column
		 */
		void selectedHeaderColumn(int columnIndex);

		/**
		 * \brief opens file dialog to select output csv file to export results into
		 */
		void exportResult();

	private:
		/**
		 * \brief stores all loaded SocketPlugins
		 */
		std::map<QString, std::pair<plugins::SocketPluginInterface *, QCheckBox *>> _socketPlugins;

		/**
		 * \brief the complete amount of messages that has to be processed during this test session
		 */
		uint64_t _completeMessageAmount;

		/**
		 * \brief the current message amount that was already processed during current test session
		 */
		std::atomic<uint64_t> _processedMessageAmount;

		/**
		 * \brief at this amount of processed messages a new update is triggered
		 */
		uint64_t _triggerUpdateThreshold;

		/**
		 * \brief socket used to handle basic control flow between SocketPerformanceChecker and SocketPerformanceHelper
		 */
		clockUtils::sockets::TcpSocket * _controlSocket;

		/**
		 * \brief graphics scene containing line chart
		 */
		QGraphicsScene * _lineChartGraphicsScene;

		/**
		 * \brief widget representing a line chart displaying all runs of current measurement with their timings
		 */
		LineChartWidget * _lineChartWidget;

		/**
		 * \brief graphics view to show charts
		 */
		QGraphicsView * _lineChartGraphicsView;

		/**
		 * \brief contains all measured values
		 */
		std::map<QString, std::vector<uint64_t>> _measuredDurations;

		/**
		 * \brief about dialog
		 */
		AboutDialog * _aboutDialog;

		/**
		 * \brief stores whether current job shall be aborted or not
		 */
		bool _abort;

		/**
		 * \brief used to capture pressing 'x' to close Window or close Window using ALT + F4
		 * calls closeTool()
		 */
		void closeEvent(QCloseEvent * evt) override;

		/**
		 * \brief used to get resize event to resize columns of resultTableView to new size
		 */
		void resizeEvent(QResizeEvent * evt) override;

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

		/**
		 * \brief sets all interactive GUI elements to enabled/disabled
		 */
		void enableGUI(bool enabled);
	};

} /* namespace widgets */
} /* namespace spc */

#endif /* __SPC_WIDGETS_MAINWINDOW_H__ */
