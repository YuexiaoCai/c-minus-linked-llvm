int main(void)
{
	int i;
	int j;
	i = 0;
	j = 1;
	while (i < 10)
	{
		i = i + 1;
		j = j + i;
	}
	return j;
}