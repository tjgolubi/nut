if test -z "$FOOD_PATH"
then
  echo "FOOD_PATH not set";
  exit 1
fi
grep -i "$1" "$FOOD_PATH/food.txt" | sed -e's/|//' | sort +1
