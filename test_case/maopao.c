
int main(void)
{

	int i;
	int j;
	int  l[100];
	int r[100];
	int a[10];
	int ptr;
	ptr = 1;
	a[0] = 1;
	a[1] = 1;
	a[2] = 5;
	a[3] = 6;
	a[4] = 8;
	a[5] = 9;
	a[6] = 1;
	a[7] = 3;
	a[8] = 5;
	a[9] = 4;
	/*for (int i = 0; i < 10;i++) {
		a[i] = 10 - i;
	}*/

	l[0] = 0;
	r[0] = 9;
	i = 9;
	while (i > 0) {
		j = 0;
		while (j < i) {
			if (a[j] > a[j+1]) {
				int t;
				t = a[j];
				a[j] = a[j+1];
				a[j+1] = t;
			}
			j = j + 1;
		}
		i = i - 1;
	}
	/*for (int i = 0; i < 10; i++) {
		cout << a[i] << ' ';
	}
	cout << endl;*/
	return a[8];
	
}
