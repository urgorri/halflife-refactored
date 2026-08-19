#pragma once

#ifndef UTIL_MATH_H
#define UTIL_MATH_H

#include "core/vector.h"

extern float UTIL_VecToYaw( const Vector &vec );
extern Vector UTIL_VecToAngles( const Vector &vec );
extern float UTIL_AngleMod( float a );
extern float UTIL_AngleDiff( float destAngle, float srcAngle );

extern float UTIL_Approach( float target, float value, float speed );
extern float UTIL_ApproachAngle( float target, float value, float speed );
extern float UTIL_AngleDistance( float next, float cur );

extern Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );
extern float UTIL_SplineFraction( float value, float scale );

extern float UTIL_DotPoints( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir );

int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

#endif // UTIL_MATH_H
