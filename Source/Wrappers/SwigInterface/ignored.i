%ignore imstk::PbdCollisionConstraint;
/* %ignore imstk::PbdFEMConstraint; */
%ignore imstk::PbdModel::getIntegratePositionNode();
%ignore imstk::PbdModel::getUpdateCollisionGeometryNode();
%ignore imstk::PbdModel::getSolveNode();
%ignore imstk::PbdModel::getUpdateVelocityNode();

%ignore imstk::DataArray::iterator; /* fix the multiple-definition problem. */
%ignore imstk::DataArray::const_iterator; /* fix the multiple-definition problem. */
%ignore imstk::DataArray::begin(); /* fix the multiple-definition problem. */
%ignore imstk::DataArray::cbegin() const; /* fix the multiple-definition problem. */
%ignore imstk::DataArray::end(); /* fix the multiple-definition problem. */
%ignore imstk::DataArray::cend() const; /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::iterator; /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::const_iterator;
%ignore imstk::VecDataArray::begin(); /* fix the multiple-definition problem. */ 
%ignore imstk::VecDataArray::cbegin() const; /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::end(); /* fix the multiple-definition problem. */ 
%ignore imstk::VecDataArray::cend() const; /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::setData();
%ignore imstk::stdSink;
%ignore imstk::LogManager;
%ignore imstk::Logger::Logger();
%ignore imstk::Logger::getInstance();
%ignore imstk::Logger::addStdoutSink();
%ignore imstk::Logger::addFileSink;
%ignore imstk::Logger::addSink;
%ignore imstk::Logger::initialize();
%ignore imstk::Logger::destroy();
%ignore imstk::Log;
%rename("%s") imstk::Logger::startLogger();

%ignore imstk::Scene::getCameras() const;

%ignore imstk::InteractionPair::getTaskNodeInputs();
%ignore imstk::InteractionPair::getTaskNodeOutputs();
%ignore imstk::CollisionGraph::getInteractionPairMap();

%ignore imstk::FeDeformableObject::getFEMModel();
%ignore imstk::FemDeformableBodyModel::initializeEigenMatrixFromVegaMatrix;

%ignore imstk::RbdConstraint;
%ignore imstk::CollisionHandling::getTaskNode();

%ignore imstk::VTKTextStatusManager::getTextActor();
%ignore imstk::AbstractVTKViewer::getVtkRenderWindow() const;

%ignore imstk::GeometryUtils::coupleVtkDataArray;
%ignore imstk::GeometryUtils::coupleVtkImageData;
%ignore imstk::GeometryUtils::copyToVtkDataArray;
%ignore imstk::GeometryUtils::copyToVtkImageData;
%ignore imstk::GeometryUtils::copyToPointSet;
%ignore imstk::GeometryUtils::copyToSurfaceMesh;
%ignore imstk::GeometryUtils::copyToLineMesh;
%ignore imstk::GeometryUtils::copyToVolumetricMesh;
%ignore imstk::GeometryUtils::copyToVtkPointSet;
%ignore imstk::GeometryUtils::copyToVtkPolyData;
%ignore imstk::GeometryUtils::copyToVtkUnstructuredGrid;
%ignore imstk::GeometryUtils::copyToVtkVecDataArray;
%ignore imstk::GeometryUtils::copyToVtkPoints;
%ignore imstk::GeometryUtils::copyToVtkCellArray;
%ignore imstk::GeometryUtils::copyToDataArray;
%ignore imstk::GeometryUtils::copyToVecDataArray;
%ignore imstk::GeometryUtils::copyToImageData;

%ignore imstk::CollisionElement::m_element;
%ignore imstk::CollisionElement::m_type;

%ignore imstk::HapticDeviceClient::hapticCallback;

%ignore imstk::VRPNDeviceClient::trackerPositionChangeHandler;
%ignore imstk::VRPNDeviceClient::trackerVelocityChangeHandler;
%ignore imstk::VRPNDeviceClient::analogChangeHandler;
%ignore imstk::VRPNDeviceClient::buttonChangeHandler;