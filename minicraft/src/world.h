#ifndef __WORLD_H__
#define __WORLD_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "engine/noise/perlin.h"

class MWorld
{
public:
	typedef uint8 MAxis;
	static const int AXIS_X = 0b00000001;
	static const int AXIS_Y = 0b00000010;
	static const int AXIS_Z = 0b00000100;

#ifdef _DEBUG
	static const int MAT_SIZE = 4; // en nombre de chunks
#else
	static const int MAT_SIZE = 1; // en nombre de chunks
#endif // DEBUG

	static const int MAT_HEIGHT = 1; // en nombre de chunks
	static const int MAT_SIZE_CUBES = (MAT_SIZE * MChunk::CHUNK_SIZE);
	static const int MAT_HEIGHT_CUBES = (MAT_HEIGHT * MChunk::CHUNK_SIZE);
	static const int MAT_SIZE_METERS = (MAT_SIZE * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE);
	static const int MAT_HEIGHT_METERS = (MAT_HEIGHT * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE);
	static const int WATER_LEVEL = (MAT_HEIGHT_CUBES * 5 / 11);

	MChunk *Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];

	MWorld()
	{
		// On crée les chunks
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					Chunks[x][y][z] = new MChunk(x, y, z);

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					MChunk *cxPrev = NULL;
					if (x > 0)
						cxPrev = Chunks[x - 1][y][z];
					MChunk *cxNext = NULL;
					if (x < MAT_SIZE - 1)
						cxNext = Chunks[x + 1][y][z];

					MChunk *cyPrev = NULL;
					if (y > 0)
						cyPrev = Chunks[x][y - 1][z];
					MChunk *cyNext = NULL;
					if (y < MAT_SIZE - 1)
						cyNext = Chunks[x][y + 1][z];

					MChunk *czPrev = NULL;
					if (z > 0)
						czPrev = Chunks[x][y][z - 1];
					MChunk *czNext = NULL;
					if (z < MAT_HEIGHT - 1)
						czNext = Chunks[x][y][z + 1];

					Chunks[x][y][z]->setVoisins(cxPrev, cxNext, cyPrev, cyNext, czPrev, czNext);
				}
	}

	inline MCube *getCube(int x, int y, int z)
	{
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;
		if (x >= MAT_SIZE * MChunk::CHUNK_SIZE)
			x = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * MChunk::CHUNK_SIZE)
			y = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * MChunk::CHUNK_SIZE)
			z = (MAT_HEIGHT * MChunk::CHUNK_SIZE) - 1;

		return &(Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->_Cubes[x % MChunk::CHUNK_SIZE][y % MChunk::CHUNK_SIZE][z % MChunk::CHUNK_SIZE]);
	}

	void updateCube(int x, int y, int z)
	{
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;
		if (x >= MAT_SIZE * MChunk::CHUNK_SIZE)
			x = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * MChunk::CHUNK_SIZE)
			y = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * MChunk::CHUNK_SIZE)
			z = (MAT_HEIGHT * MChunk::CHUNK_SIZE) - 1;

		Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
		Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();

		if ((x - 1) / MChunk::CHUNK_SIZE > 0 && (x - 1) / MChunk::CHUNK_SIZE != x / MChunk::CHUNK_SIZE)
		{
			Chunks[(x - 1) / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[(x - 1) / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();
		}

		if ((x + 1) / MChunk::CHUNK_SIZE < MAT_SIZE && (x + 1) / MChunk::CHUNK_SIZE != x / MChunk::CHUNK_SIZE)
		{
			Chunks[(x + 1) / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[(x + 1) / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();
		}

		if ((y - 1) / MChunk::CHUNK_SIZE > 0 && (y - 1) / MChunk::CHUNK_SIZE != y / MChunk::CHUNK_SIZE)
		{
			Chunks[x / MChunk::CHUNK_SIZE][(y - 1) / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[x / MChunk::CHUNK_SIZE][(y - 1) / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();
		}

		if ((y + 1) / MChunk::CHUNK_SIZE < MAT_SIZE && (y + 1) / MChunk::CHUNK_SIZE != y / MChunk::CHUNK_SIZE)
		{
			Chunks[x / MChunk::CHUNK_SIZE][(y + 1) / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[x / MChunk::CHUNK_SIZE][(y + 1) / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();
		}

		if ((z - 1) / MChunk::CHUNK_SIZE > 0 && (z - 1) / MChunk::CHUNK_SIZE != z / MChunk::CHUNK_SIZE)
		{
			Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][(z - 1) / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][(z - 1) / MChunk::CHUNK_SIZE]->toVbos();
		}

		if ((z + 1) / MChunk::CHUNK_SIZE < MAT_HEIGHT && (z + 1) / MChunk::CHUNK_SIZE != z / MChunk::CHUNK_SIZE)
		{
			Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][(z + 1) / MChunk::CHUNK_SIZE]->disableHiddenCubes();
			Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][(z + 1) / MChunk::CHUNK_SIZE]->toVbos();
		}
	}

	void deleteCube(int x, int y, int z)
	{
		MCube *cube = getCube(x, y, z);
		cube->setType(MCube::CUBE_AIR);
		cube->setDraw(false);
		cube = getCube(x - 1, y, z);
		updateCube(x, y, z);
	}

	void init_world(int seed)
	{
		YLog::log(YLog::USER_INFO, (toString("Creation du monde seed ") + toString(seed)).c_str());

		srand(seed);

		YPerlin pGen;
		pGen.setFreq(0.2f);

		YPerlin pHeight;
		pHeight.setFreq(0.03f);

		// Reset du monde
		for (int x = 0; x < MAT_SIZE; x++)
		{
			for (int y = 0; y < MAT_SIZE; y++)
			{
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					Chunks[x][y][z]->reset();
				}
			}
		}

		for (int x = 0; x < MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
			{
				for (int z = 0; z < MAT_HEIGHT_CUBES; z++)
				{

					float val = pGen.sample((float)x, (float)y, (float)z, 5);
					val *= z;
					val /= MAT_HEIGHT_CUBES / 2;

					int height = pHeight.sample(x, y, 0, 0.8) * MAT_HEIGHT_CUBES;

					MCube *cube = getCube(x, y, z);
					if (z < WATER_LEVEL)
					{
						cube->setType(MCube::CUBE_EAU);
					}
					if (z <= height)
					{
						if (z == height && height >= WATER_LEVEL)
						{
							cube->setType(MCube::CUBE_HERBE);
						}
						else if (z == height && height < WATER_LEVEL)
						{
							cube->setType(MCube::CUBE_SABLE_01);
						}
						else if (val <= 0.3f)
						{
							cube->setType(MCube::CUBE_PIERRE);
						}
						else
						{
							cube->setType(MCube::CUBE_TERRE);
						}
					}
				}
			}
		}

		for (int x = 0; x < MAT_SIZE; x++)
		{
			for (int y = 0; y < MAT_SIZE; y++)
			{
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					Chunks[x][y][z]->disableHiddenCubes();
				}
			}
		}

		add_world_to_vbo();
	}

	void add_world_to_vbo(void)
	{
		for (int x = 0; x < MAT_SIZE; x++)
		{
			for (int y = 0; y < MAT_SIZE; y++)
			{
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					Chunks[x][y][z]->toVbos();
				}
			}
		}
	}

	// Boites de collisions plus petites que deux cubes
	MAxis getMinCol_(YVec3f pos, YVec3f dir, float width, float height, float &valueColMin, bool oneShot)
	{
		int x = (int)(pos.X / MCube::CUBE_SIZE);
		int y = (int)(pos.Y / MCube::CUBE_SIZE);
		int z = (int)(pos.Z / MCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / MCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / MCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / MCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / MCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / MCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / MCube::CUBE_SIZE);

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;

		if (xPrev < 0)
			xPrev = 0;
		if (yPrev < 0)
			yPrev = 0;
		if (zPrev < 0)
			zPrev = 0;

		if (xNext < 0)
			xNext = 0;
		if (yNext < 0)
			yNext = 0;
		if (zNext < 0)
			zNext = 0;

		if (x >= MAT_SIZE_CUBES)
			x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)
			y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)
			z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)
			xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)
			yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)
			zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)
			xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)
			yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)
			zNext = MAT_HEIGHT_CUBES - 1;

		// On fait chaque axe
		MAxis axis = 0x00;
		valueColMin = oneShot ? 0.5f : 10000.0f;
		float seuil = 0.000001f;
		float prodScalMin = 1.0f;
		if (dir.getSqrSize() > 1)
			dir.normalize();

		// On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				  getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				  getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				  getCube(xPrev + 1, yNext, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = ((xPrev + 1) * MCube::CUBE_SIZE) - (pos.X - width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		// float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		// On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				  getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				  getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				  getCube(xNext - 1, yNext, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = (xNext * MCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		// float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		// On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				  getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				  getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				  getCube(xNext, yNext - 1, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = (yNext * MCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		// float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		// On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				  getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				  getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				  getCube(xNext, yPrev + 1, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = ((yPrev + 1) * MCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		// On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				  getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				  getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				  getCube(xNext, yNext, zNext - 1)->isSolid()) ||
				!oneShot)
			{
				float depassement = (zNext * MCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		// On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				  getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				  getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				  getCube(xNext, yNext, zPrev + 1)->isSolid()) ||
				!oneShot)
			{
				float depassement = ((zPrev + 1) * MCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		return axis;
	}

	MAxis getMinCol(YVec3f pos, YVec3f dir, float width, float height, float &valueColMin, bool oneShot = true)
	{
		int x = (int)(pos.X / MCube::CUBE_SIZE);
		int y = (int)(pos.Y / MCube::CUBE_SIZE);
		int z = (int)(pos.Z / MCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / MCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / MCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / MCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / MCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / MCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / MCube::CUBE_SIZE);

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;

		if (xPrev < 0)
			xPrev = 0;
		if (yPrev < 0)
			yPrev = 0;
		if (zPrev < 0)
			zPrev = 0;

		if (xNext < 0)
			xNext = 0;
		if (yNext < 0)
			yNext = 0;
		if (zNext < 0)
			zNext = 0;

		if (x >= MAT_SIZE_CUBES)
			x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)
			y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)
			z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)
			xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)
			yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)
			zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)
			xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)
			yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)
			zNext = MAT_HEIGHT_CUBES - 1;

		// On fait chaque axe
		MAxis axis = 0x00;
		valueColMin = oneShot ? 0.5f : 10000.0f;
		float seuil = 0.0000001f;
		float prodScalMin = 1.0f;
		if (dir.getSqrSize() > 1)
			dir.normalize();

		float depList[6] = {valueColMin, valueColMin, valueColMin, valueColMin, valueColMin, valueColMin};

		// On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				  getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				  getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				  getCube(xPrev + 1, yNext, zNext)->isSolid()) ||
				!oneShot)
			{
				float a = (xPrev + 1) * MCube::CUBE_SIZE;
				float b = pos.X - width / 2.0f;
				float depassement = (a - b);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
				{
					depList[0] = depassement;
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
				}
			}
		}

		// float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		// On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				  getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				  getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				  getCube(xNext - 1, yNext, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = (xNext * MCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
				{
					depList[1] = depassement;
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
				}
			}
		}

		// float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		// On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				  getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				  getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				  getCube(xNext, yNext - 1, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = (yNext * MCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
				{

					depList[2] = depassement;
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
				}
			}
		}

		// float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		// On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				  getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				  getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				  getCube(xNext, yPrev + 1, zNext)->isSolid()) ||
				!oneShot)
			{
				float depassement = ((yPrev + 1) * MCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
				{

					depList[3] = depassement;
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
				}
			}
		}

		// On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				  getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				  getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				  getCube(xNext, yNext, zNext - 1)->isSolid()) ||
				!oneShot)
			{
				float depassement = (zNext * MCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
				{

					depList[4] = depassement;
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
				}
			}
		}

		// On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			// On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				  getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				  getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				  getCube(xNext, yNext, zPrev + 1)->isSolid()) ||
				!oneShot)
			{
				float depassement = ((zPrev + 1) * MCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil) {

					depList[5] = depassement;
				if (abs(depassement) < abs(valueColMin))
				{

					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
				}
				}
			}
		}

		if (axis == 0 && oneShot)
			return getMinCol(pos, dir, width, height, valueColMin, false);

		return axis;
	}

	void render_world_basic(GLuint shader, YVbo *vboCube)
	{
		for (int x = MAT_SIZE_CUBES - 1; x >= 0; x--)
			for (int y = MAT_SIZE_CUBES - 1; y >= 0; y--)
				for (int z = MAT_HEIGHT_CUBES - 1; z >= 0; z--)
				{
					MCube *cube = getCube(x, y, z);
					MCube::MCubeType type = cube->getType();

					// Si il faut le dessiner
					if (type != MCube::CUBE_AIR && cube->getDraw())
					{
						YColor color(40.0f / 255.0f, 25.0f / 255.0f, 0.0f, 1.0f);
						if (type == MCube::CUBE_EAU)
							color = YColor(0, 0, 1, 0.5);
						if (type == MCube::CUBE_PIERRE)
							color = YColor(.2, .2, .2, 1);
						if (type == MCube::CUBE_HERBE)
							color = YColor(0, 1, 0, 1);

						glPushMatrix();
						glTranslatef(x * MCube::CUBE_SIZE, y * MCube::CUBE_SIZE, z * MCube::CUBE_SIZE);
						YRenderer::getInstance()->updateMatricesFromOgl();
						YRenderer::getInstance()->sendMatricesToShader(shader);
						GLuint var = glGetUniformLocation(shader, "cube_color");
						glUniform4f(var, color.R, color.V, color.B, color.A);
						vboCube->render();
						glPopMatrix();
					}
				}
	}

	void render_world_vbo(bool debug, bool doTransparent)
	{
		glDisable(GL_BLEND);
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = MAT_HEIGHT - 1; z >= 0; z--)
				{
					glPushMatrix();
					glTranslatef((float)(x * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE), (float)(y * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE), (float)(z * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE));
					YRenderer::getInstance()->updateMatricesFromOgl();
					YRenderer::getInstance()->sendMatricesToShader(YRenderer::CURRENT_SHADER);
					Chunks[x][y][z]->render(false);
					glPopMatrix();
				}

		glEnable(GL_BLEND);
		if (doTransparent)
		{
			for (int x = 0; x < MAT_SIZE; x++)
				for (int y = 0; y < MAT_SIZE; y++)
					for (int z = 0; z < MAT_HEIGHT; z++)
					{
						glPushMatrix();
						glTranslatef((float)(x * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE), (float)(y * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE), (float)(z * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE));
						YRenderer::getInstance()->updateMatricesFromOgl();
						YRenderer::getInstance()->sendMatricesToShader(YRenderer::CURRENT_SHADER);
						Chunks[x][y][z]->render(true);
						glPopMatrix();
					}
		}
		glDepthMask(true);
	}

	/**
	 * Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
	 * versions optimisées de ce calcul.
	 */
	inline bool intersecDroitePlan(const YVec3f &debSegment, const YVec3f &finSegment,
								   const YVec3f &p1Plan, const YVec3f &p2Plan, const YVec3f &p3Plan,
								   YVec3f &inter)
	{

		return true;
	}

	/**
	 * Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
	 * versions optimisées de ce calcul. Il faut donner les points dans l'ordre (CW ou CCW)
	 */
	inline bool intersecDroiteCubeFace(const YVec3f &debSegment, const YVec3f &finSegment,
									   const YVec3f &p1, const YVec3f &p2, const YVec3f &p3, const YVec3f &p4,
									   YVec3f &inter)
	{

		return false;
	}

	bool getRayCollision(const YVec3f &debSegment, const YVec3f &finSegment,
						 YVec3f &inter,
						 int &xCube, int &yCube, int &zCube)
	{

		return false;
	}

	/**
	 * De meme cette fonction peut être grandement opitimisée, on a priviligié la clarté
	 */
	bool getRayCollisionWithCube(const YVec3f &debSegment, const YVec3f &finSegment,
								 int x, int y, int z,
								 YVec3f &inter)
	{

		return true;
	}
};

#endif