int a[5][3][2];
float b=-114.514;

int add(int x,int y)
{
	int z=x+y;
	z=z-y;
	z=z+y;
	return z;
}

float mul(float x,float y)
{
	float ret=x*y;
	return ret;
}

void fibonacci()
{
	int x=1,y=1,z,i;
	for(i=1;i<=10;++i)
	{
		z=x;
		x=x+y;
		y=z;
	}
}

int main()
{
	int d,e=-101;
	float c[3][2];
	if(a[1][2][1]==1)
		d=add(a[1][2][0],a[2][1][0]);
	else
		d=add(a[0][2][0],e);
	fibonacci();
}
