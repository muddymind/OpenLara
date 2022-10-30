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
#include "mesh.h"
#include "chibilarabase.h"

#include "marioMacros.h"


#define CHIBI_LARA_MAX_JOINTS 30

struct ChibiLara : Mario
{
    struct ChibiBraid {
        ChibiLara *lara;
        vec3 offset;

        Basis *basis;
        struct Joint {
            vec3 posPrev, pos;
            float length;
        } *joints;
        int jointsCount;
        float time;

        ChibiBraid(ChibiLara *lara, const vec3 &offset) : lara(lara), offset(offset), time(0.0f) {
            TR::Level *level = lara->level;
            TR::Model *model = getModel();
            jointsCount = model->mCount + 1;
            joints      = new Joint[jointsCount];
            basis       = new Basis[jointsCount - 1];

            Basis basis = getBasis();
            basis.translate(offset);

            TR::Node *node = (int)model->node < level->nodesDataSize ? (TR::Node*)&level->nodesData[model->node] : NULL;
            for (int i = 0; i < jointsCount - 1; i++) {
                TR::Node &t = node[min(i, model->mCount - 2)];
                joints[i].posPrev = joints[i].pos = basis.pos;
                joints[i].length  = float(t.z);
                basis.translate(vec3(0.0f, 0.0f, -joints[i].length));
            }
            joints[jointsCount - 1].posPrev = joints[jointsCount - 1].pos = basis.pos;
            joints[jointsCount - 1].length  = 1.0f;
        }

        ~ChibiBraid() {
            delete[] joints;
            delete[] basis;
        }

        TR::Model* getModel() {
            return &lara->level->models[lara->level->extra.braid];
        }

        Basis getBasis() {
            return lara->chibilara->head.basis;
        }

        vec3 getPos() {
            return getBasis() * offset;
        }

        void integrate() {
            float TIMESTEP = Core::deltaTime;
            float ACCEL    = 16.0f * GRAVITY * 30.0f * TIMESTEP * TIMESTEP;
            float DAMPING  = 1.5f;

            if (lara->stand == STAND_UNDERWATER) {
                ACCEL *= 0.5f;
                DAMPING = 4.0f;
            }

            DAMPING = 1.0f / (1.0f + DAMPING * TIMESTEP); // Pade approximation

            for (int i = 1; i < jointsCount; i++) {
                Joint &j = joints[i];
                vec3 delta = j.pos - j.posPrev;
                delta = delta.normal() * (min(delta.length(), 2048.0f * Core::deltaTime) * DAMPING); // speed limit
                j.posPrev  = j.pos;
                j.pos     += delta;
                if ((lara->stand == STAND_WADE || lara->stand == STAND_ONWATER || lara->stand == STAND_UNDERWATER) && (j.pos.y > lara->waterLevel))
                    j.pos.y -= ACCEL;
                else
                    j.pos.y += ACCEL;
            }
        }

        void collide() {
            TR::Level *level = lara->level;
            const TR::Model *model = lara->getModel();

            TR::Level::FloorInfo info;
            lara->getFloorInfo(lara->getRoomIndex(), lara->getViewPoint(), info);

            for (int j = 1; j < jointsCount; j++)
                if (joints[j].pos.y > info.floor)
                    joints[j].pos.y = info.floor;

            #define BRAID_RADIUS 0.0f

            lara->updateJoints();

            for (int i = 0; i < model->mCount; i++) {
                if (!(JOINT_MASK_BRAID & (1 << i))) continue;

                vec3 center    = lara->chibilara->bones[i]->basis * lara->chibilara->ChibiLaraLayers[0].layerModelParts[i].center;
                float radiusSq = lara->chibilara->ChibiLaraLayers[0].layerModelParts[i].radius + BRAID_RADIUS;
                radiusSq *= radiusSq;

                for (int j = 1; j < jointsCount; j++) {
                    vec3 dir = joints[j].pos - center;
                    float len = dir.length2() + EPS;
                    if (len < radiusSq) {
                        len = sqrtf(len);
                        dir *= (lara->chibilara->ChibiLaraLayers[0].layerModelParts[i].radius + BRAID_RADIUS- len) / len;
                        joints[j].pos += dir * 0.9f;
                    }
                }
            }

            #undef BRAID_RADIUS
        }

        void solve() {
            for (int i = 0; i < jointsCount - 1; i++) {
                Joint &a = joints[i];
                Joint &b = joints[i + 1];

                vec3 dir = b.pos - a.pos;
                float len = dir.length() + EPS;
                dir *= 1.0f / len;

                float d = a.length - len;

                if (i > 0) {
                    dir *= d * (0.5f * 1.0f);
                    a.pos -= dir;
                    b.pos += dir;
                } else
                    b.pos += dir * (d * 1.0f);
            }
        }

        void update() {
            joints[0].pos = getPos();
            integrate(); // Verlet integration step
            collide();   // check collision with Lara's mesh
            for (int i = 0; i < jointsCount; i++) // solve connections (springs)
                solve();

            vec3 headDir = getBasis().rot * vec3(0.0f, 0.0f, -1.0f);

            for (int i = 0; i < jointsCount - 1; i++) {
                vec3 d = (joints[i + 1].pos - joints[i].pos).normal();
                vec3 r = d.cross(headDir).normal();
                vec3 u = d.cross(r).normal();

                mat4 m;
                m.up()     = vec4(u, 0.0f);
                m.dir()    = vec4(d, 0.0f);
                m.right()  = vec4(r, 0.0f);
                m.offset() = vec4(0.0f, 0.0f, 0.0f, 1.0f);

                basis[i].identity();
                basis[i].translate(joints[i].pos);
                basis[i].rotate(m.getRot());
            }
        }
        
        void render(MeshBuilder *mesh) {
            Core::setBasis(basis, jointsCount - 1);
            mesh->renderModel(lara->level->extra.braid);
        }

    } *chibibraid[2];

    int braidCount = 0;

    struct ChibiLaraBase *chibilara;

    ChibiLara(IGame *game, int entity) : Mario(game, entity)
    {
        isChibiLara = true;
        animationScale = MARIO_SCALE;
        chibilara = new ChibiLaraBase(game, level, this);

        if(COUNT(braid) > 0)
        {
            switch (level->version & TR::VER_VERSION) {
                case TR::VER_TR1 :
                    //braid[0] = new Braid(this, vec3(-4.0f, 24.0f, -48.0f)); // it's just ugly :)
                    break;
                case TR::VER_TR2 :
                case TR::VER_TR3 :
                    chibibraid[0] = new ChibiBraid(this, vec3(0.0f, -45.0f, -55.0f*1.7f));
                    braidCount=1;
                    break;
                case TR::VER_TR4 :
                    if (isYoung()) {
                        chibibraid[0] = new ChibiBraid(this, vec3(-32.0f, -48.0f, -32.0f));
                        chibibraid[1] = new ChibiBraid(this, vec3( 32.0f, -48.0f, -32.0f));
                        braidCount=2;
                    } else {
                        chibibraid[0] = new ChibiBraid(this, vec3(0.0f, -23.0f, -32.0f));
                        braidCount=1;
                    }
                    break;
            }
        }        
    }

    virtual ~ChibiLara()
    {
        delete chibilara;
        
        for (int i = 0; i < braidCount; i++) 
        {
            delete chibibraid[i];
        } 
    }

    virtual void marioIdChanged()
	{
		chibilara->chibiMarioId = marioId;
	}

    virtual void saveLastMarioGeometry(vec3 submergedDisplacement)
    {
        // we don't require to do anything on this step
    }

    virtual void setNewMarioGeometry(vec3 submergedDisplacement)
    {
        chibilara->updateRig(submergedDisplacement, 0, false);
    }

    virtual void updateMarioGeometry(vec3 submergedDisplacement)
    {
        chibilara->updateRig(submergedDisplacement, marioTicks/(1./30), true);

        if(chibilara->inited)
        {
            for (int i = 0; i < braidCount; i++) 
            {
                if (chibibraid[i]) 
                {
                    chibibraid[i]->update();
                }
            }
        }
    }

    void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		if(!chibilara->inited) return;

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
                joints[j].pos = chibilara->bones[j]->position;
                joints[j].rot = chibilara->bones[j]->rot;
            }

            // render
            Core::setBasis(joints, model->mCount);
            chibilara->chibiMeshBuilder->renderModel(layers[i].model, caustics);
        }

        for (int i = 0; i < braidCount; i++) 
        {
            if (chibibraid[i]) 
            {
                chibibraid[i]->render(chibilara->chibiMeshBuilder);
            }
        }

        return;
	}
};

#endif