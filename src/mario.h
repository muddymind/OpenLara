#ifndef H_MARIO
#define H_MARIO

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
}

#include "core.h"
#include "game.h"
#include "lara.h"
#include "objects.h"
#include "sprite.h"
#include "enemy.h"
#include "inventory.h"
#include "mesh.h"

struct MarioMesh
{
	size_t num_vertices;
	uint16_t *index;
};

struct MarioRenderState
{
	MarioMesh mario;
};

struct Mario : Lara
{
	uint32_t marioId;
	struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;
	struct MarioRenderState marioRenderState;

	float marioTicks;

	Mesh* TRmarioMesh;

	Mario(IGame *game, int entity) : Lara(game, entity)
	{
		isMario = true;
		marioId = -1;
		marioTicks = 0;

		marioGeometry.position = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.color    = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.normal   = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.uv       = (float*)malloc( sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES );

		marioRenderState.mario.index = (uint16_t*)malloc( 3 * SM64_GEO_MAX_TRIANGLES * sizeof(uint16_t) );
		for( int i = 0; i < 3 * SM64_GEO_MAX_TRIANGLES; ++i )
			marioRenderState.mario.index[i] = i;

		marioRenderState.mario.num_vertices = 3 * SM64_GEO_MAX_TRIANGLES;

		TRmarioMesh = new Mesh((Index*)marioRenderState.mario.index, marioRenderState.mario.num_vertices, NULL, 0, 1, true, true);
		TRmarioMesh->initMario(&marioGeometry);

		// load sm64surfaces
		FILE* file = fopen("level.c", "w");
		fprintf(file, "#include \"level.h\"\n#include \"../src/decomp/include/surface_terrains.h\"\nconst struct SM64Surface surfaces[] = {\n");
		size_t surfaces_count = 0;
		size_t surface_ind = 0;
		static bool printed = false;

		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room &room = level->rooms[i];
			TR::Room::Data &d = room.data;

			for (int j = 0; j < d.fCount; j++)
			{
				TR::Face &f = d.faces[j];
				surfaces_count += (f.triangle) ? 1 : 2;
					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_SNOW,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[2]].pos.x)/1, d.vertices[f.vertices[2]].pos.y/1, (room.info.z + d.vertices[f.vertices[2]].pos.z)/1, (room.info.x + d.vertices[f.vertices[1]].pos.x)/1, d.vertices[f.vertices[1]].pos.y/1, (room.info.z + d.vertices[f.vertices[1]].pos.z)/1, (room.info.x + d.vertices[f.vertices[0]].pos.x)/1, d.vertices[f.vertices[0]].pos.y/1, (room.info.z + d.vertices[f.vertices[0]].pos.z)/1);
				if (!f.triangle)
					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_SNOW,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[0]].pos.x)/1, d.vertices[f.vertices[0]].pos.y/1, (room.info.z + d.vertices[f.vertices[0]].pos.z)/1, (room.info.x + d.vertices[f.vertices[3]].pos.x)/1, d.vertices[f.vertices[3]].pos.y/1, (room.info.z + d.vertices[f.vertices[3]].pos.z)/1, (room.info.x + d.vertices[f.vertices[2]].pos.x)/1, d.vertices[f.vertices[2]].pos.y/1, (room.info.z + d.vertices[f.vertices[2]].pos.z)/1);
				surface_ind++;
			}
		}

		fprintf(file, "};\nconst size_t surfaces_count = sizeof( surfaces ) / sizeof( surfaces[0] );");
		fclose(file);
		printed = true;
		surface_ind = 0;

		struct SM64Surface surfaces[surfaces_count];
		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room &room = level->rooms[i];
			TR::Room::Data &d = room.data;

			for (int j = 0; j < d.fCount; j++)
			{
				TR::Face &f = d.faces[j];
				surfaces[surface_ind] = {SURFACE_DEFAULT,0,TERRAIN_STONE, {
					{(room.info.x + d.vertices[f.vertices[2]].pos.x)/1, d.vertices[f.vertices[2]].pos.y/1, (room.info.z + d.vertices[f.vertices[2]].pos.z)/1},
					{(room.info.x + d.vertices[f.vertices[1]].pos.x)/1, d.vertices[f.vertices[1]].pos.y/1, (room.info.z + d.vertices[f.vertices[1]].pos.z)/1},
					{(room.info.x + d.vertices[f.vertices[0]].pos.x)/1, d.vertices[f.vertices[0]].pos.y/1, (room.info.z + d.vertices[f.vertices[0]].pos.z)/1},
				}};

				if (!f.triangle)
				{
					surface_ind++;
					surfaces[surface_ind] = {SURFACE_DEFAULT,0,TERRAIN_STONE, {
						{(room.info.x + d.vertices[f.vertices[0]].pos.x)/1, d.vertices[f.vertices[0]].pos.y/1, (room.info.z + d.vertices[f.vertices[0]].pos.z)/1},
						{(room.info.x + d.vertices[f.vertices[3]].pos.x)/1, d.vertices[f.vertices[3]].pos.y/1, (room.info.z + d.vertices[f.vertices[3]].pos.z)/1},
						{(room.info.x + d.vertices[f.vertices[2]].pos.x)/1, d.vertices[f.vertices[2]].pos.y/1, (room.info.z + d.vertices[f.vertices[2]].pos.z)/1},
					}};
				}

				surface_ind++;
			}
		}

		sm64_static_surfaces_load((const struct SM64Surface*)&surfaces, surfaces_count);
		marioId = sm64_mario_create(pos.x, pos.y, pos.z);
	}
	
	virtual ~Mario()
	{
		free(marioGeometry.position);
		free(marioGeometry.color);
		free(marioGeometry.normal);
		free(marioGeometry.uv);
		delete TRmarioMesh;

		if (marioId != -1) sm64_mario_delete(marioId);
	}

	void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		//Lara::render(frustum, mesh, type, caustics);

		Core::setCullMode(cmBack);
		glUseProgram(Core::marioShader);

		GAPI::Texture *dtex = Core::active.textures[sDiffuse];
		Core::marioTexture->bind(sDiffuse);

		MeshRange range;
		range.aIndex = 0;
		range.iStart = 0;
		range.vStart = 0;
		range.iCount = marioRenderState.mario.num_vertices;

		TRmarioMesh->render(range);

		Core::setCullMode(cmNone);

		if (dtex) dtex->bind(sDiffuse);
	}

	bool checkInteraction(Controller *controller, const TR::Limits::Limit *limit, bool action)
	{
		return false;
	}

	void update()
	{
		if (Input::state[camera->cameraIndex][cLook] && Input::lastState[camera->cameraIndex] == cAction)
            camera->changeView(!camera->firstPerson);

        if (level->isCutsceneLevel()) {
            updateAnimation(true);

            updateLights();

            if (fixRoomIndex()) {
                for (int i = 0; i < COUNT(braid); i++) {
                    if (braid[i]) {
                        braid[i]->update();
                    }
                }
            }
        } else {
            switch (usedItem) {
                case TR::Entity::INV_MEDIKIT_SMALL :
                case TR::Entity::INV_MEDIKIT_BIG   :
                    if (health < LARA_MAX_HEALTH) {
                        damageTime = LARA_DAMAGE_TIME;
                        health = min(LARA_MAX_HEALTH, health + (usedItem == TR::Entity::INV_MEDIKIT_SMALL ? LARA_MAX_HEALTH / 2 : LARA_MAX_HEALTH));
                        game->playSound(TR::SND_HEALTH, pos, Sound::PAN);
                        inventory->remove(usedItem);
                    }
                    usedItem = TR::Entity::NONE;
                default : ;
            }

            updateRoom();

			vec3 p = pos;
			input = getInput();
			stand = getStand();
			updateState();
			Controller::update();

			marioInputs.buttonA = false;
			marioInputs.buttonB = false;
			marioInputs.buttonZ = false;
			marioInputs.camLookX = 0;
			marioInputs.camLookZ = 0;
			marioInputs.stickX = 0;
			marioInputs.stickY = 0;

			if (flags.active) {
				// do mario64 tick here
				marioTicks += Core::deltaTime;
				while (marioTicks > 1./30)
				{
					sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
					marioTicks -= 1./30;
				}
				marioRenderState.mario.num_vertices = 3 * marioGeometry.numTrianglesUsed;
				TRmarioMesh->update(&marioGeometry);
				/*
				for (int i=0; i<9 * SM64_GEO_MAX_TRIANGLES; i++)
				{
					//marioGeometry.position[i] *= 2;
					if (i%3 != 0) // flip y and z
					{
						//marioGeometry.position[i] = -marioGeometry.position[i];
					}
				}*/

				updateVelocity();
				updatePosition();
				//sm64_mario_teleport(marioId, pos.x, pos.y, pos.z);
				if (p != pos) {
					if (updateZone())
						updateLights();
					else
						pos = p;
				}
			}
        }
        
        camera->update(vec3(marioState.position[0], marioState.position[1], marioState.position[2]));

        if (hitTimer > 0.0f) {
            hitTimer -= Core::deltaTime;
            if (hitTimer > 0.0f)
                Input::setJoyVibration(camera->cameraIndex, 0.5f, 0.5f);
            else
                Input::setJoyVibration(camera->cameraIndex, 0, 0);
        }

        if (level->isCutsceneLevel())
            return;

        if (damageTime > 0.0f)
            damageTime = max(0.0f, damageTime - Core::deltaTime);

        if (stand == STAND_UNDERWATER && !dozy) {
            if (oxygen > 0.0f)
                oxygen -= Core::deltaTime;
            else
                hit(Core::deltaTime * 150.0f);
        } else
            if (oxygen < LARA_MAX_OXYGEN && health > 0.0f)
                oxygen = min(LARA_MAX_OXYGEN, oxygen + Core::deltaTime * 10.0f);

        usedItem = TR::Entity::NONE;

        if (camera->mode != Camera::MODE_CUTSCENE && camera->mode != Camera::MODE_STATIC) {
            camera->mode = (emptyHands() || health <= 0.0f) ? Camera::MODE_FOLLOW : Camera::MODE_COMBAT;
            if (input & LOOK)
                camera->mode = Camera::MODE_LOOK;
        }

        if (keyItem) {
            keyItem->flags.invisible = animation.frameIndex < (state == STATE_USE_KEY ? 70 : 30);
            keyItem->lockMatrix = true;
            mat4 &m = keyItem->matrix;
            Basis b;

            if (state == STATE_USE_KEY) {
                b = getJoint(10);
                b.rot = b.rot * quat(vec3(0, 1, 0), PI * 0.5f);
                b.translate(vec3(0, 120, 0));
            } else {
                vec3 rot(0.0f);
                
                // TODO: hardcode item-hand alignment 8)
                rot.x = -PI * 0.5f;

                if (animation.frameIndex < 55)
                    b = getJoint(13);
                else
                    b = getJoint(10);

                b.translate(vec3(0, 48, 0));

                if (rot.x != 0.0f) b.rot = b.rot * quat(vec3(1, 0, 0), rot.x);
                if (rot.y != 0.0f) b.rot = b.rot * quat(vec3(0, 1, 0), rot.y);
                if (rot.z != 0.0f) b.rot = b.rot * quat(vec3(0, 0, 1), rot.z);
            }

            keyItem->joints[0] = b;

            m.identity();
            m.setRot(b.rot);
            m.setPos(b.pos);
            //m = getMatrix();
        }
	}

	void move()
	{
		Lara::move();
	}

	void updatePosition()
	{
		Lara::updatePosition();
		/*
		pos.x = marioState.position[0];
		pos.y = marioState.position[1];
		pos.z = marioState.position[2];
		*/
		printf("L %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
		printf("M %.2f %.2f %.2f\n", marioState.position[0], marioState.position[1], marioState.position[2]);
	}

	void updateVelocity()
	{
		Lara::updateVelocity();
	}
};

#endif
