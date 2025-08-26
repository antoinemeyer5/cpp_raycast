#ifndef HPP_UTILS
#define HPP_UTILS

// Variables

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // < One degree in radians

// Colors

static const bool BLACK(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

static const bool BLUE(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
}

static const bool GRAY(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 155, 155, 155, SDL_ALPHA_OPAQUE);
}

static const bool GREEN(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
}

static const bool LIGHT_BLUE(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 0, 0, 155, SDL_ALPHA_OPAQUE);
}

static const bool RED(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
}

static const bool LIGHT_RED(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 155, 0, 0, SDL_ALPHA_OPAQUE);
}

static const bool WHITE(SDL_Renderer *renderer)
{
    return SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
}

// Functions

static const float degToRad(float a)
{
    return a * M_PI / 180.0;
}

static const float dist(float ax, float ay, float bx, float by, float ang)
{
    return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

static const float fixAng(float a)
{
    if (a > 359) {
        a -= 360;
    }
    if (a < 0) {
        a += 360;
    }
    return a;
}

static const bool pickColor(SDL_Renderer *renderer, int value, char type)
{
    if (type == 'd') {
        switch (value) {
            case 1: return WHITE(renderer);
            case 2: return RED(renderer);
            default: return BLACK(renderer);
        }
    } else if (type == 'l') {
        switch (value) {
            case 1: return GRAY(renderer);
            case 2: return LIGHT_RED(renderer);
            default: return BLACK(renderer);
        }
    } else {
        return false;
    }
}

#endif
