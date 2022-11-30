#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"

#include "avatar.h"
#include "world.h"
#include <engine/noise/perlin.h>

constexpr int dayStartTime = 6;
constexpr int dayEndTime = 19;

enum PointId
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H
};

enum MouseClick
{
	LeftClick,
	MiddleClick,
	RightClick,
	WheelRollUp,
	WheelRolldown
};

enum MouseState
{
	Clickpress,
	ClickRelease
};

enum KeyBoard
{
	Key_space = 32,
	Key_d = 100,
	Key_g = 103,
	Key_l = 108,
	Key_p = 112,
	Key_q = 113,
	Key_ctrl = 114,
	Key_s = 115,
	Key_t = 116,
	Key_z = 122,

};

class MEngineMinicraft : public YEngine
{

	MWorld *World;

	YVbo *sun;
	YVbo *moon;
	YVbo *vbo;
	MAvatar *avatar;

	YPerlin noise;

	GLuint ShaderCube = 0;
	GLuint ShaderSun = 0;
	GLuint ShaderWorld = 0;

	YColor skyColor = YColor();
	YColor SunColor = YColor(0, 0, 0, 0.1);
	YColor MoonColor = YColor(0.8, 0.8, 0.8, 1.0);
	double time_offset = 0;
	double SunHeight = 100;

	bool CamLockPlayer = true;

	SYSTEMTIME localTime;

public:
	// Gestion singleton
	static YEngine *getInstance()
	{
		if (Instance == NULL)
			Instance = new MEngineMinicraft();
		return Instance;
	}

	/*HANDLERS GENERAUX*/
	void loadShaders()
	{
	}

	void init()
	{
		YLog::log(YLog::ENGINE_INFO, "Minicraft Started : initialisation");

		Renderer->setBackgroundColor(YColor(0.0f, 0.0f, 0.0f, 1.0f));
		Renderer->Camera->setPosition(YVec3f(-150, -150, 150));

		sun = createVBOCube(2);
		moon = createVBOCube(0.2);

		vbo = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		vbo->setElementDescription(0, YVbo::Element(3)); // Sommet
		vbo->setElementDescription(1, YVbo::Element(3)); // Normale
		vbo->setElementDescription(2, YVbo::Element(2)); // UV

		vbo->createVboCpu();
		fillVBOCube(vbo, 1);
		vbo->createVboGpu();
		vbo->deleteVboCpu();
		// Les materials
		ShaderSun = Renderer->createProgram("shaders/sun");

// Pour avoir le monde
#include "world.h"

		// Pour créer le monde
		ShaderCube = Renderer->createProgram("shaders/cube");
		ShaderWorld = Renderer->createProgram("shaders/world");
		World = new MWorld();
		World->init_world(1);

		avatar = new MAvatar(Renderer->Camera, World);
		avatar->Position = YVec3f(60, 60, 80);
		Renderer->Camera->setLookAt(avatar->Position);
	}

	void update(float elapsed)
	{
		time_offset += elapsed * 30;
		avatar->update(elapsed);
	}

	void renderObjects()
	{
		renderAxis();

		renderWorld();

		renderAstrals();

		renderSky();

		renderAvatar();

		if (CamLockPlayer)
		{
			relocateCam();
		}
	}

	// # # # # # # # # # # # # # # # # # # # # #
	// # # # # #    Render Functions   # # # # #
	// # # # # # # # # # # # # # # # # # # # # #

	void renderAvatar()
	{
		renderAxis(avatar->Position, 3);
		glPushMatrix();

		glUseProgram(ShaderSun);
		GLuint var = glGetUniformLocation(ShaderSun, "sun_color");
		glUniform3f(var, 1, 0, 0);
		glTranslatef(avatar->Position.X, avatar->Position.Y, avatar->Position.Z);
		//glTranslatef(avatar->Position.X + avatar->Width / 2, avatar->Position.Y + avatar->Width / 2, avatar->Position.Z + avatar->Height / 2);
		glScalef(avatar->Width, avatar->Width, avatar->Height);
		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(ShaderSun);
		vbo->render();

		glPopMatrix();
	}

	void renderWorld()
	{
		glPushMatrix();
		glUseProgram(ShaderWorld);
		World->render_world_vbo(false, true);
		glPopMatrix();
	}

	void renderAstrals()
	{
		glPushMatrix();
		YVec3f solarPos = getSolarPosition();
		YVec3f moonPos = getMoonPosition(solarPos);
		SunColor = getSunColor();

		glTranslatef(solarPos.X, solarPos.Y, solarPos.Z);
		// Demande au GPU de charger ce shader
		glUseProgram(ShaderSun);
		GLuint var = glGetUniformLocation(ShaderSun, "sun_color");
		glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);

		Renderer->updateMatricesFromOgl();		   // Calcule toute les matrices à partir des deux matrices OGL
		Renderer->sendMatricesToShader(ShaderSun); // Envoie les matrices au shader
		vbo->render();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(moonPos.X, moonPos.Y, moonPos.Z);
		glUniform3f(var, MoonColor.R, MoonColor.V, MoonColor.B);

		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(ShaderSun);
		vbo->render();
		glPopMatrix();
	}

	void renderSky()
	{
		skyColor = getSkyColor();
		Renderer->setBackgroundColor(skyColor);
	}

	void renderAxis(YVec3f origin = YVec3f(), int axisLength = 1000)
	{
		glUseProgram(0);
		// Rendu des axes
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3d(1, 0, 0);
		glVertex3d(origin.X, origin.Y, origin.Z);
		glVertex3d(origin.X + axisLength, origin.Y, origin.Z);
		glColor3d(0, 1, 0);
		glVertex3d(origin.X, origin.Y, origin.Z);
		glVertex3d(origin.X, origin.Y + axisLength, origin.Z);
		glColor3d(0, 0, 1);
		glVertex3d(origin.X, origin.Y, origin.Z);
		glVertex3d(origin.X, origin.Y, origin.Z + axisLength);
		glEnd();
	}

	void relocateCam()
	{
		Renderer->Camera->Position = avatar->Position;
		YVec3f camoffset = Renderer->Camera->Direction * -15;
		Renderer->Camera->move(camoffset);
		Renderer->Camera->setLookAt(avatar->Position);
	}

	// # # # # # # # # # # # # # # # # # # # # #
	// # # # # #     Tool Functions    # # # # #
	// # # # # # # # # # # # # # # # # # # # # #

	YVbo *createVBOCube(double cubeSize)
	{

		YVbo *vbo;

		YVec3f *dots = new YVec3f[8];
		dots[A] = YVec3f(-cubeSize / 2, -cubeSize / 2, -cubeSize / 2);
		dots[B] = YVec3f(cubeSize / 2, -cubeSize / 2, -cubeSize / 2);
		dots[C] = YVec3f(cubeSize / 2, cubeSize / 2, -cubeSize / 2);
		dots[D] = YVec3f(-cubeSize / 2, cubeSize / 2, -cubeSize / 2);
		dots[E] = YVec3f(-cubeSize / 2, -cubeSize / 2, cubeSize / 2);
		dots[F] = YVec3f(cubeSize / 2, -cubeSize / 2, cubeSize / 2);
		dots[G] = YVec3f(cubeSize / 2, cubeSize / 2, cubeSize / 2);
		dots[H] = YVec3f(-cubeSize / 2, cubeSize / 2, cubeSize / 2);

		int **faces = new int *[12];
		faces[0] = new int[3]{A, D, C};	 // 1bot
		faces[1] = new int[3]{A, C, B};	 // 2bot
		faces[4] = new int[3]{B, F, A};	 // 1ar g
		faces[5] = new int[3]{F, E, A};	 // 2ar g
		faces[6] = new int[3]{E, H, A};	 // 1ar d
		faces[7] = new int[3]{A, H, D};	 // 2ar d
		faces[2] = new int[3]{E, F, G};	 // 1top
		faces[3] = new int[3]{E, G, H};	 // 2top
		faces[8] = new int[3]{F, B, G};	 // 1av g
		faces[9] = new int[3]{B, C, G};	 // 2av g
		faces[10] = new int[3]{G, C, D}; // 1av d
		faces[11] = new int[3]{H, G, D}; // 2av d

		// Creation du VBO
		vbo = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		// Définition du contenu du VBO
		vbo->setElementDescription(0, YVbo::Element(3)); // Sommet
		vbo->setElementDescription(1, YVbo::Element(3)); // Normale
		vbo->setElementDescription(2, YVbo::Element(2)); // UV

		// On demande d'allouer la mémoire coté CPU
		vbo->createVboCpu();

		// On ajoute les sommets
		for (int iFace = 0; iFace < 12; iFace++)
		{
			for (int index = 0; index < 3; index++)
			{
				int iVertice = faces[iFace][index];
				vbo->setElementValue(0, 3 * iFace + index, dots[iVertice].X, dots[iVertice].Y, dots[iVertice].Z);
				vbo->setElementValue(1, 0, 1, 0, 0);
				vbo->setElementValue(2, 0, 0, 0);
			}
		}

		// On envoie le contenu au GPU
		vbo->createVboGpu();
		// On relache la mémoire CPU
		vbo->deleteVboCpu();

		return vbo;
	}

	YVec3f getMoonPosition(YVec3f sunpos)
	{
		return sunpos * -1;
	}

	YColor getSunColor()
	{
		return YColor(1, 1, 0, 1);
	}

	YColor getSkyColor()
	{
		YColor morningSkyColor = YColor(0.2, 0.2, 0.2, 1);
		YColor daySkyColor = YColor(135 / 255., 206 / 255., 235 / 255., 1);
		YColor twilightSkyColor = YColor(1, 127 / 255., 0, 1);
		YColor nightSkyColor = YColor(0.1, 0.1, 0.1, 1);

		double time = getOffsetTimeHour();

		if (time > 4 && time <= 5.5)
		{
			skyColor = nightSkyColor.interpolate(morningSkyColor, (time - 4 / 1.5));
		}
		else if (time > 6 && time <= 7)
		{
			skyColor = morningSkyColor.interpolate(daySkyColor, time - 6);
		}
		else if (time > 7 && time <= 17)
		{
			skyColor = daySkyColor;
		}
		else if (time > 17 && time <= 19)
		{
			skyColor = daySkyColor.interpolate(twilightSkyColor, (time - 17) / 2.);
		}
		else if (time >= 19 && time <= 20)
		{
			skyColor = twilightSkyColor.interpolate(nightSkyColor, time - 19);
		}
		else
		{
			skyColor = nightSkyColor;
		}

		return skyColor;
	}

	bool isDay()
	{
		int gameCurrentTimeHour = getOffsetTimeHour();
		return gameCurrentTimeHour >= dayStartTime && gameCurrentTimeHour < dayEndTime;
	}

	double getOffsetTimeHour()
	{
		return getOffsetTimeMinutes() / 60.0;
	}

	double getOffsetTimeMinutes()
	{
		return int(localTime.wHour * 60 + localTime.wMinute + time_offset) % 1440 + localTime.wSecond / 60.0 + localTime.wMilliseconds / 60000.0;
	}

	YVec3f getSolarPosition()
	{
		GetLocalTime(&localTime);

		double dayMinuteDuration = (dayEndTime - dayStartTime) * 60;
		double nightMinuteDuration = (24 * 60) - dayMinuteDuration;

		YVec3f p(SunHeight, 0, 0);
		if (isDay())
		{
			double dayProgression = (getOffsetTimeMinutes() - dayStartTime * 60) / dayMinuteDuration;
			return p.rotate(YVec3f(0, -1, 0), dayProgression * M_PI);
		}
		else
		{
			double time = getOffsetTimeHour();
			double nightProgression = 0.5;
			if (time < dayStartTime)
			{
				time += 24;
			}
			nightProgression = ((time - dayEndTime) * 60) / nightMinuteDuration;
			return p.rotate(YVec3f(0, -1, 0), nightProgression * M_PI + M_PI);
		}
		return p;
	}

	void resize(int width, int height)
	{
	}

	/*INPUTS*/

	bool maintainedCtrl = false;

	void keyPressed(int key, bool special, bool down, int p1, int p2)
	{

		if (!CamLockPlayer)
		{
			if (down)
			{
				if (key == Key_g)
				{
					time_offset += 60;
					printf("%f\n", getOffsetTimeHour());
				}
				else if (key == Key_t)
				{
					printf("time :%f\n", getOffsetTimeHour());
				}
				else if (key == Key_z)
				{
					Renderer->Camera->moveForward();
				}
				else if (key == Key_q)
				{

					Renderer->Camera->moveCamSideway(-1);
				}
				else if (key == Key_s)
				{

					Renderer->Camera->moveBackward();
				}
				else if (key == Key_d)
				{

					Renderer->Camera->moveCamSideway(1);
				}
				else if (key == Key_p)
				{
					Renderer->Camera->printProperties();
				}
				else if (key == Key_l)
				{
					CamLockPlayer = true;
					printf("La camera est maintenant verrouillé sur le joueur\n");
				}
				else if (key == Key_space)
				{
					Renderer->Camera->setLookAt(YVec3f(0, 0, 0));
				}
				else if (key == Key_ctrl)
				{
					maintainedCtrl = true;
				}
				else
				{
					printf("unknow key : %d\n", key);
				}
			}
			else
			{
				if (key == Key_ctrl)
				{
					maintainedCtrl = false;
				}
			}
		}
		// if (!CamLockPlayer)
		else
		{
			if (key == Key_z)
			{
				avatar->avance = down;
			}
			if (key == Key_s)
			{
				avatar->recule = down;
			}
			if (key == Key_q)
			{
				avatar->gauche = down;
			}
			if (key == Key_d)
			{
				avatar->droite = down;
			}
			if (key == Key_space)
			{
				avatar->Jump = down;//true;
			}
			if (key == Key_l && down)
			{
				CamLockPlayer = false;
				printf("La camera est maintenant libre\n");
			}
		}
	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		if (wheel == WheelRollUp)
		{
			Renderer->Camera->zoomIn();
		}
		else if (wheel == WheelRolldown)
		{
			Renderer->Camera->zoomOut();
		}
	}

	int delta_x = 0;
	int delta_y = 0;
	float mouseSensitivity = 0.1;

	bool maintainedClick[3] = {false, false, false};

	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		if (state == Clickpress)
		{
			maintainedClick[button] = true;
			delta_x = x;
			delta_y = y;
		}
		if (state == ClickRelease)
		{
			maintainedClick[button] = false;
		}
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		if (CamLockPlayer){
			avatar->Direction = Renderer->Camera->Direction;
		}

		if (maintainedCtrl)
		{
			if (pressed && maintainedClick[RightClick])
			{
				Renderer->Camera->moveCamUpDownAxis((y - delta_y) * mouseSensitivity);
				Renderer->Camera->moveCamSideway((delta_x - x) * mouseSensitivity);
			}
			if (pressed && maintainedClick[MiddleClick])
			{
				Renderer->Camera->moveWorld(YVec3f((delta_x - x), (y - delta_y), 0) * mouseSensitivity);
			}
		}
		else
		{
			if (pressed && maintainedClick[LeftClick])
			{
				Renderer->Camera->rotate((x - delta_x) * mouseSensitivity);
				Renderer->Camera->rotateUp((y - delta_y) * mouseSensitivity);
			}
			if (pressed && maintainedClick[RightClick])
			{
				Renderer->Camera->rotateAround((delta_x - x) * mouseSensitivity);
				Renderer->Camera->rotateUpAround((delta_y - y) * mouseSensitivity);
			}
		}
		delta_x = x;
		delta_y = y;
	}

	// # # # # # # # # # # # # # # # # # # # # #

	void fillVBOCube(YVbo *vbo, float size = 5.0f)
	{
		int iVertice = 0;

		YVec3f a(size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f b(size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f c(size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f d(size / 2.0f, -size / 2.0f, size / 2.0f);
		YVec3f e(-size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f f(-size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f g(-size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f h(-size / 2.0f, -size / 2.0f, size / 2.0f);

		iVertice += addQuadToVbo(vbo, iVertice, a, b, c, d); // x+
		iVertice += addQuadToVbo(vbo, iVertice, f, e, h, g); // x-
		iVertice += addQuadToVbo(vbo, iVertice, b, f, g, c); // y+
		iVertice += addQuadToVbo(vbo, iVertice, e, a, d, h); // y-
		iVertice += addQuadToVbo(vbo, iVertice, c, g, h, d); // z+
		iVertice += addQuadToVbo(vbo, iVertice, e, f, b, a); // z-
	}

	int addQuadToVbo(YVbo *vbo, int iVertice, YVec3f &a, YVec3f &b, YVec3f &c, YVec3f &d)
	{
		YVec3f normal = (b - a).cross(d - a);
		normal.normalize();

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, b.X, b.Y, b.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, d.X, d.Y, d.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 1);

		iVertice++;

		return 6;
	}
};

#endif