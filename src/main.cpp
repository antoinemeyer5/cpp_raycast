#define SDL_MAIN_USE_CALLBACKS 1
#define SCREEN_W 900
#define SCREEN_H 600

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <math.h>

#include "utils.hpp"

static SDL_Renderer *renderer = NULL;

// Map
#define mapX 10
#define mapY 10
#define mapS 32

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 2, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

void drawMap2D()
{
    SDL_FRect rect;
    for (int y = 0; y < mapY; y++) {
        for (int x = 0; x < mapX; x++) {
            if (map[y * mapX + x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            } else if (map[y * mapX + x] == 2) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            }

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
float pspeed;

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
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE); // < Green
    SDL_RenderLine(renderer, px, py, px + pdx * 20, py + pdy * 20);
}

void movePlayer(float fps)
{
    if (pkeys.q == 1) {
        pa += pspeed * fps;
        pa = fixAng(pa);
        pdx = cos(degToRad(pa));
        pdy = -sin(degToRad(pa));
    }
    if (pkeys.d == 1) {
        pa -= pspeed * fps;
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
float dist(float ax, float ay, float bx, float by, float ang)
{
    return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

void drawRays3D()
{
    int r, mx, my, mp, dof;
    float vx, vy, rx, ry, ra, xo, yo;
    float disT;

    ra = fixAng(pa + 30);

    for (r = 0; r < 60; r++) {
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

        // Draw ray
        if (disV < disH) { // < Vertical wall hit
            rx = vx;
            ry = vy;
            disT = disV;
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, SDL_ALPHA_OPAQUE); // < Little white
            if (mv == 2) { // < Little Blue
                SDL_SetRenderDrawColor(renderer, 0, 0, 150, SDL_ALPHA_OPAQUE);
            }
        } else { // < Horizontal wall hit
            disT = disH;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // < White
            if (mh == 2) { // < Blue
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
            }
        }
        SDL_RenderLine(renderer, px, py, rx, ry);

        // Draw 3D walls
        float ca = fixAng(pa - ra);
        disT = disT * cos(degToRad(ca));
        // ^ Fix fisheye
        float lineH = (mapS * 320) / disT;
        if (lineH > 320) {
            lineH = 320;
        }
        // ^ Line Height
        float lineO = 180 - lineH / 2; // < Line offset
        SDL_RenderLine(renderer, r * 6 + 430, lineO, r * 6 + 430, lineH + lineO);

        // Increase angle for next ray
        ra = fixAng(ra - 1);
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

    px = mapS * 1.2;
    py = mapS * 1.2;
    pa = 0;
    pdx = cos(degToRad(pa));
    pdy = -sin(degToRad(pa));
    pspeed = 0.1;

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
    } // < Update

    {
        SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        // ^ Clean canvas

        drawMap2D();
        drawRays3D();
        drawPlayer();

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
