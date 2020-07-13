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

#include "m3d.h"
#include <stddef.h>

#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <math.h>

#include <string.h>
#include <assert.h>

struct DrawParams defParams = { 
	false, 				// 	juliaSet
    0, 					// 	juliaRe
    0, 					// 	juliaIm
    					// 	0.94, 	// 	base
    0.92, 				//	0.91, 	// 	stretchRatio
  						// 	-0.8, 	//	1.5+3.14, // sampleRotation
  						//  -0.8, 	// 0, //skew
    -0.8,
    -0.8,
    0, 					// 	tilt
#ifdef DEMO_VERSION
    5, // xCenter
#else
    -0.104869104079861,
#endif
    -0.92806604397287, 	//	-0.928066, // yCenter
    6e-011, 			//	0.0000000003, // xRadius
    0, 					//	long double xCenter2;
    0, 					//	long double yCenter2;
    0, 					//	long double xRadius2;
    3, 					//	2.5631439349880, //double max_radius;
    dmOnePoint, 		// 	drawing method
    0, 
	0, 					// 	steps
};

struct ShapeParams defShapeParams = {
    0, 		// 	estimated number of points;
    0, 		// 	real number of points
    420, 	// 	y_range
    9.555, 	//	double alpha_range;
    0, 		//	double delta_y ;
    0, 		//	double delta_alpha ;
    260.0, 	//	r,
    59.0, 	//	l;
    0, 		//  coef
    true 	//	visible
};

struct DrawParams currentParams;
struct ShapeParams currentShape ;

static void GetJuliaSetParams (int juliaIndex, long double &juliaRe, long double &juliaIm)
{
	juliaIndex = juliaIndex % 4;
    
	if (juliaIndex == 0)
    {
		juliaRe = 0.3245;
		juliaIm = 0.04855;
    }
	else if (juliaIndex == 1)
    {
		juliaRe = -0.4961;
		juliaIm = 0.5432;
    }
	else if (juliaIndex == 2)
    {
		juliaRe = 0.300283;
		juliaIm = 0.48857;
    }
	else if (juliaIndex == 3)
    {
		juliaRe = -0.744819;
		juliaIm = -0.105094;
    }
}


static long GetColor (long color, long col, int &red, int &green, int &blue)
{
	long r, g, b;
	r = color & 0xff;
	color >>= 8;
	g = color & 0xff;
	color >>= 8;
	b = color & 0xff;
    
	r = r * col * 0.01; if (r > 255) r = 255;
	g = g * col * 0.01; if (g > 255) g = 255;
	b = b * col * 0.01; if (b > 255) b = 255;
    
	color = r + (g << 8) + (b << 16);
	red = (int)r;
	green = (int)g;
	blue = (int)b;
    
	return color;
}

DWORD M3DDrawer::GetExactColor (int cnt, int fraction)
{
	long color;
    
	if (cnt > 0 && cnt < 256)
    {
        cnt = fraction + ((cnt - 1) << 4) ;
        color = (cnt & 255) ;
        cnt = cnt >> 4;
    }
    else
        color = (cnt & 15) << 4 ;
    
    color += ((cnt & 48) << 10);
    
    color += ((cnt & 192) << 16);
    
    return alternate_palette ? TransformColor (color, pal_case) : color;
}


long M3DDrawer::TransformColor (long color, int pal_case)
{
	int red, green, blue;

	red = color & 0xff;
	green = (color >> 8) & 0xff;
	blue = (color >> 16) & 0xff;
 
	if (((blue>>6) > 0) && ((blue >> 6) < 4) && green==0)
    {
		green = red >> 2;
        
		if (pal_case == 0)
        {
			blue -= (63 - (red >> 2));
        }
		else if (pal_case == 1)
        {
			if ((blue >> 6) == 3)
				blue -= (63 - (red >> 2));
			else if ((blue >> 6) == 1)
				blue += (63 - (red >> 2));
        }
		else if (pal_case == 2)
        {
			if ((blue >> 6) == 3)
				blue += (63 - (red >> 2));
			else if ((blue >> 6) == 1)
				blue -= (63 - (red >> 2));
        }
        
		red = 0;
		
		color = red + (blue << 16) + (green << 8);
		//green += (red >> 1);
    } 
    
	return color;
}

DWORD M3DDrawer::Julia(long double juliaRe, long double juliaIm,
			long double Re, long double Im, int &mandel, int &fraction)
{
    int cnt=0;
    long double cRe, cIm, tRe, tIm;
    long double cRe2, cIm2, last=0, final;
    
    cRe = Re, cIm = Im;
    do {
        cRe2 = cRe * cRe;
        cIm2 = cIm * cIm;
        final = cRe2 + cIm2;
        
        if ((final <= 4.0) && cnt < 256)
		{
            last = final;
            tRe = cRe2 - cIm2;
            tIm = 2 * cRe * cIm;
            cRe = tRe + juliaRe, cIm = tIm + juliaIm;
            cnt++;
		}
        else break;
    } while (1);
    
    long color=0;
    
    fraction = 0;
    mandel = cnt;
    
    if (cnt > 0 && cnt < 256)
    {
        fraction = 16 * (4.0 - last)/(final - last) ;
    }
    
    color = GetExactColor (cnt, fraction);
    
    return color;   
}

DWORD M3DDrawer::Strange(long double Re, long double Im, int &mandel, int &fraction)
{
    int cnt=0;
    long double cRe, cIm, tRe, tIm;
    long double cRe2, cIm2, last=0, final;
    
    cRe = Re, cIm = Im;
    do {
        cRe2 = cRe * cRe;
        cIm2 = cIm * cIm;
        final = cRe2 + cIm2;
        
#ifdef DEMO_VERSION
		if ((final <= 4.0) && cnt < 256)
#else
            if ((final <= 4.0) && cnt < 256)
#endif
            {
                last = final;
                tRe = cRe2 - cIm2;
                tIm = 2 * cRe * cIm;
                cRe = tRe + Re, cIm = tIm + Im;
                cnt++;
            }
            else break;
    } while (1);
    
    long color = 0;
    
    fraction = 0;
    mandel = cnt;
    
    if (cnt > 0 && cnt < 256)
    {
        fraction = 16 * (4.0 - last)/(final - last) ;
    }
    color = GetExactColor (cnt, fraction);
    
    return color;   
}

M3DDrawer::M3DDrawer (float w, float h)
{
    client_width = w;
    client_height = h;

	assert (client_width > 0 && client_height > 0);
    
    bright_k = 1;
    
    alternate_palette = false;
    
    pal_case = PALETTE_STD;
    
    m_total_points = 0;
    
#ifdef USE_RUNTIME
    set_gamma = set_tilt = 0;
#endif
    
    rotationArray = NULL;
    spArray = NULL;
	InitRotationArray ();
}

/*-------------------------------------------------------*/

M3DDrawer::~M3DDrawer (void)
{    
    delete[] rotationArray;
    delete[] spArray;   
}

/*-------------------------------------------------------*/

bool M3DDrawer::GetParamsByIndex (int index, struct DrawParams &dp, struct ShapeParams &sp)
{
    index = (index % PRESET_COUNT);
    
    memcpy (&dp, &rotationArray[index], sizeof (struct DrawParams));
    memcpy (&sp, &spArray[index], sizeof (struct ShapeParams));
    
	return true;
}

/*-------------------------------------------------------*/
void M3DDrawer::SetSaverParams (struct DrawParams *dp, struct ShapeParams *sp)
{
	memcpy (&currentParams, dp, sizeof (struct DrawParams));
	memcpy (&currentShape, sp, sizeof (struct ShapeParams));
}
/*-------------------------------------------------------*/
void M3DDrawer::SetDefaultPrams (void)
{
	SetSaverParams (&defParams, &defShapeParams);
    
}
/*-------------------------------------------------------*/
void M3DDrawer::SetAltPalette (bool isAlternative, int index)
{
    alternate_palette = isAlternative;
    pal_case = (ColorPalette)(index % 3);
}

/*-------------------------------------------------------*/
void M3DDrawer::DisableBrightness ()
{
    bright_k = 0;
}
/*-------------------------------------------------------*/

bool M3DDrawer::Paint (int set_index, bool preview, struct pointData ** pointData,
                      struct corePointData ** corePoints)
{
    bool ret = false;

	if (preview)
    {
		struct DrawParams previewDraw;
		struct ShapeParams previewShape;
        
		memcpy (&previewDraw, &defParams, sizeof (struct DrawParams));
		memcpy (&previewShape, &defShapeParams, sizeof (struct ShapeParams));
        
        GetParamsByIndex (set_index, previewDraw, previewShape);
                
		ret = ShowInPreviewWindow (&previewShape, &previewDraw, pointData, corePoints);
    }
	  else
    {
		// draw_zpr (hdc, &currentShape, &currentParams);
    }

  return (ret);
}

/*-------------------------------------------------------*/

long M3DDrawer::SetGlobals (struct ShapeParams *sp, struct DrawParams *dp)
{
    //double k = 1.414213562373095048801688724209698078;
	double k = sqrt (2.0) ;

	double incircle = (floor)(k * (2 * M_PI * sp->l) + 0.5);
	double cycles = k * hypot ((sp->r + sp->l) * sp->alpha_range, sp->y_range);
    
    exposed_y_range = sp->y_range / sp->r;
    
	//double cycles_in2pi = 2 * M_PI * cycles / sp->alpha_range;
    
    // Jan 2015 we need this for the mesh:
    m_points_in_cycle = (int)incircle;
    
    /*  if ((m_points_in_cycle & 1) != 0)
    {
        m_points_in_cycle++;
        incircle = m_points_in_cycle;
    } */
    
	sp->total_points = incircle * cycles;
    
	sp->delta_y = sp->y_range / sp->total_points;
	sp->delta_alpha = sp->alpha_range / sp->total_points;
	//sp->coef = hypot (2*PI * (sp->r + sp->l), sp->y_range * (2 * PI/sp->alpha_range)) ;
    
	//sp->coef = 2 * PI * (sp->l + sp->r);
	sp->coef = 2 * M_PI * cycles / sp->alpha_range;
	//sp->coef = cycles * PI * 2 * sp->l/ sp->alpha_range; //cycles_in2pi;
    
	if (dp->method == 0)
    {
		if (dp->max_radius < 0)
			dp->max_radius = dp->xRadius / (pow ((double)POWER_BASE, (double)(POWER_RANGE + POWER_ADD) * dp->stretchRatio));
		else //f (dp->stretchRatio == 0)
        {
			double t = log (dp->max_radius / dp->xRadius);
			dp->stretchRatio = (t / log (1/POWER_BASE)) / (POWER_RANGE + POWER_ADD);
        }
    }
    
	if (dp->method == 1) // two points
    {
		dp->steps1 = log (dp->max_radius / dp->xRadius) / log (1/POWER_BASE) - POWER_ADD;
		dp->steps2 = log (dp->max_radius / dp->xRadius2) / log (1/POWER_BASE) - POWER_ADD;
    }
    
    m_total_points = sp->total_points;
    
	return sp->total_points;
}

/*-------------------------------------------------------*/

void M3DDrawer::SetMaterialType (TMaterialOption option)
{    
	if (option == moBoth)
    {
		option = ((rand() & 1)==0) ? moPlastic : moMetal;
    }
    
	if (option == moPlastic)
    {
		k1 = 40; k2 = 70; k3 = 46; k_pow = 3; // light coefficients
    }
	else
    {
		k1 = 10, k2 = 70, k3 = 30, k_pow = 13; // light coefficients
    }
}

/*-------------------------------------------------------*/

long M3DDrawer::GetMandelColor (struct DrawParams &params, struct ShapeParams &sp,
								long double alpha, long double alp, int &mandel, int &fraction)
{
	long double xCenter = params.xCenter,
    			yCenter = params.yCenter, 
    			xRadius = params.xRadius;
    
	long color = 0;
	long double Re, Im;
    
	double power = POWER_ADD + POWER_RANGE * (alpha/sp.alpha_range);
        
	Re = xRadius * cos (alp + params.sampleRotation);
	Im = xRadius * sin (alp + params.sampleRotation);
    
	double k = pow (POWER_BASE, power * params.stretchRatio);
    
	Re = Re / k;
	Im = Im / k;
    
	Re += xCenter;
	Im += yCenter;
    
	if (params.juliaSet)
    {
		color = Julia(params.juliaRe, params.juliaIm, Re, Im, mandel, fraction);
    }
	else
    {
		color = Strange(Re, Im, mandel, fraction);
    }
    
	return color;
}

/*-------------------------------------------------------*/

long M3DDrawer::GetMandelColor3 (struct DrawParams &params, struct ShapeParams &sp,
                                 long double alpha, long double alp, int &mandel, int &fraction)
{
    long double xRadius = params.xRadius;
    
	long double currentRadius;
	double k ;
	double deltaRe = (params.xCenter2 - params.xCenter);
	double deltaIm = (params.yCenter2 - params.yCenter);
	double half_dist = sqrt (deltaRe * deltaRe + deltaIm * deltaIm) / 2;
	double ellipse_half_dist ;
	double horizRadius;
	double xDelta = 0, yDelta = 0, ellipseXCenter, ellipseYCenter;
	double cos_angle = 0.0, sin_angle = 1.0;
	double focus_dist = 2 * sqrt (half_dist * half_dist + params.max_radius * params.max_radius);
	double a = focus_dist / 2;
	double height = params.max_radius; // a always >= than b.
    
	long color = 0;
	long double Re, Im, tRe, tIm;
	long double magic = (sp.alpha_range) * params.steps1 / (params.steps1 + params.steps2);
    
	if (alpha < magic)
    {
		k = pow ((double)POWER_BASE, (double)(POWER_ADD + params.steps1 * alpha / magic));
		currentRadius = xRadius / k;
		horizRadius = currentRadius * a / height;
		ellipse_half_dist = half_dist * currentRadius / params.max_radius;
        
		if (half_dist > 0)
        {
			xDelta = ellipse_half_dist * deltaRe / (2 * half_dist);
			yDelta = ellipse_half_dist * deltaIm / (2 * half_dist);
        }
        
		ellipseXCenter =  params.xCenter + xDelta;
		ellipseYCenter =  params.yCenter + yDelta;
        
		tRe = horizRadius * cos (alp + params.sampleRotation);
		tIm = currentRadius * sin (alp + params.sampleRotation);
        
		if (half_dist > 0)
        {
			cos_angle = deltaRe / (2 *half_dist);
			sin_angle = deltaIm / (2 *half_dist);
            
			Re = tRe * cos_angle - tIm * sin_angle;
			Im = tRe * sin_angle + tIm * cos_angle;
        }
		else
        {
			Re = tRe ;
			Im = tIm ;
        }
        
		Re += ellipseXCenter; //currentParams.xCenter;
		Im += ellipseYCenter; //currentParams.yCenter;
    }
	else
    {
		k = pow ((double)POWER_BASE, (double)(POWER_ADD + params.steps2 * (sp.alpha_range - alpha) / (sp.alpha_range - magic)));
        
		currentRadius = params.xRadius2 / k;
		horizRadius = currentRadius * a / height;
		ellipse_half_dist = half_dist * currentRadius / params.max_radius;
        
		if (half_dist > 0)
        {
			xDelta = ellipse_half_dist * deltaRe / (2 * half_dist);
			yDelta = ellipse_half_dist * deltaIm / (2 * half_dist);
        }
        
		ellipseXCenter =  params.xCenter2 - xDelta;
		ellipseYCenter =  params.yCenter2 - yDelta;
        
		tRe = horizRadius * cos (alp + params.sampleRotation);
		tIm = currentRadius * sin (alp + params.sampleRotation);
        
		if (half_dist > 0)
        {
			cos_angle = deltaRe / (2 * half_dist);
			sin_angle = deltaIm / (2 * half_dist);
            
			Re = tRe * cos_angle - tIm * sin_angle;
			Im = tRe * sin_angle + tIm * cos_angle;
        }
		else
        {
			Re = tRe ;
			Im = tIm ;
        }
        
		Re += ellipseXCenter; //currentParams.xCenter;
		Im += ellipseYCenter; //currentParams.yCenter;
    }
    
	if (params.juliaSet)
    {
		color = Julia (params.juliaRe, params.juliaIm, Re, Im, mandel, fraction);
    }
	else
    {
		color = Strange(Re, Im, mandel, fraction);
    }
    
	return color;
}

/*-------------------------------------------------------*/

long M3DDrawer::getPointCount (int & inOneCycle)
{
    inOneCycle = m_points_in_cycle;
    
    return m_total_points;
}

/*-------------------------------------------------------*/

void M3DDrawer::GetPixelCoordinates (double y1, double r, double l, long double alpha, long double beta,
                                     long double coef, double slope,
                                     double & x_real, double & y_real, double & z_real,
                                     double & x_norm, double & y_norm, double & z_norm)
{
	double temp_x ;
    
	x_real = r * cos (alpha);
	y_real = y1 ;
	z_real = r * sin (alpha);
    
	// initial values (assuming l=1)
	// x_norm = cos (coef*alp);
	// y_norm = sin (coef*alp);
    
    x_norm = cos (beta);
    y_norm = sin (beta);
    
	z_norm = 0;
    
	// first, slope (around x=0)
	z_norm = -y_norm * sin (slope);
	y_norm = y_norm * cos (slope); // ignoring z it's 0 anyway
    
	// turn by alpha (around y=0)
    
	temp_x = x_norm;
	x_norm = x_norm * cos(alpha) - z_norm * sin (alpha);
	//y_norm = y_norm;
	z_norm = temp_x * sin (alpha) + z_norm * cos (alpha);
    
	// add to centerpoints
	x_real += x_norm * l;
	y_real += y_norm * l;
	z_real += z_norm * l;
}

/*-------------------------------------------------------*/
#ifdef USE_RUNTIME

void M3DDrawer::SetGamma (double gamma, double tilt)
{
    set_gamma = gamma;
    set_tilt = tilt;
}

#endif

/*-------------------------------------------------------*/

void M3DDrawer::SetLightDirection (double x, double y, double z)
{
    light_dir_x = x, light_dir_y = y, light_dir_z = z;
}
/*-------------------------------------------------------*/

bool M3DDrawer::ShowInPreviewWindow (struct ShapeParams *sp, struct DrawParams *dp,
                                     struct pointData ** pointData,
                                     struct corePointData ** corePoints)
{
	double x_real, y_real, z_real;
	unsigned long raw_color = 0;
	int mandel = 0, fraction = 0;
	double l = sp->l, r = sp->r;
	long col = 0;
	double y_median = sp->y_range / 2;
	int red, green, blue;
    
	float bright = 0, bright0 = 0; // brightness
	long double y1 = 0, alp = 0;
    
#ifdef USE_RUNTIME
	double tilt = set_tilt;
	double cos_tilt, sin_tilt;
#endif

    //	struct MiniMax minimax;
	double slope = atan (sp->y_range / (sp->alpha_range * sp->r));
	double x_norm, y_norm, z_norm;
    
	// trying to allocate space for 640*350=224000 pixels
	// space is neccesary to store z-cordinates
    
    double light_x = light_dir_x, light_y = light_dir_y, light_z = light_dir_z;
    
    double normalizer = sqrt (light_x * light_x + light_y * light_y + light_z * light_z);
	light_x /= normalizer;
	light_y /= normalizer;
	light_z /= normalizer;
    
#ifdef USE_RUNTIME
	  cos_tilt = cos (tilt);
	  sin_tilt = sin (tilt);
#endif
    
	SetMaterialType (moBoth) ; //Metal);    
    
	divisor = sqrt (800.0f * 600.0f / ((client_width) * (client_height)));
    
	sp->r /= divisor;
	sp->l /= divisor;
	sp->y_range /= divisor;
	y_median = y_median / divisor;
	l = sp->l, r = sp->r;
    
	SetGlobals (sp, dp);
        
	pix_count = 0;
    
    *pointData = new struct pointData [sp->total_points];
    
    long core_total = (sp->total_points/m_points_in_cycle);
    
    *corePoints = new struct corePointData [core_total];
    
    if (pointData == NULL || corePoints == NULL)
    {
        m_total_points = 0;
		    return (false);
    }
    
    memset(*corePoints, 0, sizeof(struct corePointData) * core_total);
        
    pix_count = 0;
    
    int core_cnt = 0;
    
    // initial y1 is 0;
    // r is
	while ((alp < sp->alpha_range) && (pix_count < sp->total_points))
    {
        long double alp2 = (((int)(pix_count / m_points_in_cycle)) * m_points_in_cycle) * sp->delta_alpha;
        
        long double beta = (pix_count % m_points_in_cycle) * (sp->delta_alpha * sp->coef);
        
        y1 = ((pix_count / m_points_in_cycle) * m_points_in_cycle) * sp->delta_y;
        
        GetPixelCoordinates (y1, r, l, alp2, beta, sp->coef, slope,
                             x_real, y_real, z_real,
                             x_norm, y_norm, z_norm);
        
        struct pointData *pd = &((*pointData)[pix_count]);
        
        if ((pix_count % m_points_in_cycle) == 0 /*half_cycle*/ && (core_cnt < core_total))
        {
            struct corePointData * cpd = &((*corePoints)[core_cnt]);
            
            alp2 = ((((pix_count / m_points_in_cycle) + 0.5f) * (double)m_points_in_cycle) + 0.5f) * sp->delta_alpha;
            
            //double extra = 0.5f / m_points_in_cycle;
            
            y1 = ((((pix_count / m_points_in_cycle) + 0.5f) * (double)m_points_in_cycle) + 0.5f) * sp->delta_y;
            
            double x_real2, y_real2, z_real2;
            double x_norm2, y_norm2, z_norm2;
            
            GetPixelCoordinates (y1, r, l, alp2, 0, sp->coef, slope,
                                 x_real2, y_real2, z_real2,
                                 x_norm2, y_norm2, z_norm2);

            
            cpd->x = (x_real2 - x_norm2 * l) / r;
            cpd->y = (y_real2 - y_norm2 * l) / r;
            cpd->z = (z_real2 - z_norm2 * l) / r;
            
            cpd->index = (int)pix_count;
            
            core_cnt++;
        }
        
        pd->x = x_real / (r);
        pd->y = y_real / (r);
        pd->z = z_real / (r);
        
        pd->normx = x_norm;
        pd->normy = y_norm;
        pd->normz = z_norm;        
                
        if (true)
        {            
            if (bright_k == 1)
            {
                bright0 = CalcBrightness (light_x, light_y, light_z, x_norm, y_norm, z_norm, 0, 0, 1);
            }
            else
            {
                bright0 = 50;
            }            
            
            // all following manipulations are to smooth the surface:
            // if actual brightness is too high, part of it goes as a bonus
            // to the next point
            
            bright += bright0;
            
            if (bright > 0)
            {
                col = (int)(bright + 0.5);
                
                if (col > 150)
                    col = 150;
                
                // SetPixel(hdc, xs, ys, color);
                if (dp->method == dmOnePoint)
                {
                    raw_color = GetMandelColor (*dp, *sp, alp, alp * sp->coef + alp * currentParams.skew, mandel, fraction);
                }
                else
                {
                    raw_color = GetMandelColor3 (*dp, *sp, alp, alp * sp->coef + alp * currentParams.skew, mandel, fraction);
                }
                
                GetColor (raw_color, col, red, green, blue);
                                
                if (raw_color != 0)
                {                    
                    pd->ucRed = red ;
                    pd->ucBlue = blue ;
                    pd->ucGreen = green ;
                }
                else
                {
                    pd->ucRed = 0;
                    pd->ucBlue = 0;
                    pd->ucGreen = 0;
                }
            }
            bright-=col;
        }
        
        y1 += sp->delta_y ;
        alp += sp->delta_alpha ;
        pix_count++;
    }
    
	pix_count = 0;

    return (true);
}

/*-------------------------------------------------------*/
void M3DDrawer::SetPointColor (unsigned char *array, long which, int mandel, int fraction)
{
	long index;
    
	if (0 == (which & 1)) // even
    {
		index = (which * 3)>>1;
		array [index] = mandel;
		array [index+1] = (array [index+1] & 0xf0) | (fraction & 0x0f);
    }
	else
    {
		index = (which * 3 + 1)>>1;
		array [index] = mandel;
		array [index - 1] = (array [index - 1] & 0x0f) | ((fraction << 4) & 0xf0);
    }	
}

/*-------------------------------------------------------*/

float M3DDrawer::CalcBrightness (float light_x, float light_y, float light_z,
                      float x_norm, float y_norm, float z_norm,
                      float view_x, float view_y, float view_z)
{
	float cosg, cosf, bright = 0; // brightness
        
	cosf = (light_x * x_norm + light_y * y_norm + light_z * z_norm);
    
	if (cosf == 0)
    {
		cosf = cosg = 0;
    }
	else
    {
		// in generic case:
		// Refl_vector = -Light_vector + 2 * Norm_vector * (light_v * norm_v)
		// but light_v * norm_v is already calculated : it is cosf
		// Refl_vector = -Light_vector + 2 * Norm_vector * cosf
		// cosg is Refl_vector * viewpoint_vector (which by default is (0,0,1))
		// so :
		double refl_x, refl_y, refl_z;
		refl_x = -light_x + (2 * x_norm * cosf);
		refl_y = -light_y + (2 * y_norm * cosf);
		refl_z = -light_z + (2 * z_norm * cosf);
		cosg = (refl_x * view_x) + (refl_y * view_y) + (refl_z * view_z);
        
		// this assumes that viewpoint vector is (0, 0, 1)
        //	cosg = 2 * z_norm * cosf - light_z;
		if (cosg < 0)
            cosg=0;
    }
    //	if (cosf < 0)
    //	cosf = 0;
	// 	now, let's figure out the brightness at a given point:
	// 	k1 is diffuse reflection coefficient for Lambert's component (how well surface
	// 	is illuminated. If light is opposite to the normal
	// 	then illumination is maximal
	// 	k2 is specular reflection coefficient (Phong component) it depends on the viewer's eye point
	// 	which translates into v - vectior (0, 0, 1)
	// 	k3 is ambient light coefficient
    
	bright = 10 + 180 * (-k1 * (cosf) + k2 * pow((cosg), k_pow) + k3) / (k1+k2+k3);
	if (bright > 150) 
        bright = 150;
    
	return bright ;
}

////////////////////////////////////////////////////////////////////////
// This was written back a while ago.
// DrawParams control what part of which fractal is painted on the tube
// ShapeParams control parameters of the spiral tube (thickness, etc). 
////////////////////////////////////////////////////////////////////////

void M3DDrawer::InitRotationArray (void)
{
	const int in_rotation = PRESET_COUNT ;

	rotationArray = new struct DrawParams [in_rotation];
	spArray = new struct ShapeParams [in_rotation];
    
	memcpy (&spArray[0], &defShapeParams, sizeof (struct ShapeParams));
	memcpy (&rotationArray[0], &defParams, sizeof (struct DrawParams));
    
	memcpy (&spArray[1], &defShapeParams, sizeof (struct ShapeParams));
	memcpy (&spArray[2], &defShapeParams, sizeof (struct ShapeParams));
	memcpy (&spArray[3], &defShapeParams, sizeof (struct ShapeParams));
	memcpy (&spArray[4], &defShapeParams, sizeof (struct ShapeParams));
    
	//leave as is
	//********************************************************************
	memcpy (&rotationArray[1], &defParams, sizeof (struct DrawParams));
    
	// this one is julia set (picture #2)
	rotationArray[1].xCenter = -0.171035469619938; //0.0300786343590521;
	rotationArray[1].yCenter = -0.318802963959513; //0.520811231249443;
	rotationArray[1].xRadius = 0.000583506991765284; //7.3510528665225e-008;
    
	rotationArray[1].xCenter2 = -rotationArray[1].xCenter;
	rotationArray[1].yCenter2 = -rotationArray[1].yCenter;
	rotationArray[1].xRadius2 = rotationArray[1].xRadius;
    
    //	rotationArray[1].base = 0.94;
	rotationArray[1].method = dmTwoPoints;
    rotationArray[1].juliaSet = true;
	rotationArray[1].max_radius = 0.99;
	spArray[1].alpha_range = 5;
    
	GetJuliaSetParams (0, rotationArray[1].juliaRe, rotationArray[1].juliaIm);
    
	//********************************************************************
	memcpy (&rotationArray[2], &defParams, sizeof (struct DrawParams));
	rotationArray[2].xCenter = -1.0013121704951;
	rotationArray[2].yCenter = -0.304196021963963;
	rotationArray[2].xRadius = 1.0e-008;
    //	rotationArray[2].base = 0.91;
	rotationArray[2].method = dmOnePoint;
	rotationArray[2].xCenter2 = -0.11; //-0.104869104079861;
	rotationArray[2].yCenter2 = 0.0; //-0.92806604397287;
	rotationArray[2].xRadius2 = 0.41; //6e-011;
	rotationArray[2].max_radius = 3; //2.0386550422325;
    
	spArray[2].l = 70; //cx * 7 / 128; //70
	spArray[2].r = 300; //cx * 15 / 64; //300;
	spArray[2].alpha_range /= 1.95;
	spArray[2].y_range = 420; //cy * 21/51;
    
	//spArray[2].delta_y /=1.95;
	//rotationArray[2].stretchRatio /= 1.95;
    
	//********************************************************************
	memcpy (&rotationArray[3], &defParams, sizeof (struct DrawParams));
	spArray[3].l = 80; //cx/16; //80;
	spArray[3].r = 380; //cx * 19/64; //380;
	spArray[3].alpha_range = 2 * M_PI;
	spArray[3].y_range = 1000; //cy/1.024;
	//********************************************************************
	
	memcpy (&rotationArray[4], &defParams, sizeof (struct DrawParams));
    
	rotationArray[4].xCenter = -0.76295980213721;
	rotationArray[4].yCenter = -0.094826839433747;
	rotationArray[4].xRadius = 8.1706250576269e-009;
	rotationArray[4].method = dmTwoPoints;
	rotationArray[4].xCenter2 = -0.76518309844711;
	rotationArray[4].yCenter2 = -0.098669746089906;
	rotationArray[4].xRadius2 = 2.8644189916084e-008;
	rotationArray[4].max_radius = 0.0045964089747317;
    
	spArray[4].l = 70;
	spArray[4].r = 300;
	spArray[4].alpha_range = 1.2 * M_PI;
	spArray[4].y_range = 400;
    
	for (int i = 0; i < in_rotation; i++)
    {
		spArray[i].visible = true;
    }
}