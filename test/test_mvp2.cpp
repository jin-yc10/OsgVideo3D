//
// Created by 金宇超 on 15/11/17.
//

#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osg/io_utils>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture2DArray>
#include <osg/TexGen>
#include <osg/Geode>

#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>

#include <iostream>

osg::StateSet* createProjectorState() {

    osg::StateSet* stateset = new osg::StateSet;
    osg::ref_ptr<osg::Program> projProg(new osg::Program);
    osg::ref_ptr<osg::Shader> projvertexShader( osg::Shader::readShaderFile(osg::Shader::VERTEX, "shaders/video.vert"));
    osg::ref_ptr<osg::Shader> projfragShader( osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "shaders/video.debug.frag"));
    projProg->addShader(projvertexShader.get());
    projProg->addShader(projfragShader.get());

    osgDB::Registry::instance()->addFileExtensionAlias( "jpeg", "jpeg" );
    osgDB::Registry::instance()->addFileExtensionAlias( "jpg", "jpeg" );

/* 1. Load the texture that will be projected */
    osg::Image* img1 = osgDB::readImageFile("images/1.jpeg");
    osg::Image* img2 = osgDB::readImageFile("images/2.jpeg");
    if( !img1 && !img2) {
        std::cout << "invalid input textures" << std::endl;
    }
    img1->scaleImage(256,256,1);
    img2->scaleImage(256,256,1);
    if( img1->getPixelFormat() != img2->getPixelFormat()) {
        std::cout << "not compatible" << std::endl;
    }
    osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
    texture->setFilter(osg::Texture2DArray::MIN_FILTER, osg::Texture2DArray::LINEAR);
    texture->setFilter(osg::Texture2DArray::MAG_FILTER, osg::Texture2DArray::LINEAR);
    texture->setWrap(osg::Texture2DArray::WRAP_R, osg::Texture2DArray::REPEAT);
    texture->setImage(0, img1);
    texture->setImage(1, img2);
    stateset->setTextureAttributeAndModes(0, texture.get(),osg::StateAttribute::ON);
//    osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "textures");
//    texUniform->set(0);
//    stateset->addUniform( texUniform );
    stateset->addUniform( new osg::Uniform( "textures", 0 ) );

    osg::Texture2D* texSingle = new osg::Texture2D();
    texSingle->setImage(osgDB::readImageFile("images/1.jpeg"));
    texSingle->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    texSingle->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    texSingle->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER);
    stateset->setTextureAttributeAndModes(1, texSingle, osg::StateAttribute::ON);
    osg::Uniform* texSingleUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "textureSingle");
    texSingleUniform->set(1);
    stateset->addUniform(texSingleUniform);

/* 2. Load the Shaders */
    float projectorAngle = 90;
    osg::Matrixd view = osg::Matrixd(-6.6591516310164645e-01, -7.4602747640506362e-01,
                                     -1.6653345369377348e-16, 0., 2.4576943752767533e-01,
                                     -2.1937743615729777e-01, 9.4417737956507986e-01, 0.,
                                     -7.0438226775568236e-01, 6.2874203370996518e-01,
                                     3.2943751443577152e-01, 0., 5.0784580030722219e+02,
                                     1.1665999253182690e+01, 3.9053352963362147e+02, 1.);
    osg::Vec3 projectorPos = osg::Vec3(500.0f, 500.0f, 500.0f);
    osg::Vec3 projectorDirection = osg::Vec3(-1.0f, -1.0f, -1.0f);
    osg::Vec3 up(-1.0f, -1.0f, 1.0f);
    osg::Matrixd biasMatrix = osg::Matrixd(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            -0.5, -0.5, -0.5, 1.0
    );
    osg::Matrixd view2 = osg::Matrixd::lookAt(projectorPos, projectorPos + projectorDirection, up);
    osg::Matrix mat;
    mat =   biasMatrix *
            view2
            * osg::Matrixd::perspective(projectorAngle, 1.0, 0.1, 100);
//    osg::Uniform* cams = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"cameraMVP[0]",2);
//    cams->setElement(0, mat);
//    stateset->addUniform( cams );
    osg::Uniform* camUniform = new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "cameraMVP", 2);
    camUniform->setNumElements(2);
    camUniform->setElement(0,mat);
    camUniform->setElement(1,mat);
    std::cout << mat << std::endl;
    stateset->addUniform(camUniform);

    stateset->setAttribute(projProg.get());

    return stateset;
}

osg::Node* createModel() {
    osg::Group* root = new osg::Group;
    osg::Node* terr = osgDB::readNodeFile("/Users/jin-yc10/Desktop/floor2.obj");
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    osg::Matrix m;
    m.makeTranslate(0.f, 0.0f, 0.f);
    m.makeScale(1.f, 1.f, 1.f);
    mt->setMatrix(m);
    mt->addChild(terr);
    root->addChild(mt.get());
    root->setStateSet(createProjectorState());
    return root;
}

int main(int, char**) {
    osgViewer::Viewer viewer;
    osg::Vec3 projectorPos = osg::Vec3(500.0f, 500.0f, 500.0f);
    osg::Vec3 projectorDirection = osg::Vec3(-1.0f, -1.0f, -1.0f);
    osg::Vec3 up(-1.0f, -1.0f, 1.0f);
    osg::Matrixd view2 = osg::Matrixd::lookAt(projectorPos, projectorPos + projectorDirection, up);
    viewer.setSceneData(createModel());
    viewer.setUpViewInWindow(0, 0, 1024, 768);
    viewer.getCameraManipulator()->setByMatrix(view2);
//    viewer.getCamera()->setViewMatrixAsLookAt(projectorPos, projectorPos + projectorDirection, up);
    return viewer.run();
}