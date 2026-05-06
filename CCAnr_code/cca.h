/************************************=== CCAnr ===***************************************          
 ** CCAnr is a local search solver for the Boolean Satisfiability (SAT) problem,
 ** which is especially designed for non-random instances.
 ** CCAnr is designed and implemented by Shaowei Cai (email: shaoweicai.cs@gmail.com),                                        
 *****************************************************************************************/

/*****************************=== Develpment history ===*************************************
** 2011.5
** SWCC (Smoothed Weighting and Configuration Checking) by Shaowei Cai
** New Idea: Configuration Checking (CC)
** A variable is configuration changed, if since its last flip, at least one of its 
** neighboring var has been flipped. 
** In the greedy mode, Swcc picks the best Configuration Changed Decreasing  var to flip.
** In the random mode, it updates weights, and flips the oldest var in a random unsat clause.
 
** 2011.9 
** SWCCA (Smoothed Weighting and Configuration Checking with Aspiration) by Shaowei Cai                 
** New Idea: CC with Aspiration (CCA)
** Modification: in greedy mode, it first prefers to flip the best CCD var. If there is 
** no CCD variable, then flip the best significant decreasing var, i.e., with a great 
** positive score (in Swcca, bigger than averaged clause weight), if there exsit such vars.                  
 
** 2013.4	
** CCAnr (CCA for non-random SAT)
** Modifications: Generalize the smoothig fomula as w(ci)=w(ci)*p+ave_w*q; pick the greediest
** variable in the diversification mode.
************************************************************************************************/


#ifndef _CCA_H_
#define _CCA_H_

#define pop(stack) stack[--stack ## _fill_pointer]
#define push(item, stack) stack[stack ## _fill_pointer++] = item

inline void unsat(int clause);
inline void sat(int clause);
void init();
void flip(int flipvar);
int multi_armed_bandit(int unsat_clauses[],int nb_unsat_clauses);

#endif

