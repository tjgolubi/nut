#!/bin/bash

prog=$(basename $0)
fdc_list="$HOME/.nut/fdc_files.txt"
nut_list="$HOME/.nut/nut_files.txt"

# Only update if file doesn't exist OR is older than 30 days
if [ ! -f "$fdc_list" ] || find "$fdc_list" -mtime +30 -print -quit | grep -q .
then
  temp_file=$(mktemp)
  # Fetch and write to temp file first
  if curl -sL https://fdc.nal.usda.gov/download-datasets \
    | grep -o 'href="/fdc-datasets/FoodData_Central_csv_[0-9-]*\.zip"' \
    | sed -e 's|^href="/fdc-datasets/||;s|"$||' \
    | sort -u > "$temp_file" && [ -s "$temp_file" ]
  then

    mv "$temp_file" "$fdc_list"
  else
    echo "$prog: Warning: failed to update $fdc_list."
    rm -f "$temp_file"
  fi
fi
if [ "$fdc_list" -nt "$nut_list" ]
then
  if cmp --silent "$fdc_list" "$nut_list"
  then
    touch "$nut_list"
  else
    echo -e "\033[0;36m$prog: USDA FDC update is available.\033[0m"
    comm -23 "$fdc_list" "$nut_list"
  fi
fi
