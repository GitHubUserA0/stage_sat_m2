# Pondération de clauses et redémarrages dans la recherche locale pour SAT

https://github.com/GitHubUserA0/stage_sat_m2/

Le code est réparti en cinq branches, une branche par contribution et une branche les fusionnant toutes : 

* Branche main : CCAnr version refactorisée

* Branche MABs : CCAnr version Multi-Armed-Bandit

* Branche choose_cc_clauses_only : CCAnr version CC_CLAUSES

* Branche Restart_keep_data : CCAnr version weight_conservation

* Branche general_versions : CCAnr fusion de toutes les contributions

Pour chaque branche, il suffit de télécharger le dépot. La compilation se fait via la commande "make" grâce à un makefile spécifique à chaque version (dans chaque branche, se trouve uniquement le makefile approprié).

Pour chaque version, l'exécution avec paramètres par défaut, se fait de la manière suivante : 

{chemin vers l'exécutable} -inst {chemin vers l'instance}. 

Pour plus de détails concernant les paramètres d'exécution, une documentation exhaustive et détaillée est disponible dans le document présent dans ce même dépôt sous le nom de {Documentation_exhaustive.pdf}, veuillez vous y référer pour de plus amples informations sur les paramètres d'exécution de chaque version de CCAnr.
