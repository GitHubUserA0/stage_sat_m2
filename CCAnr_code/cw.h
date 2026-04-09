#ifndef _CW_H_
#define _CW_H_

#define sigscore	ave_weight   //significant score needed for aspiration

int		ave_weight=1;
int		delta_total_weight=0;

/**************************************** clause weighting for 3sat **************************************************/

int		threshold;
float	p_scale;//w=w*p+ave_w*q
float	q_scale=0;
int		scale_ave;//scale_ave==ave_weight*q_scale

int 	q_init=0;

void smooth_clause_weights();
void update_clause_weights();
void set_clause_weighting();


#endif
