SELECT hash,nb_vars,nb_clauses,"verified-result",family,track FROM "results_non-rand_2022" WHERE
"verified-result"="sat" AND
NOT(track like "%main_2024%" OR track like "%main_2023%")
AND
NOT (family like "%random%")
ORDER BY(CAST(nb_vars AS INTEGER));
---CAST(nb_vars AS INTEGER)<=2000;---