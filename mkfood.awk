BEGIN {
  FS = "\t";
}
	{
  printf("%d\t|%s\n", $1, $9);
}
