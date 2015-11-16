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

#include "opencv_imagestream.h"
#include "virtual_camera.h"

class QTMainWindow;

class QTVideoWidget : public QWidget, public osgViewer::CompositeViewer {
    class ModelViewProjectionMatrixCallback: public osg::Uniform::Callback {
        ModelViewProjectionMatrixCallback(osg::Camera* camera) :
                _camera(camera) {
        }

        virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {
            osg::Matrixd viewMatrix = _camera->getViewMatrix();
            osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
            osg::Matrixd modelViewProjectionMatrix = modelMatrix * viewMatrix * _camera->getProjectionMatrix();
            uniform->set(modelViewProjectionMatrix);
        }

        osg::Camera* _camera;
    };

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
    osg::Node* createAxis() {
        //Begin draw Axis
        deprecated_osg::Geometry *Axis = new deprecated_osg::Geometry;
        osg::Vec3Array *vecArray = new osg::Vec3Array;
        vecArray->push_back(osg::Vec3(0.1, 0.1, 0.1));
        vecArray->push_back(osg::Vec3(3000.0, 0.1, 0.1));
        vecArray->push_back(osg::Vec3(0.1, 3000.0, 0.1));
        vecArray->push_back(osg::Vec3(0.10, 0.10, 500.0));

        osg::Vec3Array *color = new osg::Vec3Array;
        color->push_back(osg::Vec3(1.0, 0.0, 0.0));
        color->push_back(osg::Vec3(0.0, 1.0, 0.0));
        color->push_back(osg::Vec3(0.0, 0.0, 1.0));

        osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 4, 4> *colorIndexArray
                = new osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 4, 4>;
        colorIndexArray->push_back(0);
        colorIndexArray->push_back(1);
        colorIndexArray->push_back(2);

        Axis->setVertexArray(vecArray);
        Axis->setColorArray(color);
        Axis->setColorIndices(colorIndexArray);
        Axis->setColorBinding(deprecated_osg::Geometry::BIND_PER_PRIMITIVE);

        osg::DrawElementsUInt *pXaxisPrimitiveSet = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
        pXaxisPrimitiveSet->push_back(0);
        pXaxisPrimitiveSet->push_back(1);
        osg::DrawElementsUInt *pYaxisPrimitiveSet = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
        pYaxisPrimitiveSet->push_back(0);
        pYaxisPrimitiveSet->push_back(2);
        osg::DrawElementsUInt *pZaxisPrimitiveSet = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
        pZaxisPrimitiveSet->push_back(0);
        pZaxisPrimitiveSet->push_back(3);

        Axis->addPrimitiveSet(pXaxisPrimitiveSet);
        Axis->addPrimitiveSet(pYaxisPrimitiveSet);
        Axis->addPrimitiveSet(pZaxisPrimitiveSet);

        osg::Geode *AxisGeode = new osg::Geode;
        AxisGeode->addDrawable(Axis);

        //set some attribute
        osg::LineWidth *lineW = new osg::LineWidth;
        lineW->setWidth(5.0);
        osg::StateSet *stateset = AxisGeode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setAttribute(lineW, osg::StateAttribute::ON);
        //End Draw Axix
        osgText::Text* text2D = new osgText::Text;
        text2D->setFont(0);
        text2D->setCharacterSize(32.0f); // medium
        text2D->setFontResolution(256,256);
        text2D->setDrawMode(osgText::Text::TEXT);
        text2D->setAxisAlignment(osgText::Text::XZ_PLANE);
        text2D->setText("Z[500]");
        text2D->setPosition(osg::Vec3(0.0, 0.0, 500.0));
        osg::Group *temp = new osg::Group;
        temp->addChild(AxisGeode);
        temp->addChild(text2D);
        return temp->asNode();
    }
    osg::Node* initScene() {
        osg::Node* scene = osgDB::readNodeFile("/Users/jin-yc10/Desktop/floor2.obj");
        osg::StateSet* sceneStateSet = scene->getOrCreateStateSet();
        osg::Program* programObject = new osg::Program;
        osg::Shader* vert = new osg::Shader( osg::Shader::VERTEX );
        osg::Shader* frag = new osg::Shader( osg::Shader::FRAGMENT );
        programObject->addShader(vert);
        programObject->addShader(frag);
        vert->loadShaderSourceFromFile("shaders/video.vert");
        frag->loadShaderSourceFromFile("shaders/video.frag");
        sceneStateSet->setAttributeAndModes(programObject, osg::StateAttribute::ON);
        sceneStateSet->setTextureAttributeAndModes( 0, textures.get(),osg::StateAttribute::ON);
        sceneStateSet->addUniform( new osg::Uniform( "textures", 0 ) );
        sceneStateSet->addUniform( new osg::Uniform( "cameraCnt", (int)cameras.size() ) );
        for(auto it = cameras.begin(); it != cameras.end(); it++) {
            sceneStateSet->addUniform((*it).getMVPUniform());
        }
        return scene;
    }

    std::vector<virtual_camera> cameras;
    osg::Matrixd mainCamMatrix;
    osg::Uniform* cameraCnt;
    osg::ref_ptr<osg::Texture2DArray> textures = new osg::Texture2DArray;

    void setupCameras(std::string path = "cameras.yaml") {
        cv::FileStorage fs(path, cv::FileStorage::READ);
        cv::FileNode cams = fs["Cameras"];
        cv::FileNodeIterator it = cams.begin();
        unsigned int idx = 0;
        for(;it!=cams.end();it++){
            virtual_camera cam(it, idx);
            cameras.push_back(cam);
            textures->setImage(idx, cam.getImage());
            idx++;
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
            mainCamManipulator->setByMatrix(*cameras[idx].matrixd);
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

        textures->setFilter(osg::Texture2DArray::MIN_FILTER, osg::Texture2DArray::LINEAR);
        textures->setFilter(osg::Texture2DArray::MAG_FILTER, osg::Texture2DArray::LINEAR);
        textures->setWrap(osg::Texture2DArray::WRAP_R, osg::Texture2DArray::REPEAT);

        setThreadingModel(threadingModel);
        setKeyEventSetsDone(0);
        setupCameras();

        // Application Part
        CurrentCamIdx = -1;

        // Osg Part
        mainView = new osgViewer::View;
        osg::Group* rootNode = new osg::Group;
        rootNode->addChild(initScene());
        rootNode->addChild(createAxis());

        mainCamManipulator = new osgGA::FirstPersonManipulator;
        mainView->setCameraManipulator(mainCamManipulator);
        mainView->setSceneData(rootNode);
        mainCamMatrix = mainCamManipulator->getMatrix();
        for( auto it = cameras.begin(); it != cameras.end(); it++ ) {
            (*it).setProjection(this->mainView->getCamera()->getProjectionMatrix());
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
            (*it).saveCamera(fs);
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
