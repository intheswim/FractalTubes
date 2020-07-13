/* FractalTubes (formerly M3DSaver) screensaver. 
 * Copyright (c) 2003-2020 Yuriy Yakimenko
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#pragma once

#include "AppConsts.h"

#define _USE_MATH_DEFINES

#define PRESET_COUNT 5
#define POWER_ADD 0
#define POWER_BASE 0.9
#define POWER_RANGE 420

enum TMaterialOption { moPlastic = 0, moMetal, moBoth } ;
enum TDrawMethod { dmOnePoint = 0, dmTwoPoints };

struct DrawParams
{
	bool juliaSet;
	long double juliaRe;
	long double juliaIm;
	float stretchRatio;
	float sampleRotation;
	float skew;
	double tilt;
	long double xCenter;
	long double yCenter;
	long double xRadius;
	long double xCenter2;
	long double yCenter2;
	long double xRadius2;
	double max_radius;
	TDrawMethod method;  // 0 for one point, 1 for two points. 
	float steps1, steps2;
};

struct ShapeParams
{
	long    total_points;      // estimated pixel count
	long    pix_count;         // real pixel count
	double  y_range;
	double  alpha_range;
	long double delta_y ;
	long double delta_alpha ;
	double  r;               // r is radius of the central (core) spiral.
    double  l;               // l is radius (thickness) of the tube
	long double coef;
	bool    visible;
};

struct pointData // points on the surface of the spiral
{
    unsigned char ucRed, ucGreen, ucBlue;
    char cMult;
    double x, y, z;
    double normx, normy, normz;
};

struct corePointData // points on the spiral itself (centers of the circes)
{
    double x, y, z;
    double dist;
    int index;
};

typedef unsigned long DWORD;

class M3DDrawer
{
    
private:
    
    // Phong components (used in brightness calculations)
    int k1, k2, k3, k_pow;
    
    double light_dir_x, light_dir_y, light_dir_z;
    
#ifdef USE_RUNTIME
    double set_gamma, set_tilt;
#endif
    
    float exposed_y_range;
    
    float delta_in_radians;
    
    float client_width, client_height;
    float divisor ;
    long pix_count;
    
    struct DrawParams *rotationArray;
	struct ShapeParams *spArray;
    
    long m_total_points;
    int m_points_in_cycle;
    
    int bright_k;
    
    bool alternate_palette;
    
    ColorPalette pal_case;
    
    float CalcBrightness (float light_x, float light_y, float light_z,
                                     float x_norm, float y_norm, float z_norm,
                          float view_x, float view_y, float view_z);

    
    long SetGlobals (struct ShapeParams *sp, struct DrawParams *dp);
    

    bool ShowInPreviewWindow (struct ShapeParams * sp, struct DrawParams *dp,
                              struct pointData ** pointData, struct corePointData ** corePoints);
    
    void SetMaterialType (TMaterialOption option);
    
    void SetSaverParams (struct DrawParams *dp, struct ShapeParams *sp);
    
    void SetPointColor (unsigned char *array, long which, int mandel, int fraction);
    
    long GetMandelColor (struct DrawParams &params, struct ShapeParams &sp,
                         long double alpha, long double alp, int &mandel, int &fraction);
    
    long GetMandelColor3 (struct DrawParams &params, struct ShapeParams &sp,
                          long double alpha, long double alp, int &mandel, int &fraction);
    
    void GetPixelCoordinates (double y1, double r, double l, long double alpha, long double beta,
                                         long double coef, double slope,
                                         double & x_real, double & y_real, double & z_real,
                              double & x_norm, double & y_norm, double & z_norm);


    void InitRotationArray (void);
    bool GetParamsByIndex (int index, struct DrawParams &dp, struct ShapeParams &sp);
    
    static long TransformColor (long color, int pal_case);
    
    DWORD GetExactColor (int cnt, int fraction);
    
    DWORD Strange(long double Re, long double Im, int &mandel, int &fraction);
    
    DWORD Julia(long double juliaRe, long double juliaIm,
                long double Re, long double Im, int &mandel, int &fraction);

    
public:
    M3DDrawer (float w, float h);
    ~M3DDrawer (void);

    void SetDefaultPrams (void);
    void SetAltPalette (bool isAlternative, int index);

    void DisableBrightness ();
    
    void SetLightDirection (double x, double y, double z);
    
    void SetGamma (double v, double v2);
    
    float getYRange (void) { return exposed_y_range; }
    
    bool Paint(int set_index, bool preview, struct pointData ** pointData,
               struct corePointData ** corePoints);
    
    long getPointCount (int & inOneCycle);

};
