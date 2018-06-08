#ifndef _CLUSTER_
#define _CLUSTER_
#define NEG_INF  -1e300
int affinityPropagation(double** simMatrix, int* idxVec, int n)
{
	int convits = 20;
	int maxiter = 200;
	double lam = 0.5;
	double** A = new double*[n];
	for (int i = 0; i < n; i++)
	{
		A[i] = new double[n];
		memset(A[i], 0, sizeof(double)*n);
	}

	double** R = new double*[n];
	for (int i = 0; i < n; i++)
	{
		R[i] = new double[n];
		memset(R[i], 0, sizeof(double)*n);
	}

	double** AS = new double*[n];
	for (int i = 0; i < n; i++)
		AS[i] = new double[n];

	double** Rp = new double*[n];
	for (int i = 0;i < n; i++)
		Rp[i] = new double[n];
	double* sum = new double[n];

	int** dec = new int*[convits];
	for (int i = 0; i < convits; i++)
	{
		dec[i] = new int[n];
		memset(dec[i], 0, sizeof(int)*n);
	}
	int* decsum = new int[n];
	memset(decsum, 0, sizeof(int)*n);

	int iter = 0;
	int decit = convits;
	bool finish = false;
	int K;

	for(int i = 0; i < n;i++)
		for (int j = 0;j < n; j++)
			simMatrix[i][j] += 1e-16*simMatrix[i][j]*(rand()/((double)RAND_MAX+1));

	while (!finish)
	{
		iter++;
		// Compute Responsibilities
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				AS[i][j] = A[i][j] + simMatrix[i][j];

		for (int i = 0; i < n; i++)
		{
			double max = AS[i][0];
			double secmax =NEG_INF;
			int I = 0;

			for (int j = 1; j < n; j++)
			{
				if (AS[i][j] > secmax)
				{
					if (AS[i][j] > max)
					{
						secmax = max;
						max = AS[i][j];
						I = j;
					}
					else
					{
						secmax = AS[i][j];
					}
				}
			}
			for (int j = 0; j < n; j++)
			{
				if (I == j)
					R[i][j] = (1 - lam) * (simMatrix[i][j] - secmax) + lam * R[i][j];
				else
					R[i][j] = (1 - lam) * (simMatrix[i][j] - max) + lam * R[i][j];
			}
		}

		// Compute Availabilities
		for (int i = 0;i < n; i++)
			for (int j = 0;j < n; j++)
				Rp[i][j] = max(0, R[i][j]);

		for (int j = 0; j < n; j++)
		{
			sum[j] = 0.0;
			for (int i = 0;i < n; i++)
				if (i != j)
					sum[j] += Rp[i][j];
				else
					sum[j] += R[i][j];
		}

		for (int i = 0;i < n; i++)
		{
			for (int k = 0;k < n; k++)
			{
				if (i == k)
					A[i][k] = (1 - lam) * (sum[k] - R[k][k]) + lam * A[i][k];
				else
					A[i][k] = (1 - lam) * min(0, sum[k] - Rp[i][k]) + lam * A[i][k];
			}
		}

		decit++;
		if (decit >= convits)
			decit = 0;

		for (int i = 0;i < n; i++)
			decsum[i] -= dec[decit][i];

		for (int i = 0;i < n; i++)
			if (A[i][i] + R[i][i] > 0.0)
				dec[decit][i] = 1;
			else
				dec[decit][i] = 0;

		K = 0;
		for (int i = 0;i < n; i++)
			K += dec[decit][i];

		for (int i = 0;i < n; i++)
			decsum[i] += dec[decit][i];

		if ((iter >= convits) || (iter >= maxiter))
		{
			bool conv = true;
			for (int i = 0;i < n; i++)
				if ((decsum[i] != 0) && (decsum[i] != convits))
					conv = false;
			if ((conv && K > 0) || iter == maxiter)
				finish = true;
		}
	}

	int count = 0;
	if (K > 0)
	{
		for (int i = 0;i < n; i++)
			if (dec[decit][i] == 1)
				idxVec[count++] = i;
	}

	for (int i = 0; i < n; i++)
	{
		delete[] A[i];
		delete[] R[i];
		delete[] AS[i];
		delete[] Rp[i];
	}
	for (int i = 0;i < convits; i++)
		delete[] dec[i];
	delete[] dec;
	delete[] A;
	delete[] R;
	delete[] AS;
	delete[] Rp;
	delete[] decsum;
	delete[] sum;

	return K;
}
int cluster(double **simMatrix,int nNodes,int *index)
{
	int *idxVec=new int[nNodes];
	double sum = 0.0;
	for (int i = 0;i < nNodes; i++)
	{
		for (int j = 0;j < i; j++)
		{
			sum += simMatrix[i][j];
		}
	}
	sum /= ((nNodes-1)*nNodes/2.0);
	int nMedoids = 0;
	double thres = 0.95, times = 4.0;//-0.005, times = 2.0;
	do {//val = min(thres, v*times);
		///val值越高，分类数越多，val值越低，分类数越少
		double val = min(thres, sum*times);//(sum-1)*times);
		for (int i = 0; i < nNodes; i++)
			simMatrix[i][i] = val;
		//initAffinityMatrix(sum, thres, times);
		nMedoids = affinityPropagation(simMatrix, idxVec, nNodes);
		printf("%f %f : %d\n", thres, times, nMedoids);
		if (nMedoids <= 0)
			thres=0.5+thres*0.5;
	} while (nMedoids <= 0);

	for (int i=0;i<nNodes;i++)
	{
		int maxPos=-1;
		double maxSim=-1;
		for (int j=0;j<nMedoids;j++)
		{
			int t=idxVec[j];
			if (simMatrix[i][t]>maxSim)
			{
				maxPos=t;
				maxSim=simMatrix[i][t];
			}
		}
		index[i]=maxPos;
	}
	for (int i=0;i<nMedoids;i++)
	{
		int t=idxVec[i];
		index[t]=t;
	}
	delete []idxVec;
	return nMedoids;
}

#endif