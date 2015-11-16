#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

osg::StateSet* createProjectorState() {
    osg::StateSet* stateset = new osg::StateSet;
/* 1. Load the texture that will be projected */
	osg::Texture2D* texture = new osg::Texture2D();
	texture->setImage(osgDB::readImageFile("images/1.jpeg"));
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER);
	stateset->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);

// set up tex gens

	stateset->setTextureMode(1, GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
	stateset->setTextureMode(1, GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
	stateset->setTextureMode(1, GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
	stateset->setTextureMode(1, GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
/* 2. Load the Shaders */

    osg::ref_ptr<osg::Program> projProg(new osg::Program);
    osg::ref_ptr<osg::Shader> projvertexShader( osg::Shader::readShaderFile(osg::Shader::VERTEX, "shaders/project.vert"));
    osg::ref_ptr<osg::Shader> projfragShader( osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "shaders/project.frag"));
    projProg->addShader(projvertexShader.get());
    projProg->addShader(projfragShader.get());

/* 3. Handover the texture to the fragment shader via uniform */
	osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "projectionMap");
	texUniform->set(1);
	stateset->addUniform(texUniform);
/* 4. set Texture matrix*/
    osg::TexMat* texMat = new osg::TexMat;
    osg::Matrix mat;
    osg::Vec3 projectorPos = osg::Vec3(0.0f, 0.0f, 300.0f);
    osg::Vec3 projectorDirection = osg::Vec3(0.0f, 0.0f, -1.0f);
    osg::Vec3 up(0.0f, 1.0f, 0.0f);
    float projectorAngle = 110;
    osg::Matrixd view = osg::Matrixd(-6.6591516310164645e-01, -7.4602747640506362e-01,
                                     -1.6653345369377348e-16, 0., 2.4576943752767533e-01,
                                     -2.1937743615729777e-01, 9.4417737956507986e-01, 0.,
                                     -7.0438226775568236e-01, 6.2874203370996518e-01,
                                     3.2943751443577152e-01, 0., 5.0784580030722219e+02,
                                     1.1665999253182690e+01, 3.9053352963362147e+02, 1.);
    mat =
            osg::Matrixd::lookAt(projectorPos, projectorPos + projectorDirection, up)
//          view
          * osg::Matrixd::perspective(projectorAngle, 1.0, 0.1, 100);
    texMat->setMatrix(mat);
    stateset->setTextureAttributeAndModes(1, texMat, osg::StateAttribute::ON);
    stateset->setAttribute(projProg.get());
    return stateset;
}

osg::Node* createModel() {

    osg::Group* root = new osg::Group;

/* Load the terrain which will be the receiver of out projection */
	osg::Node* terr = osgDB::readNodeFile("Terrain2.3ds");
	osg::Image* shot = new osg::Image();
/* Scale the terrain and move it. */
    osg::Matrix m;
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    m.makeTranslate(0.f, 0.0f, 0.f);
    m.makeScale(2.f, 2.f, 2.f);
    mt->setMatrix(m);
    mt->addChild(terr);

/* Add the transformed node to our graph */

    root->addChild(mt.get());
/* Enable projective texturing for all objects of this node */
	root->setStateSet(createProjectorState());
	return root;
}

int main(int, char **) {
	osgViewer::Viewer viewer;
	viewer.setSceneData(createModel());
	viewer.setUpViewInWindow(0, 0, 1024, 768);
	return viewer.run();

}