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

#include "widgets/MainWindow.h"

#include <thread>

#include "spcBuildSettings.h"

#include "widgets/AboutDialog.h"
#include "widgets/LineChartWidget.h"

#include "MessageStructs.h"

#include "plugins/SocketPluginInterface.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDir>
#include <QGraphicsView>
#include <QMessageBox>
#include <QFileDialog>
#include <QPluginLoader>
#include <QStandardItemModel>

namespace spc {
namespace widgets {

	class NumberStandardItem : public QStandardItem {
	public:
		NumberStandardItem(int number) : QStandardItem(QString::number(number)), _number(number) {
		}

		bool operator<(const QStandardItem & other) const override {
			return _number < dynamic_cast<const NumberStandardItem *>(&other)->_number;
		}

		int type() const override {
			return UserType;
		}

	private:
		int _number;
	};

	MainWindow::MainWindow(QMainWindow * par) : QMainWindow(par), _socketPlugins(), _completeMessageAmount(), _processedMessageAmount(0), _triggerUpdateThreshold(1), _controlSocket(nullptr), _lineChartGraphicsScene(nullptr), _lineChartWidget(nullptr), _lineChartGraphicsView(nullptr), _measuredDurations(), _aboutDialog(new AboutDialog(this)), _abort(false) {
		setupUi(this);

		setWindowIcon(QIcon(":/icon.png"));

		setWindowTitle(QString("SocketPerformanceChecker (v ") + QString::number(SPC_VERSION_MAJOR) + QString(".") + QString::number(SPC_VERSION_MINOR) + QString(".") + QString::number(SPC_VERSION_PATCH) + QString(")"));

		progressBar->hide();
		abortButton->hide();

		QHeaderView * header = new QHeaderView(Qt::Orientation::Horizontal, resultTableView);
		QStandardItemModel * model = new QStandardItemModel(header);
		QStringList horizontalHeader;
		horizontalHeader.append("Socket implementation");
		horizontalHeader.append("Complete Duration in ms");
		horizontalHeader.append("Avg. Duration in ms");
		horizontalHeader.append("Min. Duration in ms");
		horizontalHeader.append("Max. Duration in ms");
		horizontalHeader.append("Throughput in messages per seconds");
		horizontalHeader.append("Runs");
		horizontalHeader.append("Message Count");
		horizontalHeader.append("Payload Size in Bytes");
		model->setHorizontalHeaderLabels(horizontalHeader);
		header->setModel(model);
		header->setSectionsClickable(true);
		resultTableView->setHorizontalHeader(header);
		resultTableView->resizeRowsToContents();
		resultTableView->resizeColumnsToContents();

		resultTableView->setModel(model);

		_lineChartGraphicsScene = new QGraphicsScene(this);
		_lineChartGraphicsView = new QGraphicsView(_lineChartGraphicsScene);

		_lineChartGraphicsView->hide();
		_lineChartGraphicsView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
		_lineChartGraphicsView->setCacheMode(QGraphicsView::CacheBackground);

		_lineChartGraphicsView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

		lineChartLayout->addWidget(_lineChartGraphicsView);

		actionExport_result->setDisabled(true);

		loadPlugins();

		addSocketCheckboxes();

		qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint64_t>");

		connect(this, SIGNAL(finishedSocket(QString, std::vector<uint64_t>)), this, SLOT(updateSocketResults(QString, std::vector<uint64_t>)), Qt::QueuedConnection);
		connect(this, SIGNAL(updateProgress()), this, SLOT(updateProgressBar()), Qt::QueuedConnection);
		connect(this, SIGNAL(finishedTest()), this, SLOT(testFinished()), Qt::QueuedConnection);
		connect(this, SIGNAL(addErrorMessageBox(QString, QString)), this, SLOT(showErrorMessageBox(QString, QString)), Qt::QueuedConnection);
		connect(resultTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectedRow(const QModelIndex &)));
		connect(header, SIGNAL(sectionClicked(int)), this, SLOT(selectedHeaderColumn(int)));
	}

	MainWindow::~MainWindow() {
		delete _controlSocket;
	}

	void MainWindow::closeTool() {
		qApp->exit();
	}

	void MainWindow::showAboutDialog() {
		_aboutDialog->show();
	}

	void MainWindow::startTest() {
		_abort = false;
		QString ip = ipLineEdit->text();
		uint16_t port = uint16_t(portSpinBox->value());
		QStringList socketList;
		for (auto sp : _socketPlugins) {
			if (sp.second.second->isChecked()) {
				socketList.append(sp.first);
			}
		}
		uint32_t runs = uint32_t(runsSpinBox->value());
		uint32_t messageCount = uint32_t(messageCountSpinBox->value());
		uint32_t payloadSize = uint32_t(payloadSizeSpinBox->value());

		_completeMessageAmount = socketList.size() * runs * messageCount;
		_processedMessageAmount = 0;
		_measuredDurations.clear();

		actionExport_result->setDisabled(true);

		if (_completeMessageAmount == 0) {
			emit addErrorMessageBox("Can't start test.", "No socket implementations chosen to be tested. Starting test is stopped.");
			return;
		}

		// 0.1% of _completeMessageAmount or 1
		_triggerUpdateThreshold = std::max(uint64_t(1), _completeMessageAmount / 1000);
		
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(resultTableView->model());
		while (model->rowCount() > 0) {
			model->removeRow(0);
		}

		enableGUI(false);

		progressBar->setValue(0);
		progressBar->setMaximum(_completeMessageAmount);

		delete _controlSocket;
		_controlSocket = nullptr;

		std::thread(std::bind(&MainWindow::performTest, this, ip, port, socketList, runs, messageCount, payloadSize)).detach();
	}

	void MainWindow::abortTest() {
		_abort = true;
	}

	void MainWindow::updateSocketResults(QString pluginName, std::vector<uint64_t> durations) {
		uint64_t minimumDuration = UINT64_MAX;
		uint64_t maximumDuration = 0;
		uint64_t durationSum = 0;
		for (uint64_t duration : durations) {
			if (minimumDuration > duration) {
				minimumDuration = duration;
			}
			if (maximumDuration < duration) {
				maximumDuration = duration;
			}
			durationSum += duration;
		}

		_measuredDurations.insert(std::make_pair(pluginName, durations));

		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(resultTableView->model());
		int rowCount = model->rowCount();
		// first column: plugin name
		QStandardItem * newItem = new QStandardItem(pluginName);
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 0, newItem);

		// second column: complete duration
		newItem = new NumberStandardItem(durationSum / 1000);
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 1, newItem);

		// third column: average duration
		newItem = new NumberStandardItem(uint64_t(durationSum / double(durations.size()) / 1000));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 2, newItem);

		// fourth column: minimum duration
		newItem = new NumberStandardItem(minimumDuration / 1000);
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 3, newItem);

		// fifth column: maximum duration
		newItem = new NumberStandardItem(maximumDuration / 1000);
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 4, newItem);

		// sixth column: throughput
		newItem = new NumberStandardItem(uint64_t(messageCountSpinBox->value() / (durationSum / double(durations.size()) / 1000000)));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 5, newItem);

		// seventh column: runs
		newItem = new NumberStandardItem(durations.size());
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 6, newItem);

		// eighth column: message count
		newItem = new NumberStandardItem(messageCountSpinBox->value());
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 7, newItem);

		// ninth column: payloadSize
		newItem = new NumberStandardItem(payloadSizeSpinBox->value());
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		model->setItem(rowCount, 8, newItem);

		resultTableView->sortByColumn(2, Qt::SortOrder::AscendingOrder);
	}

	void MainWindow::updateProgressBar() {
		progressBar->setValue(_processedMessageAmount);
	}

	void MainWindow::testFinished() {
		enableGUI(true);
	}

	void MainWindow::showErrorMessageBox(QString title, QString message) const {
		QMessageBox box;
		box.setWindowTitle(title);
		box.setInformativeText(message);
		box.setStandardButtons(QMessageBox::StandardButton::Ok);
		box.exec();
	}

	void MainWindow::selectedRow(const QModelIndex & index) {
		int idx = index.row();
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(resultTableView->model());

		if (_lineChartWidget) {
			_lineChartGraphicsScene->removeItem(_lineChartWidget);
			delete _lineChartWidget;
			_lineChartWidget = nullptr;
		}
		_lineChartWidget = new LineChartWidget(_measuredDurations[model->item(idx, 0)->text()]);
		_lineChartWidget->setXAxisText("Run");
		_lineChartWidget->setYAxisText("Duration in microseconds");
		_lineChartGraphicsScene->addItem(_lineChartWidget);
		_lineChartGraphicsView->show();
		_lineChartGraphicsView->fitInView(_lineChartWidget);
	}

	void MainWindow::selectedHeaderColumn(int columnIndex) {
		resultTableView->sortByColumn(columnIndex);
	}

	void MainWindow::exportResult() {
		QString filename = QFileDialog::getSaveFileName(this, "Export results", ".", "Comma-separated values (*.csv)");
		if (!filename.isEmpty()) {
			QFileInfo info(filename);
			if (info.suffix() != "csv") {
				filename += ".csv";
			}
			QFile output(filename);
			if (output.open(QIODevice::WriteOnly)) {
				std::vector<std::vector<std::string>> table;
				size_t maxLength = 0;
				for (auto p : _measuredDurations) {
					std::vector<std::string> column;
					column.push_back(p.first.toStdString());
					for (uint64_t v : p.second) {
						column.push_back(std::to_string(v));
					}
					table.push_back(column);
					maxLength = std::max(maxLength, p.second.size());
				}

				for (size_t i = 0; i < maxLength + 1; i++) { // +1 because label also contained
					std::string line;
					for (size_t j = 0; j < table.size(); j++) {
						if (table[j].size() >= i) {
							line += table[j][i];
						}
						if (j < table[j].size() - 1) {
							line += ";";
						}
					}
					output.write(line.c_str(), line.length());
					output.write("\n");
				}
			}
		}
	}

	void MainWindow::closeEvent(QCloseEvent * evt) {
		closeTool();
		evt->ignore();
	}

	void MainWindow::resizeEvent(QResizeEvent * evt) {
		QMainWindow::resizeEvent(evt);
		for (int i = 0; i < resultTableView->model()->columnCount(); i++) {
			resultTableView->setColumnWidth(i, width() / resultTableView->model()->columnCount());
		}
	}

	void MainWindow::loadPlugins() {
		loadSocketPlugins();
	}

	void MainWindow::loadSocketPlugins() {
		QDir pluginsDir = QDir(qApp->applicationDirPath() + "/plugins/sockets");
		foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
			QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
			QObject * plugin = loader.instance();
			if (plugin) {
				plugins::SocketPluginInterface * spi = qobject_cast<plugins::SocketPluginInterface *>(plugin);
				QCheckBox * cb = new QCheckBox(spi->getName(), this);
				cb->setChecked(true);
				_socketPlugins.insert(std::make_pair(spi->getName(), std::make_pair(spi, cb)));
			} else {
				emit addErrorMessageBox("Error loading plugin.", loader.errorString());
			}
		}
	}

	void MainWindow::addSocketCheckboxes() {
		uint32_t counter = 0;
		for (auto sp : _socketPlugins) {
			socketPluginLayout->addWidget(sp.second.second, counter / 3, counter % 3);
			counter++;
		}
	}

	void MainWindow::performTest(QString ip, uint16_t port, QStringList socketList, uint32_t runs, uint32_t messageCount, uint32_t payloadSize) {
		QString message(payloadSize, QChar('a'));
		_controlSocket = new clockUtils::sockets::TcpSocket();
		if (clockUtils::ClockError::SUCCESS != _controlSocket->connectToIP(ip.toStdString(), CONTROL_PORT, 1000)) {
			emit addErrorMessageBox("Connection failed.", "Can't connect to SocketPerformanceHelper. Maybe you haven't started it yet or the entered ip address or port are wrong.");
			emit finishedTest();
			return;
		}
		uint32_t pluginCounter = 0;
		for (QString socketPluginName : socketList) {
			if (_abort) {
				break;
			}
			std::vector<uint64_t> durations;
			plugins::SocketPluginInterface * socketPlugin = _socketPlugins[socketPluginName].first;
			uint8_t retries = 1;
			for (uint32_t i = 0; i < runs; i++) {
				// connect to helper and communicate which SocketPlugin to prepare
				common::ListenOnPluginAndPortMessage lopapm;
				lopapm.pluginName = socketPluginName.toStdString();
				lopapm.port = port;
				_controlSocket->writePacket(lopapm.Serialize());

				std::string reply;
				if (clockUtils::ClockError::SUCCESS != _controlSocket->receivePacket(reply)) {
					emit addErrorMessageBox("SocketPerformanceHelper not responding.", "SocketPerformanceHelper doesn't respond. Seems like an error occured.");
					emit finishedTest();
					return;
				}
				common::Message * msg = common::Message::Deserialize(reply);
				if (msg->type != common::MessageType::LISTENING) {
					emit addErrorMessageBox("SocketPerformanceHelper sent wrong message.", "SocketPerformanceHelper has sent a wrong message. Maybe the version is wrong and not compatible.");
					delete msg;
					emit finishedTest();
					return;
				}
				// received LISTENING from helper, so real test can start now
				delete msg;
				// connect to socket
				if (!socketPlugin->connect(ip, port, QString::fromStdString(_controlSocket->getPublicIP()), std::bind(&MainWindow::receivedMessage, this))) {
					emit addErrorMessageBox("Plugin can't connect.", "Failed to connect plugin: " + socketPlugin->getName() + "\n" + QString::number(3 - retries) + " retries left.");
					emit updateProgress();
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					if (retries++ < 3) {
						continue;
					} else {
						break;
					}
				}
				// start time measuring
				std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
				// send all messages
				for (uint32_t j = 0; j < messageCount && !_abort; j++) {
					socketPlugin->sendMessage(message);
				}
				if (_abort) {
					break;
				}
				// wait until all messages are echoed or timeout is reached
				bool b = socketPlugin->waitForMessages(messageCount, 10000);
				// calculate duration to receive all message
				uint64_t duration = uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
				if (!b) {
					emit addErrorMessageBox("Plugin timed out.", "Plugin '" + socketPlugin->getName() + "' hasn't received all messages and timed out.\n" + QString::number(3 - retries) + " retries left.");
					if (retries++ == 3) {
						break;
					}
				}
				// disconnect plugin to be able to do a clean reconnect for next repetition or new test later on
				socketPlugin->disconnect();
				// store current result
				if (b) { // otherwise not all messages were received
					durations.push_back(duration);
				}
			}
			if (!durations.empty()) {
				emit finishedSocket(socketPluginName, durations);
			}
			pluginCounter++;
			_processedMessageAmount = pluginCounter * messageCount * runs;
			emit updateProgress();
		}
		_processedMessageAmount = _completeMessageAmount;
		emit updateProgress();
		emit finishedTest();
	}

	void MainWindow::receivedMessage() {
		_processedMessageAmount++;
		if (_processedMessageAmount % _triggerUpdateThreshold == 0) {
			emit updateProgress();
		}
	}

	void MainWindow::enableGUI(bool enabled) {
		runsSpinBox->setEnabled(enabled);
		messageCountSpinBox->setEnabled(enabled);
		payloadSizeSpinBox->setEnabled(enabled);
		ipLineEdit->setEnabled(enabled);
		portSpinBox->setEnabled(enabled);
		if (enabled) {
			progressBar->hide();
			abortButton->hide();
			startButton->show();
		} else {
			progressBar->show();
			abortButton->show();
			startButton->hide();
		}
		for (auto sp : _socketPlugins) {
			sp.second.second->setEnabled(enabled);
		}

		actionExport_result->setEnabled(enabled && !_measuredDurations.empty());
	}

} /* namespace widgets */
} /* namespace spc */
