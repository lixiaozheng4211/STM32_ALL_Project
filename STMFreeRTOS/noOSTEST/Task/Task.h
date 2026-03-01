#ifndef NOOSTEST_TASK_H
#define NOOSTEST_TASK_H
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "main.h"


#ifdef __cplusplus
extern "C" {


#endif

void vTask(void);

void vButtonTask(void);

void vUartTask(void);

void vAdcTask(void);


#ifdef	__cplusplus
{
#endif
#endif //NOOSTEST_TASK_H
