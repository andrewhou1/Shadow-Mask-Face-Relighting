
#ifndef _JITTER_H
#define _JITTER_H

#include "GrPoint.h"

const CGrPoint JITTER1[] = {CGrPoint(0.5, 0.5, 0.0)};
const CGrPoint JITTER2[] = {CGrPoint(0.25, 0.75), 
                            CGrPoint(0.75, 0.25)};
const CGrPoint JITTER3[] = {CGrPoint(0.5033922635, 0.8317967229), 
                            CGrPoint(0.7806016275, 0.2504380877),
                            CGrPoint(0.2261828938, 0.4131553612)};
const CGrPoint JITTER4[] = {CGrPoint(0.375, 0.25), 
                            CGrPoint(0.125, 0.75), 
                            CGrPoint(0.875, 0.25), 
                            CGrPoint(0.625, 0.75)};
const CGrPoint JITTER5[] = {CGrPoint(0.5, 0.5), 
                            CGrPoint(0.3, 0.1), 
                            CGrPoint(0.7, 0.9), 
                            CGrPoint(0.9, 0.3), 
                            CGrPoint(0.1, 0.7)};
const CGrPoint JITTER6[] = {CGrPoint(0.4646464646, 0.4646464646), 
                            CGrPoint(0.1313131313, 0.7979797979),
                            CGrPoint(0.5353535353, 0.8686868686), 
                            CGrPoint(0.8686868686, 0.5353535353),
                            CGrPoint(0.7979797979, 0.1313131313), 
                            CGrPoint(0.2020202020, 0.2020202020)};
const CGrPoint JITTER8[] = {CGrPoint(0.5625, 0.4375), 
                            CGrPoint(0.0625, 0.9375), 
                            CGrPoint(0.3125, 0.6875), 
                            CGrPoint(0.6875, 0.8125),  
                            CGrPoint(0.8125, 0.1875), 
                            CGrPoint(0.9375, 0.5625), 
                            CGrPoint(0.4375, 0.0625), 
                            CGrPoint(0.1875, 0.3125)};
const CGrPoint JITTER9[] = {CGrPoint(0.5, 0.5), 
                            CGrPoint(0.1666666666, 0.9444444444), 
                            CGrPoint(0.5, 0.1666666666), 
                            CGrPoint(0.5, 0.8333333333), 
                            CGrPoint(0.1666666666, 0.2777777777), 
                            CGrPoint(0.8333333333, 0.3888888888), 
                            CGrPoint(0.1666666666, 0.6111111111),
                            CGrPoint(0.8333333333, 0.7222222222), 
                            CGrPoint(0.8333333333, 0.0555555555)};
const CGrPoint JITTER12[] = {CGrPoint(0.4166666666, 0.625), 
                             CGrPoint(0.9166666666, 0.875), 
                             CGrPoint(0.25, 0.375),
                             CGrPoint(0.4166666666, 0.125), 
                             CGrPoint(0.75, 0.125), 
                             CGrPoint(0.0833333333, 0.125), 
                             CGrPoint(0.75, 0.625),
                             CGrPoint(0.25, 0.875), 
                             CGrPoint(0.5833333333, 0.375), 
                             CGrPoint(0.9166666666, 0.375),
                             CGrPoint(0.0833333333, 0.625), 
                             CGrPoint(0.583333333, 0.875)};
const CGrPoint JITTER16[] = {CGrPoint(0.375, 0.4375), 
                             CGrPoint(0.625, 0.0625), 
                             CGrPoint(0.875, 0.1875), 
                             CGrPoint(0.125, 0.0625), 
                             CGrPoint(0.375, 0.6875), 
                             CGrPoint(0.875, 0.4375), 
                             CGrPoint(0.625, 0.5625), 
                             CGrPoint(0.375, 0.9375), 
                             CGrPoint(0.625, 0.3125), 
                             CGrPoint(0.125, 0.5625), 
                             CGrPoint(0.125, 0.8125), 
                             CGrPoint(0.375, 0.1875), 
                             CGrPoint(0.875, 0.9375), 
                             CGrPoint(0.875, 0.6875), 
                             CGrPoint(0.125, 0.3125), 
                             CGrPoint(0.625, 0.8125)};

const CGrPoint * const JITTER[] = {NULL, JITTER1, JITTER2, JITTER3,
                                   JITTER4, JITTER5, JITTER6, NULL, JITTER8, JITTER9,
                                   NULL, NULL, JITTER12, NULL, NULL, NULL, JITTER16};

const int JITTERMAX = 16;

#endif
