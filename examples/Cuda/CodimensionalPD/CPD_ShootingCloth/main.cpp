#include <GlfwApp.h>
#include "Peridynamics/Cloth.h"
#include <SceneGraph.h>
#include <Log.h>
#include <ParticleSystem/StaticBoundary.h>

#include <Multiphysics/VolumeBoundary.h>

#include <GLRenderEngine.h>
#include <GLPointVisualModule.h>
#include <GLSurfaceVisualModule.h>
#include "TriangleMeshWriter.h"
#include "Peridynamics/CodimensionalPD.h"
#include "StaticTriangularMesh.h"
#include "ManualControl.h"
using namespace std;
using namespace dyno;

std::shared_ptr<SceneGraph> createScene()
{
	std::shared_ptr<SceneGraph> scn = std::make_shared<SceneGraph>();
	scn->setLowerBound(Vec3f(-5, 0, -5));
	scn->setUpperBound(Vec3f(5, 3, 5));
	
	auto boundary = scn->addNode(std::make_shared<VolumeBoundary<DataType3f>>());
	boundary->loadCube(Vec3f(-4, -1.5, -3), Vec3f(2, -0.001, 3), 0.005f, false);
	auto cloth = scn->addNode(std::make_shared<CodimensionalPD<DataType3f>>(0.15,4000,0.005,0.0005));
	//can try��
	//auto cloth = scn->addNode(std::make_shared<CodimensionalPD<DataType3f>>(0.15,8000,0.005,0.0005));
	//auto cloth = scn->addNode(std::make_shared<CodimensionalPD<DataType3f>>(0.15,2000,0.005,0.0005));
	//auto cloth = scn->addNode(std::make_shared<CodimensionalPD<DataType3f>>(0.15,40000, 0.005, 0.0005));

	cloth->loadSurface(getAssetPath() + "cloth_shell/shootingCloth.obj");
	cloth->connect(boundary->importTriangularSystems()); 

	auto custom = std::make_shared<ManualControl<DataType3f>>();
	cloth->statePosition()->connect(custom->inPosition());
	cloth->stateVelocity()->connect(custom->inVelocity());
	cloth->stateFrameNumber()->connect(custom->inFrameNumber());
	cloth->stateAttribute()->connect(custom->inAttribute());
	cloth->animationPipeline()->pushModule(custom);

	auto surfaceRendererCloth = std::make_shared<GLSurfaceVisualModule>();
	surfaceRendererCloth->setColor(Color(1,1,1));
	cloth->stateTriangleSet()->connect(surfaceRendererCloth->inTriangleSet());
	cloth->graphicsPipeline()->pushModule(surfaceRendererCloth);
	cloth->setVisible(true);

	scn->printNodeInfo(true);
	scn->printModuleInfo(true);

	return scn;
}

void RecieveLogMessage(const Log::Message& m)
{
	switch (m.type)
	{
	case Log::Info:
		cout << ">>>: " << m.text << endl; break;
	case Log::Warning:
		cout << "???: " << m.text << endl; break;
	case Log::Error:
		cout << "!!!: " << m.text << endl; break;
	case Log::User:
		cout << ">>>: " << m.text << endl; break;
	default: break;
	}
}


int main()
{
	Log::setUserReceiver(&RecieveLogMessage);

	GlfwApp window;
	window.setSceneGraph(createScene());

	window.initialize(1024, 768);
	window.mainLoop();

	return 0;
}