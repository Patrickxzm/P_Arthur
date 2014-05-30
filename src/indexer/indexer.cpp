#include "indexer.h"

void
intersect(vector<unsigned> &a, vector<unsigned> &b, vector<unsigned> &c)
{
	sort(a.begin(), a.end());
	sort(b.begin(), b.end());
	c.clear();
	for (unsigned i=0, j=0; i<a.size() && j<b.size();)
	{
		if (a[i] == b[j])
		{
			c.push_back(a[i]);
			i++;
			j++;
		}
		else if (a[i] > b[j])
		{
			j++;
		}
		else if (a[i] < b[j])
		{
			i++;
		}
	}
	return;
}
