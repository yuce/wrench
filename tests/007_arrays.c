/*~ ~*/

array = { 20 };
if ( array[0] != 20 ) print("lk1");
for( a : array )
{
	if ( a != 20 ) print("lk2");
	array = 0; // null the memory so it can be gc'ed (but shouldn't!)
	s = "allocate memory";
	if ( a != 20 ) print("lk2");
}


ff[] = { 1.2, 1.3, 1.4, 1.5 };
if ( ff[0] != 1.2 ) { print("ff1"); }
if ( ff[2] != 1.4 ) { print("ff2"); }
ff[0] = 1.7;
ff[2] = 45.76;
if ( ff[0] != 1.7 ) { print("ff3"); }
if ( ff[2] != 45.76 ) { print("ff4"); }
ff[1] = ff[1] + ff[3];
if ( ff[1] != 2.8 ) { print("ff5"); }
a = 7.8;
ff[1] = a;
if ( ff[1] != 7.8 ) { print("ff6"); }


aa[10];
a[500] = { aa };
if ( a[0]._count != 11 ) print("bad size2");
if ( a._count!= 501 ) print("bad size");


for( gi=0; gi<500; ++gi )
{
	allocArray();
}
for( gi=0; gi<500; ++gi )
{
	allocArray();
}

b[40000];
b[1] = 10;
b[35000] = 20;
if ( b[1] != 10 || b[35000] != 20 )
{
	print("bad b");
}

c[2000000];
c[1] = 11;
c[1913500] = 21;
if ( c[1] != 11 || c[1913500] != 21 )
{
	print("bad c");
}

a = { 4, 2, 3, 4, 5 };
if ( a[2] != 3 )
{
	print("bad a1");
}

b[] = { 4, 2, 3, 4, 5 };
if ( b[2] != 3 )
{
	print("bad b1");
}

c[100] = { 4, 2, 3, 4, 5, };
if ( c[2] != 3 )
{
	print("bad c1");
}

d[1] = { 4, 2, 3, 4, 5 };
if ( d[2] != 3 )
{
	print("bad d1");
}

a[400] = 4;
if ( a[400] != 4 )
{
	print("bad a2");
}

ggi = 0;


grow[1] = 10;
grow[2] = 11;
grow[3] = 12;
grow[4] = 13;
grow[5] = 14;
grow[6] = 15;
if ( grow[6] != 15 ) print("badref0");
if ( grow[5] != 14 ) print("badref1");
if ( grow[4] != 13 ) print("badref2");
if ( grow[3] != 12 ) print("badref3");
if ( grow[2] != 11 ) print("badref4");



function allocArray()
{

	_a = { 4, 2, 3, 4, 5 };
	if ( _a[2] != 3 )
	{
		print("F bad a1");
	}

	_b[] = { 4, 2, 3, 4, 5 };
	if ( _b[2] != 3 )
	{
		print("F bad b1");
	}

	_c[100] = { 4, 2, 3, 4, 5 };
	if ( _c[2] != 3 )
	{
		print("F bad c1");
	}

	_d[1] = { 4, 2, 3, 4, 5 };
	if ( _d[2] != 3 )
	{
		print("F bad d1");
	}

	ab[500];
	i = 2;
	ab = { 0, 2, 3 };
	if ( ab[2] != 3 )
	{
		print("bad ab2");
	}
	ab[401] = 41;
	if ( ab[401] != 41 )
	{
		print("bad ab41");
	}

	ac[50];

	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			print( "oops" );
		}
	}

	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		::a[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			print( "oops_LL" );
		}
	}

	for( i=0; i<50; ++i )
	{
		if ( ::a[i] != i+1 )
		{
			print( "oops_GL" );
		}
	}

	for( ::ggi=0; ::ggi<50; ++::ggi )
	{
		if ( ac[::ggi] != ::ggi+1 )
		{
			print( "oops_LG" );
		}
	}

	for( ::ggi=0; ::ggi<50; ++::ggi )
	{
		if ( ::a[::ggi] != ::ggi+1 )
		{
			print( "oops_GG" );
		}
	}
}

a[10] = { 1, 2, 3 };
leakCheck( a[0] );

function leakCheck( value )
{
	::a = 10;
	b[20]; // cause GC to run

	value = 20;
	if ( value != 20 ) print("B1");
	if ( 20 != value ) print("B2");
}


