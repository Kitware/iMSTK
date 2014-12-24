#include "CollisionDetectionExampleWithHash.h"

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smCore/smEventData.h"




CollisionDetectionExampleWithHash::CollisionDetectionExampleWithHash()
{
    ///create the sdk
    simmedtkSDK = smSDK::createSDK();
    ///create scene
    scene1 = simmedtkSDK->createScene();

    ///intializes the spatial hashing
    hash = new smSpatialHash(smSDK::getErrorLog(), 10000, 2, 2, 2);

    ///enable filtering any duplicate copies
    hash->enableDuplicateFilter = true;



    ///create dummy simulator
    dummySim = new smDummySimulator(smSDK::getErrorLog());


    ///init dispacther for events
    simmedtkSDK->getEventDispatcher()->registerEventHandler(dummySim, SIMMEDTK_EVENTTYPE_KEYBOARD);

    ///init texture manager and specify the textures needed for the current application
    smTextureManager::init(smSDK::getErrorLog());
    smTextureManager::loadTexture("../../resources/textures/fat9.bmp", "livertexture1");
    smTextureManager::loadTexture("../../resources/textures/blood.jpg", "livertexture2");


    smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg", "groundImage");
    smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg", "groundBumpImage");
    smTextureManager::loadTexture("../../resources/textures/brick.jpg", "wallImage");
    smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg", "wallBumpImage");


    ///create a static scene
    object1 = new smStaticSceneObject();
    ///load mesh
    object1->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    ///assign a texture
    object1->mesh->assignTexture("livertexture1");
    ///set the rendering features
    object1->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    object1->mesh->translate(7, 3, 0);
    object1->renderDetail.lineSize = 2;
    object1->renderDetail.pointSize = 5;
    ////attach object to the dummy simulator. it will be simulated by dummy simulator
    object1->attachObjectSimulator(dummySim);
    hash->addMesh(object1->mesh);

    ///initialize the scecond object
    object2 = new smStaticSceneObject();
    object2->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS", SM_FILETYPE_3DS);
    object2->mesh->translate(smVec3<smFloat>(2, 0, 0));

    object2->mesh->assignTexture("livertexture2");
    object2->renderDetail.shadowColor.rgba[0] = 1.0;
    object2->renderDetail.renderType = (SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE | SIMMEDTK_RENDER_MATERIALCOLOR);
    hash->addMesh(object2->mesh);

    ///we want to get information with pipe
    this->myCollInformation.listenerObject = &*this;
    this->myCollInformation.regType = SIMMEDTK_PIPE_BYREF;

    hash->pipeTriangles->registerListener(&this->myCollInformation);


    //add object to the scene
    scene1->addSceneObject(object1);
    scene1->addSceneObject(object2);
    ///create a simulator module
    simulator = simmedtkSDK->createSimulator();
    simulator->registerObjectSimulator(dummySim);
    simulator->registerCollisionDetection(hash);

    ///create a viewer
    viewer = simmedtkSDK->createViewer();
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_GROUND;
    viewer->camera()->setFieldOfView(SM_DEGREES2RADIANS(60));
    viewer->camera()->setZClippingCoefficient(1000);
    viewer->camera()->setZNearCoefficient(0.001);
    viewer->list();
    viewer->setWindowTitle("SimMedTK TEST");

    ///we want viewer to render this object
    viewer->addObject(this);

    ///set event distpacher to the viewer
    viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());
    simulator->registerSimulationMain(this);

    ///run the SDK
    simmedtkSDK->run();
}

///Draw the collided triangles. This will be called due to the function call viewer->addObject(this)
void CollisionDetectionExampleWithHash::draw(smDrawParam p_params)
{
    smCollidedTriangles *tris;

    if (myCollInformation.data.dataReady)
    {
        if (myCollInformation.data.nbrElements > 0)
        {
            tris = (smCollidedTriangles *)myCollInformation.data.dataLocation;
            glBegin(GL_TRIANGLES);

            for (smInt i = 0; i < myCollInformation.data.nbrElements; i++)
            {
                glVertex3fv((GLfloat*)&tris[i].tri1.vert[0]);
                glVertex3fv((GLfloat*)&tris[i].tri1.vert[1]);
                glVertex3fv((GLfloat*)&tris[i].tri1.vert[2]);

                glVertex3fv((GLfloat*)&tris[i].tri2.vert[0]);
                glVertex3fv((GLfloat*)&tris[i].tri2.vert[1]);
                glVertex3fv((GLfloat*)&tris[i].tri2.vert[2]);

            }

            glEnd();


        }


    }



}

void CollisionDetectionExampleWithHash::simulateMain(smSimulationMainParam p_param)
{
    cout << "simulation called" << endl;
}

CollisionDetectionExampleWithHash::~CollisionDetectionExampleWithHash()
{
    delete object1;
    delete object2;
    delete dummySim;
    delete hash;


}

void main(int argc, char** argv)
{
    CollisionDetectionExampleWithHash *ex = new CollisionDetectionExampleWithHash();
    delete ex;


}
