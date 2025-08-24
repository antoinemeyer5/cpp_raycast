#define SDL_MAIN_USE_CALLBACKS 1
#define SCREEN_W 900
#define SCREEN_H 600

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <math.h>
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // < One degree in radians

static SDL_Renderer *renderer = NULL;

// Player
float px, py;
float pdx, pdy, pa; // < Deltas and angle

void drawPlayer()
{
    // Player body
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderPoint(renderer, px, py);

    // Player direction
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE); // < Green
    SDL_RenderLine(renderer, px, py, px + pdx * 5, py + pdy * 5);
}

void movePlayer(SDL_Keycode sym)
{
    if (sym == SDLK_Z) { px += pdx; py += pdy; }
    if (sym == SDLK_Q) {
        pa -= 0.1;
        if (pa<0) { pa += 2*PI; }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (sym == SDLK_S) { px -= pdx; py -= pdy; }
    if (sym == SDLK_D) {
        pa += 0.1;
        if (pa>2*PI) { pa -= 2*PI; }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
}

// Map
int mapX = 6;
int mapY = 6;
int mapS = 64;

int map[] = {
    1, 1, 1, 1, 1, 1,
    1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 1, 0, 2, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1
};

void drawMap2D()
{
    SDL_FRect rect;
    for (int y = 0; y < mapY; y++) {
        for (int x = 0; x < mapX; x++) {
            if (map[y * mapX + x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
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

// Rays
float dist(float ax, float ay, float bx, float by, float ang)
{
    return ( sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)) );
}

void drawRays3D()
{
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo;
    float disT;

    ra = pa - DR * 30;
    if (ra < 0) {
        ra += 2 * PI;
    }
    if (ra > 2 * PI) {
        ra -= 2 * PI;
    }

    for (r = 0; r < 60; r++) {
        // Check horizontal lines
        dof = 0;
        float disH = 1000000;
        float hx = px;
        float hy = py;
        float aTan = -1 / tan(ra);
        if (ra > PI) { // < Looking up
            ry = (((int) py >> 6) << 6) - 0.0001;
            rx = (py - ry) * aTan + px;
            yo = -64;
            xo = -yo * aTan;
        }
        if (ra < PI) { // < Looking down
            ry = (((int) py >> 6) << 6) + 64;
            rx = (py - ry) * aTan + px;
            yo = 64;
            xo = -yo * aTan;
        }
        if (ra == 0 || ra == PI) { // < Looking straight left or right
            rx = px;
            ry = py;
            dof = 8;
        }
        int mv = 0;
        int mh = 0;
        while (dof < 8) {
            mx = (int) (rx) >> 6;
            my = (int) (ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] > 0) { // < Hit wall
                mh = map[mp];
                hx = rx;
                hy = ry;
                disH = dist(px, py, hx, hy, ra);
                dof = 8;
            } else { // < Next line
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        // Check vertical lines
        dof = 0;
        float disV = 1000000;
        float vx = px;
        float vy = py;
        float nTan = -tan(ra);
        if (ra > P2 && ra < P3) { // < Looking left
            rx = (((int) px >> 6) << 6) - 0.0001;
            ry = (px - rx) * nTan + py;
            xo = -64;
            yo = -xo * nTan;
        }
        if (ra < P2 || ra > P3) { // < Looking right
            rx = (((int) px >> 6) << 6) + 64;
            ry = (px - rx) * nTan + py;
            xo = 64;
            yo = -xo * nTan;
        }
        if (ra == 0 || ra == PI) { // < Looking straight up or down
            rx = px;
            ry = py;
            dof = 8;
        }
        while (dof < 8) {
            mx = (int) (rx) >> 6;
            my = (int) (ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] > 0) { // < Hit wall
                mv = map[mp];
                vx = rx;
                vy = ry;
                disV = dist(px, py, vx, vy, ra);
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
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, SDL_ALPHA_OPAQUE); // < Little Red
            if (mv == 2) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 200, SDL_ALPHA_OPAQUE); // < Little Blue
            }
        }
        if (disH < disV) { // < Horizontal wall hit
            rx = hx;
            ry = hy;
            disT = disH;
            SDL_SetRenderDrawColor(renderer, 100, 0, 0, SDL_ALPHA_OPAQUE); // < Little Red
            if (mh == 2) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 100, SDL_ALPHA_OPAQUE); // < Little Blue
            }
        }
        SDL_RenderLine(renderer, px, py, rx, ry);

        // Draw 3D walls
        float ca = pa - ra;
        if (ca < 0) { ca += 2 * PI; }
        if (ca > 2 * PI) { ca -= 2 * PI; }
        disT = disT * cos(ca);
        // ^ Fix fisheye
        float lineH = (mapS * 320) / disT;
        if (lineH > 320) {
            lineH = 320;
        }
        // ^ Line Height
        float lineO = 160 - lineH / 2; // < Line offset
        SDL_RenderLine(renderer, r * 6 + 430, lineO, r * 6 + 430, lineH + lineO);

        // Increase angle for next ray
        ra += DR;
        if (ra < 0) {
            ra += 2 * PI;
        }
        if (ra > 2 * PI) {
            ra -= 2 * PI;
        }
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

    px = 300;
    py = 300;
    pdx = cos(pa) * 5;
    pdy = sin(pa) * 5;

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
        if (sym == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }

        movePlayer(sym);
    }

    // Mouse
    /*if (event->type == SDL_EVENT_MOUSE_MOTION) {
        float x, y;
        SDL_GetMouseState(&x, &y);
        basket.move(x);
    }*/

    return SDL_APP_CONTINUE;
}

// Once per frame
SDL_AppResult SDL_AppIterate(void *appstate)
{
    {
        // basket.update();
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
