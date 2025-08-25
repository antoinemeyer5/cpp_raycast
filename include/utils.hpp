#ifndef HPP_UTILS
#define HPP_UTILS

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // < One degree in radians

static const float degToRad(float a)
{
    return a * M_PI / 180.0;
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

#endif
