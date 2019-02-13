#include "MapView.hpp"

#include <QMouseEvent>
#include <QScrollBar>


#define POLYFILL_WIDTH_FACTOR 0.45
#define POLYFILL_INDEX_THRESHOLD 100000


void drawEdgesAntimeridian(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	// FIXME: Recheck this when it's not 6 am
	for(int i = 0; i < geoBoundary->numVerts; ++i)
	{
		const GeoCoord* p = &geoBoundary->verts[i];
		const GeoCoord* q = &geoBoundary->verts[(i+1) % geoBoundary->numVerts];
		
		if(edgeCrossesAntimeridian(p->lon, q->lon))
		{
			double c_lon, q_lon;
			if(p->lon > 0)
			{
				c_lon = PI;
				q_lon = q->lon + 2*PI; // Move `q` longitude to `p` longitude
			}
			else
			{
				c_lon = -PI;
				q_lon = q->lon - 2*PI; // Move `q` longitude to `p` longitude
			}
			
			// c = a + t*(b-a) -> t = (c-a)/(b-a)
			double t = (c_lon - p->lon) / (q_lon - p->lon);
			GeoCoord amCrossPoint = {
				.lat = p->lat + t * (q->lat - p->lat),
				.lon = c_lon
			};
			
			{
				QLineF line(toSurfaceSpace(*p, surfaceSize), toSurfaceSpace(amCrossPoint, surfaceSize));
				painter->drawLine(line);
			}
			amCrossPoint.lon = -amCrossPoint.lon;
			{
				QLineF line(toSurfaceSpace(amCrossPoint, surfaceSize), toSurfaceSpace(*q, surfaceSize));
				painter->drawLine(line);
			}
		}
		else
		{
			QLineF line(toSurfaceSpace(*p, surfaceSize), toSurfaceSpace(*q, surfaceSize));
			painter->drawLine(line);
		}
	}
}


void drawEdges(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	if(!polyCrossesAntimeridian(geoBoundary))
	{
		QPointF points[MAX_CELL_BNDRY_VERTS];
		for(int i = 0; i < geoBoundary->numVerts; ++i)
			points[i] = toSurfaceSpace(geoBoundary->verts[i], surfaceSize);
		
		painter->drawPolygon(points, geoBoundary->numVerts, Qt::FillRule::OddEvenFill);
	}
	else
	{
		drawEdgesAntimeridian(painter, geoBoundary, surfaceSize);
	}
}


void drawPolygonAntimeridian(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	QPointF pointsEast[MAX_CELL_BNDRY_VERTS];
	int     pointsEastCount = 0;
	QPointF pointsWest[MAX_CELL_BNDRY_VERTS];
	int     pointsWestCount = 0;
	
	for(int i = 0; i < geoBoundary->numVerts; ++i)
	{
		const GeoCoord* p = &geoBoundary->verts[i];
		if(p->lon > 0)
		{
			pointsEast[pointsEastCount++] = toSurfaceSpace(*p, surfaceSize);
		}
		else
		{
			pointsWest[pointsWestCount++] = toSurfaceSpace(*p, surfaceSize);
		}
		
		const GeoCoord* q = &geoBoundary->verts[(i+1) % geoBoundary->numVerts];
		if(edgeCrossesAntimeridian(p->lon, q->lon))
		{
			// c = p + t*(q-p) -> t = (c-p)/(q-p)
			double t;
			if(q->lon < 0)
			{
				double q_lon = q->lon + 2*PI;
				t = ( PI - p->lon) / (q_lon - p->lon);
			}
			else
			{
				double q_lon = q->lon - 2*PI;
				t = (-PI - p->lon) / (q_lon - p->lon);
			}
			
			GeoCoord amCrossPoint = {
				.lat = p->lat + t * (q->lat - p->lat),
				.lon = PI,
			};
			pointsEast[pointsEastCount++] = toSurfaceSpace(amCrossPoint, surfaceSize);
			amCrossPoint.lon = -amCrossPoint.lon;
			pointsWest[pointsWestCount++] = toSurfaceSpace(amCrossPoint, surfaceSize);
		}
	}
	
	assert(pointsEastCount > 0);
	assert(pointsWestCount > 0);
	painter->drawPolygon(pointsEast, pointsEastCount, Qt::FillRule::OddEvenFill);
	painter->drawPolygon(pointsWest, pointsWestCount, Qt::FillRule::OddEvenFill);
}


void drawPolygon(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	if(!polyCrossesAntimeridian(geoBoundary))
	{
		QPointF points[MAX_CELL_BNDRY_VERTS];
		for(int i = 0; i < geoBoundary->numVerts; ++i)
			points[i] = toSurfaceSpace(geoBoundary->verts[i], surfaceSize);
		
		painter->drawPolygon(points, geoBoundary->numVerts, Qt::FillRule::OddEvenFill);
	}
	else
	{
		drawPolygonAntimeridian(painter, geoBoundary, surfaceSize);
	}
}


void MapView::drawForeground(QPainter* painter, const QRectF& exposed)
{
	if(!this->h3State)
		return;
	
	qreal strokeWidth = getLineThickness(this->h3State->resolution);
	
	if(!this->h3State->cellsData.empty())
	{
		painter->setPen(QPen(QColor(0, 0, 255, 63), strokeWidth));
		painter->setBrush(QBrush(QColor(0, 0, 255, 63), Qt::BrushStyle::SolidPattern));
		
		GeoBoundary geoBoundary;
		for(auto& it : this->h3State->cellsData)
		{
			h3ToGeoBoundary(it.first, &geoBoundary);
			drawPolygon(painter, &geoBoundary, this->sceneRect().size());
		}
	}
	
	if(this->h3State->polyfillIndices)
	{
		painter->setPen(QPen(QColor(0, 0, 0, 255), strokeWidth));
		painter->setBrush(QBrush(QColor(0, 0, 0, 0), Qt::BrushStyle::NoBrush));
		
		GeoBoundary geoBoundary;
		for(uint64_t i = 0; i < this->h3State->polyfillIndicesCount; ++i)
		{
			if(this->h3State->polyfillIndices[i] != H3_INVALID_INDEX)
			{
				h3ToGeoBoundary(this->h3State->polyfillIndices[i], &geoBoundary);
				drawEdges(painter, &geoBoundary, this->sceneRect().size());
			}
		}
	}
	
	if(this->h3State->activeIndex != H3_INVALID_INDEX)
	{
		painter->setPen(QPen(QColor(255, 0, 0, 63), strokeWidth));
		painter->setBrush(QBrush(QColor(255, 0, 0, 63), Qt::BrushStyle::SolidPattern));
		
		GeoBoundary geoBoundary;
		h3ToGeoBoundary(this->h3State->activeIndex, &geoBoundary);
		drawPolygon(painter, &geoBoundary, this->sceneRect().size());
	}
}


MapView::MapView(QWidget* parent) : QGraphicsView(parent),
	h3State(&globalH3State)
{
	setMouseTracking(true);
	this->rubberband = new QRubberBand(QRubberBand::Rectangle, this);
}


void MapView::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		event->accept();
		this->vsMouseLeftDownPos = event->pos();
		
		this->rubberband->setGeometry(QRect(event->pos(), QSize()));
		this->rubberband->show();
	}
	else
	if(event->button() == Qt::RightButton)
	{
		event->accept();
		this->vsMouseRightDownPos = event->pos();
		this->setCursor(Qt::ClosedHandCursor);
	}
}


void MapView::mouseMoveEvent(QMouseEvent* event)
{
	if(event->buttons() & Qt::RightButton)
	{
		event->accept();
		QPoint delta = event->pos() - this->vsMouseRightDownPos;
		this->vsMouseRightDownPos = event->pos();
		this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - delta.x());
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->value()     - delta.y());
	}
	
	if(event->buttons() & Qt::LeftButton)
	{
		event->accept();
		this->vsMouseMovePos = event->pos();
		
		QRectF ssRect = QRectF(mapToScene(this->vsMouseLeftDownPos), mapToScene(this->vsMouseMovePos));
		if(ssRect.width() < -sceneRect().width() * POLYFILL_WIDTH_FACTOR)
			ssRect.setWidth(-sceneRect().width() * POLYFILL_WIDTH_FACTOR);
		if(ssRect.width() > sceneRect().width() * POLYFILL_WIDTH_FACTOR)
			ssRect.setWidth(sceneRect().width() * POLYFILL_WIDTH_FACTOR);
		
		QRect vsRect = mapFromScene(ssRect).boundingRect();
		this->rubberband->setGeometry(vsRect);
	}
	
	if(event->buttons() == Qt::NoButton)
	{
		event->ignore();
	}
}


void MapView::mouseReleaseEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		event->accept();
		
		QRectF area = mapToScene(this->rubberband->geometry()).boundingRect();
		delete[] this->h3State->polyfillIndices;
		this->h3State->polyfillIndicesCount = 0;
		
		uint64_t polyfillIndicesCount = polyfillAreaCount(area, sceneRect().size(), this->h3State->resolution);
		if(polyfillIndicesCount < POLYFILL_INDEX_THRESHOLD)
		{
			int error = polyfillArea(area, sceneRect().size(), this->h3State->resolution, &this->h3State->polyfillIndices); 
			if(error == 0)
			{
				this->h3State->polyfillIndicesCount = polyfillIndicesCount;
			}
			else
			{
				emit polyfillFailed(PolyfillError::MEMORY_ALLOCATION);
			}
		}
		else
		{
			emit polyfillFailed(PolyfillError::THRESHOLD_EXCEEDED);
		}
		this->scene()->invalidate();
		
		this->rubberband->setGeometry(0, 0, 0, 0);
		this->rubberband->hide();
	}
	else
	if(event->button() == Qt::RightButton)
	{
		event->accept();
		this->setCursor(Qt::ArrowCursor);
	}
	else
	{
		event->ignore();
	}
}


void MapView::wheelEvent(QWheelEvent* event)
{
	// NOTE: https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView for a smooth zoom implementation
	event->accept();
	QPoint angleDelta = event->angleDelta();
	double steps = angleDelta.y() / 8.0 / 15.0; // see QWheelEvent documentation
	this->zoom(event->pos(), steps);
}


void MapView::zoom(QPoint vsAnchor, double steps)
{
	QPointF ssAnchor = mapToScene(vsAnchor);
	qreal factor = std::pow(1.2, steps);
	scale(factor, factor);
	
	centerOn(ssAnchor);
	QPointF delta_viewport_pos = vsAnchor - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
	QPointF viewport_center = mapFromScene(ssAnchor) - delta_viewport_pos;
	centerOn(mapToScene(viewport_center.toPoint()));
}
