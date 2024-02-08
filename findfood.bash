grep -i "$1" food.txt | sed -e's/|//' | sort +1
