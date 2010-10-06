/* Generated by BMP2C from: greenball.bmp */
#include "bitmap.h"

static SDL_Color colors [20] = {
    { 0,0,0,0 },
    { 82,148,82,0 },
    { 82,156,82,0 },
    { 82,165,82,0 },
    { 82,173,82,0 },
    { 82,181,82,0 },
    { 82,189,82,0 },
    { 82,198,82,0 },
    { 82,206,82,0 },
    { 82,214,82,0 },
    { 82,222,82,0 },
    { 82,231,82,0 },
    { 82,239,82,0 },
    { 82,247,82,0 },
    { 82,255,82,0 },
    { 107,181,107,0 },
    { 107,239,107,0 },
    { 123,181,123,0 },
    { 123,206,123,0 },
    { 255,255,255,0 }
};

static Uint8 pixels [16][16] = {
    { 0,0,0,0,19,19,19,19,19,19,19,19,0,0,0,0 },
    { 0,0,19,19,12,13,13,12,11,9,7,5,19,19,0,0 },
    { 0,19,16,13,14,14,14,14,13,11,9,7,5,17,19,0 },
    { 0,19,13,14,14,14,14,14,14,13,11,8,6,3,19,0 },
    { 19,12,14,14,14,14,14,14,14,13,12,9,6,4,1,19 },
    { 19,13,14,14,14,14,14,14,14,14,12,9,7,4,1,19 },
    { 19,13,14,14,14,14,14,14,14,14,12,9,7,4,1,19 },
    { 19,13,14,14,14,14,14,14,14,13,11,9,6,3,1,19 },
    { 19,12,13,14,14,14,14,14,14,12,10,8,5,3,1,19 },
    { 19,10,12,13,14,14,14,13,12,11,9,7,4,2,2,19 },
    { 19,8,10,11,12,12,12,12,11,9,7,5,3,1,3,19 },
    { 19,6,7,9,10,10,10,9,8,7,5,3,1,2,5,19 },
    { 0,19,5,6,7,7,7,7,6,4,3,1,2,4,19,0 },
    { 0,19,15,3,4,4,4,4,3,2,1,2,4,18,19,0 },
    { 0,0,19,19,2,2,2,1,1,1,3,5,19,19,0,0 },
    { 0,0,0,0,19,19,19,19,19,19,19,19,0,0,0,0 }
};

extern bitmap greenball_bitmap;
bitmap greenball_bitmap = {
    16 /* width */, 16 /* height */,
    8 /* depth */, 16 /* pitch */,
    20 /* ncolors */,
    1 /* has_key */, 0 /* key */,
    colors, pixels };

