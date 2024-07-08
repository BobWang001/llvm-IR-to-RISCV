int gcd(int x,int y)
{
	if(y==0)
	return x;
	else return gcd(y,x%y);
}

int mul(int x,int y)
{
	int ret=1;
	while(y)
	{
		if(y&1)
		ret=ret*x;
		x=x*x;
		y/=2;
	}
	return ret;
}

int main()
{
	int a=4,b=6,c=1,d,e;
	if(c)
	d=mul(a,b);
	else d=mul(b,a);
	e=gcd(a,b);
	return d+e;
}