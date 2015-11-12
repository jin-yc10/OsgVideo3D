//
// Created by 金宇超 on 15/11/11.
//

#ifndef OSGVIDEO3D_QT_MAINWINDOW_H
#define OSGVIDEO3D_QT_MAINWINDOW_H

#include <iostream>

#include <QObject>
#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>

#include <osg/io_utils>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventHandler>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/FirstPersonManipulator>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>

#include <osgQt/GraphicsWindowQt>

#include "opencv_imagestream.h"
#include "virtual_camera.h"

class QTMainWindow;

class QTVideoWidget : public QWidget, public osgViewer::CompositeViewer {
Q_OBJECT
private:
    osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false ) {
        osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->windowName = name;
        traits->windowDecoration = windowDecoration;
        traits->x = x;
        traits->y = y;
        traits->width = w;
        traits->height = h;
        traits->doubleBuffer = true;
        traits->alpha = ds->getMinimumNumAlphaBits();
        traits->stencil = ds->getMinimumNumStencilBits();
        traits->sampleBuffers = ds->getMultiSamples();
        traits->samples = ds->getNumMultiSamples();
        return new osgQt::GraphicsWindowQt(traits.get());
    }
    QWidget* addViewWidget( osgQt::GraphicsWindowQt* gw, osgViewer::View* view) {
        addView( view );
        osg::Camera* camera = view->getCamera();
        camera->setGraphicsContext( gw );
        const osg::GraphicsContext::Traits* traits = gw->getTraits();
        camera->setClearColor( osg::Vec4(0.2, 0.2, 0.3, 1.0) );
        camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
        camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
        gw->setTouchEventsEnabled( true );
        return gw->getGLWidget();
    }
    osg::Geode* createCameraPlane() {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        texture->setTextureSize( 1280, 1024);
        texture->setFilter(osg::Texture::MIN_FILTER , osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER , osg::Texture::LINEAR);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        texture->setResizeNonPowerOfTwoHint(false);
        // Create OpenCVImageStream
        osg::ref_ptr<opencv_imagestream> openCVImageStream = new opencv_imagestream();
        openCVImageStream->openCamera(0);
        texture->setImage(openCVImageStream);
        float width = openCVImageStream->aspectRatio();
        float height = 1.0f;
        osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                                                           osg::Vec3(width,0.0f,0.0f),
                                                                           osg::Vec3(0.0f,0.0f,height),
                                                                           0.0f,
                                                                           1.0f,
                                                                           1.0f,
                                                                           0.0f);
        osg::ref_ptr<osg::Geode> quad = new osg::Geode;
        quad->addDrawable( geom );
        int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
        quad->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), values );
        quad->getOrCreateStateSet()->setMode( GL_LIGHTING, values );

        // Apply texture to quad
        osg::ref_ptr<osg::StateSet> stateSet = quad->getOrCreateStateSet();
        stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

        return quad.release();
    }

    std::vector<virtual_camera> cameras;
    osg::Matrixd mainCamMatrix;

    void setupCameras(std::string path = "cameras.yaml") {
        cv::FileStorage fs(path, cv::FileStorage::READ);
        cv::FileNode cams = fs["Cameras"];
        cv::FileNodeIterator it = cams.begin();
        for(;it!=cams.end();it++){
            virtual_camera cam(it);
            cameras.push_back(cam);
        }
        fs.release();
    }
    void changeCamera(int idx) {
        if( idx == CurrentCamIdx ) {
            // no change, do nothing
            return;
        }
        if(idx == -1) {
            // main cam
            cameras[CurrentCamIdx].setMatrix(mainCamManipulator->getMatrix());
            mainCamManipulator->setByMatrix(mainCamMatrix);
        } else {
            // virtual cam
            if( CurrentCamIdx == -1) {
                // save to main cam
                mainCamMatrix = mainCamManipulator->getMatrix();
            } else {
                cameras[CurrentCamIdx].setMatrix(mainCamManipulator->getMatrix());
            }
            mainCamManipulator->setByMatrix(cameras[idx].matrixd);
        }
        CurrentCamIdx = idx; // cache the idx;
    }

protected:
    QTimer _timer;
    QTableView* CameraList;
    QTableView* InfoList;
    QStandardItemModel *CameraListModel;
    QStandardItemModel *InfoListModel;

    osgGA::FirstPersonManipulator* mainCamManipulator;
    osgViewer::View* mainView;
    osgViewer::View* popupView;

    int CurrentCamIdx;

public:
    QTVideoWidget(QWidget* parent = 0, Qt::WindowFlags f = 0,
                  osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded)
            : QWidget(parent, f) {
        setThreadingModel(threadingModel);
        setKeyEventSetsDone(0);
        setupCameras();

        // Application Part
        CurrentCamIdx = -1;

        // Osg Part
        mainView = new osgViewer::View;
        osg::Group* rootNode = new osg::Group;
        osg::Node* scene = osgDB::readNodeFile("/Users/jin-yc10/Desktop/floor2.obj");
        rootNode->addChild(scene);
        mainCamManipulator = new osgGA::FirstPersonManipulator;
        mainView->setCameraManipulator(mainCamManipulator);
        mainView->setSceneData(rootNode);
        mainCamMatrix = mainCamManipulator->getMatrix();
        for( auto it = cameras.begin(); it != cameras.end(); it++ ) {
            rootNode->addChild((*it).node);
        }
        // Qt Part
        QWidget* viewerWidget = addViewWidget( createGraphicsWindow(0,0,300,300),
                                               mainView);
        popupView = new osgViewer::View;
        popupView->setSceneData(createCameraPlane());
        QWidget* popupWidget = addViewWidget( createGraphicsWindow(900,100,320,240,"Popup window",true),
                                              popupView);
        popupView->setCameraManipulator(new osgGA::TrackballManipulator);
        popupWidget->show();

        QVBoxLayout* vbox = new QVBoxLayout;
        QPushButton* btn = new QPushButton;
        btn->setText(tr("主摄像机"));
        vbox->addWidget(btn);
        CameraListModel = new QStandardItemModel;
        CameraListModel->setColumnCount(2);
        CameraListModel->setHeaderData(0, Qt::Horizontal,QString::fromLocal8Bit("摄像头编号"));
        CameraListModel->setHeaderData(1, Qt::Horizontal,QString::fromLocal8Bit("描述"));
        int CamCnt = 0;
        for( auto it = cameras.begin(); it != cameras.end(); it++ ) {
            CameraListModel->setItem(CamCnt, 0, new QStandardItem(it->getCamera()->getName().c_str()));
            CameraListModel->setItem(CamCnt, 1, new QStandardItem(it->getCamera()->getDescription(0).c_str()));
            CamCnt ++;
        }

        CameraList = new QTableView;
        CameraList->setModel(CameraListModel);
        CameraList->setFixedWidth(200);
        CameraList->setSelectionBehavior(QAbstractItemView::SelectRows);
        CameraList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        InfoList = new QTableView;
        InfoList->setFixedWidth(200);
        vbox->addWidget(CameraList);
        vbox->addWidget(InfoList);
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(viewerWidget);
        hbox->setContentsMargins(5,5,5,5);
        hbox->addLayout(vbox);
        this->setLayout(hbox);

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
        connect( btn, &QPushButton::clicked, this, &QTVideoWidget::mainCamClickerHandler );
        connect( CameraList, &QTableView::clicked, this, &QTVideoWidget::cameraSelectHandler );

        _timer.start( 10 );
        changeCamera(-1); // set current cam to main cam
    }
    ~QTVideoWidget() {
    }
    void paintEvent( QPaintEvent* event ) {
        frame();
    }

signals:
public slots:

    void mainCamClickerHandler() {
        changeCamera(-1);
    }
    void cameraSelectHandler(const QModelIndex &index) {
        std::cout << "Select Camera: " << index.row() << std::endl;
        changeCamera(index.row());
    }
    void saveCameras() {
        cv::FileStorage fs;
        fs.open("cameras.yaml", cv::FileStorage::WRITE);
        fs << "CameraCount" << (int)cameras.size();
        fs << "Cameras" << "[" ;
        for(auto it = cameras.begin(); it!=cameras.end(); it++) {
            fs << "{"
            << "name" << (*it).getCamera()->getName()
            << "desc" << (*it).getCamera()->getDescription(0)
            << "Matrix" << cv::Mat(4,4,CV_64F,((*it).matrixd.ptr()))
            << "Source" << "1" << "}";
        }
        fs << "]";
        fs.release();
    }
};

class QTMainWindow : public QMainWindow{
Q_OBJECT
    QTVideoWidget* video;
public:
    QTMainWindow() {
        setWindowTitle("Video In 3D");
        setGeometry(30,30,1024,768);
        video = new QTVideoWidget;
        setCentralWidget(video);
        QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
        QAction* testAction = fileMenu->addAction(tr("test"));
        connect( testAction, &QAction::triggered, video, &QTVideoWidget::saveCameras );
    }
};


#endif //OSGVIDEO3D_QT_MAINWINDOW_H
