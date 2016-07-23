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

#ifndef __SPC_WIDGETS_LINECHARTWIDGET_H__
#define __SPC_WIDGETS_LINECHARTWIDGET_H__

#include <cstdint>

#include <QGraphicsItem>

namespace spc {
namespace widgets {

	class LineChartWidget : public QObject, public QGraphicsItem {
		Q_OBJECT
		Q_INTERFACES(QGraphicsItem)

	public:
		LineChartWidget(const std::vector<uint64_t> & durations);
		~LineChartWidget();

		public slots:
		void setXAxisText(QString text);
		void setYAxisText(QString text);

		void setXAxisStepWidth(int stepWidth);
		void setYAxisStepWidth(int stepWidth);

	private:
		std::vector<uint64_t> _durations;

		/**
		 * \brief bounding rect of the rendered path
		 */
		QRectF _boundingRect;

		QString _xAxisText;
		QString _yAxisText;

		int _xAxisStepWidth;
		int _yAxisStepWidth;

		QRectF boundingRect() const override {
			return _boundingRect;
		}

		void paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *) override;
	};

} /* namespace widgets */
} /* namespace spc */

#endif /* __SPC_WIDGETS_LINECHARTWIDGET_H__ */
