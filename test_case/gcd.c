int gcd(int u, int v)
{	if (v==0) return u;
	else return gcd(v, u-u/v*v);
	/*u-u/v*v == u mod v*/
}

int main(void)
{	int x; int y;
	x = 18;
	y=6;
	return (gcd(x, y));
}


