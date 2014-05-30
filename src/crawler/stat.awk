#!/bin/awk -f
BEGIN {count=0; aa=""}
{
	if (aa==$1)
	{
		count++;
	}
	else
	{
		if (count > 0)
			print aa "\t" count
		aa=$1;
		count=1;
	}
}

