//
// Created by 金宇超 on 15/11/11.
//

#include "opencv_imagestream.h"

#include <iostream>
#include <OpenThreads/ScopedLock>

opencv_imagestream::opencv_imagestream() : m_cameraThread(0) {}

opencv_imagestream::~opencv_imagestream() {
    if (m_cameraThread->isRunning()) {
        m_cameraThread->cancel();
    }

    delete m_cameraThread;
}

bool opencv_imagestream::openCamera(std::string path) {
    m_cameraThread = new opencv_imagestream_thread(path);
    m_cameraThread->start();
    return true;
}

bool opencv_imagestream::openCamera(int deviceId) {
    m_cameraThread = new opencv_imagestream_thread(deviceId);
    m_cameraThread->start();
    return true;
}

void opencv_imagestream::update(osg::NodeVisitor* /*nv*/) {
    if (m_cameraThread->initialized() && m_cameraThread->newImageAvailable()) {
        m_cameraThread->getData(m_frame);
        this->setImage(m_cameraThread->sensorSizeX(),
                       m_cameraThread->sensorSizeY(),
                       1,
                       m_cameraThread->internalTextureFormat(),
                       m_cameraThread->pixelFormat(),
                       m_cameraThread->dataType(),
                       (unsigned char*)(m_frame.data),
                       osg::Image::NO_DELETE,
                       1);
    }
}

void opencv_imagestream_thread::init() {
    if( !m_videoCaptureDevice ) {
        return;
    }
    // check if we succeeded
    if (!m_videoCaptureDevice->isOpened()) {
        return;
    }
    // Get camera properties
    m_videoCaptureDevice->set(CV_CAP_PROP_FRAME_WIDTH,640);
    m_videoCaptureDevice->set(CV_CAP_PROP_FRAME_HEIGHT,480);
    m_sensorSizeX = 256;//static_cast<int> (m_videoCaptureDevice->get(CV_CAP_PROP_FRAME_WIDTH));
    m_sensorSizeY = 256;//static_cast<int> (m_videoCaptureDevice->get(CV_CAP_PROP_FRAME_HEIGHT));
    m_frameRate = m_videoCaptureDevice->get(CV_CAP_PROP_FPS);

    // Grab one image to determine format
    m_videoCaptureDevice->grab();
    m_videoCaptureDevice->retrieve(m_backBuffer);

    // Make sure that grabbed images is continuous
    if (!m_backBuffer.isContinuous()) {
        return;
    }

    // Get the data type of image
    switch (m_backBuffer.depth()) {
        case CV_8U:
            m_dataType = GL_UNSIGNED_BYTE;
            break;
        case CV_8S:
            m_dataType = GL_BYTE;
            break;
        default:
            std::cerr << "Error: Unknown data type." << std::endl;
            return; break;
    }

    // Get the pixel format of image
    switch (m_backBuffer.channels()) {
        case 3:
            m_pixelFormat = GL_BGR;
            m_internalTextureFormat = GL_RGB;
            break;
        case 1:
            m_pixelFormat = GL_LUMINANCE;
            m_internalTextureFormat = GL_LUMINANCE;
            break;
        default:
            std::cerr << "Error: Unknown pixel format." << std::endl;
            return; break;
    }

    m_init = true;
}

opencv_imagestream_thread::opencv_imagestream_thread(std::string path) : OpenThreads::Thread(),
        m_done(false), m_flip(false),
        m_newImageAvailable(false),
        m_videoCaptureDevice(0),
        m_init(false),
        m_sensorSizeX(0),
        m_sensorSizeY(0),
        m_frameRate(0.0),
        m_dataType(GL_UNSIGNED_BYTE),
        m_pixelFormat(GL_BGR),
        m_internalTextureFormat(GL_RGB) {
    m_videoCaptureDevice = new cv::VideoCapture(path);
    init();
}

opencv_imagestream_thread::opencv_imagestream_thread(int deviceId) : OpenThreads::Thread(),
                                                       m_done(false), m_flip(false),
                                                       m_newImageAvailable(false),
                                                       m_videoCaptureDevice(0),
                                                       m_init(false),
                                                       m_sensorSizeX(0),
                                                       m_sensorSizeY(0),
                                                       m_frameRate(0.0),
                                                       m_dataType(GL_UNSIGNED_BYTE),
                                                       m_pixelFormat(GL_BGR),
                                                       m_internalTextureFormat(GL_RGB) {
    m_videoCaptureDevice = new cv::VideoCapture(deviceId);
    init();
}

void opencv_imagestream_thread::run() {
    while (!m_done) {
        if (m_init) {
            // Get new image
            m_videoCaptureDevice->grab();
            m_videoCaptureDevice->retrieve(m_backBuffer);
            if( m_backBuffer.size().width == 0 ) {
                m_videoCaptureDevice->set(CV_CAP_PROP_POS_FRAMES, 0);
                // TODO: should handle video seprately
                m_videoCaptureDevice->grab();
                m_videoCaptureDevice->retrieve(m_backBuffer);
            }
            if( m_flip ) {
                cv::flip(m_backBuffer, m_backBuffer, 0);
            }
        }

        {
            OpenThreads::ScopedLock< OpenThreads::Mutex > lock( m_mutex );
            m_backBuffer.copyTo(m_frontBuffer);
            m_newImageAvailable = true;
        }

        OpenThreads::Thread::microSleep(1);
    }
}

int opencv_imagestream_thread::cancel() {
    m_done = true;

    // Release camera device
    if (m_videoCaptureDevice->isOpened()) {
        m_videoCaptureDevice->release();

    }
    m_init = false;

    delete m_videoCaptureDevice;

    return 0;
}

void opencv_imagestream_thread::getData(cv::Mat& frame) {
    // Copy image in thread to frame
    OpenThreads::ScopedLock< OpenThreads::Mutex > lock( m_mutex );
    cv::resize(m_frontBuffer, m_frontBuffer, cv::Size(256,256), 1.0,1.0, CV_INTER_CUBIC);
    m_frontBuffer.copyTo(frame);
    m_newImageAvailable = false;
}