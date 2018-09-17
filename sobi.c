#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <stdint.h>
#include <string.h>
#include <mat.h>
#include <mkl_lapacke.h>
#include <mkl_cblas.h>
#include <omp.h>
#include <time.h>

// EEG_DATA 是 128 x 81626 的矩阵
#define EEG_COLS 81626
#define EEG_LINE 128

// 按列分块
#define BLOCK_COL 1000

double maxValue(double *array, int n) {
    if (n <= 0 || array == NULL) return NAN;
    double m = array[0];
	int i;
#pragma omp parallel for
    for (i=0; i<n; i++) {
        if (array[i] > m) m = array[i];
    }
    return m;
}

// 矩阵转置，原矩阵 M行N列
// dst = src'
void transposef(float *src, size_t m, size_t n, float *dst) {
	if (src == NULL || dst == NULL) return;

	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < n; j++) {
			dst[j*m + i] = src[i*n + j];
		}
	}
}

void transpose(double *src, size_t m, size_t n, double *dst) {
	if (src == NULL || dst == NULL) return;

	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < n; j++) {
			dst[j*m + i] = src[i*n + j];
		}
	}
}

void SOBI(double (*EEG_DATA)[EEG_COLS], double *TAU, int nTAU, double fs, double jthresh) 
{
	int i,t,j,c;

    // 第一部分

    // TODO: 加入输入检测

	// TAU预处理
#pragma omp parallel for
	for (i = 0; i < nTAU; i++)
	{
		TAU[i] = round((TAU[i] / 1000.0) * fs);
	}
    double maxTAU = maxValue(TAU, nTAU);

	// 矩阵 X，128x1350
	int colX = BLOCK_COL + (int)maxTAU;
	#define MAT_X(i,j) X[i*colX+j]
	double *X = (double*)calloc(EEG_LINE*colX, sizeof(double));

	// 矩阵 Y, 128x1000
	#define MAT_Y(i,j) Y[i*BLOCK_COL+j]
	double *Y = (double*)calloc(EEG_LINE*BLOCK_COL, sizeof(double));

	// 矩阵 Z, 128x1000
	#define MAT_Z(i,j) Z[i*BLOCK_COL+j]
	double *Z = (double*)calloc(EEG_LINE*BLOCK_COL, sizeof(double));

	// 矩阵 RR, 128x128
	#define MAT_RR(i,j) RR[i*EEG_LINE+j]
	double *RR = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));

	// RTAU 第一部分的结果矩阵 128x128x42
	int pRTAU = nTAU;
	#define MAT_RTAU(i,j,k) RTAU[i*EEG_LINE*pRTAU + j*pRTAU + k]
	double *RTAU = (double*)calloc((size_t)EEG_LINE*EEG_LINE*pRTAU, sizeof(double));

    int end = EEG_COLS - BLOCK_COL - (int)maxTAU;
	int TT = 0;
    for (t=0; t<end; t+=BLOCK_COL) 
	{
		// X=EEG_DATA[][t ~ t+1349] 再减去每行的均值
		for (i = 0; i < EEG_LINE; i++) 
		{
			double avg = 0.0f;
			for (j = 0; j < colX; j++) 
			{
				avg += EEG_DATA[i][j + t];
				MAT_X(i,j) = EEG_DATA[i][j + t];
			}
			avg /= (double)colX;
			for (j = 0; j < colX; j++) MAT_X(i, j) -= avg;
		}
		// Y=X[][0-999]
		for (i=0; i<EEG_LINE; i++) 
		{
			for (j=0; j<BLOCK_COL; j++) 
			{
				MAT_Y(i,j) = MAT_X(i,j);
			}
		}
		for (c = 0; c < nTAU; c++) 
		{
			int cTAU = (int)TAU[c];

			// Z = X[][0~cTAU+999]
			for (i=0; i<EEG_LINE; i++) 
			{
				for (j=0; j<BLOCK_COL; j++) 
				{
					MAT_Z(i,j) = MAT_X(i, j+cTAU);
				}
			}

			// RR = Y*Z'
			cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, 
				EEG_LINE, EEG_LINE, BLOCK_COL, 1.0f, Y, BLOCK_COL, Z, BLOCK_COL, 0.0f, RR, EEG_LINE);
			
			// RTAU[][][c] = 0.5*(RR+RR')
			for (i = 0; i < EEG_LINE; i++) 
			{
				for (j = 0; j < EEG_LINE; j++) 
				{
					MAT_RTAU(i,j,c) += 0.5 * (double)(MAT_RR(i, j) + MAT_RR(j, i));
				}
			}

		}
		TT += BLOCK_COL;
		printf("Current block samples: %d-%d\n", t, t + BLOCK_COL);
    }

	for (int c = 0; c < nTAU; c++) {
		for (int i = 0; i < EEG_LINE; i++) {
			for (int j = 0; j < EEG_LINE; j++) {
				MAT_RTAU(i, j, c) /= (double)TT;
			}
		}
	}

	free(X);
	free(Y);
	free(Z);
	free(RR);

    // 第二部分

	// 矩阵 RX, 128x128
	// RX = 0.5*(RTAU[][][0]+RTAU[][][0]')
	#define MAT_RX(i,j) RX[i*EEG_LINE+j]
	double *RX = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));
	for (int i = 0; i < EEG_LINE; i++) {
		for (int j = 0; j < EEG_LINE; j++) {
			MAT_RX(i, j) = 0.5 * (MAT_RTAU(i,j,0) + MAT_RTAU(j,i,0));
		}
	}
	
	// 求矩阵RX特征值与特征向量
	// 特征值LAM 特征向量保存回RX
	double LAM[EEG_LINE];
	LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U', EEG_LINE, RX, EEG_LINE, LAM);

	// LAM上下翻转，RX左右翻转
	for (int j=0; j<EEG_LINE/2; j++) {
		double tmp = LAM[j];
		LAM[j] = LAM[EEG_LINE-j-1];
		LAM[EEG_LINE-j-1] = tmp;

		for (int i=0; i<EEG_LINE; i++) {
			tmp = MAT_RX(i,j);
			MAT_RX(i, j) = MAT_RX(i,EEG_LINE-j-1);
			MAT_RX(i, EEG_LINE-j-1) = tmp;
		}
	}
	
	// 矩阵 LX, 128x128
	// LX = diag(sqrt(1 ./(LAM+1e-20)))
	#define MAT_LX(i,j) LX[i*EEG_LINE+j]
	double *LX = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));
	for (int i=0; i<EEG_LINE; i++) {
		MAT_LX(i,i) = sqrtf(1.0f / (LAM[i] + 1e-20f));
	}

	// 矩阵 BM0，128x128
	// BM0 = RX*LX
	#define MAT_BM0(i,j) BM0[i*EEG_LINE+j]
	double *BM0 = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, EEG_LINE, 
		EEG_LINE, EEG_LINE, 1.0, RX, EEG_LINE, LX, EEG_LINE, 0.0, BM0, EEG_LINE);

	// 矩阵 BM, 128x128
	// BM = BM0'
	#define MAT_BM(i,j) BM[i*EEG_LINE+j]
	double *BM = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));
	transpose(BM0, EEG_LINE, EEG_LINE, BM);




	nTAU--;
	printf("Presphering correlation matrices...\n");

	// 矩阵 R0, 128x128
	#define MAT_R0(i,j) R0[i*EEG_LINE+j]
	double *R0 = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));

	// 矩阵 R1, 128x128
	#define MAT_R1(i,j) R1[i*EEG_LINE+j]
	double *R1 = (double*)calloc(EEG_LINE*EEG_LINE, sizeof(double));

	// RTAU_presphered 矩阵 128x128x41
	int pRTAUpresph = nTAU;
	#define MAT_RTAU_presphered(i,j,k) RTAU_presphered[i*EEG_LINE*pRTAUpresph + j*pRTAUpresph + k]
	double *RTAU_presphered = (double*)calloc((size_t)EEG_LINE*EEG_LINE*pRTAUpresph, sizeof(double));


	for (int c=0; c<nTAU; c++) {

		for (int i=0; i<EEG_LINE; i++) {
			for (int j=0; j<EEG_LINE; j++) {
				MAT_R0(i,j) = MAT_RTAU(i, j, c+1);
			}
		}

		// R1 = BM*R0
		// R0 = R1*BM0
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, EEG_LINE, 
			EEG_LINE, EEG_LINE, 1.0, BM, EEG_LINE, R0, EEG_LINE, 0.0, R1, EEG_LINE);
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, EEG_LINE, 
			EEG_LINE, EEG_LINE, 1.0, R1, EEG_LINE, BM0, EEG_LINE, 0.0, R0, EEG_LINE);

		for (int i=0; i<EEG_LINE; i++) {
			for (int j=0; j<EEG_LINE; j++) {
				MAT_RTAU_presphered(i,j,c) = MAT_R0(i,j);
			}
		}
	}

	int nA = EEG_LINE * pRTAU;
	#define MAT_A(i,j) RTAU[i*nA + j]


/*
	// 复数矩阵 B, 3x3
	// 以下两列组成一个数，第一列为实部，第二列为虚部
	// MAT_B 宏中的 real, 为 true 取实部，为 false 取虚部
	#define MAT_B(i,j,real) real? B[i*3*2+j*2] : B[i*3*2+j*2+1]
	double B[3*3*2] = {
		1.0, 0.0,      0.0, 0.0,      0.0, 0.0,
		0.0, 0.0,      1.0, 0.0,      1.0, 0.0,
		0.0, 0.0,      0.0, -1.0,     0.0, 1.0
	};

	// BT = B'
	#define MAT_BT(i,j,real) real? BT[i*3*2+j*2] : BT[i*3*2+j*2+1]
	double BT[3*3*2] = {
		1.0, 0.0,      0.0, 0.0,      0.0, 0.0,
		0.0, 0.0,      1.0, 0.0,      0.0, 1.0,
		0.0, 0.0,      1.0, 0.0,      0.0, -1.0
	};

*/
	// 128x128 单位阵 V
	double *V = R0;
	#define MAT_V(i,j) MAT_R0(i,j)
	for (int i=0; i<EEG_LINE; i++) {
		for (int j=0; j<EEG_LINE; j++) {
			MAT_V(i, j) = (i==j) ? 1.0 : 0.0;
		}
	}

	int encore = 1;
	int iter_count = 0;
	double smax = 0.0;

/*	while (encore) {
		encore = 0;
		smax = 0.0;
		for(int p=1; p<=;) {
			for(;;) {

			}
		}	
	}
*/	
	/*
	MATFile *file = matOpen("test.mat", "w");
	mxArray *array = mxCreateNumericMatrix(EEG_LINE, EEG_LINE, mxDOUBLE_CLASS, mxREAL);
	double *data = (double*)mxGetPr(array);
	transpose(BM, EEG_LINE, EEG_LINE, data);
	matPutVariable(file, "Baa", array);
	matClose(file);
*/

	free(RTAU);
	free(RTAU_presphered);
	free(R0);
	free(R1);
	free(RX);
	free(LX);
	free(BM0);
	free(BM);


	
/*
	MATFile *file = matOpen("B.mat", "w");
	mxArray *array = mxCreateNumericMatrix(EEG_LINE, EEG_LINE, mxDOUBLE_CLASS, mxREAL);
	double *data = (double*)mxGetPr(array);
	transpose(B, EEG_LINE, EEG_LINE, data);
	matPutVariable(file, "Baa", array);
	matClose(file);
*/
}

int main(int argc, char **argv) {
	double start, end;
	int i, j;
	start = omp_get_wtime();
	printf("Loading DATA.mat...\n");

	MATFile *file = matOpen("DATA.mat", "r");
	if (file == NULL) {
		printf("DATA.mat not found.\n");
		return 0;
	}

	double(*EEG_DATA)[EEG_COLS] =
		(double(*)[EEG_COLS])calloc(sizeof(double), (size_t)EEG_LINE*EEG_COLS);

	// MATLAB是以列优先存储，在C语言环境里相当于是 81626 x 128
	// 所以这里要转置一下
	float *data = (float*)mxGetPr(matGetVariable(file, "DATA"));
//#pragma omp parallel for private(j)
		for (i = 0; i < EEG_COLS; i++)
		{
			for (j = 0; j < EEG_LINE; j++)
			{
				EEG_DATA[j][i] = (double)data[i*EEG_LINE + j];
			}
		}
		//for (i = 0; i < EEG_LINE; i++)
		//{
		//	for (j = 0; j < EEG_COLS; j++)
		//	{
		//		EEG_DATA[i][j] = (double)data[i + j*EEG_LINE];
		//	}
		//}
	//transposef(data, EEG_COLS, EEG_LINE, *EEG_DATA);
	matClose(file);
	
    double tau[42] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                   12, 14, 16, 18, 20,
                   25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100,
                   125, 150, 175, 200, 225, 250, 275, 300, 325, 350};

    SOBI(EEG_DATA, tau, 42, 1000.0, 1e-5);
	end = omp_get_wtime();
	printf("%f second\n", (end - start));
	system("pause");
	return 0;
}
