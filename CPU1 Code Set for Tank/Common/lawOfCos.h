/*
 * lawOfCos.h
 *
 *  Created on: Mar 19, 2020
 *      Author: Rober
 */

#ifndef COMMON_LAWOFCOS_H_
#define COMMON_LAWOFCOS_H_

int calcSoundRange(float t1, float t2, float t3, float * ans);
int16 calibrateResult(float theta, float r, volatile Uint16 * order);



#endif /* COMMON_LAWOFCOS_H_ */
