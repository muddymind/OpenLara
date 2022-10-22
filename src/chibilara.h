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

#define CHIBI_LARA_MAX_JOINTS 30

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
        struct ChibiLara *rig;
        
        void initBone(struct ChibiLara *parentRig)
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

        void updateBone(ChibiLara::SM64BodyParts partId, vec3 submergedDisplacement, vec3 limbDisplacement, float ticks, bool doLerp)
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

    bool inited;

    vec3 leftArmDisplacement, rightArmDisplacement;
    vec3 leftLegDisplacement, rightLegDisplacement;

    struct MarioBone head, torso, pelvis, 
        leftUpperArm, leftLowerArm, leftHand,
        rigthUpperArm, rightLowerArm, rightHand,
        leftUpperLeg, leftLowerLeg, leftFoot,
        rightUpperLeg, rightLowerLeg, rightFoot;

    struct MarioBone *bones[15];
    int bonesCount;

    void initRig()
    {
        inited = false;

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

    struct LayerModelPart
    {
        TR::Mesh::Vertex *customVertices;
        TR::Mesh::Vertex *origVertices;
        TR::Mesh *mesh;
        int vCount;
    };

    struct LayerModel
    {
        struct LayerModelPart layerModelParts[CHIBI_LARA_MAX_JOINTS];
        int partsCount;
    };

    struct LayerModel ChibiLaraLayers[MAX_LAYERS];

    TR::Mesh::Vertex *origTransformedVertices[MAX_LAYERS*CHIBI_LARA_MAX_JOINTS];
    TR::Mesh::Vertex *customTransformedVertices[MAX_LAYERS*CHIBI_LARA_MAX_JOINTS];
    bool verticesAlreadyTransformed[MAX_LAYERS*CHIBI_LARA_MAX_JOINTS];
    
    int origTransformedVerticesCount=0;

    TR::Mesh::Vertex *getTransformedVertices(TR::Mesh::Vertex *origVertices)
    {
        for(int i=0; i<origTransformedVerticesCount; i++)
        {
            if(origVertices == origTransformedVertices[i])
            {
                return customTransformedVertices[i];
            }
        }
        return NULL;
    }

    int getOriginalVerticesIndex(TR::Mesh::Vertex *origVertices)
    {
        for(int i=0; i<origTransformedVerticesCount; i++)
        {
            if(origVertices == origTransformedVertices[i])
            {
                return i;
            }
        }
        printf("Chibi lara: Could not find the original vertices to transform! This is going to wrong now! Returning 0 to avoid segfault!\n");
        return 0;
    }

    void LoadLayerModelVertices(int layerNumber)
    {
        const TR::Model *model = &(level->models[layers[layerNumber].model]);
        ChibiLaraLayers[layerNumber].partsCount = model->mCount;

        for(int i=0; i<ChibiLaraLayers[layerNumber].partsCount; i++)
        {
            int index = level->meshOffsets[model->mStart + i];
            TR::Mesh *mesh = &(level->meshes[index]);
            ChibiLaraLayers[layerNumber].layerModelParts[i].mesh=mesh;

            TR::Mesh::Vertex *customVertices = getTransformedVertices(mesh->vertices);
            TR::Mesh::Vertex *originalVertices = mesh->vertices;

            // if customVertices == NULL then these are brand new vertices. Otherwise these were already used on another layer (parts can be reused between different layers)
            if(customVertices == NULL)
            {
                origTransformedVertices[origTransformedVerticesCount] = mesh->vertices;
                customTransformedVertices[origTransformedVerticesCount] = (TR::Mesh::Vertex *)malloc(sizeof(TR::Mesh::Vertex)*mesh->vCount);
                for(int j=0; j<mesh->vCount; j++)
                {
                    customTransformedVertices[origTransformedVerticesCount][j] = mesh->vertices[j];
                }
                verticesAlreadyTransformed[origTransformedVerticesCount] = false;
                customVertices = customTransformedVertices[origTransformedVerticesCount];
                origTransformedVerticesCount++;
            }
            
            ChibiLaraLayers[layerNumber].layerModelParts[i].customVertices = customVertices;
            ChibiLaraLayers[layerNumber].layerModelParts[i].origVertices = originalVertices;
            ChibiLaraLayers[layerNumber].layerModelParts[i].vCount = mesh->vCount;
        }
    }

    void transformAndRotateLaraLayerBodyPart(int layer, BodyParts partIndex, vec3 displacement, vec3 scale, vec3 rotation)
    {
        struct LayerModelPart *part = &(ChibiLaraLayers[layer].layerModelParts[partIndex]);
        int originalVertexIdx = getOriginalVerticesIndex(part->origVertices);

        if(!verticesAlreadyTransformed[originalVertexIdx])
        {
            mat4 m;
            m.identity();
            if(rotation.x!=0)
            {
                m.rotateX(rotation.x);
            }
            for(int i=0; i<part->vCount; i++)
            {
                vec3 vertex = vec3(part->customVertices[i].coord.x, part->customVertices[i].coord.y, part->customVertices[i].coord.z);
                
                vertex = m*vertex;

                part->customVertices[i].coord.x = (int16)vertex.x;
                part->customVertices[i].coord.y = (int16)vertex.y;
                part->customVertices[i].coord.z = (int16)vertex.z;

                vertex = vec3(part->customVertices[i].normal.x, part->customVertices[i].normal.y, part->customVertices[i].normal.z);
                
                vertex = m*vertex;

                part->customVertices[i].normal.x = (int16)vertex.x;
                part->customVertices[i].normal.y = (int16)vertex.y;
                part->customVertices[i].normal.z = (int16)vertex.z;
            }

            mat4 T = mat4(
                1.0f,  0.0f,  0.0f,  0.0f,
                0.0f, -1.0f,  0.0f,  0.0f,
                0.0f,  0.0f, -1.0f,  0.0f,
                0.0f,  0.0f,  0.0f,  1.0f
            );

            for(int i=0; i<part->vCount; i++)
            {
                vec3 vertex = vec3(part->customVertices[i].coord.x, part->customVertices[i].coord.y, part->customVertices[i].coord.z);
                
                vertex = T*vertex;

                part->customVertices[i].coord.x = (int16)vertex.x;
                part->customVertices[i].coord.y = (int16)vertex.y;
                part->customVertices[i].coord.z = (int16)vertex.z;

                vertex = vec3(part->customVertices[i].normal.x, part->customVertices[i].normal.y, part->customVertices[i].normal.z);
                
                vertex = T*vertex;

                part->customVertices[i].normal.x = (int16)vertex.x;
                part->customVertices[i].normal.y = (int16)vertex.y;
                part->customVertices[i].normal.z = (int16)vertex.z;
            }
            

            for(int i=0; i<part->vCount; i++)
            {
                part->customVertices[i].coord.x=(int16)(part->customVertices[i].coord.x*scale.x);
                part->customVertices[i].coord.x=(int16)(part->customVertices[i].coord.x + displacement.x);

                part->customVertices[i].coord.y=(int16)(part->customVertices[i].coord.y*scale.y);
                part->customVertices[i].coord.y=(int16)(part->customVertices[i].coord.y + displacement.y);

                part->customVertices[i].coord.z=(int16)(part->customVertices[i].coord.z*scale.z);
                part->customVertices[i].coord.z=(int16)(part->customVertices[i].coord.z + displacement.z);
            }
            verticesAlreadyTransformed[originalVertexIdx] = true;
            return;
        }
    }

    void transformLaraLayerBodyParts(int layer)
    {
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::HEAD, vec3(0.0f, -30.0f, -20.0f), vec3(1.4f), vec3(M_PI, 0.0f, 0.0f));
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::TORSO, vec3(0.0f, 20.0f, 10.0f), vec3(0.75f, 0.7f, 0.75f), vec3(M_PI, 0.0f, 0.0f));
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::PELVIS, vec3(0.0f, -20.0f, 0.0f), vec3(0.75f, 0.6f, 0.75f), vec3(M_PI, 0.0f, 0.0f));

        vec3 upperArmDisplacement = vec3(0.0f, 20.0f, 0.0f);
        vec3 upperArmScaling = vec3(0.8f, 0.8f, 0.8f);
        vec3 upperArmRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 lowerArmDisplacement = vec3(-10.0f, 0.0f, 0.0f);
        vec3 lowerArmScaling = vec3(0.8f, 0.65f, 0.8f);
        vec3 lowerArmRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 handDisplacement = vec3(-10.0f, 0.0f, 0.0f);
        vec3 handScaling = vec3(1.1f, 1.1f, 1.1f);
        vec3 handRotation = vec3(0.0f, 0.0f, 0.0f);

        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_UPPER_ARM, upperArmDisplacement, upperArmScaling, upperArmRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_LOWER_ARM, lowerArmDisplacement, lowerArmScaling, lowerArmRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_HAND, handDisplacement, handScaling, handRotation);

        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_UPPER_ARM, upperArmDisplacement.mirrorX(), upperArmScaling, upperArmRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_LOWER_ARM, lowerArmDisplacement.mirrorX(), lowerArmScaling, lowerArmRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_HAND, handDisplacement.mirrorX(), handScaling, handRotation);

        vec3 upperLegDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 upperLegScaling = vec3(0.7f,0.45f,0.7f);
        vec3 upperLegRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 lowerLegDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 lowerLegScaling = vec3(0.7f, 0.4f, 0.7f);
        vec3 lowerLegRotation = vec3(0.0f, 0.0f, 0.0f);

        vec3 footDisplacement = vec3(0.0f, 0.0f, 0.0f);
        vec3 footScaling = vec3(0.7f, 0.7f, 0.7f);
        vec3 footRotation = vec3(-M_PI/2.5, 0.0f, 0.0f);

        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_UPPER_LEG, upperLegDisplacement, upperLegScaling, upperLegRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_LOWER_LEG, lowerLegDisplacement, lowerLegScaling, lowerLegRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::LEFT_FOOT, footDisplacement, footScaling, footRotation);

        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_UPPER_LEG, upperLegDisplacement.mirrorX(), upperLegScaling, upperLegRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_LOWER_LEG, lowerLegDisplacement.mirrorX(), lowerLegScaling, lowerLegRotation);
        transformAndRotateLaraLayerBodyPart(layer, BodyParts::RIGHT_FOOT, footDisplacement.mirrorX(), footScaling, footRotation);
    }

    void LoadVerticesFromAllLayersAndTransform()
    {
        for(int i=0; i<MAX_LAYERS; i++)
        {
            LoadLayerModelVertices(i);
            transformLaraLayerBodyParts(i);
        }
    }

    void switchVertices(bool custom)
    {
        for(int i=0; i<MAX_LAYERS; i++)
        {
            struct LayerModel *laraLayer = &(ChibiLaraLayers[i]);
            for(int j=0; j<laraLayer->partsCount; j++)
            {
                struct LayerModelPart *laraPart = &(laraLayer->layerModelParts[j]);
                laraPart->mesh->vertices = custom ? laraPart->customVertices : laraPart->origVertices;
            }
        }
    }
    
    MeshBuilder *chibiMeshBuilder;

    ChibiLara(IGame *game, int entity) : Mario(game, entity)
    {
        isChibiLara = true;
        animationScale = MARIO_SCALE;
        LoadVerticesFromAllLayersAndTransform();
        switchVertices(true);
        chibiMeshBuilder = new MeshBuilder(level, game->getMesh()->atlas);
        chibiMeshBuilder->transparent=0;
        switchVertices(false);
        initRig();
    }

    virtual ~ChibiLara()
    {
        delete chibiMeshBuilder;
        for(int i; i<origTransformedVerticesCount; i++)
        {
            free(customTransformedVertices[i]);
        }    
    }

    virtual void saveLastMarioGeometry(vec3 submergedDisplacement)
    {
        // we don't require to do anything on this step
    }

    virtual void setNewMarioGeometry(vec3 submergedDisplacement)
    {
        updateRig(marioId, submergedDisplacement, 0, false);
    }

    virtual void updateMarioGeometry(vec3 submergedDisplacement)
    {
        updateRig(marioId, submergedDisplacement, marioTicks/(1./30), true);
    }

    void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		if(!inited) return;

        const TR::Model *model = getModel();

        mat4 matrix = getMatrix();

        flags.rendered = true;

        Core::mModel = matrix;

        updateJoints();

        uint32 mask = 0;

        for (int i = MAX_LAYERS - 1; i >= 0; i--) {
            uint32 vmask = (layers[i].mask & ~mask) | (!i ? explodeMask : 0);
            vmask &= visibleMask;
            if (!vmask) {
                //printf("Layer %d invisible!\n", i);
                continue;
            }
            mask |= layers[i].mask;

            // set meshes visibility
            for (int j = 0; j < model->mCount; j++)
            {
                joints[j].w = (vmask & (1 << j)) ? 1.0f : 0.0f; // hide invisible parts
                joints[j].pos = bones[j]->position;
                joints[j].rot = bones[j]->rot;
            }

            // render
            Core::setBasis(joints, model->mCount);
            chibiMeshBuilder->renderModel(layers[i].model, caustics);
        }

        return;
	}
};

#endif