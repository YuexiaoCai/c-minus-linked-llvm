int add(int a, int b) {
	return a + b;
}

int main(void) {
	int a1;
	int b;
	int sum;
	a = 10;
	b = 5;
	sum = 0;
	while (a > 0) {
		if (b > 0) {
			sum = add(sum, a);
			b = b - 1;
		}
		else {
			sum = sum + 1;
		}
		a = a - 1;
	}
	return sum;
}
