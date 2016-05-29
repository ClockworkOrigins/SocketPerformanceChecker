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

#include <thread>

#include "spcBuildSettings.h"

#include "MessageStructs.h"

#include "plugins/SocketPluginInterface.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QPluginLoader>
#include <QStandardItemModel>

#ifdef max
	#undef max
#endif

namespace spc {
namespace widgets {

	class QNumberStandardItem : public QStandardItem {
	public:
		QNumberStandardItem(QString text) : QStandardItem(text) {
		}

		bool operator<(const QNumberStandardItem & other) {
			return text().toInt() < other.text().toInt();
		}
	};

	MainWindow::MainWindow(QMainWindow * par) : QMainWindow(par), _socketPlugins(), _completeMessageAmount(), _processedMessageAmount(0), _triggerUpdateThreshold(1), _controlSocket(nullptr) {
		setupUi(this);

		setWindowTitle(QString("SocketPerformanceChecker (v ") + QString::number(SPC_VERSION_MAJOR) + QString(".") + QString::number(SPC_VERSION_MINOR) + QString(".") + QString::number(SPC_VERSION_PATCH) + QString(")"));

		QHeaderView * header = new QHeaderView(Qt::Orientation::Horizontal, resultTableView);
		QStandardItemModel * model = new QStandardItemModel(header);
		QStringList horizontalHeader;
		horizontalHeader.append("Socket implementation");
		horizontalHeader.append("Complete Duration in ms");
		horizontalHeader.append("Avg. Duration in ms");
		horizontalHeader.append("Min. Duration in ms");
		horizontalHeader.append("Max. Duration in ms");
		horizontalHeader.append("Runs");
		horizontalHeader.append("Message Count");
		horizontalHeader.append("Payload Size in Bytes");
		model->setHorizontalHeaderLabels(horizontalHeader);
		header->setModel(model);
		resultTableView->setHorizontalHeader(header);
		resultTableView->resizeRowsToContents();
		resultTableView->resizeColumnsToContents();

		resultTableView->setModel(model);

		loadPlugins();

		addSocketCheckboxes();

		qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint64_t>");

		connect(this, SIGNAL(finishedSocket(QString, std::vector<uint64_t>)), this, SLOT(updateSocketResults(QString, std::vector<uint64_t>)), Qt::QueuedConnection);
		connect(this, SIGNAL(updateProgress()), this, SLOT(updateProgressBar()), Qt::QueuedConnection);
		connect(this, SIGNAL(finishedTest()), this, SLOT(testFinished()), Qt::QueuedConnection);
	}

	MainWindow::~MainWindow() {
		delete _controlSocket;
	}

	void MainWindow::closeTool() {
		qApp->exit();
	}

	void MainWindow::startTest() {
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

		if (_completeMessageAmount == 0) {
			// TODO: error message
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
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(resultTableView->model());
		int rowCount = model->rowCount();
		// first column: plugin name
		QStandardItem * newItem = new QStandardItem(pluginName);
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 0, newItem);

		// second column: complete duration
		newItem = new QStandardItem(QString::number(durationSum / 1000));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 1, newItem);

		// third column: average duration
		newItem = new QNumberStandardItem(QString::number(uint64_t(durationSum / double(durations.size()) / 1000)));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 2, newItem);

		// fourth column: minimum duration
		newItem = new QStandardItem(QString::number(minimumDuration / 1000));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 3, newItem);

		// fifth column: maximum duration
		newItem = new QStandardItem(QString::number(maximumDuration / 1000));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 4, newItem);

		// sixth column: runs
		newItem = new QStandardItem(QString::number(durations.size()));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 5, newItem);

		// seventh column: message count
		newItem = new QStandardItem(QString::number(messageCountSpinBox->value()));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 6, newItem);

		// eighth column: payloadSize
		newItem = new QStandardItem(QString::number(payloadSizeSpinBox->value()));
		newItem->setCheckable(false);
		newItem->setEditable(false);
		newItem->setSelectable(false);
		newItem->setEnabled(false);
		model->setItem(rowCount, 7, newItem);

		resultTableView->sortByColumn(2, Qt::SortOrder::AscendingOrder);
	}

	void MainWindow::updateProgressBar() {
		progressBar->setValue(_processedMessageAmount);
	}

	void MainWindow::testFinished() {
		enableGUI(true);
	}

	void MainWindow::closeEvent(QCloseEvent * evt) {
		closeTool();
		evt->ignore();
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
				QMessageBox box;
				box.setWindowTitle(QApplication::tr("Error loading plugin!"));
				box.setInformativeText(loader.errorString());
				box.setStandardButtons(QMessageBox::StandardButton::Ok);
				box.exec();
			}
		}
	}

	void MainWindow::addSocketCheckboxes() {
		for (auto sp : _socketPlugins) {
			socketPluginLayout->addWidget(sp.second.second);
		}
	}

	void MainWindow::performTest(QString ip, uint16_t port, QStringList socketList, uint32_t runs, uint32_t messageCount, uint32_t payloadSize) {
		QString message(payloadSize, QChar('a'));
		_controlSocket = new clockUtils::sockets::TcpSocket();
		if (clockUtils::ClockError::SUCCESS != _controlSocket->connect(ip.toStdString(), CONTROL_PORT, 1000)) {
			// TODO: (Daniel) can't connect to helper, print some error message
			return;
		}
		for (QString socketPluginName : socketList) {
			std::vector<uint64_t> durations;
			plugins::SocketPluginInterface * socketPlugin = _socketPlugins[socketPluginName].first;
			for (uint32_t i = 0; i < runs; i++) {
				// connect to helper and communicate which SocketPlugin to prepare
				common::ListenOnPluginAndPortMessage lopapm;
				lopapm.pluginName = socketPluginName.toStdString();
				lopapm.port = port;
				_controlSocket->writePacket(lopapm.Serialize());

				std::string reply;
				if (clockUtils::ClockError::SUCCESS != _controlSocket->receivePacket(reply)) {
					// TODO: (Daniel) received error from helper, stop test and print error
					return;
				}
				common::Message * msg = common::Message::Deserialize(reply);
				if (msg->type != common::MessageType::LISTENING) {
					// TODO: (Daniel) received wrong message type from helper, stop test and print error
					delete msg;
					return;
				}
				// received LISTENING from helper, so real test can start now
				delete msg;
				// connect to socket
				if (!socketPlugin->connect(ip, port, std::bind(&MainWindow::receivedMessage, this))) {
					//_processedMessageAmount += messageCount;
					emit updateProgress();
					continue;
				}
				// start time measuring
				std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
				// send all messages
				for (uint32_t j = 0; j < messageCount; j++) {
					socketPlugin->sendMessage(message);
				}
				// wait until all messages are echoed or timeout is reached
				socketPlugin->waitForMessages(messageCount, 10000);
				// calculate duration to receive all message
				uint64_t duration = uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
				// disconnect plugin to be able to do a clean reconnect for next repetition or new test later on
				socketPlugin->disconnect();
				// store current result
				durations.push_back(duration);
			}
			if (!durations.empty()) {
				emit finishedSocket(socketPluginName, durations);
			}
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
		startButton->setEnabled(enabled);
		if (enabled) {
			progressBar->hide();
		} else {
			progressBar->show();
		}
	}

} /* namespace widgets */
} /* namespace spc */
