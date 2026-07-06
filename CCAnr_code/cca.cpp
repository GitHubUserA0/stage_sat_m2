#include "basis.h"
#include "cca.h"
#include "cw.h"
#include "preprocessor.h"
#include "mab.h"
#include "contribs.h"

#include <string.h>
#include <sys/times.h> //these two h files are for linux
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <cmath>

char * inst;
int seed;

long long ls_no_improv_times;

bool aspiration_active;

//begin definiton of basis.h functions
void free_memory()
{
	int i;
	for (i = 0; i < num_clauses; i++)
	{
		delete[] clause_lit[i];
	}

	for(i=1; i<=num_vars; ++i)
	{
		delete[] var_lit[i];
		delete[] var_neighbor[i];
	}

	if (mab) mab_free();
}
/*
 * Read in the problem.
 */
int build_instance(char *filename)
{
	char    line[1000000];
	char    tempstr1[10];
	char    tempstr2[10];
	int     cur_lit;
	int     i,j;
	int		v,c;//var, clause

	ifstream infile(filename);
	if(!infile)
		return 0;

	/*** build problem data structures of the instance ***/
	infile.getline(line,1000000);
	while (line[0] != 'p')
		infile.getline(line,1000000);

	sscanf(line, "%s %s %d %d", tempstr1, tempstr2, &num_vars, &num_clauses);
	ratio = double(num_clauses)/num_vars;

	if(num_vars>=MAX_VARS || num_clauses>=MAX_CLAUSES)
	{
		cout<<"the size of instance exceeds out limitation, please enlarge MAX_VARS and (or) MAX_CLAUSES."<<endl;
		exit(-1);
	}

	for (c = 0; c < num_clauses; c++)
	{
		clause_lit_count[c] = 0;
		clause_delete[c] = 0;
	}
	for (v=1; v<=num_vars; ++v)
	{
		var_lit_count[v] = 0;
		fix[v] = 0;
	}

	max_clause_len = 0;
	min_clause_len = num_vars;

	//Now, read the clauses, one at a time.
	for (c = 0; c < num_clauses; c++)
	{
		infile>>cur_lit;

		while (cur_lit != 0) {
			temp_lit[clause_lit_count[c]] = cur_lit;
			clause_lit_count[c]++;

			infile>>cur_lit;
		}

		clause_lit[c] = new lit[clause_lit_count[c]+1];

		for(i=0; i<clause_lit_count[c]; ++i)
		{
			clause_lit[c][i].clause_num = c;
			clause_lit[c][i].var_num = abs(temp_lit[i]);
			if (temp_lit[i] > 0) clause_lit[c][i].sense = 1;
				else clause_lit[c][i].sense = 0;

			var_lit_count[clause_lit[c][i].var_num]++;
		}
		clause_lit[c][i].var_num=0;
		clause_lit[c][i].clause_num = -1;

        //unit clause
        if(clause_lit_count[c]==1)
        {
            unitclause_queue[unitclause_queue_end_pointer++] = clause_lit[c][0];
            clause_delete[c]=1;
        }

		if(clause_lit_count[c] > max_clause_len)
			max_clause_len = clause_lit_count[c];
		else if(clause_lit_count[c] < min_clause_len)
			min_clause_len = clause_lit_count[c];

		formula_len += clause_lit_count[c];
	}
	infile.close();

	avg_clause_len = (double)formula_len/num_clauses;

	if(unitclause_queue_end_pointer>0)
	{
		simplify = 1;
		for (c = 0; c < num_clauses; c++)
		{
			org_clause_lit_count[c] = clause_lit_count[c];
			org_clause_lit[c] = new lit[clause_lit_count[c]+1];
			for(i=0; i<org_clause_lit_count[c]; ++i)
			{
				org_clause_lit[c][i] = clause_lit[c][i];
			}

		}
	}


	//creat var literal arrays
	for (v=1; v<=num_vars; ++v)
	{
		var_lit[v] = new lit[var_lit_count[v]+1];
		var_lit_count[v] = 0;	//reset to 0, for build up the array
	}
	//scan all clauses to build up var literal arrays
	for (c = 0; c < num_clauses; ++c)
	{
		for(i=0; i<clause_lit_count[c]; ++i)
		{
			v = clause_lit[c][i].var_num;
			var_lit[v][var_lit_count[v]] = clause_lit[c][i];
			++var_lit_count[v];
		}
	}
	for (v=1; v<=num_vars; ++v) //set boundary
		var_lit[v][var_lit_count[v]].clause_num=-1;

	return 1;
}
void build_neighbor_relation()
{
	int*	neighbor_flag = new int[num_vars+1];
	int		i,j,count;
	int 	v,c;

	for(v=1; v<=num_vars; ++v)
	{
		var_neighbor_count[v] = 0;

		if(fix[v]==1) continue;

		for(i=1; i<=num_vars; ++i)
			neighbor_flag[i] = 0;
		neighbor_flag[v] = 1;

		for(i=0; i<var_lit_count[v]; ++i)
		{
			c = var_lit[v][i].clause_num;
			if(clause_delete[c]==1) continue;

			for(j=0; j<clause_lit_count[c]; ++j)
			{
				if(neighbor_flag[clause_lit[c][j].var_num]==0)
				{
					var_neighbor_count[v]++;
					neighbor_flag[clause_lit[c][j].var_num] = 1;
				}
			}
		}

		neighbor_flag[v] = 0;

		var_neighbor[v] = new int[var_neighbor_count[v]+1];

		count = 0;
		for(i=1; i<=num_vars; ++i)
		{
			if(fix[i]==1) continue;

			if(neighbor_flag[i]==1)
			{
				var_neighbor[v][count] = i;
				count++;
			}
		}
		var_neighbor[v][count]=0;
	}

	delete[] neighbor_flag; neighbor_flag=NULL;
}
void print_solution()
{
     int    i;

     cout<<"v ";
     for (i=1; i<=num_vars; i++) {
         if(cur_soln[i]==0) cout<<"-";
         cout<<i;
         if(i%10==0) cout<<endl<<"v ";
         else	cout<<' ';
     }
     cout<<"0"<<endl;
}
int verify_sol()
{
	int c,j;
	int flag;

	if(simplify==0)
	{

		for (c = 0; c<num_clauses; ++c)
		{
			flag = 0;
			for(j=0; j<clause_lit_count[c]; ++j)
				if (cur_soln[clause_lit[c][j].var_num] == clause_lit[c][j].sense) {flag = 1; break;}

			if(flag ==0){//output the clause unsatisfied by the solution
				cout<<"c clause "<<c<<" is not satisfied"<<endl;

				cout<<"c ";
				for(j=0; j<clause_lit_count[c]; ++j)
				{
					if(clause_lit[c][j].sense==0)cout<<"-";
					cout<<clause_lit[c][j].var_num<<" ";
				}
				cout<<endl;

				for(j=0; j<clause_lit_count[c]; ++j)
					cout<<cur_soln[clause_lit[c][j].var_num]<<" ";
				cout<<endl;

				return 0;
			}
		}
	}

	if(simplify==1)
	{
		for (c = 0; c<num_clauses; ++c)
		{
			flag = 0;
			for(j=0; j<org_clause_lit_count[c]; ++j)
				if (cur_soln[org_clause_lit[c][j].var_num] == org_clause_lit[c][j].sense) {flag = 1; break;}

			if(flag ==0){//output the clause unsatisfied by the solution
				cout<<"c clause "<<c<<" is not satisfied"<<endl;

				if(clause_delete[c]==1)cout<<"c this clause is deleted by UP."<<endl;

				cout<<"c ";
				for(j=0; j<org_clause_lit_count[c]; ++j)
				{
					if(org_clause_lit[c][j].sense==0)cout<<"-";
					cout<<org_clause_lit[c][j].var_num<<" ";
				}
				cout<<endl;

				for(j=0; j<org_clause_lit_count[c]; ++j)
					cout<<cur_soln[org_clause_lit[c][j].var_num]<<" ";
				cout<<endl;

				return 0;
			}
		}

	}

	return 1;
}
//end of definition of basis.h functions


//begin definition of cca.h functions
inline void unsat(int clause)
{
	index_in_unsat_stack[clause] = unsat_stack_fill_pointer;
	push(clause,unsat_stack);

	//update appreance count of each var in unsat clause and update stack of vars in unsat clauses
	int v;
	for(lit* p=clause_lit[clause]; (v=p->var_num)!=0; p++)
	{
		unsat_app_count[v]++;
		if(unsat_app_count[v]==1)
		{
			index_in_unsatvar_stack[v] = unsatvar_stack_fill_pointer;
			push(v,unsatvar_stack);
		}
	}
}
inline void sat(int clause)
{
	int index,last_unsat_clause;

	//since the clause is satisfied, its position can be reused to store the last_unsat_clause
	last_unsat_clause = pop(unsat_stack);
	index = index_in_unsat_stack[clause];
	unsat_stack[index] = last_unsat_clause;
	index_in_unsat_stack[last_unsat_clause] = index;

	//update appreance count of each var in unsat clause and update stack of vars in unsat clauses
	int v,last_unsat_var;
	for(lit* p=clause_lit[clause]; (v=p->var_num)!=0; p++)
	{
		unsat_app_count[v]--;
		if(unsat_app_count[v]==0)
		{
			last_unsat_var = pop(unsatvar_stack);
			index = index_in_unsatvar_stack[v];
			unsatvar_stack[index] = last_unsat_var;
			index_in_unsatvar_stack[last_unsat_var] = index;
		}
	}
}

void inline reset_weights()
{
	int c;

	for (c = 0; c < num_clauses; c++)
		clause_weight[c] = 1;
}

//initiation of the algorithm
void init(int current_try)
{
	int 		v,c;
	int			i,j;
	int			clause;

	if ( ! weight_conservation || current_try==0)
		reset_weights();

	//init unsat_stack
	unsat_stack_fill_pointer = 0;
	unsatvar_stack_fill_pointer = 0;

	//init solution
	for (v = 1; v <= num_vars; v++) {

        if(fix[v]==0){
            if(rand()%2==1) cur_soln[v] = 1;
            else cur_soln[v] = 0;

			time_stamp[v] = 0;
			conf_change[v] = 1;
			unsat_app_count[v] = 0;

			//pscore[v] = 0;
		}

	}

	/* figure out sat_count, and init unsat_stack */
	for (c=0; c<num_clauses; ++c)
	{
		if(clause_delete[c]==1) continue;

		sat_count[c] = 0;

		for(j=0; j<clause_lit_count[c]; ++j)
		{
			if (cur_soln[clause_lit[c][j].var_num] == clause_lit[c][j].sense)
			{
				sat_count[c]++;
				sat_var[c] = clause_lit[c][j].var_num;
			}
		}

		if (sat_count[c] == 0)
			unsat(c);
	}

	/*figure out var score*/
	int lit_count;
	for (v=1; v<=num_vars; v++)
	{
		if(fix[v]==1)
		{
			score[v] = -100000;
			continue;
		}

		score[v] = 0;

		lit_count = var_lit_count[v];

		for(i=0; i<lit_count; ++i)
		{
			c = var_lit[v][i].clause_num;
			if (sat_count[c]==0) score[v]++;
			else if (sat_count[c]==1 && var_lit[v][i].sense==cur_soln[v]) score[v]--;
		}
	}


	//init goodvars stack
	goodvar_stack_fill_pointer = 0;
	for (v=1; v<=num_vars; v++)
	{
		if(fix[v]==1)  continue;
		if(score[v]>0)// && conf_change[v]==1)
		{
			already_in_goodvar_stack[v] = 1;
			push(v,goodvar_stack);

		}
		else already_in_goodvar_stack[v] = 0;
	}

	//setting for the virtual var 0
	time_stamp[0]=0;


	this_try_best_unsat_stack_fill_pointer = unsat_stack_fill_pointer;
}
void flip(int flipvar)
{
	cur_soln[flipvar] = 1 - cur_soln[flipvar];

	int i,j;
	int v,c;

	lit* clause_c;

	int org_flipvar_score = score[flipvar];

	//update related clauses and neighbor vars
	for(lit *q = var_lit[flipvar]; (c=q->clause_num)>=0; q++)
	{
		clause_c = clause_lit[c];
		if(cur_soln[flipvar] == q->sense)
		{
			++sat_count[c];

			if (sat_count[c] == 2) //sat_count from 1 to 2
				score[sat_var[c]] += clause_weight[c];
			else if (sat_count[c] == 1) // sat_count from 0 to 1
			{
				sat_var[c] = flipvar;//record the only true lit's var
				for(lit* p=clause_c; (v=p->var_num)!=0; p++) score[v] -= clause_weight[c];

				sat(c);
			}
		}
		else // cur_soln[flipvar] != cur_lit.sense
		{
			--sat_count[c];
			if (sat_count[c] == 1) //sat_count from 2 to 1
			{
				for(lit* p=clause_c; (v=p->var_num)!=0; p++)
				{
					if(p->sense == cur_soln[v] )
					{
						score[v] -= clause_weight[c];
						sat_var[c] = v;
						break;
					}
				}
			}
			else if (sat_count[c] == 0) //sat_count from 1 to 0
			{
				for(lit* p=clause_c; (v=p->var_num)!=0; p++) score[v] += clause_weight[c];
				unsat(c);
			}//end else if

		}//end else
	}

	score[flipvar] = -org_flipvar_score;

	/*update CCD */
	int index;

	conf_change[flipvar] = 0;
	//remove the vars no longer goodvar in goodvar stack
	for(index=goodvar_stack_fill_pointer-1; index>=0; index--)
	{
		v = goodvar_stack[index];
		if(score[v]<=0)
		{
			goodvar_stack[index] = pop(goodvar_stack);
			already_in_goodvar_stack[v] = 0;
		}
	}

	//update all flipvar's neighbor's conf_change to be 1, add goodvar
	int* p;
	for(p=var_neighbor[flipvar]; (v=*p)!=0; p++)
	{
		conf_change[v] = 1;

		if(score[v]>0 && already_in_goodvar_stack[v] ==0)
		{
			push(v,goodvar_stack);
			already_in_goodvar_stack[v] = 1;
		}
	}
}
//end definition of cca.h functions

//begin definition of cw.h functions
void smooth_clause_weights()
{
	int i,j,c,v;
	int new_total_weight=0;

	for (v=1; v<=num_vars; ++v)
		score[v] = 0;

	//smooth clause score and update score of variables
	for (c = 0; c<num_clauses; ++c)
	{
		clause_weight[c] = clause_weight[c]*p_scale+scale_ave;
		if(clause_weight[c]<1) clause_weight[c] = 1;

		new_total_weight+=clause_weight[c];

		//update score of variables in this clause
		if (sat_count[c]==0)
		{
			for(j=0; j<clause_lit_count[c]; ++j)
			{
				score[clause_lit[c][j].var_num] += clause_weight[c];
			}
		}
		else  if(sat_count[c]==1)
			score[sat_var[c]]-=clause_weight[c];
	}

	ave_weight=new_total_weight/num_clauses;
}
void update_clause_weights()
{
	int i,v;

	for(i=0; i < unsat_stack_fill_pointer; ++i)
		clause_weight[unsat_stack[i]]++;

	for(i=0; i<unsatvar_stack_fill_pointer; ++i)
	{
		v = unsatvar_stack[i];
		score[v] += unsat_app_count[v];
		if(score[v]>0 &&  conf_change[v]==1 && already_in_goodvar_stack[v] ==0)
		{
			push(v,goodvar_stack);
			already_in_goodvar_stack[v] =1;
		}
	}

	delta_total_weight+=unsat_stack_fill_pointer;
	if(delta_total_weight>=num_clauses)
	{
		ave_weight+=1;
		delta_total_weight -= num_clauses;

		//smooth weights
		if(ave_weight>threshold)
			smooth_clause_weights();
	}
}
void set_clause_weighting()
{
	threshold=300;
	p_scale=0.3;
	if(q_init==0)
	{
		if(ratio<=15) q_scale=0;
		else q_scale=0.7;
	}
	else
	{
		if(q_scale<0.5)  //0
			q_scale = 0.7;
		else
			q_scale = 0;
	}

	scale_ave=(threshold+1)*q_scale;
	q_init = 1;
}
//end definition of cw.h functions

//begin definition of preprocessor.h
void unit_propagation()
{
    lit uc_lit;
    int uc_clause;
    int uc_var;
    bool uc_sense;

    int c,v;
    int i,j;
    lit cur, cur_c;


    //while (unitclause_queue_beg_pointer < unitclause_queue_end_pointer)
    for(unitclause_queue_beg_pointer=0; unitclause_queue_beg_pointer < unitclause_queue_end_pointer; unitclause_queue_beg_pointer++)
    {
        uc_lit = unitclause_queue[unitclause_queue_beg_pointer];

        uc_var = uc_lit.var_num;
        uc_sense = uc_lit.sense;

        if(fix[uc_var]==1) {if(uc_sense!=cur_soln[uc_var])cout<<"c wants to fix a variable twice, forbid."<<endl; continue;}

        cur_soln[uc_var] = uc_sense;//fix the variable in unit clause
        fix[uc_var] = 1;

        for(i = 0; i<var_lit_count[uc_var]; ++i)
        {
            cur = var_lit[uc_var][i];
            c = cur.clause_num;

            if(clause_delete[c]==1) continue;

            if(cur.sense == uc_sense)//then remove the clause from var's var_lit[] array
            {
                clause_delete[c]=1;
            }
            else
            {
                if(clause_lit_count[c]==2)
                {
                    if(clause_lit[c][0].var_num == uc_var)
                    {
                        unitclause_queue[unitclause_queue_end_pointer++] = clause_lit[c][1];
                    }
                    else
                    {
                        unitclause_queue[unitclause_queue_end_pointer++] = clause_lit[c][0];
                    }

                    clause_delete[c]=1;
                }
                else
                {
                    for(j=0; j<clause_lit_count[c]; ++j)
                    {
                        if(clause_lit[c][j].var_num == uc_var)
                        {
                            clause_lit[c][j]=clause_lit[c][clause_lit_count[c]-1];

                            clause_lit_count[c]--;

                            break;
                        }
                    }//for
                }
            }

        }//for



    }//begpointer to endpointer for

}
void preprocess()
{
    int c,v,i;
    int delete_clause_count=0;
    int fix_var_count=0;

    unit_propagation();

    //rescan all clauses to build up var literal arrays
    for (v=1; v<=num_vars; ++v)
        var_lit_count[v] = 0;

    max_clause_len = 0;
	min_clause_len = num_vars;
    int    formula_len=0;

    for (c = 0; c < num_clauses; ++c)
    {
        if(clause_delete[c]==1) {
            delete_clause_count++;
            continue;
        }

        for(i=0; i<clause_lit_count[c]; ++i)
        {
            v = clause_lit[c][i].var_num;
            var_lit[v][var_lit_count[v]] = clause_lit[c][i];
            ++var_lit_count[v];
        }
        clause_lit[c][i].var_num=0; //new clause boundary
        clause_lit[c][i].clause_num = -1;

        //about clause length
        formula_len += clause_lit_count[c];

        if(clause_lit_count[c] > max_clause_len)
            max_clause_len = clause_lit_count[c];
        else if(clause_lit_count[c] < min_clause_len)
            min_clause_len = clause_lit_count[c];
    }

    avg_clause_len = (double)formula_len/num_clauses;

    for (v=1; v<=num_vars; ++v)
    {
    	if(fix[v]==1)
    	{
    		fix_var_count++;
    	}
        var_lit[v][var_lit_count[v]].clause_num=-1;//new var_lit boundary
    }

    //cout<<"c unit propagation fixes "<<fix_var_count<<" variables, and delets "<<delete_clause_count<<" clauses"<<endl;

}
//end definition of preprocessor.h

static void mab_init()
{
	if (mab_V)       { free(mab_V);       mab_V       = NULL; }
	if (mab_t)       { free(mab_t);       mab_t       = NULL; }
	if (mab_history) { free(mab_history); mab_history = NULL; }

	mab_V       = (double*) calloc(num_clauses, sizeof(double));
	mab_t       = (int*)   calloc(num_clauses, sizeof(int));
	mab_history = (int*)   malloc(delay_MAB   * sizeof(int));

	if (!mab_V || !mab_t || !mab_history)
	{
		fprintf(stderr, "mab_init: malloc/calloc failed\n");
		exit(1);
	}


	for (int i = 0; i < num_clauses; i++)
		mab_V[i] = 1.0;

	for (int i = 0; i < delay_MAB; i++)
		mab_history[i] = -1;

	mab_hist_pointer   = 0;
	mab_N          = 0;
	mab_prev_unsat = num_clauses;
	mab_best_unsat = num_clauses;
	mab_initialized = true;
}

inline int pull_arm_MAB(int unsat_clauses[], int nb_unsat_clauses)
{
	if (nb_unsat_clauses == 0)
		return unsat_clauses[0];

	if (!mab_initialized)
		mab_init();


	int cur_unsat = nb_unsat_clauses;
	if (cur_unsat < mab_best_unsat)
		mab_best_unsat = cur_unsat;


	double reward = 0.0;
	if (mab_prev_unsat > cur_unsat)
	{
		double denom = (double)(mab_prev_unsat - mab_best_unsat) + 1.0;
		reward = (double)(mab_prev_unsat - cur_unsat) / denom;
	}

	if (reward != 0.0)
	{
		double discount = 1.0;

		for (int d = 0; d < delay_MAB; d++)
		{
			int pos = (mab_hist_pointer - 1 - d + delay_MAB * 2) % delay_MAB;
			int arm = mab_history[pos];
			if (arm < 0) break;
			mab_V[arm] += discount * reward;
			discount    *= gamma_MAB;
		}
	}
	mab_prev_unsat = cur_unsat;
	mab_N++;


	double ln_N = (mab_N > 1) ? log((double)mab_N) : 0.0;

	int    best_clause = unsat_clauses[rand() % nb_unsat_clauses];
	double best_ucb    = -1e18;

	int sample_size = (ArmNum_MAB < nb_unsat_clauses) ? ArmNum_MAB : nb_unsat_clauses;
	for (int i = 0; i < sample_size; i++)
	{
		int clause_index    = rand() % nb_unsat_clauses;
		int clause = unsat_clauses[clause_index];

		double ucb = mab_V[clause]
		           + lambda_MAB * sqrt(ln_N / (double)(mab_t[clause] + 1));

		if (ucb > best_ucb)
		{
			best_ucb    = ucb;
			best_clause = clause;
		}
	}

	mab_history[mab_hist_pointer] = best_clause;
	mab_hist_pointer = (mab_hist_pointer + 1) % delay_MAB;
	mab_t[best_clause]++;

	return best_clause;
}

static void mab_free()
{
	if (mab_V)       { free(mab_V);       mab_V       = NULL; }
	if (mab_t)       { free(mab_t);       mab_t       = NULL; }
	if (mab_history) { free(mab_history); mab_history = NULL; }
	mab_initialized = false;
}


int count_unsat_cc_clauses(int unsat_clauses[], int nb_unsat_clauses)
{
	int nb_unsat_cc_clauses = 0;
	for (int i = 0; i < nb_unsat_clauses; i++)
	{
		int clause = unsat_clauses[i];
		for (int j = 0; j < clause_lit_count[clause]; j++)
		{
			if (conf_change[clause_lit[clause][j].var_num] == 1)
			{
				nb_unsat_cc_clauses++;
				break;
			}
		}
	}
	return nb_unsat_cc_clauses;
}

int* find_unsat_cc_clauses(int unsat_clauses[], int nb_unsat_clauses)
{
	int nb_unsat_cc_clauses = count_unsat_cc_clauses(unsat_clauses,nb_unsat_clauses);
	int* unsat_cc_clauses_stack = (int*)malloc(sizeof(int) * nb_unsat_cc_clauses);

	if ( !unsat_cc_clauses_stack )
	{
		printf("malloc failed\n");
		return NULL;
	}

	unsat_cc_clauses_stack_fill_pointer = 0;

	for (int unsat_clause_index = 0 ; unsat_clause_index < nb_unsat_clauses ; unsat_clause_index ++)
	{
		int clause = unsat_clauses[unsat_clause_index];
		int clause_size = clause_lit_count[clause];

		for (int lit_index = 0 ; lit_index < clause_size ; lit_index ++)
		{
			lit current_lit = clause_lit[clause][lit_index];
			if (conf_change[current_lit.var_num]==1)
			{
				unsat_cc_clauses_stack[unsat_cc_clauses_stack_fill_pointer] = clause;
				unsat_cc_clauses_stack_fill_pointer ++;
				break;
			}
		}
	}
	return unsat_cc_clauses_stack;
}


static int pick_var(void)
{
	int         i,k,c,v;
	int         best_var;
	lit*		clause_c;

	/**Greedy Mode**/
	/*CCD (configuration changed decreasing) mode, the level with configuation chekcing*/
	if(goodvar_stack_fill_pointer>0)
	{

		//if(goodvar_stack_fill_pointer<balancePar)
		//{
		best_var = goodvar_stack[0];
		for(i=1; i<goodvar_stack_fill_pointer; ++i)
		{
			v=goodvar_stack[i];
			if(score[v]>score[best_var]) best_var = v;
			else if(score[v]==score[best_var])
			{
				if(time_stamp[v]<time_stamp[best_var]) best_var = v;
			}
		}
		return best_var;
	}


	/*aspiration*/
	if (aspiration_active)
	{
		best_var = 0;
		for(i=0; i<unsatvar_stack_fill_pointer; ++i)
		{
			if(score[unsatvar_stack[i]]>ave_weight)
			{
				best_var = unsatvar_stack[i];
				break;
			}
		}

		for(++i; i<unsatvar_stack_fill_pointer; ++i)
		{
			v=unsatvar_stack[i];
			if(score[v]>score[best_var]) best_var = v;
			else if(score[v]==score[best_var] && time_stamp[v]<time_stamp[best_var]) best_var = v;
		}

		if(best_var!=0) return best_var;
	}
	/*****end aspiration*******************/

	update_clause_weights();

	/*focused random walk*/

	if (mab) c = pull_arm_MAB(unsat_stack,unsat_stack_fill_pointer);

	else if (cc_unsat)
	{
		int * unsat_cc_clauses_stack = find_unsat_cc_clauses(unsat_stack, unsat_stack_fill_pointer);
		c = unsat_cc_clauses_stack[rand()%unsat_cc_clauses_stack_fill_pointer];
	}

	else
		c = unsat_stack[rand()%unsat_stack_fill_pointer];

	clause_c = clause_lit[c];
	best_var = clause_c[0].var_num;
	for(k=1; k<clause_lit_count[c]; ++k)
	{
		v=clause_c[k].var_num;

		//using unweighted make
		if(unsat_app_count[v]>unsat_app_count[best_var]) best_var = v;
		else if(unsat_app_count[v]==unsat_app_count[best_var])
		{
			if(score[v]>score[best_var]) best_var = v;
			else if(score[v]==score[best_var]&&time_stamp[v]<time_stamp[best_var]) best_var = v;
		}
	}

	return best_var;
}
//set functions in the algorithm
void settings()
{

}
void local_search(long long no_improv_times)
{
	int flipvar;
	long long notime = 1 + no_improv_times;
	if (unsat_stack_fill_pointer==0)
		return;

	while(--notime)
	{
		step++;
		
		flipvar = pick_var();
		flip(flipvar);
		time_stamp[flipvar] = step;
		
		if(unsat_stack_fill_pointer < this_try_best_unsat_stack_fill_pointer)
		{
			this_try_best_unsat_stack_fill_pointer = unsat_stack_fill_pointer;
			notime = 1 + no_improv_times;
		}

		if(unsat_stack_fill_pointer == 0)
		{
			return;
		}
	}
}

void default_settings()
{
	seed = 1;
	ls_no_improv_times = 200000;
	p_scale = 0.3;
	q_scale = 0.7;
	threshold = 50;
	
	aspiration_active = false; //

	mab = false;
	cc_unsat = false;
	weight_conservation = false;
}
bool parse_arguments(int argc, char ** argv)
{

	bool flag_inst = false;
	default_settings();
	
	for (int i=1; i<argc; i++)
	{
		if(strcmp(argv[i],"-inst")==0)
		{
			i++;
			if(i>=argc) return false;
			inst = argv[i];
			flag_inst = true;
			continue;
		}
		else if(strcmp(argv[i],"-seed")==0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &seed);
			continue;
		}
		
		else if(strcmp(argv[i],"-aspiration")==0)
		{
			if(i>=argc) return false;
			aspiration_active = true;
			continue;
		}

		else if(strcmp(argv[i],"-swt_threshold")==0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &threshold);
			continue;
		}

		else if(strcmp(argv[i],"-swt_p")==0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%f", &p_scale);
			continue;
		}

		else if(strcmp(argv[i],"-swt_q")==0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%f", &q_scale);
			continue;
		}
		
		else if(strcmp(argv[i],"-ls_no_improv_steps")==0){
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%lld", &ls_no_improv_times);
			continue;
		}

		else if(strcmp(argv[i],"-mab")==0)
		{
			if(i>=argc) return false;
			mab = true;
			continue;
		}

		else if(strcmp(argv[i],"-cc_unsat")==0)
		{
			if(i>=argc) return false;
			cc_unsat = true;
			continue;
		}

		else if(strcmp(argv[i],"-weight_conservation")==0)
		{
			if(i>=argc) return false;
			weight_conservation = true;
			continue;
		}
		else return false;
		
	}
	
	if (flag_inst) return true;
	else return false;

}
int main(int argc, char* argv[])
{
	int     seed,i;
	int		satisfy_flag=0;
	struct 	tms start, stop;
    
    //cout<<"c This is CCAnr 2.0 [Version: 2018.01.28] [Author: Shaowei Cai]."<<endl;
	
	times(&start);

	bool ret = parse_arguments(argc, argv);
	if(!ret) {cout<<"Arguments Error!"<<endl; return -1;}


	if(build_instance(inst) == 0)
	{
		cout<<"Invalid filename: "<< inst <<endl;
		return -1;
	}

	if (mab && cc_unsat)
	{
		cout<<"ERROR ! : -mab and -cc_unsat are incompatible between them, please choose only one of them."<<endl;
		return -1;
	}
	
    srand(seed);
    
    if(unitclause_queue_end_pointer>0) preprocess();
    
    build_neighbor_relation();
    
    scale_ave=(threshold+1)*q_scale; //
    
	cout<<num_vars<<";"<<endl;
	cout<<num_clauses<<";"<<endl;
	cout<<ratio<<";"<<endl;
	cout<<formula_len<<";"<<endl;
	cout<<"c Instance: Avg (Min,Max) clause length = "<<avg_clause_len<<" ("<<min_clause_len<<","<<max_clause_len<<")"<<";"<<endl;
	cout<<seed<<";"<<endl;
	cout<<ls_no_improv_times <<";"<< endl;
	cout<<p_scale <<";"<< endl;
	cout<<q_scale <<";"<< endl;
	cout<<threshold <<";"<< endl;
	cout<<scale_ave <<";"<< endl;
	if(aspiration_active) cout<<"true" <<";"<< endl;
	else cout<<"false" <<";"<< endl;
    
	for (tries = 0; tries <= max_tries; tries++) 
	{
		 settings();
		 
		 init(tries);
	 
		 local_search(ls_no_improv_times);

		 if (unsat_stack_fill_pointer==0) 
		 {
		 	if(verify_sol()==1) {satisfy_flag = 1; break;}
		    else cout<<"c Sorry, something is wrong;"<<endl;/////
		 }
	}

	times(&stop);
	double comp_time = double(stop.tms_utime - start.tms_utime +stop.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);

    if(satisfy_flag==1)
    {
    	cout<<"SATISFIABLE"<<";"<<endl;
		//print_solution();
    }
    else  cout<<"UNKNOWN"<<";"<<endl;
    
    cout<<"c solveSteps = "<<tries<<" tries + "<<step<<" steps (each try has "<<max_flips<<" steps)."<<";"<<endl;
    cout<<comp_time<<endl;
	 
    free_memory();

    return 0;
}
