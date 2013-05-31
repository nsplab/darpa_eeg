#include <iostream>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PolygonMode>

using namespace std;

void Add10_20Electrodes(osg::ref_ptr<osg::Group> root, vector<osg::ref_ptr<osg::ShapeDrawable> > electrodes, osg::Vec4 color=osg::Vec4(1.0,0.5,0.5,1.0), float radius=3.0);
void AddHeadLayers(osg::ref_ptr<osg::Group> root);

int main()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();

    AddHeadLayers(root);

    vector<osg::ref_ptr<osg::ShapeDrawable> > electrodes;
    Add10_20Electrodes(root, electrodes);

    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor(osg::Vec4(1.0, 1.0, 1.0, 1));
    viewer.setSceneData(root);
    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.realize();

    while( !viewer.done() )  {
        viewer.frame();
    }

    return 0;
}

void AddHeadLayers(osg::ref_ptr<osg::Group> root) {
    osg::ref_ptr<osg::Node> scalp = osgDB::readNodeFile("../meshes/skin96.stl");
    osg::ref_ptr<osg::Node> skull = osgDB::readNodeFile("../meshes/skull48.stl");
    osg::ref_ptr<osg::Node> brain = osgDB::readNodeFile("../meshes/brain48.stl");
    osg::ref_ptr<osg::Node> artery = osgDB::readNodeFile("../meshes/artery11.stl");
    osg::ref_ptr<osg::Node> vent = osgDB::readNodeFile("../meshes/ventricles11.stl");

    scalp->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    skull->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    brain->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    artery->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    vent->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

    scalp->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    skull->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    brain->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    artery->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    vent->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    vent->getOrCreateStateSet()->setRenderBinDetails(1, "DepthSortedBin");
    artery->getOrCreateStateSet()->setRenderBinDetails(2, "DepthSortedBin");
    brain->getOrCreateStateSet()->setRenderBinDetails(3, "DepthSortedBin");
    skull->getOrCreateStateSet()->setRenderBinDetails(4, "DepthSortedBin");
    scalp->getOrCreateStateSet()->setRenderBinDetails(5, "DepthSortedBin");

    // define materials
    osg::ref_ptr<osg::Material> redMat = new osg::Material;
    osg::ref_ptr<osg::Material> blueMat = new osg::Material;
    osg::ref_ptr<osg::Material> greenMat = new osg::Material;
    osg::ref_ptr<osg::Material> whiteMat = new osg::Material;
    osg::Vec4 darkBlue(0.2,0.2,0.6,0.3);
    osg::Vec4 blue(0.2,0.2,0.6,0.3);
    osg::Vec4 darkGreen(0.2,0.6,0.2,0.3);
    osg::Vec4 green(0.2,0.6,0.2,0.3);
    osg::Vec4 darkRed(0.6,0.2,0.2,0.3);
    osg::Vec4 red(0.6,0.2,0.2,0.3);
    osg::Vec4 gray(0.3,0.3,0.3,0.3);
    osg::Vec4 white(0.3,0.3,0.3,0.3);

    osg::PolygonMode * polygonMode = new osg::PolygonMode;
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

    blueMat->setAmbient (osg::Material::FRONT_AND_BACK, blue);
    blueMat->setDiffuse (osg::Material::FRONT_AND_BACK, blue);
    blueMat->setSpecular(osg::Material::FRONT_AND_BACK, darkBlue);

    greenMat->setAmbient (osg::Material::FRONT_AND_BACK, green);
    greenMat->setDiffuse (osg::Material::FRONT_AND_BACK, green);
    greenMat->setSpecular(osg::Material::FRONT_AND_BACK, darkGreen);

    redMat->setAmbient (osg::Material::FRONT_AND_BACK, red);
    redMat->setDiffuse (osg::Material::FRONT_AND_BACK, red);
    redMat->setSpecular(osg::Material::FRONT_AND_BACK, darkRed);

    whiteMat->setAmbient (osg::Material::FRONT_AND_BACK, white);
    whiteMat->setDiffuse (osg::Material::FRONT_AND_BACK, white);
    whiteMat->setSpecular(osg::Material::FRONT_AND_BACK, gray);

    // set material of layers
    scalp->getOrCreateStateSet()->setAttributeAndModes(whiteMat.get() ,
                                                      osg::StateAttribute::ON |
                                                      osg::StateAttribute::OVERRIDE);
    skull->getOrCreateStateSet()->setAttributeAndModes(blueMat.get() ,
                                                      osg::StateAttribute::ON |
                                                      osg::StateAttribute::OVERRIDE);
    brain->getOrCreateStateSet()->setAttributeAndModes(redMat.get() ,
                                                      osg::StateAttribute::ON |
                                                      osg::StateAttribute::OVERRIDE);
    artery->getOrCreateStateSet()->setAttributeAndModes(greenMat.get() ,
                                                      osg::StateAttribute::ON |
                                                      osg::StateAttribute::OVERRIDE);
    vent->getOrCreateStateSet()->setAttributeAndModes(redMat.get() ,
                                                      osg::StateAttribute::ON |
                                                      osg::StateAttribute::OVERRIDE);

    root->addChild(vent);
    root->addChild(artery);
    root->addChild(brain);
    root->addChild(skull);
    root->addChild(scalp);
}

void Add10_20Electrodes(osg::ref_ptr<osg::Group> root, vector<osg::ref_ptr<osg::ShapeDrawable> > electrodes, osg::Vec4 color, float radius) {
    osg::ref_ptr<osg::Geode> electrodsGeode = new osg::Geode;

    // G
    osg::ref_ptr<osg::ShapeDrawable> g = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(4.88623, -216.92967, -107.01745), radius));
    // FP1
    osg::ref_ptr<osg::ShapeDrawable> fp1 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(22.66097, -215.17047, -106.02161), radius));
    // Fp2
    osg::ref_ptr<osg::ShapeDrawable> fp2 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-14.51707, -215.99734, -107.96710), radius));
    // F7
    osg::ref_ptr<osg::ShapeDrawable> f7 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(64.50138, -180.06104, -115.39294), radius));
    // F3
    osg::ref_ptr<osg::ShapeDrawable> f3 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(44.53565, -183.85083, -77.47854), radius));
    // Fz
    osg::ref_ptr<osg::ShapeDrawable> fz = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(1.87596, -196.02194, -68.69604), radius));
    // F4
    osg::ref_ptr<osg::ShapeDrawable> f4 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-37.78682, -183.39616, -77.88834), radius));
    // F8
    osg::ref_ptr<osg::ShapeDrawable> f8 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-60.20803, -176.53879, -119.37214), radius));
    // T3
    osg::ref_ptr<osg::ShapeDrawable> t3 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(77.96370, -132.19577, -123.96312), radius));
    // C3
    osg::ref_ptr<osg::ShapeDrawable> c3 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(48.23156, -141.30139, -53.65706), radius));
    // Cz
    osg::ref_ptr<osg::ShapeDrawable> cz = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-1.38297, -144.61928, -40.58036), radius));
    // C4
    osg::ref_ptr<osg::ShapeDrawable> c4 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-51.34485, -132.28114, -63.19135), radius));
    // T4
    osg::ref_ptr<osg::ShapeDrawable> t4 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-73.09639, -129.91707, -126.25740), radius));
    // T5
    osg::ref_ptr<osg::ShapeDrawable> t5 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(73.95229, -94.09358, -108.64140), radius));
    // P3
    osg::ref_ptr<osg::ShapeDrawable> p3 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(44.53437, -85.88811, -52.85554), radius));
    // Pz
    osg::ref_ptr<osg::ShapeDrawable> pz = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(4.47472, -87.27579, -40.55307), radius));
    // P4
    osg::ref_ptr<osg::ShapeDrawable> p4 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-36.97533, -86.45993, -55.13177), radius));
    // T6
    osg::ref_ptr<osg::ShapeDrawable> t6 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-60.07651, -86.27083, -120.12472), radius));
    // O1
    osg::ref_ptr<osg::ShapeDrawable> o1 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(42.53815, -49.62876, -98.95287), radius));
    // O2
    osg::ref_ptr<osg::ShapeDrawable> o2 = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(-11.83266, -44.56505, -99.30063), radius));

    electrodsGeode->addDrawable(g);   electrodes.push_back(g);
    electrodsGeode->addDrawable(fp1); electrodes.push_back(fp1);
    electrodsGeode->addDrawable(fp2); electrodes.push_back(fp2);
    electrodsGeode->addDrawable(f7);  electrodes.push_back(f7);
    electrodsGeode->addDrawable(f3);  electrodes.push_back(f3);
    electrodsGeode->addDrawable(fz);  electrodes.push_back(fz);
    electrodsGeode->addDrawable(f4);  electrodes.push_back(f4);
    electrodsGeode->addDrawable(f8);  electrodes.push_back(f8);
    electrodsGeode->addDrawable(t3);  electrodes.push_back(t3);
    electrodsGeode->addDrawable(c3);  electrodes.push_back(c3);
    electrodsGeode->addDrawable(cz);  electrodes.push_back(cz);
    electrodsGeode->addDrawable(c4);  electrodes.push_back(c4);
    electrodsGeode->addDrawable(t4);  electrodes.push_back(t4);
    electrodsGeode->addDrawable(t5);  electrodes.push_back(t5);
    electrodsGeode->addDrawable(p3);  electrodes.push_back(p3);
    electrodsGeode->addDrawable(pz);  electrodes.push_back(pz);
    electrodsGeode->addDrawable(p4);  electrodes.push_back(p4);
    electrodsGeode->addDrawable(t6);  electrodes.push_back(t6);
    electrodsGeode->addDrawable(o1);  electrodes.push_back(o1);
    electrodsGeode->addDrawable(o2);  electrodes.push_back(o2);

    for(size_t i=0; i<electrodes.size(); i++) {
        electrodes[i]->setColor(color);
    }

    root->addChild(electrodsGeode);
}
