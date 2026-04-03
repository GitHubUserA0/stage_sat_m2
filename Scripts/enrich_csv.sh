#!/bin/bash

set -e

CSV="$1"
CNF_DIR="$2"
DB="$3"
OUT="$4"
TMP="mapping.tmp"

if [ $# -ne 4 ]; then
    echo "Usage: $0 input.csv cnf_folder meta.db output.csv"
    exit 1
fi



sqlite3 -separator ',' "$DB" "
SELECT
    f.hash,
    f.family,
    COALESCE(GROUP_CONCAT(track.value, ' | '), '')
FROM features f
LEFT JOIN track ON f.hash = track.hash
GROUP BY f.hash;
" > "$TMP"


head -n 1 "$CSV" | awk -F',' '{
    print $0 ",nb_vars,nb_clauses,family,track,avg_time"
}' > "$OUT"


parse_cnf() {
    file="$1"

    header=$(awk '/^[[:space:]]*p[[:space:]]+cnf/ {print; exit}' "$file")

    if [ -z "$header" ]; then
        echo ","
        return
    fi

    vars=$(echo "$header" | awk '{print $3}')
    clauses=$(echo "$header" | awk '{print $4}')

    echo "$vars,$clauses"
}


declare -A family_map
declare -A track_map

while IFS=',' read -r hash family track; do
    family_map["$hash"]="$family"
    track_map["$hash"]="$track"
done < "$TMP"


declare -A cnf_map

for f in "$CNF_DIR"/*.cnf; do
    fname=$(basename "$f")
    h="${fname%%-*}"
    cnf_map["$h"]="$f"
done


tail -n +2 "$CSV" | while IFS=',' read -r line; do

    hash=$(echo "$line" | cut -d',' -f1)

    cnf_file="${cnf_map[$hash]}"
    family="${family_map[$hash]}"
    track="${track_map[$hash]}"

    if [ -f "$cnf_file" ]; then
        parsed=$(parse_cnf "$cnf_file")
        vars=$(echo "$parsed" | cut -d',' -f1)
        clauses=$(echo "$parsed" | cut -d',' -f2)
    else
        echo "File not found for hash : $hash" >&2
        vars=""
        clauses=""
    fi


    avg=$(echo "$line" | awk -F',' '{
        sum=0; count=0;
        for(i=5;i<=NF;i++){
            if($i ~ /^[0-9.]+$/){
                sum+=$i; count++;
            }
        }
        if(count>0) printf "%.3f", sum/count;
        else print "";
    }')

    echo "$line,$vars,$clauses,$family,$track,$avg" >> "$OUT"

done


rm -f "$TMP"

echo "Result written in file : $OUT"
