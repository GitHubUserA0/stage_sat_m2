set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <input.csv> <cnf_dir> [output.csv]" >&2
    exit 1
fi

INPUT_CSV="$1"
CNF_DIR="$2"
OUTPUT_CSV="${3:-result.csv}"

if [[ ! -f "$INPUT_CSV" ]]; then
    echo "Erreur : fichier CSV introuvable : $INPUT_CSV" >&2
    exit 1
fi

if [[ ! -d "$CNF_DIR" ]]; then
    echo "Erreur : dossier CNF introuvable : $CNF_DIR" >&2
    exit 1
fi

HEADER=$(head -1 "$INPUT_CSV")

COL2=$(echo "$HEADER" | cut -d',' -f2)
if [[ "$COL2" == "benchmark" ]]; then
    HAS_BENCHMARK=1
    TIME_START_COL=5
    echo "${HEADER},nb_variables,nb_clauses,avg_time" > "$OUTPUT_CSV"
else
    HAS_BENCHMARK=0
    COL1=$(echo "$HEADER" | cut -d',' -f1)
    REST=$(echo "$HEADER" | cut -d',' -f2-)
    echo "${COL1},benchmark,${REST},nb_variables,nb_clauses,avg_time" > "$OUTPUT_CSV"
    TIME_START_COL=6
fi

MISSING=0
PROCESSED=0

while IFS= read -r line; do
    [[ -z "$line" ]] && continue

    HASH=$(echo "$line" | cut -d',' -f1)

    CNF_FILE=$(find "$CNF_DIR" -maxdepth 1 -name "${HASH}-*.cnf" | head -1)

    if [[ "$HAS_BENCHMARK" -eq 1 ]]; then
        BENCH=$(echo "$line" | cut -d',' -f2)
    elif [[ -n "$CNF_FILE" ]]; then
        BENCH=$(basename "$CNF_FILE" | sed "s/^${HASH}-//")
    else
        BENCH="NA"
    fi

    if [[ "$HAS_BENCHMARK" -eq 0 ]]; then
        COL1_VAL=$(echo "$line" | cut -d',' -f1)
        REST_VAL=$(echo "$line" | cut -d',' -f2-)
        line="${COL1_VAL},${BENCH},${REST_VAL}"
    fi

    NB_VARS="NA"
    NB_CLAUSES="NA"

    if [[ -n "$CNF_FILE" && -f "$CNF_FILE" ]]; then
        P_LINE=$(grep -m1 "^p cnf" "$CNF_FILE" 2>/dev/null || true)
        if [[ -n "$P_LINE" ]]; then
            NB_VARS=$(echo "$P_LINE"    | awk '{print $3}')
            NB_CLAUSES=$(echo "$P_LINE" | awk '{print $4}')
        fi
    else
        echo "Avertissement : aucun fichier CNF trouvé pour le hash : $HASH" >&2
        ((MISSING++)) || true
    fi

    AVG_TIME=$(echo "$line" | awk -F',' -v start="$TIME_START_COL" '
    {
        sum = 0; count = 0
        for (i = start; i <= NF; i++) {
            val = $i
            gsub(/[[:space:]]/, "", val)
            if (val != "" && val+0 == val) {
                sum += val
                count++
            }
        }
        if (count > 0)
            printf "%.6f", sum / count
        else
            print "NA"
    }')

    echo "${line},${NB_VARS},${NB_CLAUSES},${AVG_TIME}" >> "$OUTPUT_CSV"
    ((PROCESSED++)) || true

done < <(tail -n +2 "$INPUT_CSV")

echo "Terminé : $PROCESSED lignes traitées, $MISSING fichiers CNF manquants."
echo "Résultat : $OUTPUT_CSV"
