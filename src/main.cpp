#define SDL_MAIN_USE_CALLBACKS 1
#define SCREEN_W 800
#define SCREEN_H 700

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <math.h>

#include "utils.hpp"

static SDL_Renderer *renderer = NULL;

// View 3D
#define viewWidth 700.0
#define maxLineH 600

// Map
#define mapX 10
#define mapY 10
#define mapS 16

bool showMiniMap = false;

int map[] = { // < Walls
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 2, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 2, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 2, 0, 0, 1,
    1, 0, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

void drawMap2D()
{
    SDL_FRect rect;

    // Clear background
    BLACK(renderer);
    rect.x = 0;
    rect.y = 0;
    rect.w = mapX * mapS;
    rect.h = mapY * mapS;
    SDL_RenderFillRect(renderer, &rect);

    // Draw
    for (int y = 0; y < mapY; y++) {
        for (int x = 0; x < mapX; x++) {
            pickColor(renderer, map[y * mapX + x], 'd');

            rect.x = x * mapS;
            rect.y = y * mapS;
            rect.w = mapS - 1;
            rect.h = mapS - 1;

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Player
float px, py;
float pdx, pdy, pa; // < Deltas and angle
float pspeed, pspeedrot;

typedef struct
{
    int z, q, s, d; // < Button state on off
} ButtonKeys;

ButtonKeys pkeys;

void drawPlayer()
{
    // Player body
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderPoint(renderer, px, py);

    // Player direction
    GREEN(renderer);
    SDL_RenderLine(renderer, px, py, px + pdx * 20, py + pdy * 20);
}

void movePlayer(float fps)
{
    if (pkeys.q == 1) {
        pa += pspeedrot * fps;
        pa = fixAng(pa);
        pdx = cos(degToRad(pa));
        pdy = -sin(degToRad(pa));
    }
    if (pkeys.d == 1) {
        pa -= pspeedrot * fps;
        pa = fixAng(pa);
        pdx = cos(degToRad(pa));
        pdy = -sin(degToRad(pa));
    }

    // collisions
    int limit = mapS * 0.35;
    int xo = 0; if (pdx < 0) { xo = -limit; } else { xo = limit; }
    int yo = 0; if (pdy < 0) { yo = -limit; } else { yo = limit; }
    int ipx = px / mapS;
    int ipx_add_xo = (px + xo) / mapS;
    int ipx_sub_xo = (px - xo) / mapS;
    int ipy = py / mapS;
    int ipy_add_yo = (py + yo) / mapS;
    int ipy_sub_yo = (py - yo) / mapS;

    if (pkeys.z == 1) {
        if (map[ipy * mapX + ipx_add_xo] == 0) { px += pdx * pspeed * fps; }
        if (map[ipy_add_yo * mapX + ipx] == 0) { py += pdy * pspeed * fps; }
    }
    if (pkeys.s == 1) {
        if (map[ipy * mapX + ipx_sub_xo] == 0) { px -= pdx * pspeed * fps; }
        if (map[ipy_sub_yo * mapX + ipx] == 0) { py -= pdy * pspeed * fps; }
    }
}

// Rays
const int raysFactor = 7; // resolution control
const int rays = 120 * raysFactor;

typedef struct
{
    float startX, startY;
    float endX, endY;
    float ra;
    float disT;
    int m;
    char c;
} StructRay;

StructRay raysToDraw[rays];

void drawRays2D()
{
    StructRay* ptr = raysToDraw;
    for (int i = 0; i < rays; i++, ptr++ ) {
        pickColor(renderer, ptr->m, ptr->c);
        SDL_RenderLine(renderer, ptr->startX, ptr->startY, ptr->endX, ptr->endY);
    }
}

void draw3DCeilings(float startX, float startOffset, float lineO)
{
    BLUE(renderer);

    SDL_RenderLine(renderer,
        startX, lineO,
        startX, startOffset - maxLineH / 2
    );
}

void draw3DFloors(float startX, float startOffset, float lineO, float lineH)
{
    GREEN(renderer);

    SDL_RenderLine(renderer,
        startX, lineO + lineH,
        startX, startOffset + maxLineH / 2
    );
}

void draw3DWalls(int m, char c, float startX, float lineO, float lineH)
{
    pickColor(renderer, m, c);

    SDL_RenderLine(renderer,
        startX, lineO,
        startX, lineO + lineH
    );
}

void drawRays3D()
{
    StructRay* ptr = raysToDraw;
    for (int i = 0; i < rays; i++, ptr++ ) {
        // Fix fisheye
        float ca = fixAng(pa - ptr->ra);
        float disT = ptr->disT * cos(degToRad(ca));

        // Center on screen
        float startOffset = SCREEN_H / 2;

        // Line height
        float lineH = (mapS * maxLineH) / ptr->disT;
        if (lineH > maxLineH) { lineH = maxLineH; }

        // Line offset
        float lineO = startOffset - lineH / 2;

        // Center on screen
        float startX = (SCREEN_W - viewWidth) / 2 + i * (viewWidth / rays);

        // Draws
        draw3DWalls(ptr->m, ptr->c, startX, lineO, lineH);
        draw3DFloors(startX, startOffset, lineO, lineH);
        draw3DCeilings(startX, startOffset, lineO);
    }
}

void computeRays()
{
    int r, mx, my, mp, dof;
    float vx, vy, rx, ry, ra, xo, yo;
    float disT;

    ra = fixAng(pa + rays / (4 * raysFactor));

    for (r = 0; r < rays; r++) {
        int mv = 0;
        int mh = 0;

        // Check vertical lines
        dof = 0;
        float disV = 1000000;
        float tan = tanf(degToRad(ra));
        if (cos(degToRad(ra)) > 0.001) { // < Looking left
            rx = (((int) px / mapS) * mapS) + mapS;
            ry = (px - rx) * tan + py;
            xo = mapS;
            yo = -xo * tan;
        } else if (cos(degToRad(ra)) < -0.001) { // < Looking right
            rx = (((int) px / mapS) * mapS) - 0.0001;
            ry = (px - rx) * tan + py;
            xo = -mapS;
            yo = -xo * tan;
        } else { // < Looking straight up or down
            rx = px;
            ry = py;
            dof = 8;
        }
        while (dof < 8) {
            mx = (int) (rx) / mapS;
            my = (int) (ry) / mapS;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] > 0) { // < Hit wall
                mv = map[mp];
                disV = dist(px, py, rx, ry, ra);
                dof = 8;
            } else { // < Next line
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }
        vx = rx;
        vy = ry;

        // Check horizontal lines
        dof = 0;
        float disH = 1000000;
        tan = 1.0 / tan;
        if (sin(degToRad(ra)) > 0.001) { // < Looking up
            ry = (((int) py / mapS) * mapS) - 0.0001;
            rx = (py - ry) * tan + px;
            yo = -mapS;
            xo = -yo * tan;
        } else if (sin(degToRad(ra)) < -0.001) { // < Looking down
            ry = (((int) py / mapS) * mapS) + mapS;
            rx = (py - ry) * tan + px;
            yo = mapS;
            xo = -yo * tan;
        } else { // < Looking straight left or right
            rx = px;
            ry = py;
            dof = 8;
        }
        while (dof < 8) {
            mx = (int) (rx) / mapS;
            my = (int) (ry) / mapS;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] > 0) { // < Hit wall
                mh = map[mp];
                disH = dist(px, py, rx, ry, ra);
                dof = 8;
            } else { // < Next line
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        // Ray mini map
        int m;
        char c;
        if (disV < disH) { // < Vertical wall hit
            rx = vx;
            ry = vy;
            disT = disV;
            m = mv;
            c = 'l';
        } else { // < Horizontal wall hit
            disT = disH;
            m = mh;
            c = 'd';
        }

        // Save ray
        StructRay currentRay = {
            px, py, // startX, startY
            rx, ry, // endX, endY
            ra, // ra
            disT, // disV or disH
            m, // mv or mh
            c, // 'l' or 'd'
        };
        raysToDraw[r] = currentRay;

        // Increase angle for next ray
        ra = fixAng(ra - (0.5/raysFactor));
    }
}

// Startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("[TO_FIND]", SCREEN_W, SCREEN_H, 0);

    renderer = SDL_CreateRenderer(window, NULL);

    // Init. player
    px = mapS * 1.2;
    py = mapS * 1.2;
    pa = 0;
    pdx = cos(degToRad(pa));
    pdy = -sin(degToRad(pa));
    pspeed = 0.025;
    pspeedrot = 0.3;

    return SDL_APP_CONTINUE;
}

// Events (mouse input, keypresses)
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    // Exit
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    // Key down
    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode sym = event->key.key;
        if (sym == SDLK_ESCAPE) { return SDL_APP_SUCCESS; }
        if (sym == SDLK_Z) { pkeys.z = 1; }
        if (sym == SDLK_Q) { pkeys.q = 1; }
        if (sym == SDLK_S) { pkeys.s = 1; }
        if (sym == SDLK_D) { pkeys.d = 1; }
        if (sym == SDLK_M) {
            showMiniMap = !showMiniMap;
        }
    }

    // Key up
    if (event->type == SDL_EVENT_KEY_UP) {
        SDL_Keycode sym = event->key.key;
        if (sym == SDLK_Z) { pkeys.z = 0; }
        if (sym == SDLK_Q) { pkeys.q = 0; }
        if (sym == SDLK_S) { pkeys.s = 0; }
        if (sym == SDLK_D) { pkeys.d = 0; }
    }

    return SDL_APP_CONTINUE;
}

float frame1, frame2, fps;

// Once per frame
SDL_AppResult SDL_AppIterate(void *appstate)
{
    {
        // Frames per second
        frame2 = SDL_GetTicks();
        fps = frame2 - frame1;
        frame1 = SDL_GetTicks();

        movePlayer(fps);

        computeRays();
    } // < Update

    {
        SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        // ^ Clean canvas

        drawRays3D();

        if (showMiniMap) {
            drawMap2D();
            drawRays2D();
            drawPlayer();
        }

        SDL_RenderPresent(renderer); // < Put on screen
    } // < Draw

    return SDL_APP_CONTINUE;
}

// Shutdown
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}
