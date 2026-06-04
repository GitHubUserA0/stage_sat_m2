
#ifndef WORKING_REPO_MAB_H
#define WORKING_REPO_MAB_H
static int    ArmNum_MAB   = 20;
static double lambda_MAB   = 1.0;
static int    delay_MAB    = 20;
static double gamma_MAB    = 0.9;


static double* mab_V       = NULL;
static int*    mab_t       = NULL;
static int*    mab_history = NULL;
static int     mab_hist_pointer = 0;
static long long mab_N     = 0;
static int     mab_prev_unsat = 0;
static int     mab_best_unsat = 0;
static bool    mab_initialized = false;

static void mab_init();
static void mab_free();
#endif //WORKING_REPO_MAB_H
