/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#include "smCollision/smCollisionModel.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include <stack>
#include "smCore/smEvent.h"

/// \brief initialize the surface tree structure
template <typename smSurfaceTreeCell>
void smSurfaceTree<smSurfaceTreeCell>::initStructure(){

	smVec3f center;
	smFloat edge;
	vector<smInt> triangles;

	for(smInt i=0;i<mesh->nbrTriangles;i++)
		triangles.push_back(i);

	center=mesh->aabb.center();
	edge=SIMMEDTK_MAX(SIMMEDTK_MAX(mesh->aabb.halfSizeX(), mesh->aabb.halfSizeY()),
                                                        mesh->aabb.halfSizeZ());
	root.setCenter(center);
	root.setLength(2*edge);
	root.filled=true;

	treeAllLevels[0]=root;
	createTree(treeAllLevels[0],triangles,0,0);
	memcpy(initialTreeAllLevels,treeAllLevels,this->totalCells*sizeof(smSurfaceTreeCell));
	triangles.clear();

}

/// \brief destructor
template<typename smSurfaceTreeCell>
smSurfaceTree<smSurfaceTreeCell>::~smSurfaceTree(){
	delete[]	treeAllLevels;
	delete[]	levelStartIndex;
}

/// \brief
template<typename smSurfaceTreeCell>
smSurfaceTree<smSurfaceTreeCell>::smSurfaceTree(smSurfaceMesh *p_mesh,smInt p_maxLevels,SIMMEDTK_TREETYPE p_treeType){
	mesh=p_mesh;
	totalCells=0;
	switch(p_treeType){
		case SIMMEDTK_TREETYPE_OCTREE:
			nbrDivision=SIMMEDTK_TREE_DIVISION_OCTREE;
			break;
	}

	//set the total levels
	currentLevel=maxLevel=p_maxLevels;

	//compute the total cells
	for(smInt i=0;i<maxLevel;i++)
		totalCells+=smMath::pow(nbrDivision,i);

	treeAllLevels=new smSurfaceTreeCell[totalCells];
	initialTreeAllLevels=new smSurfaceTreeCell[totalCells];
	levelStartIndex=new smLevelIndex[maxLevel];

	//compute the levels start and end
	smInt previousIndex=totalCells;
	levelStartIndex[0].startIndex=0;
	levelStartIndex[0].endIndex=1;

	for(smInt i=1;i<maxLevel;i++){
		levelStartIndex[i].startIndex=levelStartIndex[i-1].startIndex+smMath::pow(nbrDivision,i-1);
		levelStartIndex[i-1].endIndex=levelStartIndex[i].startIndex;
	}
	levelStartIndex[maxLevel-1].endIndex=totalCells;

	minTreeRenderLevel=0;
	renderSurface=false;
	enableShiftPos=false;
	shiftScale=1.0;
	enableTrianglePos=false;
	renderOnlySurface=false;
}

/// \brief Initialize the drawing structures
template<typename smSurfaceTreeCell>
void smSurfaceTree<smSurfaceTreeCell>::initDraw(smDrawParam p_param){

	smViewer *viewer;
	viewer=p_param.rendererObject;
	viewer->addText(QString("octree"));
}

/// \brief draw the surface tree
template<typename smSurfaceTreeCell> void smSurfaceTree<smSurfaceTreeCell>
                                                   ::draw(smDrawParam p_params){

	smVec3f center;
	smFloat length;
	smSurfaceTreeCell* current;
	glColor3fv(smColor::colorGreen.toGLColor());

	glEnable(GL_LIGHTING);
	glPushAttrib(GL_LIGHTING_BIT);
	glColor3fv(smColor::colorGreen.toGLColor());
	static smInt counter=0;
	counter=0;
	glColor3fv(smColor::colorBlue.toGLColor());

	if(renderOnlySurface==false){
		for(smInt i=levelStartIndex[minTreeRenderLevel].startIndex;i<levelStartIndex[minTreeRenderLevel].endIndex;i++){

			current=&treeAllLevels[i];
			center=current->getCenter();
			length=current->getLength();

			if(current->filled){
				glPushMatrix();
				glColor3fv(smColor::colorPink.toGLColor());
				glTranslatef(center.x,center.y,center.z);
				glutSolidSphere(length,10,10);
				glPopMatrix();
				counter++;
			}
		}
	}

	glPopAttrib();
	glEnable(GL_LIGHTING);
	p_params.rendererObject->updateText("octree",QString("Total Spheres at Level:")+QString().setNum(counter));
}

/// \brief handle key press events
template<typename smSurfaceTreeCell> void smSurfaceTree<smSurfaceTreeCell>
                                                ::handleEvent(smEvent *p_event){

	smKeyboardEventData *keyBoardData;

	switch(p_event->eventType.eventTypeCode){

				case SIMMEDTK_EVENTTYPE_KEYBOARD:
					
					keyBoardData=(smKeyboardEventData*)p_event->data;
					if(keyBoardData->keyBoardKey==Qt::Key_Plus){
						minTreeRenderLevel++;
						
						if(minTreeRenderLevel>maxLevel)
							minTreeRenderLevel=maxLevel;

						if(minTreeRenderLevel<0)
							minTreeRenderLevel=0;

						currentLevel=minTreeRenderLevel;
					}

					if(keyBoardData->keyBoardKey==Qt::Key_Minus){
						minTreeRenderLevel--;
						if(minTreeRenderLevel>maxLevel)
							minTreeRenderLevel=maxLevel;
						if(minTreeRenderLevel<0)
							minTreeRenderLevel=0;
							currentLevel=minTreeRenderLevel;
					}

					if(keyBoardData->keyBoardKey==Qt::Key_R)
						this->renderSurface=!this->renderSurface;

					if(keyBoardData->keyBoardKey==Qt::Key_P)
						this->enableShiftPos=!this->enableShiftPos;

					if(keyBoardData->keyBoardKey==Qt::Key_K)
						this->renderOnlySurface=!this->renderOnlySurface;

					if(keyBoardData->keyBoardKey==Qt::Key_T)
						updateStructure();

					break;
	}
}

/// \brief create the surface tree
template<typename smSurfaceTreeCell> smBool smSurfaceTree<smSurfaceTreeCell>::
                  createTree(smSurfaceTreeCell &p_Node, vector<smInt> &p_triangles,
                  smInt p_level, smInt p_siblingIndex){

	smSurfaceTreeCell *subDividedNodes=new smSurfaceTreeCell[nbrDivision];
	vector<smInt>**triangles;

	if(p_level>=maxLevel)
		return false;
	p_Node.level=p_level;

	triangles=new vector<smInt>*[nbrDivision];

	if(p_level==maxLevel-1){

		smInt nbrTriangles=p_triangles.size();
		smFloat totalDistance=0.0;
		for(smInt i=0;i<nbrTriangles;i++){

			p_Node.verticesIndices.insert(mesh->triangles[p_triangles[i]].vert[0]);
			p_Node.verticesIndices.insert(mesh->triangles[p_triangles[i]].vert[1]);
			p_Node.verticesIndices.insert(mesh->triangles[p_triangles[i]].vert[2]);
		}

		for(set<smInt>::iterator it=p_Node.verticesIndices.begin();it!=p_Node.verticesIndices.end();it++){
			totalDistance+=p_Node.cube.center.distance(mesh->vertices[*it]);
		}

		smFloat weightSum=0;
		smFloat weight;
		for(set<smInt>::iterator it=p_Node.verticesIndices.begin();it!=p_Node.verticesIndices.end();it++){
			weight=1-(p_Node.getCenter().distance(mesh->vertices[*it])*p_Node.getCenter().distance(mesh->vertices[*it]))/(totalDistance*totalDistance);
			weight=1-(p_Node.getCenter().distance(mesh->vertices[*it])*p_Node.getCenter().distance(mesh->vertices[*it]))/(totalDistance*totalDistance);
			weightSum+=weight;
			p_Node.weights.push_back(weight);
		}

		smInt counter=0;
		for(set<smInt>::iterator it=p_Node.verticesIndices.begin();it!=p_Node.verticesIndices.end();it++){
			p_Node.weights[counter]=p_Node.weights[counter]/weightSum;
			counter++;
		}

		delete[] subDividedNodes;
		return true;
	}

	p_Node.subDivide(2.0,subDividedNodes);

	for(smInt i=0;i<nbrDivision;i++){
		triangles[i]=new vector<smInt>();
		//aabb[i].expand(0.2);
		subDividedNodes[i].expand(0.01);
	}

	smAABB tempAABB;

	for(smInt i=0;i<p_triangles.size();i++){
		for(smInt j=0;j<nbrDivision;j++){
			if(subDividedNodes[j].isCollidedWithTri(
                        mesh->vertices[mesh->triangles[p_triangles[i]].vert[0]],
                        mesh->vertices[mesh->triangles[p_triangles[i]].vert[1]],
                        mesh->vertices[mesh->triangles[p_triangles[i]].vert[2]])){
				triangles[j]->push_back(p_triangles[i]);
			}
		}
	}

	smInt index;
	smInt parentLevel=(p_level-1);
	if(p_level==0)
		parentLevel=0;

	smInt offset=nbrDivision*(p_siblingIndex-levelStartIndex[parentLevel].startIndex);
	if(p_level==0)
		offset=0;

	for(smInt j=0;j<nbrDivision;j++){

		index=levelStartIndex[p_level].endIndex+offset+j;
		if(triangles[j]->size()>0){
			treeAllLevels[index].copyShape(subDividedNodes[j]);	
			treeAllLevels[index].level=p_level+1;
			treeAllLevels[index].filled=true;
			createTree(treeAllLevels[index],*triangles[j],p_level+1,j+levelStartIndex[p_level].startIndex+offset);
		}
		else
			treeAllLevels[index].filled=false;

		triangles[j]->clear();
		delete triangles[j];

	}

	delete[] subDividedNodes;
	return true;
}

/// \brief !!
template <typename smSurfaceTreeCell> 
                smSurfaceTreeIterator<smSurfaceTreeCell> smSurfaceTree<smSurfaceTreeCell>
                ::get_LevelIterator(smInt p_level){

	smSurfaceTreeIterator<smSurfaceTreeCell> iter(this);
	iter.startIndex=iter.currentIndex=this->levelStartIndex[p_level].startIndex;
	iter.endIndex=this->levelStartIndex[p_level].endIndex;
	iter.currentLevel=p_level;
	return iter;
}

/// \brief !!
template <typename smSurfaceTreeCell>
smSurfaceTreeIterator<smSurfaceTreeCell>  smSurfaceTree<smSurfaceTreeCell>::get_LevelIterator(){

	smSurfaceTreeIterator<smSurfaceTreeCell> iter(this);
	iter.startIndex=iter.currentIndex=this->levelStartIndex[currentLevel].startIndex;
	iter.endIndex=this->levelStartIndex[currentLevel].endIndex;
	iter.currentLevel=currentLevel;
	return iter;
}

/// \brief update the surface tree
template <typename smSurfaceTreeCell>
                       void smSurfaceTree<smSurfaceTreeCell>::updateStructure(){

	smSurfaceTreeCell *current;
	for(smInt i=levelStartIndex[maxLevel-1].startIndex;i<levelStartIndex[maxLevel-1].endIndex;i++){

		current=&treeAllLevels[i];
		smInt nbrVertices=current->verticesIndices.size();
		smVec3f tempCenter(0,0,0);
		smInt counter=0;
		
		if(current->filled){
			for(set<smInt>::iterator it=current->verticesIndices.begin();
                                       it!=current->verticesIndices.end();it++){
				tempCenter=tempCenter+(mesh->vertices[*it]-mesh->origVerts[*it])*current->weights[counter];
				counter++;
			}
			current->cube.center=tempCenter;
		}
	}
}

/// \brief !!
template <typename smSurfaceTreeCell>
void smSurfaceTree<smSurfaceTreeCell>::translateRot(){

	smSurfaceTreeCell *current;
	smSurfaceTreeCell *initial;

	for(smInt i=levelStartIndex[maxLevel-1].startIndex;
                                    i<levelStartIndex[maxLevel-1].endIndex;i++){

		current=&treeAllLevels[i];
		initial=&initialTreeAllLevels[i];

		if(current->filled){
			current->cube.center=transRot*initial->cube.center;

		}
	}
}
