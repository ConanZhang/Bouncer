/*
 * Authors: Charles Khong & Conan Zhang
 * Date: March 25, 2015
 *
 * Description:
 *      Include statements for ffmpeg libraries.
 */

#ifndef BOUNCER_H
#define BOUNCER_H

#include <string.h>
#include <math.h>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

/* Includes from C language */
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#endif

