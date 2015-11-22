//
// Created by 金宇超 on 15/11/11.
//

#ifndef OSGVIDEO3D_OPENCV_TEXTURE_H
#define OSGVIDEO3D_OPENCV_TEXTURE_H

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <osg/ImageStream>

#include <opencv2/opencv.hpp>

class opencv_imagestream_thread : public OpenThreads::Thread {
private:
    void init();

public:
    opencv_imagestream_thread(int deviceId);
    opencv_imagestream_thread(std::string path);

    void flip(bool flag){ m_flip = flag; }
    virtual void run();
    virtual int cancel();
    bool initialized() const { return m_init; }
    int sensorSizeX() const { return m_sensorSizeX; }
    int sensorSizeY() const { return m_sensorSizeY; }
    double getFrameRate() const { return m_frameRate; }
    GLenum dataType() const { return m_dataType; }
    GLenum pixelFormat() const { return m_pixelFormat; }
    GLenum internalTextureFormat() const { return m_internalTextureFormat; }
    void getData(cv::Mat& frame);
    bool newImageAvailable() const { return m_newImageAvailable; }
protected:
    bool m_init;
    bool m_flip;
    bool m_done;
    bool m_newImageAvailable;
    cv::VideoCapture* m_videoCaptureDevice;
    cv::Mat m_backBuffer;
    cv::Mat m_frontBuffer;
    int m_sensorSizeX;
    int m_sensorSizeY;
    double m_frameRate;
    OpenThreads::Mutex m_mutex;
    GLenum m_dataType;
    GLenum m_pixelFormat;
    GLenum m_internalTextureFormat;
};

class opencv_imagestream : public osg::ImageStream {
public:
    opencv_imagestream();
    void flip(bool flag) { if(m_cameraThread) m_cameraThread->flip(flag); }
    bool openCamera(int deviceId);
    bool openCamera(std::string path);
    int sensorSizeX() const { return m_cameraThread ? m_cameraThread->sensorSizeX() : 0; }
    int sensorSizeY() const { return m_cameraThread ? m_cameraThread->sensorSizeY() : 0; }
    double getFrameRate() const { return m_cameraThread ? m_cameraThread->getFrameRate() : 0.0; }
    float aspectRatio() const { return m_cameraThread->sensorSizeY() != 0 ? float(sensorSizeX()) / float(sensorSizeY()) : 1.0f; }

    /** ImageSequence requires a call to update(NodeVisitor*) during the update traversal so return true.*/
    virtual bool requiresUpdateCall() const { return true; }
    virtual void update(osg::NodeVisitor* nv);

protected:
    ~opencv_imagestream(); // Since we inherit from osg::Referenced we must make destructor protected
    opencv_imagestream_thread* m_cameraThread;
    cv::Mat m_frame;
};


#endif //OSGVIDEO3D_OPENCV_TEXTURE_H
