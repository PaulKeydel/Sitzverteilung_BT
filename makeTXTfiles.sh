#!/bin/bash

make

printf '5\n3\n' | ./bt_sitzvert kerg_mod.csv 1 > BTW21_reform2024.txt
{ head -c 84 BTW21_reform2024.txt; printf '5\n'; tail -c +85 BTW21_reform2024.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW21_reform2024.txt

printf '5\n3\n' | ./bt_sitzvert kerg_mod.csv 2 > BTW21_reform2020.txt
{ head -c 84 BTW21_reform2020.txt; printf '5\n'; tail -c +85 BTW21_reform2020.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW21_reform2020.txt

printf '5\n3\n' | ./bt_sitzvert kerg_mod.csv 3 > BTW21_before2020.txt
{ head -c 84 BTW21_before2020.txt; printf '5\n'; tail -c +85 BTW21_before2020.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW21_before2020.txt

printf '5\n3\n' | ./bt_sitzvert kerg2025_mod.csv 1 > BTW25_reform2024.txt
{ head -c 84 BTW25_reform2024.txt; printf '5\n'; tail -c +85 BTW25_reform2024.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW25_reform2024.txt

printf '5\n3\n' | ./bt_sitzvert kerg2025_mod.csv 2 > BTW25_reform2020.txt
{ head -c 84 BTW25_reform2020.txt; printf '5\n'; tail -c +85 BTW25_reform2020.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW25_reform2020.txt

printf '5\n3\n' | ./bt_sitzvert kerg2025_mod.csv 3 > BTW25_before2020.txt
{ head -c 84 BTW25_before2020.txt; printf '5\n'; tail -c +85 BTW25_before2020.txt; } > dummy.txt
{ head -c 187 dummy.txt; printf '3\n'; tail -c +188 dummy.txt; } > BTW25_before2020.txt

rm dummy.txt
