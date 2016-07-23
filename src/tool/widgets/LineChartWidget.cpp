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

#include "widgets/LineChartWidget.h"

#include <QGraphicsScene>
#include <QPainter>

namespace spc {
namespace widgets {

	LineChartWidget::LineChartWidget(const std::vector<uint64_t> & durations) : QGraphicsItem(), _durations(durations), _boundingRect(), _xAxisText(), _yAxisText(), _yAxisStepWidth(5) {
	}

	LineChartWidget::~LineChartWidget() {
	}

	void LineChartWidget::setXAxisText(QString text) {
		_xAxisText = text;
		update();
	}

	void LineChartWidget::setYAxisText(QString text) {
		_yAxisText = text;
		update();
	}

	void LineChartWidget::setYAxisStepWidth(int stepWidth) {
		_yAxisStepWidth = stepWidth;
		update();
	}

	void LineChartWidget::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *) {
		if (_durations.empty()) {
			return;
		}
		uint64_t minValue = UINT64_MAX;
		uint64_t maxValue = 0;
		uint64_t sum = 0;

		for (auto it = _durations.cbegin(); it != _durations.cend(); it++) {
			minValue = std::min(minValue, *it);
			maxValue = std::max(maxValue, *it);
			sum += *it;
		}
		qreal stepWidth = 1000 / _durations.size();
		std::vector<QPointF> points;
		for (size_t i = 0; i < _durations.size(); i++) {
			points.push_back(QPointF(_durations.size() / 2.0 * -stepWidth + i * stepWidth, -100.0 * (_durations[i] - minValue) / (maxValue - minValue)));
		}
		if (points.empty()) {
			return;
		}
		QPainterPath path = QPainterPath(points[0]);
		for (size_t i = 1; i < points.size(); i++) {
			path.lineTo(points[i]);
		}

		QPen pen(QBrush(QColor(0, 0, 255)), Qt::PenStyle::SolidLine);
		pen.setWidth(1);
		painter->setPen(pen);
		painter->drawPath(path);

		prepareGeometryChange();
		_boundingRect = path.boundingRect();

		scene()->setSceneRect(_boundingRect);

		QPointF avgStartPoint(_durations.size() / 2.0 * -stepWidth, -100.0 * (sum / _durations.size() - minValue) / (maxValue - minValue));
		QPointF avgEndPoint(_durations.size() / 2.0 * stepWidth, -100.0 * (sum / _durations.size() - minValue) / (maxValue - minValue));
		QPainterPath avgPath = QPainterPath(avgStartPoint);
		avgPath.lineTo(avgEndPoint);
		QPen avgPen(QBrush(QColor(0, 255, 0)), Qt::PenStyle::DashLine);
		avgPen.setWidth(1);
		painter->setPen(avgPen);
		painter->drawPath(avgPath);

		// x axis
		QPointF xStartPoint(_durations.size() / 2.0 * -stepWidth, 0.0);
		QPointF xEndPoint(_durations.size() / 2.0 * stepWidth + stepWidth, 0.0);
		QPainterPath xPath = QPainterPath(xStartPoint);
		xPath.lineTo(xEndPoint);
		QPen xPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		xPen.setWidth(2);
		painter->setPen(xPen);
		painter->drawPath(xPath);

		// arrow
		QPainterPath xArrowPath(xEndPoint - QPointF(2.0, 2.0));
		xArrowPath.lineTo(xEndPoint);
		xArrowPath.lineTo(xEndPoint + QPointF(-2.0, 2.0));
		QPen xArrowPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		xArrowPen.setWidth(2);
		painter->setPen(xArrowPen);
		painter->drawPath(xArrowPath);

		// markers for x axis

		QPen xMarkerPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		xMarkerPen.setWidth(1);
		for (int i = 0; i <= int(_durations.size() / 10); i++) {
			QPointF xMarkerPoint(_durations.size() / 2.0 * -stepWidth + i * 10 * stepWidth, 0.0);
			QPainterPath xMarkerPath = QPainterPath(xMarkerPoint + QPointF(0.0, -1.0));
			xMarkerPath.lineTo(xMarkerPoint);
			xMarkerPath.lineTo(xMarkerPoint + QPointF(0.0, 1.0));
			painter->setPen(xMarkerPen);
			painter->drawPath(xMarkerPath);

			QFont myFont;
			QString str(QString::number(i * 10));

			QFontMetrics fm(myFont);
			int width = fm.width(str);

			painter->drawText(xMarkerPoint + QPointF(-width * 0.5, 15.0), str);
		}

		// label of the x axis
		{
			QFont myFont;

			QFontMetrics fm(myFont);
			int width = fm.width(_xAxisText);
			painter->drawText(QPointF(-width * 0.5, 30.0), _xAxisText);
		}

		// y axis
		QPointF yStartPoint(_durations.size() / 2.0 * -stepWidth, 0.0);
		QPointF yEndPoint(_durations.size() / 2.0 * -stepWidth, -110.0);
		QPainterPath yPath = QPainterPath(yStartPoint);
		yPath.lineTo(yEndPoint);
		QPen yPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		yPen.setWidth(2);
		painter->setPen(yPen);
		painter->drawPath(yPath);

		// arrow
		QPainterPath yArrowPath(yEndPoint + QPointF(-2.0, 2.0));
		yArrowPath.lineTo(yEndPoint);
		yArrowPath.lineTo(yEndPoint + QPointF(2.0, 2.0));
		QPen yArrowPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		yArrowPen.setWidth(2);
		painter->setPen(yArrowPen);
		painter->drawPath(yArrowPath);

		// markers for y axis

		int maxWidth = 0;
		QPen yMarkerPen(QBrush(QColor(0, 0, 0)), Qt::PenStyle::SolidLine);
		yMarkerPen.setWidth(1);
		for (int i = 0; i <= _yAxisStepWidth; i++) {
			QPointF yMarkerPoint(_durations.size() / 2.0 * -stepWidth, -(100.0 / _yAxisStepWidth) * i);
			QPainterPath yMarkerPath = QPainterPath(yMarkerPoint + QPointF(-1.0, 0.0));
			yMarkerPath.lineTo(yMarkerPoint);
			yMarkerPath.lineTo(yMarkerPoint + QPointF(1.0, 0.0));
			painter->setPen(yMarkerPen);
			painter->drawPath(yMarkerPath);

			QFont myFont;
			QString str(QString::number(minValue + i * (maxValue - minValue) / _yAxisStepWidth));

			QFontMetrics fm(myFont);
			int height = fm.height();
			int width = fm.width(str);
			maxWidth = std::max(maxWidth, width);

			painter->drawText(yMarkerPoint + QPointF(-width - 3, height * 0.25), str);
		}

		// label of the x axis
		{
			QFont myFont;

			QFontMetrics fm(myFont);
			int width = fm.width(_yAxisText);
			int height = fm.height();
			painter->drawText(QPointF(_durations.size() / 2.0 * -stepWidth - width - maxWidth - 3 - 5, -50.0 + height * 0.25), _yAxisText);
		}
	}

} /* namespace widgets */
} /* namespace spc */
