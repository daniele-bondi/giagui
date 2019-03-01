#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QMouseEvent>
#include <QGridLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QtGui/QDoubleValidator>
#include <QtSvg/QGraphicsSvgItem>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenuBar>

#include "DoubleValidator.hpp"
#include "IntSpinBox.hpp"
#include "map.hpp"
#include "MapView.hpp"


enum MapTool
{
	Rect,
	Edit,
};



class MapWindow : public QMainWindow
{
Q_OBJECT
	
	
/* COMMENTO: perchè public ripetuto? */	
public:
	static constexpr int DECIMAL_DIGITS = 6;

public:
	H3State* h3State = nullptr;
	MapTool  mapTool = MapTool::Rect;
	
public:
	explicit MapWindow(QWidget* parent = nullptr);
	
protected:
	bool eventFilter(QObject* o, QEvent* e) override;
	
	void keyPressEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent* event) override;
	
	void onActionOpenFile();
	void onActionSaveFile();
	void onActionSaveFileAs();
	void onActionZoomOut();
	void onActionZoomIn();
	void onActionRectTool();
	void onActionEditTool();
	
	void onCellChangedWater();
	void onCellChangedIce();
	void onCellChangedSediment();
	void onCellChangedDensity();
	
	void onResolutionChanged(int value);
	void onResolutionChangedDialogFinished(int dialogResult);
	
	void onPolyfillFailed(PolyfillError error);
	
	bool handleMapEventMousePress(MapView* mapView, QMouseEvent* event);
	bool handleMapEventMouseMove(MapView* mapView, QMouseEvent* event);
	bool handleMapEventMouseRelease(MapView* mapView, QMouseEvent* event);
	
	void setupToolbar();
	void highlightCell(H3Index index);
	void setAllLineEditEnabled(bool enabled);
	void clearAllLineEditNoSignal();
	
	void setupUi();
	
private:
	QWidget*     centralWidget;
	QGridLayout* gridLayout;
	MapView*     mapView;
	QToolBar*    mainToolBar;
	QStatusBar*  statusBar;
	
	QLineEdit*  editWater;
	QLineEdit*  editIce;
	QLineEdit*  editSediment;
	QLineEdit*  editDensity;
	IntSpinBox* resolutionSpinbox;
	QLabel*     statusLabel;
};


#endif // MAINWINDOW_H
