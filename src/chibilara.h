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

struct ChibiLara : Mario
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

    enum SM64BodyParts
    {
        MARIO_PELVIS = 1,
        MARIO_TORSO = 2,
        MARIO_HEAD = 3,

        MARIO_LEFT_MAIN_ARM_PARENT = 4,
        MARIO_LEFT_UPPER_ARM = 5,
        MARIO_LEFT_LOWER_ARM = 6,
        MARIO_LEFT_HAND = 7,

        MARIO_RIGHT_MAIN_ARM_PARENT = 8,
        MARIO_RIGHT_UPPER_ARM = 9,
        MARIO_RIGHT_LOWER_ARM = 10,
        MARIO_RIGHT_HAND = 11,


        MARIO_LEFT_MAIN_LEG_PARENT = 12,
        MARIO_LEFT_UPPER_LEG = 13,
        MARIO_LEFT_LOWER_LEG = 14,
        MARIO_LEFT_FOOT = 15,

        MARIO_RIGHT_MAIN_LEG_PARENT = 16,
        MARIO_RIGHT_UPPER_LEG = 17,
        MARIO_RIGHT_LOWER_LEG = 18,
        MARIO_RIGHT_FOOT = 19
    };

    struct MarioRig
    {
        bool inited;

        vec3 leftArmDisplacement, rightArmDisplacement;
        vec3 leftLegDisplacement, rightLegDisplacement;

        int marioId;

        struct MarioBone
        {
            bool active;
            mat4 mat;
            vec3 position;
            quat rot;
            vec3 prevPosition;
            quat prevRot;
            vec3 nextPosition;
            quat nextRot;
            struct MarioRig *rig;
           
            void initBone(struct MarioRig *parentRig)
            {
                position = vec3(0.0f);
                rot = quat(0.0f,0.0f,0.0f,0.0f);
                prevPosition = vec3(0.0f);
                prevRot = quat(0.0f,0.0f,0.0f,0.0f);
                nextPosition = vec3(0.0f);
                nextRot = quat(0.0f,0.0f,0.0f,0.0f);
                mat.identity();
                active=false;
                rig=parentRig;
            }

            void updateBone(SM64BodyParts partId, vec3 submergedDisplacement, vec3 limbDisplacement, float ticks, bool doLerp)
            {
                if(active)
                {
                    mat = rig->getPartMatrix(rig->marioId, partId);

                    if(!doLerp)
                    {
                        prevPosition = vec3(nextPosition);
                        prevRot = quat(nextRot);

                        nextPosition=mat.getPos()-limbDisplacement+submergedDisplacement;
                        nextRot=mat.getRot();
                    }
                    else
                    {
                        position = prevPosition.lerp(nextPosition, ticks);
                        rot = prevRot.lerp(nextRot, ticks);
                    }
                }
                //We skip the 1st frame because there's no valid data yet.
                active=true;
            }
        };

        struct MarioBone head, torso, pelvis, 
            leftUpperArm, leftLowerArm, leftHand,
            rigthUpperArm, rightLowerArm, rightHand,
            leftUpperLeg, leftLowerLeg, leftFoot,
            rightUpperLeg, rightLowerLeg, rightFoot;

        struct MarioBone *bones[15];
        int bonesCount;

        MarioRig()
        {
            inited = false;
            marioId=-1;

            leftArmDisplacement=vec3(0.0f);
            rightArmDisplacement=vec3(0.0f);
            leftLegDisplacement=vec3(0.0f);
            rightLegDisplacement=vec3(0.0f);

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
            bones[LEFT_HAND]=&leftHand;

            bones[HEAD]=&head;

            bonesCount=15;

            for(int i=0; i<bonesCount; i++)
            {
                bones[i]->initBone(this);
            }
        }

        mat4 getPartMatrix(int marioId, SM64BodyParts partId)
        {
            mat4 mat;

            float** originalMatrix = sm64_get_mario_part_animation_matrix(marioId, partId);

            if(originalMatrix==NULL)
            {
                mat.identity();
                return mat;
            }

            mat = mat4(
                originalMatrix[0][0], originalMatrix[0][1], originalMatrix[0][2], originalMatrix[0][3], 
                originalMatrix[1][0], originalMatrix[1][1], originalMatrix[1][2], originalMatrix[1][3], 
                originalMatrix[2][0], originalMatrix[2][1], originalMatrix[2][2], originalMatrix[2][3],
                originalMatrix[3][0], originalMatrix[3][1], originalMatrix[3][2], originalMatrix[3][3]
            );

            mat.rotateX(M_PI/2);
            mat.rotateZ(-M_PI/2);

            mat4 T = mat4(
                1.0f,  0.0f,  0.0f,  0.0f,
                0.0f, -1.0f,  0.0f,  0.0f,
                0.0f,  0.0f, -1.0f,  0.0f,
                0.0f,  0.0f,  0.0f,  1.0f
            );

            mat = T.inverse()*mat*T;

            return mat;
        }

        void calculateLimpsDisplacements(SM64BodyParts leftPart, SM64BodyParts rightPart, vec3 *lefDisplacement, vec3 *rightDisplacement, float displacement)
        {
            vec3 leftPosition = getPartMatrix(marioId, leftPart).getPos();
            vec3 rightPosition = getPartMatrix(marioId, rightPart).getPos();

            *lefDisplacement = leftPosition-leftPosition.lerp(rightPosition, displacement);
            *rightDisplacement = rightPosition-rightPosition.lerp(leftPosition, displacement);
        }

        void updateRig(int marioId, vec3 submergedDisplacement, float ticks, bool doLerp)
        {
            inited = true;
            this->marioId = marioId;

            calculateLimpsDisplacements(SM64BodyParts::MARIO_LEFT_UPPER_ARM, SM64BodyParts::MARIO_RIGHT_UPPER_ARM, &leftArmDisplacement, &rightArmDisplacement, 0.15f);
            calculateLimpsDisplacements(SM64BodyParts::MARIO_LEFT_UPPER_LEG, SM64BodyParts::MARIO_RIGHT_UPPER_LEG, &leftLegDisplacement, &rightLegDisplacement, 0.15f);

            head.updateBone(SM64BodyParts::MARIO_HEAD, submergedDisplacement, vec3(0), ticks, doLerp);            
            torso.updateBone(SM64BodyParts::MARIO_TORSO, submergedDisplacement, vec3(0), ticks, doLerp);
            pelvis.updateBone(SM64BodyParts::MARIO_PELVIS, submergedDisplacement, vec3(0), ticks, doLerp);

            leftUpperArm.updateBone(SM64BodyParts::MARIO_LEFT_UPPER_ARM, submergedDisplacement, leftArmDisplacement, ticks, doLerp);
            leftLowerArm.updateBone(SM64BodyParts::MARIO_LEFT_LOWER_ARM, submergedDisplacement,leftArmDisplacement, ticks, doLerp);
            leftHand.updateBone(SM64BodyParts::MARIO_LEFT_HAND, submergedDisplacement, leftArmDisplacement, ticks, doLerp);

            rigthUpperArm.updateBone(SM64BodyParts::MARIO_RIGHT_UPPER_ARM, submergedDisplacement, rightArmDisplacement, ticks, doLerp);
            rightLowerArm.updateBone(SM64BodyParts::MARIO_RIGHT_LOWER_ARM, submergedDisplacement, rightArmDisplacement, ticks, doLerp);
            rightHand.updateBone(SM64BodyParts::MARIO_RIGHT_HAND, submergedDisplacement, rightArmDisplacement, ticks, doLerp);

            leftUpperLeg.updateBone(SM64BodyParts::MARIO_LEFT_UPPER_LEG, submergedDisplacement, leftLegDisplacement, ticks, doLerp);
            leftLowerLeg.updateBone(SM64BodyParts::MARIO_LEFT_LOWER_LEG, submergedDisplacement, leftLegDisplacement, ticks, doLerp);
            leftFoot.updateBone(SM64BodyParts::MARIO_LEFT_FOOT, submergedDisplacement, leftLegDisplacement, ticks, doLerp);

            rightUpperLeg.updateBone(SM64BodyParts::MARIO_RIGHT_UPPER_LEG, submergedDisplacement, rightLegDisplacement, ticks, doLerp);
            rightLowerLeg.updateBone(SM64BodyParts::MARIO_RIGHT_LOWER_LEG, submergedDisplacement, rightLegDisplacement, ticks, doLerp);
            rightFoot.updateBone(SM64BodyParts::MARIO_RIGHT_FOOT, submergedDisplacement, rightLegDisplacement, ticks, doLerp);

        }
    };

    MarioRig *marioRig;

    TR::Mesh::Vertex **customVertices;
    TR::Mesh::Vertex **origVertices;
    int *jointsVerticesCount;

    TR::Face **jointFaces;
    int *jointFacesCount;

    int jointsCount;

    void prepareCustomVertices()
    {
        //TR::Model *model = &(level->models[entity->modelIndex - 1]);
        const TR::Model *model = getModel(); //&(level->models[controller->layers[0].model-1]);
        jointsCount = model->mCount;

        customVertices = (TR::Mesh::Vertex **)malloc(sizeof(TR::Mesh::Vertex *)*jointsCount);
        origVertices = (TR::Mesh::Vertex **)malloc(sizeof(TR::Mesh::Vertex *)*jointsCount);
        jointFaces = (TR::Face **)malloc(sizeof(TR::Face *)*jointsCount);
        jointsVerticesCount = (int*)malloc(sizeof(int)*jointsCount);
        jointFacesCount = (int*)malloc(sizeof(int)*jointsCount);
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
            jointFaces[i]=mesh->faces;
            jointFacesCount[i]=mesh->fCount;
        }
    }

    void switchVertices(bool custom)
    {
        //TR::Model *model = &(level->models[entity->modelIndex - 1]);
        //TR::Model *model = &(level->models[controller->layers[0].model-1]);
        const TR::Model *model = getModel();

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

    void rotateLaraBodyPart(BodyParts partIndex, vec3 rotation)
    {
        mat4 m;
        m.identity();
        if(rotation.x!=0)
        {
            m.rotateX(rotation.x);
        }

        for(int i=0; i<jointsVerticesCount[partIndex]; i++)
        {
            vec3 vertex = vec3(customVertices[partIndex][i].coord.x, customVertices[partIndex][i].coord.y, customVertices[partIndex][i].coord.z);
            
            vertex = m*vertex;

            customVertices[partIndex][i].coord.x = (int16)vertex.x;
            customVertices[partIndex][i].coord.y = (int16)vertex.y;
            customVertices[partIndex][i].coord.z = (int16)vertex.z;

            vertex = vec3(customVertices[partIndex][i].normal.x, customVertices[partIndex][i].normal.y, customVertices[partIndex][i].normal.z);
            
            vertex = m*vertex;

            customVertices[partIndex][i].normal.x = (int16)vertex.x;
            customVertices[partIndex][i].normal.y = (int16)vertex.y;
            customVertices[partIndex][i].normal.z = (int16)vertex.z;
        }

        mat4 T = mat4(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );

        for(int i=0; i<jointsVerticesCount[partIndex]; i++)
        {
            vec3 vertex = vec3(customVertices[partIndex][i].coord.x, customVertices[partIndex][i].coord.y, customVertices[partIndex][i].coord.z);
            
            vertex = T*vertex;

            customVertices[partIndex][i].coord.x = (int16)vertex.x;
            customVertices[partIndex][i].coord.y = (int16)vertex.y;
            customVertices[partIndex][i].coord.z = (int16)vertex.z;

            vertex = vec3(customVertices[partIndex][i].normal.x, customVertices[partIndex][i].normal.y, customVertices[partIndex][i].normal.z);
            
            vertex = T*vertex;

            customVertices[partIndex][i].normal.x = (int16)vertex.x;
            customVertices[partIndex][i].normal.y = (int16)vertex.y;
            customVertices[partIndex][i].normal.z = (int16)vertex.z;
        }
    }

    void transformLaraBodyParts()
    {
        transformLaraBodyPart(BodyParts::HEAD, vec3(0.0f, -30.0f, -20.0f), vec3(1.4f));
        transformLaraBodyPart(BodyParts::TORSO, vec3(0.0f, 20.0f, 10.0f), vec3(0.75f, 0.7f, 0.75f));
        transformLaraBodyPart(BodyParts::PELVIS, vec3(0.0f, -20.0f, 0.0f), vec3(0.75f, 0.6f, 0.75f));

        vec3 upperArmDisplacement = vec3(0.0f, 20.0f, 0.0f);
        vec3 upperArmScaling = vec3(0.8f, 0.8f, 0.8f);
        vec3 upperArmRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 lowerArmDisplacement = vec3(-10.0f, 0.0f, 0.0f);
        vec3 lowerArmScaling = vec3(0.8f, 0.65f, 0.8f);
        vec3 lowerArmRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 handDisplacement = vec3(-10.0f, 0.0f, 0.0f);
        vec3 handScaling = vec3(1.1f, 1.1f, 1.1f);
        vec3 handRotation = vec3(0.0f, 0.0f, 0.0f);

        rotateLaraBodyPart(BodyParts::LEFT_UPPER_ARM, upperArmRotation);
        transformLaraBodyPart(BodyParts::LEFT_UPPER_ARM, upperArmDisplacement, upperArmScaling);
        rotateLaraBodyPart(BodyParts::LEFT_LOWER_ARM, lowerArmRotation);
        transformLaraBodyPart(BodyParts::LEFT_LOWER_ARM, lowerArmDisplacement, lowerArmScaling);
        rotateLaraBodyPart(BodyParts::LEFT_HAND, handRotation);
        transformLaraBodyPart(BodyParts::LEFT_HAND, handDisplacement, handScaling);

        rotateLaraBodyPart(BodyParts::RIGHT_UPPER_ARM, upperArmRotation);
        transformLaraBodyPart(BodyParts::RIGHT_UPPER_ARM, upperArmDisplacement.mirrorX(), upperArmScaling);
        rotateLaraBodyPart(BodyParts::RIGHT_LOWER_ARM, lowerArmRotation);
        transformLaraBodyPart(BodyParts::RIGHT_LOWER_ARM, lowerArmDisplacement.mirrorX(), lowerArmScaling);
        rotateLaraBodyPart(BodyParts::RIGHT_HAND, handRotation);
        transformLaraBodyPart(BodyParts::RIGHT_HAND, handDisplacement.mirrorX(), handScaling);

        vec3 upperLegDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 upperLegScaling = vec3(0.7f,0.45f,0.7f);
        vec3 upperLegRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 lowerLegDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 lowerLegScaling = vec3(0.7f, 0.4f, 0.7f);
        vec3 lowerLegRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 footDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 footScaling = vec3(0.7f, 0.7f, 0.7f);
        vec3 footRotation = vec3(-M_PI/2.5, 0.0f, 0.0f);

        rotateLaraBodyPart(BodyParts::LEFT_UPPER_LEG, upperLegRotation);
        transformLaraBodyPart(BodyParts::LEFT_UPPER_LEG, upperLegDisplacement, upperLegScaling);
        rotateLaraBodyPart(BodyParts::LEFT_LOWER_LEG, lowerLegRotation);
        transformLaraBodyPart(BodyParts::LEFT_LOWER_LEG, lowerLegDisplacement, lowerLegScaling);
        rotateLaraBodyPart(BodyParts::LEFT_FOOT, footRotation);
        transformLaraBodyPart(BodyParts::LEFT_FOOT, footDisplacement, footScaling);

        rotateLaraBodyPart(BodyParts::RIGHT_UPPER_LEG, upperLegRotation);
        transformLaraBodyPart(BodyParts::RIGHT_UPPER_LEG, upperLegDisplacement.mirrorX(), upperLegScaling);
        rotateLaraBodyPart(BodyParts::RIGHT_LOWER_LEG, lowerLegRotation);
        transformLaraBodyPart(BodyParts::RIGHT_LOWER_LEG, lowerLegDisplacement.mirrorX(), lowerLegScaling);
        rotateLaraBodyPart(BodyParts::RIGHT_FOOT, footRotation);
        transformLaraBodyPart(BodyParts::RIGHT_FOOT, footDisplacement.mirrorX(), footScaling);
    }
    
    MeshBuilder *chibiMeshBuilder;

    ChibiLara(IGame *game, int entity) : Mario(game, entity)
    {
        isChibiLara = true;
        animationScale = MARIO_SCALE;
        prepareCustomVertices();
        transformLaraBodyParts();
        switchVertices(true);
        chibiMeshBuilder = new MeshBuilder(level, game->getMesh()->atlas);
        chibiMeshBuilder->transparent=0;
        switchVertices(false);
        marioRig = new MarioRig();
    }

    virtual ~ChibiLara()
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
        free(jointFaces);
        free(jointFacesCount);        
    }

    virtual void saveLastMarioGeometry(vec3 submergedDisplacement)
    {
        // we don't require to do anything on this step
    }

    virtual void setNewMarioGeometry(vec3 submergedDisplacement)
    {
        marioRig->updateRig(marioId, submergedDisplacement, 0, false);
    }

    virtual void updateMarioGeometry(vec3 submergedDisplacement)
    {
        marioRig->updateRig(marioId, submergedDisplacement, marioTicks/(1./30), true);
    }

    void getDebugVertices(BodyParts part, vec3 vertices[1024])
    {
        for(int i=0; i<jointsVerticesCount[part]; i++)
        {
            vertices[i]=vec3(customVertices[part][i].coord.x+marioRig->bones[part]->position.x, customVertices[part][i].coord.y+marioRig->bones[part]->position.y, customVertices[part][i].coord.z+marioRig->bones[part]->position.z);
        }
    }
    TR::Face *getDebugFaces(BodyParts part, int *count)
    {
        *count = jointFacesCount[part];
        return jointFaces[part];
    }

    void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		if(!marioRig->inited) return;

        const TR::Model *model = getModel();

        mat4 matrix = getMatrix();

        Box box = animation.getBoundingBox(vec3(0, 0, 0), 0);
        if (!explodeMask && frustum && !frustum->isVisible(matrix, box.min, box.max))
            return;

        ASSERT(model);

        flags.rendered = true;

        Core::mModel = matrix;

        // set meshes visibility
        for (int j = 0; j < model->mCount; j++)
        {
            if(  marioRig->bones[j]->active )
            {
                joints[j].w = 1.0 ? 1.0f : 0.0f; // hide invisible parts
                joints[j].pos = marioRig->bones[j]->position;
                joints[j].rot = marioRig->bones[j]->rot;
            }
            else
            {
                joints[j].w = 0.0;
            }
            
        }

        // render
        Core::setBasis(joints, model->mCount);
        chibiMeshBuilder->renderModel(layers[0].model, caustics);

        return;
	}
};

#endif