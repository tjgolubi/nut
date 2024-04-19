grep -i "$1" ~/prj/nut/db/food.txt | sed -e's/|//' | sort +1
