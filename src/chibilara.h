#ifndef H_CHIBILARA
#define H_CHIBILARA

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/external_types.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
	#include <libsm64/src/decomp/include/PR/ultratypes.h>
	#include <libsm64/src/decomp/include/audio_defines.h>
	#include <libsm64/src/decomp/include/seq_ids.h>
	#include <libsm64/src/decomp/include/mario_animation_ids.h>
	#include <libsm64/src/decomp/include/sm64shared.h>
}


#include "core.h"
#include "game.h"
#include "lara.h"
#include "objects.h"
#include "sprite.h"
#include "enemy.h"
#include "inventory.h"
#include "mesh.h"
#include "format.h"
#include "enemy.h"

#include "marioMacros.h"

#include <stdarg.h>

struct ChibiLara
{
    enum BodyParts
    {
        PELVIS,
        LEFT_UPPER_LEG,
        LEFT_LOWER_LEG,
        LEFT_FOOT,
        RIGHT_UPPER_LEG,
        RIGHT_LOWER_LEG,
        RIGHT_FOOT,
        TORSO,
        RIGHT_UPPER_ARM,
        RIGHT_LOWER_ARM,
        RIGHT_HAND,
        LEFT_UPPER_ARM,
        LEFT_LOWER_ARM,
        LEFT_HAND,
        HEAD
    };

    struct MarioRig
    {
        float* marioGeometry;
        bool inited;

        vec3 getVertex(int v)
        {
            v*=3;
            return vec3(marioGeometry[v], marioGeometry[v+1], marioGeometry[v+2]);
        }

        vec3 getVertexAverage(int count, ...)
        {
            va_list valist;
            vec3 result = vec3(0.0f);
            va_start(valist, count);
            for (int i = 0; i < count; i++) {
                vec3 v = getVertex(va_arg(valist, int));
                result+=v;
            }
            va_end(valist);
            return result/((float)count);
        }

        vec3 getVertexVector(int p1, int p2)
        {
            return getVertex(p1)-getVertex(p2);
        }

        struct MarioBone
        {
            vec3 position;
            vec3 lowJointPoint;
            vec3 leftJointPoint;
            vec3 rightJointPoint;
            vec3 angle;

            void initBone()
            {
                position=vec3(0.0f);
                lowJointPoint=vec3(0.0f);
                leftJointPoint=vec3(0.0f);
                rightJointPoint=vec3(0.0f);
                angle=vec3(0.0f);
            }

            void updateHeadBone(vec3 pos, vec3 vanglex, vec3 vangley)
            {
                position=pos;
                lowJointPoint=position;
                angle=vec3(-vanglex.angleX(), -vangley.angleY(), 0.0f);
            }

            void updateTorsoBone(vec3 pos, vec3 leftJoint, vec3 rightJoint, vec3 lowJoint, vec3 vanglex, vec3 vangley)
            {
                position=pos;
                leftJointPoint = leftJoint;
                rightJointPoint = rightJoint;
                lowJointPoint=lowJoint;
                angle=vec3(-vanglex.angleX(), -vangley.angleY(), 0.0f);
            }
            void updatePelvisBone(vec3 pos, vec3 leftJoint, vec3 rightJoint, vec3 vanglex, vec3 vangley)
            {
                position=pos;
                leftJointPoint = leftJoint;
                rightJointPoint = rightJoint;
                angle=vec3(vanglex.angleX(), -vangley.angleY(), 0.0f);
            }
        };

        struct MarioBone head, torso, pelvis, 
            leftUpperArm, leftLowerArm, leftHand,
            rigthUpperArm, rightLowerArm, rightHand,
            leftUpperLeg, leftLowerLeg, leftFoot,
            rightUpperLeg, rightLowerLeg, rightFoot;

        struct MarioBone *bones[15];
        int bonesCount;

        MarioRig(float *mgeometry)
        {
            marioGeometry = mgeometry;

            inited = false;

            int idx = 0;

            bones[PELVIS]=&pelvis;

            bones[LEFT_UPPER_LEG]=&leftUpperLeg;
            bones[LEFT_LOWER_LEG]=&leftLowerLeg;		
            bones[LEFT_FOOT]=&leftFoot;

            bones[RIGHT_UPPER_LEG]=&rightUpperLeg;
            bones[RIGHT_LOWER_LEG]=&rightLowerLeg;
            bones[RIGHT_FOOT]=&rightFoot;

            bones[TORSO]=&torso;
            
            bones[RIGHT_UPPER_ARM]=&rigthUpperArm;
            bones[RIGHT_LOWER_ARM]=&rightLowerArm;
            bones[RIGHT_HAND]=&rightHand;

            bones[LEFT_UPPER_ARM]=&leftUpperArm;
            bones[LEFT_LOWER_ARM]=&leftLowerArm;
            bones[LEFT_HAND]=&rightHand;

            bones[HEAD]=&head;

            bonesCount=15;

            for(int i=0; i<bonesCount; i++)
            {
                bones[i]->initBone();
            }
        }

        void updateRig()
        {
            inited = true;

            head.updateHeadBone(getVertexAverage(3, 702, 703, 704), getVertexVector(586, 585), getVertexVector(629, 637));
            torso.updateTorsoBone(head.lowJointPoint, getVertex(536), getVertex(519), getVertexAverage(3, 129, 130, 131), getVertexVector(343,342), getVertexVector(519,536));
            pelvis.updatePelvisBone(torso.lowJointPoint, getVertex(107), getVertex(108), getVertex(205)-getVertexAverage(2, 204, 206), getVertexVector(206,204));

            // leftUpperArm.updateBone(geometry, 1224, 1225, 1226);
            // leftLowerArm.updateBone(geometry, 1251, 1252, 1253);
            // leftHand.updateBone(geometry, 1407, 1408, 1409);

            // rigthUpperArm.updateBone(geometry, 1482, 1483, 1484);
            // rightLowerArm.updateBone(geometry, 1638, 1639, 1640);
            // rightHand.updateBone(geometry, 1701, 1702, 1703);

            // leftUpperLeg.updateBone(geometry, 2115, 2116, 2117);
            // leftLowerLeg.updateBone(geometry, 2175, 2176, 2177);
            // leftFoot.updateBone(geometry, 2217, 2218, 2219);

            // rightUpperLeg.updateBone(geometry, 1860, 1861, 1862);
            // rightLowerLeg.updateBone(geometry, 1905, 1906, 1907);
            // rightFoot.updateBone(geometry, 1986, 1987, 1988);
        }
    };

    //MeshBuilder *chibiMeshBuilder;
    MarioRig *marioRig;
    Controller *controller;
    TR::Level *level;
    TR::Entity *entity;

    TR::Mesh::Vertex **customVertices;
    TR::Mesh::Vertex **origVertices;
    int *jointsVerticesCount;
    int jointsCount;

    void prepareCustomVertices()
    {
        TR::Model *model = &(level->models[entity->modelIndex - 1]);
        jointsCount = model->mCount;

        customVertices = (TR::Mesh::Vertex **)malloc(sizeof(TR::Mesh::Vertex *)*jointsCount);
        origVertices = (TR::Mesh::Vertex **)malloc(sizeof(TR::Mesh::Vertex *)*jointsCount);
        jointsVerticesCount = (int*)malloc(sizeof(int)*jointsCount);
        for(int i=0; i<jointsCount; i++)
        {
            int index = level->meshOffsets[model->mStart + i];
            TR::Mesh *mesh = &(level->meshes[index]);
            customVertices[i]=(TR::Mesh::Vertex *)malloc(sizeof(TR::Mesh::Vertex)*mesh->vCount);
            jointsVerticesCount[i]=mesh->vCount;
            for(int j=0; j<mesh->vCount; j++)
            {
                customVertices[i][j] = mesh->vertices[j];
            }
            origVertices[i] = mesh->vertices;
        }
    }

    void switchVertices(bool custom)
    {
        TR::Model *model = &(level->models[entity->modelIndex - 1]);

        for(int i=0; i<jointsCount; i++)
        {
            int index = level->meshOffsets[model->mStart + i];
            TR::Mesh *mesh = &(level->meshes[index]);

            mesh->vertices = custom ? customVertices[i] : origVertices[i];
        }
    }

    void transformLaraBodyPart(BodyParts partIndex, vec3 displacement, vec3 scale)
    {
        for(int i=0; i<jointsVerticesCount[partIndex]; i++)
        {
            customVertices[partIndex][i].coord.x=(int16)(customVertices[partIndex][i].coord.x*scale.x);
            customVertices[partIndex][i].coord.x=(int16)(customVertices[partIndex][i].coord.x + displacement.x);

            customVertices[partIndex][i].coord.y=(int16)(customVertices[partIndex][i].coord.y*scale.y);
            customVertices[partIndex][i].coord.y=(int16)(customVertices[partIndex][i].coord.y + displacement.y);

            customVertices[partIndex][i].coord.z=(int16)(customVertices[partIndex][i].coord.z*scale.z);
            customVertices[partIndex][i].coord.z=(int16)(customVertices[partIndex][i].coord.z + displacement.z);
        }
    }

    void transformLaraBodyParts()
    {
        transformLaraBodyPart(HEAD, vec3(0.0f, -20.0f, -40.0f), vec3(2.0f));
        transformLaraBodyPart(TORSO, vec3(0.0f, 120.0f, 20.0f), vec3(0.9f, 0.7f, 0.9f));
        transformLaraBodyPart(PELVIS, vec3(0.0f, 50.0f, 20.0f), vec3(0.9f, 0.8f, 0.9f));
    }
    
    MeshBuilder *chibiMeshBuilder;

    ChibiLara(IGame *game, Controller *pcontroller, float *mgeometry)
    {
        controller = pcontroller;
        level = controller->level;
        entity = &(level->entities[controller->entity]);
        prepareCustomVertices();
        transformLaraBodyParts();
        switchVertices(true);
        chibiMeshBuilder = new MeshBuilder(level, game->getMesh()->atlas);
        chibiMeshBuilder->transparent=0;
        switchVertices(false);
        marioRig = new MarioRig(mgeometry);
    }

    ~ChibiLara()
    {
        delete chibiMeshBuilder;
        delete marioRig;
        free(jointsVerticesCount);
        for(int i; i<jointsCount; i++)
        {
            free(customVertices[i]);
        }
        free(customVertices);
        free(origVertices);
    }

    void updateRig()
    {
        marioRig->updateRig();
        return;
    }

    void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		if(!marioRig->inited) return;

        const TR::Model *model = controller->getModel();

        mat4 matrix = controller->getMatrix();

        Box box = controller->animation.getBoundingBox(vec3(0, 0, 0), 0);
        if (!controller->explodeMask && frustum && !frustum->isVisible(matrix, box.min, box.max))
            return;

        ASSERT(model);

        controller->flags.rendered = true;

        controller->updateJoints();

        Core::mModel = matrix;

        if (controller->layers) {
            uint32 mask = 0;

            for (int i = MAX_LAYERS - 1; i >= 0; i--) {
                uint32 vmask = (controller->layers[i].mask & ~mask) | (!i ? controller->explodeMask : 0);
                vmask &= controller->visibleMask;
                if (!vmask) continue;
                mask |= controller->layers[i].mask;
            // set meshes visibility
                for (int j = 0; j < model->mCount; j++)
                {
                    if(j==HEAD || j==TORSO || j==PELVIS)
                    {
                        controller->joints[j].w = (vmask & (1 << j)) ? 1.0f : 0.0f; // hide invisible parts
                        controller->joints[j].pos = marioRig->bones[j]->position;
                        controller->joints[j].rot = rotYXZ(marioRig->bones[j]->angle);
                    }
                    else
                    {
                        controller->joints[j].w = 0.0;
                    }
                    
                }

                if (controller->explodeMask) {
                    ASSERT(explodeParts);
                    const TR::Model *model = controller->getModel();
                    for (int i = 0; i < model->mCount; i++)
                        if (controller->explodeMask & (1 << i))
                            controller->joints[i] = controller->explodeParts[i].basis;
                }

            // render
                Core::setBasis(controller->joints, model->mCount);
                chibiMeshBuilder->renderModel(controller->layers[i].model, caustics);
            }
        } else {
            Core::setBasis(controller->joints, model->mCount);
            chibiMeshBuilder->renderModel(model->index, caustics);
        }

        return;
	}
};

#endif