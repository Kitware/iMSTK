#include <bandit/bandit.h>

#include <memory>

#include "smCore/smErrorLog.h"
#include "smCollision/smOctreeCell.h"

using namespace bandit;

go_bandit([](){
    describe("octree data structure", []() {
        smOctreeCell treeCell;

        const int numberOfSubdivisions = smOctreeCell::numberOfSubdivisions;

        smVec3<float> center(1.,1.,1.);
        treeCell.setCenter(center);

        float lenght = 14.5;
        treeCell.setLength(lenght);

        it("initializes properly", [&]() {
          AssertThat(numberOfSubdivisions, Equals(8));
          AssertThat(treeCell.filled, IsFalse());
          AssertThat(treeCell.level, Equals(0));
        });

        it("sets the center properly", [&]() {
          AssertThat(treeCell.getCenter()[0], Equals(1.));
          AssertThat(treeCell.getCenter()[1], Equals(1.));
          AssertThat(treeCell.getCenter()[2], Equals(1.));
        });

        it("sets the lenght properly", [&]() {
          AssertThat(treeCell.getLength(), Equals(14.5));
        });
    });
});


