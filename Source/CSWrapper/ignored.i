%ignore imstk::PbdCollisionConstraint;
/* %ignore imstk::PbdFEMConstraint; */
%ignore imstk::PbdModel::getIntegratePositionNode();
%ignore imstk::PbdModel::getUpdateCollisionGeometryNode();
%ignore imstk::PbdModel::getSolveNode();
%ignore imstk::PbdModel::getUpdateVelocityNode();

%ignore imstk::DataArray::iterator; /* fix the multiple-definition problem. */
%ignore imstk::DataArray::const_iterator; /* fix the multiple-definition problem. */
%ignore imstk::DataArray::beign(); /* fix the multiple-definition problem. */
%ignore imstk::DataArray::cbeign(); /* fix the multiple-definition problem. */
%ignore imstk::DataArray::end(); /* fix the multiple-definition problem. */
%ignore imstk::DataArray::cend(); /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::iterator; /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::const_iterator;
%ignore imstk::VecDataArray::beign(); /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::cbeign(); /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::end(); /* fix the multiple-definition problem. */
%ignore imstk::VecDataArray::cend(); /* fix the multiple-definition problem. */
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

%ignore imstk::InteractionPair::getTaskNodeInputs();
%ignore imstk::InteractionPair::getTaskNodeOutputs();
%ignore imstk::CollisionGraph::getInteractionPairMap();

%ignore imstk::FeDeformableObject::getFEMModel();

%ignore imstk::GeometryUtils::coupleVtkDataArray();
%ignore imstk::GeometryUtils::coupleVtkImageData();
%ignore imstk::GeometryUtils::copyToVtkDataArray();
%ignore imstk::GeometryUtils::copyToVtkImageData();
%ignore imstk::GeometryUtils::copyToImageData();
%ignore imstk::GeometryUtils::copyToPointSet();
%ignore imstk::GeometryUtils::copyToSurfaceMesh();

%ignore imstk::expiremental::RbdConstraint;
%ignore imstk::CollisionHandling::getTaskNode();

%ignore imstk::VTKTextStatusManager::getTextActor();
