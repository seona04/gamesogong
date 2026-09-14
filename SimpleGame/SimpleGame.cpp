/*
  Tiny Harvest is a compact farming RPG prototype built on the starter
  OpenGL/freeglut project. There are no external art assets: the map and
  characters are drawn from colored tiles so the project stays easy to build.
*/
#include "stdafx.h"
#include <iostream>
#include <string>
#include "Dependencies\\glew.h"
#include "Dependencies\\freeglut.h"
#include "Renderer.h"

Renderer* g_Renderer = NULL;

const int WINDOW = 800;
const int MAP_W = 18;
const int MAP_H = 14;
const int TILE = 42;
const int FARM_LEFT = 3;
const int FARM_TOP = 3;
const int FARM_W = 7;
const int FARM_H = 6;

enum CropStage { UNTILLED, TILLED, SEED, SPROUT, RIPE };
enum TutorialStep { TUTORIAL_MOVE_TO_FIELD, TUTORIAL_TILL_SOIL, TUTORIAL_PLANT_SEED, TUTORIAL_GROW_CROP, TUTORIAL_HARVEST_CROP, TUTORIAL_COMPLETE };

CropStage g_Farm[FARM_H][FARM_W] = {};
bool g_Keys[256] = {};
int g_PlayerX = 7;
int g_PlayerY = 10;
int g_Gold = 20;
int g_Seeds = 5;
int g_Day = 1;
int g_LastMove = 0;
int g_TutorialFarmX = -1;
int g_TutorialFarmY = -1;
TutorialStep g_TutorialStep = TUTORIAL_MOVE_TO_FIELD;
std::string g_Message = "Welcome to Tiny Harvest! Space: farm  N: next day";

float PixelX(int tileX) { return (tileX - MAP_W / 2.0f + 0.5f) * TILE; }
float PixelY(int tileY) { return (MAP_H / 2.0f - tileY - 0.5f) * TILE; }

void Tile(int x, int y, float r, float g, float b)
{
	g_Renderer->DrawSolidRect(PixelX(x), PixelY(y), 0, TILE - 2, r, g, b, 1);
}

void Text(float x, float y, const std::string& text, float r = 1, float g = 1, float b = 1)
{
	// freeglut bitmap text uses the compatibility OpenGL path.
	glUseProgram(0);
	glColor3f(r, g, b);
	glRasterPos2f(x * 2.0f / WINDOW, y * 2.0f / WINDOW);
	for (size_t i = 0; i < text.size(); ++i)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
}

bool IsFarm(int x, int y)
{
	return x >= FARM_LEFT && x < FARM_LEFT + FARM_W && y >= FARM_TOP && y < FARM_TOP + FARM_H;
}

void SetMessage(const std::string& message) { g_Message = message; }

std::string GetTutorialText()
{
	switch (g_TutorialStep) {
	case TUTORIAL_MOVE_TO_FIELD:
		return "Tutorial 1/5: Walk to the brown field.";
	case TUTORIAL_TILL_SOIL:
		return "Tutorial 2/5: Press Space to till one field tile.";
	case TUTORIAL_PLANT_SEED:
		return "Tutorial 3/5: Press Space again on the tilled tile to plant.";
	case TUTORIAL_GROW_CROP:
		return "Tutorial 4/5: Press N twice to let your turnip grow.";
	case TUTORIAL_HARVEST_CROP:
		return "Tutorial 5/5: Press Space on the ripe turnip to harvest it.";
	default:
		return "Tutorial complete! Your farm is ready. Grow crops and earn gold.";
	}
}

void RenderCrop(int x, int y, CropStage stage)
{
	if (stage == UNTILLED) return;
	if (stage == TILLED) { Tile(x, y, 0.48f, 0.26f, 0.10f); return; }
	Tile(x, y, 0.39f, 0.20f, 0.08f);
	if (stage == SEED) {
		g_Renderer->DrawSolidRect(PixelX(x), PixelY(y), 0, 9, 0.95f, 0.78f, 0.20f, 1);
	}
	else if (stage == SPROUT) {
		g_Renderer->DrawSolidRect(PixelX(x), PixelY(y), 0, 19, 0.22f, 0.75f, 0.20f, 1);
		g_Renderer->DrawSolidRect(PixelX(x) - 9, PixelY(y) + 7, 0, 10, 0.45f, 0.90f, 0.25f, 1);
	}
	else {
		g_Renderer->DrawSolidRect(PixelX(x), PixelY(y), 0, 25, 0.20f, 0.67f, 0.16f, 1);
		g_Renderer->DrawSolidRect(PixelX(x), PixelY(y) + 5, 0, 12, 1.00f, 0.78f, 0.10f, 1);
	}
}

void RenderTree(int x, int y)
{
	Tile(x, y, 0.18f, 0.48f, 0.18f);
	g_Renderer->DrawSolidRect(PixelX(x), PixelY(y) - 9, 0, 11, 0.35f, 0.19f, 0.07f, 1);
	g_Renderer->DrawSolidRect(PixelX(x), PixelY(y) + 5, 0, 28, 0.08f, 0.35f, 0.10f, 1);
}

void RenderScene(void)
{
	glClearColor(0.08f, 0.13f, 0.20f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Meadow, river, and a small field.
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			Tile(x, y, x >= 14 ? 0.12f : 0.24f, x >= 14 ? 0.48f : 0.62f, x >= 14 ? 0.76f : 0.25f);

	for (int y = FARM_TOP; y < FARM_TOP + FARM_H; ++y)
		for (int x = FARM_LEFT; x < FARM_LEFT + FARM_W; ++x)
			Tile(x, y, 0.52f, 0.32f, 0.13f);

	for (int y = 0; y < FARM_H; ++y)
		for (int x = 0; x < FARM_W; ++x)
			RenderCrop(FARM_LEFT + x, FARM_TOP + y, g_Farm[y][x]);

	// Home and a merchant stall.
	Tile(1, 10, 0.68f, 0.31f, 0.18f); Tile(2, 10, 0.68f, 0.31f, 0.18f);
	Tile(1, 11, 0.84f, 0.32f, 0.18f); Tile(2, 11, 0.84f, 0.32f, 0.18f);
	Tile(11, 4, 0.75f, 0.43f, 0.18f); Tile(12, 4, 0.75f, 0.43f, 0.18f);
	g_Renderer->DrawSolidRect(PixelX(12), PixelY(6), 0, 24, 0.95f, 0.55f, 0.68f, 1); // merchant

	RenderTree(0, 0); RenderTree(1, 1); RenderTree(16, 1); RenderTree(17, 3);
	RenderTree(0, 6); RenderTree(17, 8); RenderTree(0, 13); RenderTree(12, 13);

	// Player and direction marker.
	g_Renderer->DrawSolidRect(PixelX(g_PlayerX), PixelY(g_PlayerY), 0, 27, 0.95f, 0.90f, 0.55f, 1);
	g_Renderer->DrawSolidRect(PixelX(g_PlayerX), PixelY(g_PlayerY) + 8, 0, 13, 0.30f, 0.55f, 0.92f, 1);

	Text(-385, 370, "TINY HARVEST  |  Day " + std::to_string(g_Day), 1.0f, 0.91f, 0.44f);
	Text(-385, 345, "Gold: " + std::to_string(g_Gold) + "   Seeds: " + std::to_string(g_Seeds), 0.93f, 0.96f, 1.0f);
	Text(-385, -325, GetTutorialText(), 1.0f, 0.86f, 0.36f);
	Text(-385, -350, g_Message, 0.95f, 0.95f, 0.95f);
	Text(-385, -375, "Move: WASD / arrows   Space: till, plant, harvest   B: buy seed (5g)   N: next day", 0.72f, 0.83f, 0.92f);

	glutSwapBuffers();
}

void TryFarm()
{
	if (!IsFarm(g_PlayerX, g_PlayerY)) { SetMessage("Stand on a field tile to farm."); return; }
	int fx = g_PlayerX - FARM_LEFT;
	int fy = g_PlayerY - FARM_TOP;
	CropStage& crop = g_Farm[fy][fx];
	if (crop == UNTILLED) {
		crop = TILLED;
		if (g_TutorialStep == TUTORIAL_TILL_SOIL) {
			g_TutorialFarmX = fx;
			g_TutorialFarmY = fy;
			g_TutorialStep = TUTORIAL_PLANT_SEED;
		}
		SetMessage("Soil tilled. Press Space again to plant a seed.");
	}
	else if (crop == TILLED) {
		if (g_Seeds <= 0) SetMessage("No seeds. Press B to buy one for 5 gold.");
		else {
			--g_Seeds;
			crop = SEED;
			if (g_TutorialStep == TUTORIAL_PLANT_SEED && fx == g_TutorialFarmX && fy == g_TutorialFarmY)
				g_TutorialStep = TUTORIAL_GROW_CROP;
			SetMessage("Seed planted. Sleep through a few days with N.");
		}
	}
	else if (crop == RIPE) {
		crop = UNTILLED;
		++g_Seeds;
		g_Gold += 15;
		if (g_TutorialStep == TUTORIAL_HARVEST_CROP && fx == g_TutorialFarmX && fy == g_TutorialFarmY)
			g_TutorialStep = TUTORIAL_COMPLETE;
		SetMessage("Harvested turnip! +15 gold and +1 seed.");
	}
	else SetMessage("This crop needs another day or two.");
}

void NextDay()
{
	++g_Day;
	for (int y = 0; y < FARM_H; ++y)
		for (int x = 0; x < FARM_W; ++x)
			if (g_Farm[y][x] == SEED || g_Farm[y][x] == SPROUT)
				g_Farm[y][x] = static_cast<CropStage>(g_Farm[y][x] + 1);
	if (g_TutorialStep == TUTORIAL_GROW_CROP && g_Farm[g_TutorialFarmY][g_TutorialFarmX] == RIPE)
		g_TutorialStep = TUTORIAL_HARVEST_CROP;
	SetMessage("A new morning begins. Your crops have grown!");
}

void MovePlayer(int dx, int dy)
{
	int nx = g_PlayerX + dx;
	int ny = g_PlayerY + dy;
	if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H && nx < 14) {
		g_PlayerX = nx;
		g_PlayerY = ny;
		if (g_TutorialStep == TUTORIAL_MOVE_TO_FIELD && IsFarm(g_PlayerX, g_PlayerY)) {
			g_TutorialStep = TUTORIAL_TILL_SOIL;
			SetMessage("You found the field. Let's prepare some soil.");
		}
	}
}

void Update()
{
	int now = glutGet(GLUT_ELAPSED_TIME);
	if (now - g_LastMove > 125) {
		if (g_Keys['w'] || g_Keys['W']) MovePlayer(0, -1);
		else if (g_Keys['s'] || g_Keys['S']) MovePlayer(0, 1);
		else if (g_Keys['a'] || g_Keys['A']) MovePlayer(-1, 0);
		else if (g_Keys['d'] || g_Keys['D']) MovePlayer(1, 0);
		g_LastMove = now;
	}
	glutPostRedisplay();
}

void KeyInput(unsigned char key, int, int)
{
	g_Keys[key] = true;
	if (key == ' ') TryFarm();
	else if (key == 'n' || key == 'N') NextDay();
	else if (key == 'b' || key == 'B') {
		if (g_Gold >= 5) { g_Gold -= 5; ++g_Seeds; SetMessage("Bought one turnip seed for 5 gold."); }
		else SetMessage("The merchant says: come back with 5 gold.");
	}
	else if (key == 27) exit(0);
}

void KeyUp(unsigned char key, int, int) { g_Keys[key] = false; }

void SpecialKeyInput(int key, int, int)
{
	if (key == GLUT_KEY_UP) MovePlayer(0, -1);
	else if (key == GLUT_KEY_DOWN) MovePlayer(0, 1);
	else if (key == GLUT_KEY_LEFT) MovePlayer(-1, 0);
	else if (key == GLUT_KEY_RIGHT) MovePlayer(1, 0);
}

int main(int argc, char** argv)
{
	// Initialize OpenGL and the window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(40, 40);
	glutInitWindowSize(WINDOW, WINDOW);
	glutCreateWindow("Tiny Harvest - Farming RPG");
	glewInit();

	g_Renderer = new Renderer(WINDOW, WINDOW);
	if (!g_Renderer->IsInitialized()) std::cout << "Renderer could not be initialized.\n";

	glutDisplayFunc(RenderScene);
	glutIdleFunc(Update);
	glutKeyboardFunc(KeyInput);
	glutKeyboardUpFunc(KeyUp);
	glutSpecialFunc(SpecialKeyInput);
	glutMainLoop();
	delete g_Renderer;
	return 0;
}
