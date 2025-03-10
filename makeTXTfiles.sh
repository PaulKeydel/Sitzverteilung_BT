#!/bin/bash

make

makeoutput () {
    printf '5\n3\n' | ./bt_sitzvert "$1" "$2" > "$3"
    { head -c 84 "$3"; printf '5\n'; tail -c +85 "$3"; } > dummy.txt
    { head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > "$3"
}

makeoutput kerg2017_mod.csv 1 BTW17_reform2024.txt
makeoutput kerg2017_mod.csv 2 BTW17_reform2020.txt
makeoutput kerg2017_mod.csv 3 BTW17_before2020.txt

makeoutput kerg2021_mod.csv 1 BTW21_reform2024.txt
makeoutput kerg2021_mod.csv 2 BTW21_reform2020.txt
makeoutput kerg2021_mod.csv 3 BTW21_before2020.txt

makeoutput kerg2025_mod.csv 1 BTW25_reform2024.txt
makeoutput kerg2025_mod.csv 2 BTW25_reform2020.txt
makeoutput kerg2025_mod.csv 3 BTW25_before2020.txt

rm dummy.txt
