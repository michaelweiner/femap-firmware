#ifndef RINGER_H
#define RINGER_H

void init_bell(TIM_HandleTypeDef *htim_dutycycle, TIM_HandleTypeDef *htim_pmos, TIM_HandleTypeDef *htim_nmos);
void start_bell(void);
void stop_bell(void);

#endif /* RINGER_H */
