#ifndef _BASIS_H_
#define _BASIS_H_

#include <iostream>


using namespace std;

enum type{SAT3, SAT5, SAT7, strSAT} probtype;

/* limits on the size of the problem. */
#define MAX_VARS    4000010
#define MAX_CLAUSES 20000000


// Define a data structure for a literal in the SAT problem.
struct lit {
    int clause_num;		//clause num, begin with 0
    int var_num;		//variable num, begin with 1
    int sense;			//is 1 for true literals, 0 for false literals.
};

/*parameters of the instance*/
int     num_vars;		//var index from 1 to num_vars
int     num_clauses;	//clause index from 0 to num_clauses-1
int		max_clause_len;
int		min_clause_len;
int		formula_len=0;
double	avg_clause_len;
double 	ratio;

/* literal arrays */				
lit*	var_lit[MAX_VARS];				//var_lit[i][j] means the j'th literal of var i.
int		var_lit_count[MAX_VARS];        //amount of literals of each var
lit*	clause_lit[MAX_CLAUSES];		//clause_lit[i][j] means the j'th literal of clause i.
int		clause_lit_count[MAX_CLAUSES]; 	// amount of literals in each clause		

lit*	org_clause_lit[MAX_CLAUSES];		//clause_lit[i][j] means the j'th literal of clause i.
int		org_clause_lit_count[MAX_CLAUSES]; 	// amount of literals in each clause
int		simplify=0;
			
/* Information about the variables. */
int     score[MAX_VARS];				
int		time_stamp[MAX_VARS];
int		conf_change[MAX_VARS];
int*	var_neighbor[MAX_VARS];
int		var_neighbor_count[MAX_VARS];
//int		pscore[MAX_VARS];
int		fix[MAX_VARS];


/* Information about the clauses */			
int     clause_weight[MAX_CLAUSES];		
int     sat_count[MAX_CLAUSES];			
int		sat_var[MAX_CLAUSES];
//int		sat_var2[MAX_CLAUSES];

//unsat clauses stack
int		unsat_stack[MAX_CLAUSES];		//store the unsat clause number
int		unsat_stack_fill_pointer;
int		index_in_unsat_stack[MAX_CLAUSES];//which position is a clause in the unsat_stack
int     unsat_cc_clauses_stack_fill_pointer;

int		this_try_best_unsat_stack_fill_pointer;

//variables in unsat clauses
int		unsatvar_stack[MAX_VARS];		
int		unsatvar_stack_fill_pointer;
int		index_in_unsatvar_stack[MAX_VARS];
int		unsat_app_count[MAX_VARS];		//a varible appears in how many unsat clauses

//configuration changed decreasing variables (score>0 and confchange=1)
int		goodvar_stack[MAX_VARS];		
int		goodvar_stack_fill_pointer;
int		already_in_goodvar_stack[MAX_VARS];

//unit clauses preprocess
lit		unitclause_queue[MAX_VARS];		
int		unitclause_queue_beg_pointer=0;
int     unitclause_queue_end_pointer=0;
int     clause_delete[MAX_CLAUSES];

/* Information about solution */
int     cur_soln[MAX_VARS];	//the current solution, with 1's for True variables, and 0's for False variables

//cutoff
int		max_tries = 10000;
int		tries;
int		max_flips = 2000000000;
int		step;

int temp_lit[MAX_VARS]; //the max length of a clause can be MAX_VARS

void setup_datastructure();
void free_memory();
int build_instance(char *filename);
void build_neighbor_relation();
void print_solution();
int verify_sol();

#endif

