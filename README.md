# The German Bundestag and its composition

This C++ program calculates the share of seats in the German Bundestag based on the election results from 2017, 2021 and 2025. Regardless of the election year, three different adaptations of the ballot system are available to compute the share of seats in parliament:
* The "old" system from 2013 (mode `2013`): Every single _Überhangmandat_ is balanced by additional seats (_Ausgleichsmandate_).This can prevent disproportionalities in terms of second votes but it also leads to a large number of MPs.
* The 2020 electoral law reform (mode `2020`): Minor modifications in compensating surplus mandates. To achieve a smaller amount of _Ausgleichsmandaten_, the total number of surplus mandates is reduced by 3.
* The 2024 electoral law reform (mode `2024`): No _Direktmandate_ (winning a constitueny does not automatically guarantee a seat in parliament). The parliament has a fixed size of 630 MPs.

It's also possible to adjust cross-cutting parameters like the electoral threshold (usually 5%) and the _Grundmandatsklausel_ (the minimum number of won constituencies if the share of votes is less than 5%, usually 3).

### Election results and data files
The official results of the elections can be found here (CSV files):
* https://www.bundeswahlleiterin.de/dam/jcr/72f186bb-aa56-47d3-b24c-6a46f5de22d0/btw17_kerg.csv
* https://www.bundeswahlleiterin.de/bundestagswahlen/2021/ergebnisse/opendata.html#39734920-0eaf-4633-8858-ae792d5d610b
* https://www.bundeswahlleiterin.de/bundestagswahlen/2025/ergebnisse/opendata.html#a2ed5bd6-ea23-4641-8f5b-e5fbb634d352

In order to simplify file reading the original CSV files were slightly modified. Within this project, please use `kerg2017_mod.csv`, `kerg2021_mod.csv` and `kerg2025_mod.csv` as data source.

### How to use the program?
First, build the project by running `make` or `make debug` (gcc is needed). If the build process finishes successfully, you can start the calculations. Just run `bt_sitzvert <modified csv file> <reformMode>` and enter the values for _Sperrklausel_ and _Grundmandatsklausel_.

Regarding the elections in 2017, 2021 and 2025, pre-calculated seat distributions for different electoral systems can be found in the txt-files (for exmaple [the actual allocation of seats after the election in 2021](BTW21_reform2020.txt) or [the actual allocation of seats after the election in 2025](BTW25_reform2024.txt)).

All output files in this repo (with 5% electoral threshold and 3 won mandates as _Grundmandatsklausel_) can be generated by running the bash script `makeTXTfiles.sh`.