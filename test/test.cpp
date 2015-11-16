//
// Created by 金宇超 on 15/11/12.
//

#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgText/Font>
#include <osgText/Text3D>

#include <osg/Group>
#include <osg/Quat>
#include <osgUtil/CullVisitor>

#include <osgText/Font>
#include <osgText/String>
#include <osgText/Glyph>
#include <osgText/Style>
#include <osgText/Text>


int main(int argc, char** argv) {
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);
    std::string fontFile("fonts/Vera.ttf");
    osg::ref_ptr<osgText::Font> font = osgText::readFontFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE <<"Read font "<< fontFile << " font=" << font.get() << std::endl;
    osg::ref_ptr<osg::Group> group = new osg::Group;
    osgText::Text* text2D = new osgText::Text;
    text2D->setFont(0);
    text2D->setCharacterSize(2.0f); // medium
//    text2D->setAxisAlignment(osgText::Text::SCREEN);
//    text2D->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    text2D->setFontResolution(256,256);
    text2D->setDrawMode(osgText::Text::TEXT | osgText::Text::BOUNDINGBOX);
    text2D->setAxisAlignment(osgText::Text::XZ_PLANE);
    text2D->setText("Hello!");
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(text2D);
    group->addChild(geode);
    viewer.setSceneData(group.get());
    return viewer.run();
}