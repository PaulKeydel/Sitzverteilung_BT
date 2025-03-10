#!/bin/bash

make

makeoutput () {
    printf '5\n3\n' | ./bt_sitzvert "$1" "$2" > "$3"
    { head -c 84 "$3"; printf '5\n'; tail -c +85 "$3"; } > dummy.txt
    { head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > "$3"
}

makeoutput kerg2017_mod.csv 2024 BTW17_reform2024.txt
makeoutput kerg2017_mod.csv 2020 BTW17_reform2020.txt
makeoutput kerg2017_mod.csv 2013 BTW17_reform2013.txt

makeoutput kerg2021_mod.csv 2024 BTW21_reform2024.txt
makeoutput kerg2021_mod.csv 2020 BTW21_reform2020.txt
makeoutput kerg2021_mod.csv 2013 BTW21_reform2013.txt

makeoutput kerg2025_mod.csv 2024 BTW25_reform2024.txt
makeoutput kerg2025_mod.csv 2020 BTW25_reform2020.txt
makeoutput kerg2025_mod.csv 2013 BTW25_reform2013.txt

rm dummy.txt
