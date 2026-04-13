WITH filtered AS (
  SELECT
    hash,
    CAST(nb_vars AS INTEGER) AS nb_vars,
    "vresult",
    family,
    track
  FROM "results_non-rand_2023"
  WHERE
    "vresult" = "sat"
    AND NOT (track LIKE "%main_2024%" OR track LIKE "%main_2022%")
    AND NOT (family LIKE "%random%")
)
SELECT
  *,
  NTILE(10) OVER (ORDER BY nb_vars) AS decile
FROM filtered;