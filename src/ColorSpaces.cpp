#include "ColorSpaces.h"

/********************************************************************************************************************************/
/* RGB processing */
/********************************************************************************************************************************/
void processing_RGB(const uchar rgbInputImg[], int x, int y, uchar rgbOutputImg[], double R, double G, double B)
{
	for(int i = 0; i< x; i++)
	{
		for(int j=0; j<y; j++)
		{
			rgbOutputImg[j*x*3+i*3] = R*rgbInputImg[j*x*3+i*3];
			rgbOutputImg[j*x*3+i*3+1] = G*rgbInputImg[j*x*3+i*3+1];
			rgbOutputImg[j*x*3+i*3+2] = B*rgbInputImg[j*x*3+i*3+2];
		}
	}
}

/********************************************************************************************************************************/
/* YUV444 processing */
/********************************************************************************************************************************/
void RGBtoYUV444(const uchar rgbImg[], int x, int y, uchar Y_buff[], char U_buff[], char V_buff[]) 
{
	uchar R, G, B;
	for(int i = 0; i<x; i++)
	{
		for(int j = 0; j<y; j+=1)
		{
			R = rgbImg[j*3*x+i*3];
			G = rgbImg[j*3*x+i*3 + 1];
			B = rgbImg[j*3*x+i*3 + 2];

			Y_buff[j*x+i] =  0.299*R + 0.587*G + 0.114*B;
			U_buff[j*x+i] =  - 0.14713*R - 0.28886*G + 0.436*B;
			V_buff[j*x+i] =  R*0.615 - 0.51499*G - 0.10001*B;
		}
	}
}

void YUV444toRGB(const uchar Y_buff[], const char U_buff[], const char V_buff[], int x, int y, uchar rgbImg[]) 
{
	double R,G,B;
	double Y, U, V;
	for(int i = 0; i<x; i++)
	{
		for(int j = 0; j<y; j+=1)
		{
			Y = Y_buff[j*x+i];
			U = U_buff[j*x+i];
			V = V_buff[j*x+i];

			R = Y + 1.13983*V;
			G = Y - 0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;
			
			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G< 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;


			rgbImg[j*3*x+i*3] =  R;
			rgbImg[j*3*x+i*3 + 1] = G;
			rgbImg[j*3*x+i*3 + 2] =  B;
	
		}
	}
}

void procesing_YUV444(uchar Y_buff[], char U_buff[], char V_buff[], int x, int y, double Y, double U, double V)
{
	for(int i = 0; i<x; i++)
	{
		for(int j = 0; j<y; j++)
		{
			Y_buff[j*x+i] *= Y; 
			U_buff[j*x+i] *= U; 
			V_buff[j*x+i] *= V; 
		}
	}

}

/*******************************************************************************************************************************/
/* YUV422 processing */
/********************************************************************************************************************************/
void RGBtoYUV422(const uchar rgbImg[], int x, int y, uchar Y_buff[], char U_buff[], char V_buff[]) 
{
	uchar R, G, B;
	for(int i = 0; i<x; i+=2)
	{
		for(int j = 0; j<y; j++)
		{
			R = rgbImg[j*3*x+i*3];
			G = rgbImg[j*3*x+i*3 + 1];
			B = rgbImg[j*3*x+i*3 + 2];

			Y_buff[j*x+i] =  0.299*R + 0.587*G + 0.114*B;
			U_buff[j*x/2+i/2] =  (- 0.14713*R - 0.28886*G + 0.436*B)/2;
			V_buff[j*x/2+i/2] =  (R*0.615 - 0.51499*G - 0.10001*B)/2;

			R = rgbImg[j*3*x+(i+1)*3];
			G = rgbImg[j*3*x+(i+1)*3 + 1];
			B = rgbImg[j*3*x+(i+1)*3 + 2];

			Y_buff[j*x+(i+1)] =  0.299*R + 0.587*G + 0.114*B;
			U_buff[j*x/2+i/2] +=  (- 0.14713*R - 0.28886*G + 0.436*B)/2;
			V_buff[j*x/2+i/2] +=  (R*0.615 - 0.51499*G - 0.10001*B)/2;
		}
	}
}

void YUV422toRGB(const uchar Y_buff[], const char U_buff[], const char V_buff[], int x, int y, uchar rgbImg[]) 
{
	double R,G,B;
	double Y, U, V;
	for(int i = 0; i<x; i+=2)
	{
		for(int j = 0; j<y; j++)
		{
			U = U_buff[j*x/2+i/2];
			V = V_buff[j*x/2+i/2];

			Y = Y_buff[j*x+i];

			R = Y + 1.13983*V;
			G = Y -0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G < 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[j*3*x+i*3] =  R;
			rgbImg[j*3*x+i*3 + 1] = G;
			rgbImg[j*3*x+i*3 + 2] = B;

			Y = Y_buff[j*x+(i+1)];

			R = Y + 1.13983*V;
			G = Y -0.39465*U - 0.58060*V;
			B = Y + 2.03211*U;

			if (R < 0)
				R = 0;
			else if (R > 255)
				R = 255;
			if (G< 0)
				G = 0;
			else if (G> 255)
				G = 255;
			if (B < 0)
				B = 0;
			else if (B > 255)
				B = 255;

			rgbImg[j*3*x+(i+1)*3] =  R;
			rgbImg[j*3*x+(i+1)*3 + 1] = G;
			rgbImg[j*3*x+(i+1)*3 + 2] = B;
		}
	}
}

void procesing_YUV422(uchar Y_buff[], char U_buff[], char V_buff[], int x, int y, double Y, double U, double V)
{
	for(int i = 0; i<x; i+=2)
	{
		for(int j = 0; j<y; j++)
		{
			Y_buff[j*x+i] *= Y; 
			Y_buff[j*x+i+1] *= Y; 
			U_buff[j*x/2+i/2] *= U; 
			V_buff[j*x/2+i/2] *= V; 
		}
	}

}

/*******************************************************************************************************************************/
/* YUV420 processing */
/*******************************************************************************************************************************/
void RGBtoYUV420(const uchar rgbImg[], int x, int y, uchar Y_buff[], char U_buff[], char V_buff[]) 
{
    uchar R, G, B;
    double sumU, sumV;

    // Prolazimo kroz sliku u koracima od po 2 piksela (pravimo 2x2 blokove)
    for (int j = 0; j < y; j += 2) {
        for (int i = 0; i < x; i += 2) {

            sumU = 0;
            sumV = 0;

            // Unutrašnja petlja koja obrađuje 4 piksela unutar jednog 2x2 bloka
            for (int jb = 0; jb < 2; jb++) {
                for (int ib = 0; ib < 2; ib++) {

                    // Indeks trenutnog piksela u RGB nizu
                    int current_pixel_idx = ((j + jb) * x + (i + ib)) * 3;

                    R = rgbImg[current_pixel_idx + 0];
                    G = rgbImg[current_pixel_idx + 1];
                    B = rgbImg[current_pixel_idx + 2];

                    // 1. Izračunaj Y (luminansu) za svaki od 4 piksela u punoj rezoluciji
                    Y_buff[(j + jb) * x + (i + ib)] = (uchar)(0.299 * R + 0.587 * G + 0.114 * B);

                    // 2. Akumuliraj U i V vrednosti za ovaj blok (biće podeljeno sa 4 kasnije)
                    sumU += (-0.14713 * R - 0.28886 * G + 0.436 * B);
                    sumV += (0.615 * R - 0.51499 * G - 0.10001 * B);
                }
            }

            // 3. Upiši prosečnu vrednost boje u smanjeni U i V bafer (Chroma Subsampling)
            // Indeks u manjem baferu je (j/2 * širina/2 + i/2)
            U_buff[(j / 2) * (x / 2) + (i / 2)] = (char)(sumU / 4.0);
            V_buff[(j / 2) * (x / 2) + (i / 2)] = (char)(sumV / 4.0);
        }
    }
}



void YUV420toRGB(const uchar Y_buff[], const char U_buff[], const char V_buff[], int x, int y, uchar rgbImg[]) 
{
	double R,G,B;
	double Y, U, V;

	// TO DO
    for (int j = 0; j < y; j++) {
        for (int i = 0; i < x; i++) {

            Y = Y_buff[j * x + i];

            // U i V iz manjeg bafera (x/2, y/2)
            U = U_buff[(j / 2) * (x / 2) + (i / 2)];
            V = V_buff[(j / 2) * (x / 2) + (i / 2)];

            R = Y + 1.13983 * V;
            G = Y - 0.39465 * U - 0.58060 * V;
            B = Y + 2.03211 * U;

            // clamp
            if (R < 0) R = 0; else if (R > 255) R = 255;
            if (G < 0) G = 0; else if (G > 255) G = 255;
            if (B < 0) B = 0; else if (B > 255) B = 255;

            rgbImg[(j * x + i) * 3]     = (uchar)R;
            rgbImg[(j * x + i) * 3 + 1] = (uchar)G;
            rgbImg[(j * x + i) * 3 + 2] = (uchar)B;
        }
    }

}

void procesing_YUV420(uchar Y_buff[], char U_buff[], char V_buff[], int x, int y, double Y, double U, double V)
{
	// TO DO
    for (int j = 0; j < y; j++) {
        for (int i = 0; i < x; i++) {
            Y_buff[j * x + i] *= Y;
        }
    }

    for (int j = 0; j < y / 2; j++) {
        for (int i = 0; i < x / 2; i++) {
            U_buff[j * (x / 2) + i] *= U;
            V_buff[j * (x / 2) + i] *= V;
        }
    }

}

/*******************************************************************************************************************************/
/* Y decimation */
/*******************************************************************************************************************************/
void decimate_Y(uchar Y_buff[], int x, int y)
{
	uchar YY;
	// TO DO
    for (int j = 0; j < y; j += 2) {
        for (int i = 0; i < x; i += 2) {

            uchar avg = (
                            Y_buff[j * x + i] +
                            Y_buff[j * x + i + 1] +
                            Y_buff[(j + 1) * x + i] +
                            Y_buff[(j + 1) * x + i + 1]
                            ) / 4;

            Y_buff[j * x + i] = avg;
        }
    }

}
