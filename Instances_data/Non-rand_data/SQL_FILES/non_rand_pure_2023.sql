SELECT hash,nb_vars,vresult,family,track FROM "results_non-rand_2023" WHERE
"vresult"="sat" AND
NOT(track like "%main_2024%" OR track like "%main_2022%")
AND
NOT (family like "%random%")
ORDER BY(CAST(nb_vars AS INTEGER));