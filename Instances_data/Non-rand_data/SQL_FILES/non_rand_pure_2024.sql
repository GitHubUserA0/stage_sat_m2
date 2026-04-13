SELECT hash,nb_vars,vresult,family,track FROM "results_non-rand_2024" WHERE
"vresult"="sat" AND
NOT(track like "%main_2022%" OR track like "%main_2023%")
AND
NOT (family like "%random%")
ORDER BY(CAST(nb_vars AS INTEGER));