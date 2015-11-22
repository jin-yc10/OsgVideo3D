//
// Created by 金宇超 on 15/11/12.
//

#ifndef OSGVIDEO3D_VIRTUAL_CAMERA_H
#define OSGVIDEO3D_VIRTUAL_CAMERA_H

#include <opencv2/opencv.hpp>

#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/io_utils>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/Texture2D>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Texture2DArray>

#include "opencv_imagestream.h"

class ModelViewProjectionMatrixCallback: public osg::Uniform::Callback {
public:
    ModelViewProjectionMatrixCallback(osg::Matrixd mat, osg::Matrixd proj) :
            _mat(mat), _proj(proj) {
    }

    virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {
        osg::Matrixd biasMatrix = osg::Matrixd(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        );
//        osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
        osg::Matrixd modelViewProjectionMatrix = biasMatrix * (_mat) * (_proj);
        uniform->set(modelViewProjectionMatrix);
    }

    osg::Matrixd _mat;
    osg::Matrixd _proj;
};

class virtual_camera {

private:
    osg::Camera* cam;
    osg::Image* image;
    std::string sourceType;
    std::string sourcePath;
    osg::Uniform* mvp;

    // if updated, render new shadow map
    osg::Texture* shadowMap;

public:
    static osg::Geode* createPyramid() {
        osg::Geode* pyramidGeode = new osg::Geode();
        osg::Geometry* pyramidGeometry = new osg::Geometry();

        //Associate the pyramid geometry with the pyramid geode
        //   Add the pyramid geode to the root node of the scene graph.

        pyramidGeode->addDrawable(pyramidGeometry);

        //Declare an array of vertices. Each vertex will be represented by
        //a triple -- an instances of the vec3 class. An instance of
        //osg::Vec3Array can be used to store these triples. Since
        //osg::Vec3Array is derived from the STL vector class, we can use the
        //push_back method to add array elements. Push back adds elements to
        //the end of the vector, thus the index of first element entered is
        //zero, the second entries index is 1, etc.
        //Using a right-handed coordinate system with 'z' up, array
        //elements zero..four below represent the 5 points required to create
        //a simple pyramid.

        osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
        pyramidVertices->push_back( osg::Vec3(-50,-50,  0) ); // front left
        pyramidVertices->push_back( osg::Vec3( 50,-50,  0) ); // front right
        pyramidVertices->push_back( osg::Vec3( 50, 50,  0) ); // back right
        pyramidVertices->push_back( osg::Vec3(-50, 50,  0) ); // back left
        pyramidVertices->push_back( osg::Vec3(  0,  0,-50) ); // peak

        //Associate this set of vertices with the geometry associated with the
        //geode we added to the scene.

        pyramidGeometry->setVertexArray( pyramidVertices );

        //Next, create a primitive set and add it to the pyramid geometry.
        //Use the first four points of the pyramid to define the base using an
        //instance of the DrawElementsUint class. Again this class is derived
        //from the STL vector, so the push_back method will add elements in
        //sequential order. To ensure proper backface cullling, vertices
        //should be specified in counterclockwise order. The arguments for the
        //constructor are the enumerated type for the primitive
        //(same as the OpenGL primitive enumerated types), and the index in
        //the vertex array to start from.

        osg::DrawElementsUInt* pyramidBase =
                new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        pyramidBase->push_back(3);
        pyramidBase->push_back(2);
        pyramidBase->push_back(1);
        pyramidBase->push_back(0);
        pyramidGeometry->addPrimitiveSet(pyramidBase);

        //Repeat the same for each of the four sides. Again, vertices are
        //specified in counter-clockwise order.

        osg::DrawElementsUInt* pyramidFaceOne =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceOne->push_back(0);
        pyramidFaceOne->push_back(1);
        pyramidFaceOne->push_back(4);
        pyramidGeometry->addPrimitiveSet(pyramidFaceOne);

        osg::DrawElementsUInt* pyramidFaceTwo =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceTwo->push_back(1);
        pyramidFaceTwo->push_back(2);
        pyramidFaceTwo->push_back(4);
        pyramidGeometry->addPrimitiveSet(pyramidFaceTwo);

        osg::DrawElementsUInt* pyramidFaceThree =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceThree->push_back(2);
        pyramidFaceThree->push_back(3);
        pyramidFaceThree->push_back(4);
        pyramidGeometry->addPrimitiveSet(pyramidFaceThree);

        osg::DrawElementsUInt* pyramidFaceFour =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceFour->push_back(3);
        pyramidFaceFour->push_back(0);
        pyramidFaceFour->push_back(4);
        pyramidGeometry->addPrimitiveSet(pyramidFaceFour);

        //Declare and load an array of Vec4 elements to store colors.

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
        colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) ); //index 1 green
        colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) ); //index 2 blue
        colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); //index 3 white
        colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 4 red

        //The next step is to associate the array of colors with the geometry,
        //assign the color indices created above to the geometry and set the
        //binding mode to _PER_VERTEX.

        pyramidGeometry->setColorArray(colors);
        pyramidGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        return pyramidGeode;
    }

    osg::Node*
    makeFrustumFromCamera( osg::Matrixd proj, osg::Matrixd mv )
    {
        // Get near and far from the Projection matrix.
        const double near = proj(3,2) / (proj(2,2)-1.0);
        const double far = proj(3,2) / (1.0+proj(2,2));

        // Get the sides of the near plane.
        const double nLeft = near * (proj(2,0)-1.0) / proj(0,0);
        const double nRight = near * (1.0+proj(2,0)) / proj(0,0);
        const double nTop = near * (1.0+proj(2,1)) / proj(1,1);
        const double nBottom = near * (proj(2,1)-1.0) / proj(1,1);

        // Get the sides of the far plane.
        const double fLeft = far * (proj(2,0)-1.0) / proj(0,0);
        const double fRight = far * (1.0+proj(2,0)) / proj(0,0);
        const double fTop = far * (1.0+proj(2,1)) / proj(1,1);
        const double fBottom = far * (proj(2,1)-1.0) / proj(1,1);

        // Our vertex array needs only 9 vertices: The origin, and the
        // eight corners of the near and far planes.
        osg::Vec3Array* v = new osg::Vec3Array;
        v->resize( 9 );
        (*v)[0].set( 0., 0., 0. );
        (*v)[1].set( nLeft, nBottom, -near );
        (*v)[2].set( nRight, nBottom, -near );
        (*v)[3].set( nRight, nTop, -near );
        (*v)[4].set( nLeft, nTop, -near );
        (*v)[5].set( fLeft, fBottom, -far );
        (*v)[6].set( fRight, fBottom, -far );
        (*v)[7].set( fRight, fTop, -far );
        (*v)[8].set( fLeft, fTop, -far );

        osg::Geometry* geom = new osg::Geometry;
        geom->setUseDisplayList( false );
        geom->setVertexArray( v );

        osg::Vec4Array* c = new osg::Vec4Array;
        c->push_back( osg::Vec4( 1., 1., 1., 1. ) );
        geom->setColorArray( c, osg::Array::BIND_OVERALL );

        GLushort idxLines[8] = {
                0, 5, 0, 6, 0, 7, 0, 8 };
        GLushort idxLoops0[4] = {
                1, 2, 3, 4 };
        GLushort idxLoops1[4] = {
                5, 6, 7, 8 };
        geom->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINES, 8, idxLines ) );
        geom->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINE_LOOP, 4, idxLoops0 ) );
        geom->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINE_LOOP, 4, idxLoops1 ) );

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( geom );

        geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );


        // Create parent MatrixTransform to transform the view volume by
        // the inverse ModelView matrix.
        osg::MatrixTransform* mt = new osg::MatrixTransform;
        mt->setMatrix( osg::Matrixd::inverse( mv ) );
        mt->addChild( geode );

        return mt;
    }


    osg::Matrixd matrixf;
    osg::Matrixd projection;
    osg::Geode* pyramid;
    osg::PositionAttitudeTransform* node;
    virtual_camera(const cv::FileNodeIterator it, unsigned int index) {
        cam = new osg::Camera();
        cam->setName((*it)["name"]);
        cam->addDescription((*it)["desc"]);
        cv::Mat mat(4,4,CV_32F);
        (*it)["Matrix"] >> mat;
        matrixf = osg::Matrixd((float*)mat.data);
        projection = osg::Matrixd();
        projection.makePerspective( 30., 1., 1., 10. );
        node = new osg::PositionAttitudeTransform;
        node->setPosition(matrixf.getTrans());
        node->setAttitude(matrixf.getRotate());
        node->addChild(makeFrustumFromCamera(matrixf, projection));
        cv::FileNode SourceNode = (*it)["Source"];
        if ( !SourceNode.empty() ) {
            this->sourceType = (std::string)SourceNode["Type"];
            this->sourcePath = (std::string)SourceNode["Path"];
            std::cout << "Camera Source, Type:" << this->sourceType << ", Path:" << this->sourcePath << std::endl;
        }
    }

    void saveCamera(cv::FileStorage& fs) {
        fs << "{"
        << "name" << this->getCamera()->getName()
        << "desc" << this->getCamera()->getDescription(0)
        << "Matrix" << cv::Mat(4,4,CV_32F,(this->matrixf.ptr()))
        << "Source" << "{"
        << "Type" << this->sourceType
        << "Path" << this->sourcePath
        << "}" << "}";
    }

    void setMatrix(osg::Matrixf mat) {
        this->matrixf = mat;
        node->setPosition(matrixf.getTrans());
        node->setAttitude(matrixf.getRotate());
    }

    void setProjection(osg::Matrixf proj ) {
        this->projection = proj;
    }

    void setupCamera(int idx, osg::Texture2DArray* array) {
        if( this->sourceType.find("Image") != std::string::npos) {
            std::cout << "setImageSource: " << this->sourcePath << std::endl;
            osg::Image* img = osgDB::readImageFile(this->sourcePath);
            img->scaleImage(256,256,1);
            array->setImage(idx, img);
        } else if( this->sourceType.find("CameraId") != std::string::npos ) {
            int camId = std::stoi(this->sourcePath);
            std::cout << "setVideoSource: camId=" << camId << std::endl;
            osg::ref_ptr<opencv_imagestream> imageStream = new opencv_imagestream();
            imageStream->openCamera(camId);
            array->setImage(idx, imageStream);
        } else if( this->sourceType.find("Video") != std::string::npos) {
            std::cout << "setVideoSource: sourcePath=" << this->sourcePath << std::endl;
            osg::ref_ptr<opencv_imagestream> imageStream = new opencv_imagestream();
            imageStream->openCamera(this->sourcePath);
            array->setImage(idx, imageStream);
        }
    }

    osg::Image* getImage() { return image; }
    osg::Camera* getCamera() { return cam; }
};

#endif //OSGVIDEO3D_VIRTUAL_CAMERA_H
