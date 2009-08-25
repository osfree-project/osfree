#include <stdio.h>

char* pData[] = {"this", "is", "the", "blat", "of", "bleet!"};

int myCompare(const void* p1, const void* p2)
{
	/* Note that we get passed pointers to char, unlike in the qsort
		version */
	return strcmp((char*)p1, (char*)p2);
}

void swap(void*v[], int i, int j)
{
	void* temp;

	temp = v[i]; v[i] = v[j]; v[j] = temp;
}

void qsort2(void *v[], int left, int right,
	int (*comp)(const void*, const void*))
{
	int i, last;

	if (left >= right) return;
	swap(v, left, (left + right)/2);
	last = left;
	for (i=left+1; i <= right; i++)
		if ((*comp)(v[i], v[left]) < 0)
			swap(v, ++last, i);
	swap(v, left, last);
	qsort2(v, left, last-1, comp);
	qsort2(v, last+1, right, comp);
}


int main(void)
{
	int i = sizeof(pData);
	int j = i*i;
	int k = i+3;
	int n = i / (sizeof(char*));
	printf("Size of the data is %d\n", i);
	printf("j and k and n are %d and %d and %d\n", j, k, n);
	qsort2((void**)pData, 0, n-1,  myCompare);
	printf("After the sort:\n");
	for (i=0; i < n; i++)
	{
		printf("%s\n", pData[i]);
	}

	return 0;
}
