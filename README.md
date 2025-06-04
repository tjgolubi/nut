# nut
Macro-nutrient recipe helper

The following environment variables must be set so the following commands can
find the default databases.

INGRED_PATH the directory containing ingred.dat.
FOOD_PATH   the directory containing the USDA food databases.

## Commands

The src directory builds the following commands:

| **Command** | **Description** |
| nut      | Parse a recipe text file and determine ingredients from the ingred.dat database. |
| digest   | Parse a nut file (default ingred.nut) into a nutrient database (default ingred.dat). |
| barf     | Output a nutrient database (default ingred.dat) as text. |
| findfood | Search the USDA food descriptions. |
| lookup   | lookup.txt --> lookout.nut from USDA food database. |

~~~ bash
$ cd src
$ make install
~~~

The above will build the commands and install links to them in ~/bin.

Uninstall these commands with...

~~~ bash
$ cd src
$ make scour uninstall
~~~

## USDA Food Database

The usda directory downloads and builds the USDA food databases.

~~~ bash
$ cd usda
$ make
~~~

The above will download datasets from Food Data Central (FDC) and the
Agricultural Research Service (ARS), and process them to generate the food
databases contained in the db directory.

After the food databases are build successfully, use...

~~~ bash
$ cd usda
$ make scour
~~~

... to remove the downloaded databases and temporary files.

The USDA food databases are generated from the following websites.

Food Data Central
[fdc](https://fdc.nal.usda.gov/fdc-datasets/FoodData_Central_csv_2025-04-24.zip)

Agricultural Research Service, Methods and Application of Food Composition
Laboratory (MAFCL), Standard Reference 28 (May 2016)
[sr](https://www.ars.usda.gov/ARSUserFiles/80400535/DATA/SR/sr28/dnload/sr28asc.zip)
