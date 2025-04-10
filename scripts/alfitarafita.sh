for alpha1 in $(seq 5 0.1 5.6); do
    for alpha2 in $(seq 6.5 0.1 6.7); do
        echo -n "$alpha1 - $alpha2: "
        scripts/run_vad.sh $alpha1 $alpha2 | grep TOTAL
    done
done
# alpha2 = 130
# scripts/fotli | sort -t: -k3n