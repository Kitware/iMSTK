#ifndef SMIMPORTEXPORT_H
#define SMIMPORTEXPORT_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smMesh/smMesh.h"

/// \brief
enum smExportOptions{
	SOFMIS_EXPORT_ALL
};

/// \brief class for importing and exporting meshes
class smImportExport:public smCoreClass{

public:
	static smBool convertToJSON(smMesh *mesh, char *outputFileName,
                                    smExportOptions p_export=SOFMIS_EXPORT_ALL);
};

#endif
