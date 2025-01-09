# The German Bundestag and its composition

This C++ program calculates the share of seats in the German Bundestag based on the election results from September 2021. Three different adaptations of the ballot system are available:
* The "old" system before 2020 (mode 3): Every single _Überhangmandat_ will be balanced. This leads to a large number of MPs.
* The 2020 electoral law reform (mode 2): Minor modifications of the compensation mechanism to achieve a smaller amount of _Ausgleichsmandaten_.
* The 2024 electoral law reform (mode 1): No _Direktmandate_ (winning a constitueny does not automatically guarantee a seat in parliament). The parliament has a fixed size of 630 MPs.

It's also possible to adjust cross-cutting parameters like the election threshold (usually 5%) and the _Grundmandatsklausel_ (the minimum number of won constituencies if the share of votes is less than 5%, usually 3).

### Election results and data files
The results of the election can be found here: https://www.bundeswahlleiterin.de/bundestagswahlen/2021/ergebnisse/opendata.html#39734920-0eaf-4633-8858-ae792d5d610b. In order to simplify file reading the original CSV file was slightly modified. For this project, please use `kerg_mod.csv`.

### How to use the program?
First, build the project by running `make` or `make debug` (gcc is needed). If the build process finishes successfully, you can start the calculations. Just run `bt_sitzvert kerg_mod.csv <reformMode> <electThr> <minNeededDirektmandate>`.

For exmaple, we can calculate the actual allocation of seats after the election in 2021: `bt_sitzvert kerg_mod.csv 2 0.05 3`. The results are in [this](results_reform2020.txt) text file.

You can find some more results [here](results_before2020.txt) (old system, 5%, 3 Direktmandate) and [here](results_reform2024.txt) (reform 2024, 5%, 3 Direktmandate).