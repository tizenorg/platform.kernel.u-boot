/*
 * header file for pwm driver.
 *
 * copyright (c) 2011 samsung electronics
 * donghwa lee <dh09.lee@samsung.com>
 *
 * this program is free software; you can redistribute it and/or modify
 * it under the terms of the gnu general public license version 2 as
 * published by the free software foundation.
*/

#ifndef _pwm_h_
#define _pwm_h_

int	pwm_init		(void);
int	pwm_config		(int pwm_id, int duty_ns, int period_ns);
int	pwm_enable		(int pwm_id);
void	pwm_disable		(int pwm_id);

#endif /* _pwm_h_ */

