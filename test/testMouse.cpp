//
// Created by 金宇超 on 15/11/11.
//

#ifndef OSGVIDEO3D_QT_MAINWINDOW_H
#define OSGVIDEO3D_QT_MAINWINDOW_H

#include <iostream>

#include <QApplication>
#include <QObject>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>

#include <osg/io_utils>
#include <osg/LineWidth>
#include <osg/Texture2DArray>

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
#include "../opencv_imagestream.h"

class QTMainWindow;

class QTVideoWidget : public QWidget, public osgViewer::CompositeViewer {
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
        camera->setClearColor( osg::Vec4(0.8, 0.8, 0.9, 1.0) );
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

    osg::Node* initScene() {

        osg::Node* scene = osgDB::readNodeFile("/Users/jin-yc10/Desktop/floor2.obj");

        return scene;
    }

protected:
    osgViewer::View* mainView;
    QTimer _timer;
    osgGA::FirstPersonManipulator* mainCamManipulator = new osgGA::FirstPersonManipulator;
public:
    QTVideoWidget(QWidget* parent = 0, Qt::WindowFlags f = 0,
                  osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded)
            : QWidget(parent, f) {

        osgDB::Registry::instance()->addFileExtensionAlias( "jpeg", "jpeg" );
        osgDB::Registry::instance()->addFileExtensionAlias( "jpg", "jpeg" );

        setThreadingModel(threadingModel);
        setKeyEventSetsDone(0);

        // Osg Part
        mainView = new osgViewer::View;
        osg::Group* rootNode = new osg::Group;
        rootNode->addChild(initScene());

        mainCamManipulator = new osgGA::FirstPersonManipulator;
        mainView->setCameraManipulator(mainCamManipulator);
        mainView->setSceneData(rootNode);

        // Qt Part
        QWidget* viewerWidget = addViewWidget( createGraphicsWindow(0,0,300,300),
                                               mainView);
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(viewerWidget);
        hbox->setContentsMargins(5,5,5,5);
        this->setLayout(hbox);

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );

        _timer.start( 10 );
    }
    ~QTVideoWidget() {
    }
    void paintEvent( QPaintEvent* event ) {
        frame();
    }
};

class QTMainWindow : public QMainWindow{
    QTVideoWidget* video;
public:
    QTMainWindow() {
        setWindowTitle("Video In 3D");
        setGeometry(30,30,1024,768);
        video = new QTVideoWidget;
        setCentralWidget(video);
        QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
        QAction* testAction = fileMenu->addAction(tr("test"));
    }
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QTMainWindow main;
    main.show();
    return app.exec();
}

#endif //OSGVIDEO3D_QT_MAINWINDOW_H
