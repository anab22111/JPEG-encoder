#include "JPEG.h"
#include "NxNDCT.h"
#include <math.h>

#include "JPEGBitStreamWriter.h"


#define DEBUG(x) do{ qDebug() << #x << " = " << x;}while(0)



// quantization tables from JPEG Standard, Annex K
uint8_t QuantLuminance[8*8] =
    { 16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68,109,103, 77,
      24, 35, 55, 64, 81,104,113, 92,
      49, 64, 78, 87,103,121,120,101,
      72, 92, 95, 98,112,100,103, 99 };
uint8_t QuantChrominance[8*8] =
    { 17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99 };

static char quantizationMatrix[64] =
{
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

struct imageProperties{
    int width;
    int height;
    int16_t* coeffs;
};


void DCTUandV(const char input[], int16_t output[], int N, double* DCTKernel)
{
    double* temp = new double[N*N];
    double* DCTCoefficients = new double[N*N];

    double sum;
    for (int i = 0; i <= N - 1; i++)
    {
        for (int j = 0; j <= N - 1; j++)
        {
            sum = 0;
            for (int k = 0; k <= N - 1; k++)
            {
                sum = sum + DCTKernel[i*N+k] * (input[k*N+j]);
            }
            temp[i*N + j] = sum;
        }
    }

    for (int i = 0; i <= N - 1; i++)
    {
        for (int j = 0; j <= N - 1; j++)
        {
            sum = 0;
            for (int k = 0; k <= N - 1; k++)
            {
                sum = sum + temp[i*N+k] * DCTKernel[j*N+k];
            }
            DCTCoefficients[i*N+j] = sum;
        }
    }

    for(int i = 0; i < N*N; i++)
    {
        output[i] = floor(DCTCoefficients[i]+0.5);
    }

    delete[] temp;
    delete[] DCTCoefficients;

    return;
}

uint8_t quantQuality(uint8_t quant, uint8_t quality) {
    // Convert to an internal JPEG quality factor, formula taken from libjpeg
    int16_t q = quality < 50 ? 5000 / quality : 200 - quality * 2;
    return clamp((quant * q + 50) / 100, 1, 255);
}

static void doZigZag(int16_t block[], uint8_t quantizationBlock[], int N, int DCTorQuantization)
{
    // block - niz od 64 broja - isecak slike - i pozitivni i negativni brojecvi(int16_t)
    // quantizatioonBlock - niz od 64 broja - tabele kvantizacije - samo pozitivni brojevi (int8_t)
    // N je velicina bloka ugl uvek 8
    // DCTorQuantization -oznaka da li se radi nad slikom ili tabelom kvantizacije
    /* TO DO */

    // JPEG zigzag mapa
    static const int zigzagMap[64] = {
        0,  1,  8, 16,  9,  2,  3, 10,
        17, 24, 32, 25, 18, 11,  4,  5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13,  6,  7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63
    };

    if (DCTorQuantization == 0) {
        int16_t temp[64];
        for (int i = 0; i < 64; i++) {
            temp[i] = block[zigzagMap[i]];
        }
        for (int i = 0; i < 64; i++) {
            block[i] = temp[i];
        }
    }
    else {
        uint8_t temp[64];
        for (int i = 0; i < 64; i++) {
            temp[i] = quantizationBlock[zigzagMap[i]];
        }
        for (int i = 0; i < 64; i++) {
            quantizationBlock[i] = temp[i];
        }
    }

}

/* perform DCT */
imageProperties performDCT(char input[], int xSize, int ySize, int N, uint8_t quality, bool quantType)
{
    // TO DO

    imageProperties result;
    result.width = N;
    result.height = N;
    result.coeffs = new int16_t[N*N];

    double* DCTKernel = new double[N*N];
    GenerateDCTmatrix(DCTKernel, N);

    // 1. Kopiranje NxN bloka iz slike + level shift za Y
    char block[64];
    for(int row = 0; row < N; row++) {
        for(int col = 0; col < N; col++) {
            if(!quantType) {
                // Y komponenta je uchar (0-255), mora se pomeriti u signed opseg
                // castujemo kroz uint8_t da izbegnemo signed/unsigned UB
                uint8_t pixel = (uint8_t)input[row * xSize + col];
                block[row * N + col] = (char)((int)pixel - 128);
            } else {
                // U i V su vec char (-128 do 127), bez shifta
                block[row * N + col] = input[row * xSize + col];
            }
        }
    }

    // 2. DCT transformacija
    DCTUandV(block, result.coeffs, N, DCTKernel);

    // 3. Pripremi tabelu kvantizacije sa quality faktorom
    // quantQuality se poziva za svaki element, pa tek onda delimo
    uint8_t* baseTable = quantType ? QuantChrominance : QuantLuminance;
    uint8_t quantTable[64];
    for(int i = 0; i < N*N; i++) {
        quantTable[i] = quantQuality(baseTable[i], quality);
    }

    // 4. Kvantizacija - deli pa zaokruzi
    // quantTable je u linearnom redosledu (nije zigzag)
    // pa mora biti pre zigzaga!
    for(int i = 0; i < N*N; i++) {
        result.coeffs[i] = (int16_t)round((double)result.coeffs[i] / (double)quantTable[i]);
    }

    // 5. Zigzag NAKON kvantizacije
    doZigZag(result.coeffs, nullptr, N, 0);

    delete[] DCTKernel;
    return result;

}

//JPEGBitStreamWriter streamer("example.jpg");
void performJPEGEncoding(uchar Y_buff[], char U_buff[], char V_buff[], int xSize, int ySize, int quality)
{
    DEBUG(quality);


    auto s = new JPEGBitStreamWriter("example.jpg");
    // TO DO

    // osnovni nizovi za tabele kvantizacije
    uint8_t QL[64];
    uint8_t QC[64];

    // skaliranje tabela - podesavanje kvaliteta
    for(int i = 0; i < 64; i++){
        QL[i] = quantQuality(QuantLuminance[i], quality);
        QC[i] = quantQuality(QuantChrominance[i], quality);
    }

    // za zapis u header mora redosled biti zigzag
    doZigZag(nullptr, QL, 8, 1);
    doZigZag(nullptr, QC, 8, 1);

    // upis u fajl tj header
    // potrebno kako bi se znalo enkodovati
    // sadrzi osnovne podatke  o slici
    s->writeHeader();
    s->writeQuantizationTables(QL, QC); // Sada šaljemo modifikovane QL i QC
    s->writeImageInfo(xSize, ySize);
    s->writeHuffmanTables();



    // potrebno je proci kroz celu sliku i obraditi 8*8 blokove preko DCT-a
    // posto se upisuje 4 bloka Y pa blok u i blok V
    // petlja ide po 16 piksela u sirinu i duzinu (dva bloka u redu dva u koloni = 4 bloka)
    for(int j = 0; j < ySize; j += 16){
        for(int i = 0; i < xSize; i+= 16){
            //imageProperties performDCT(char input[], int xSize, int ySize, int N, uint8_t quality, bool quantType)
            // quantType ukoliko je false koristi se QuantLuminance to je za Y
            // quantType ukoliko je true koristi se QuantChrominance to je za U i V

            imageProperties blockY1 = performDCT(reinterpret_cast<char*>(&Y_buff[j * xSize + i]), xSize, ySize, 8, quality, false);
            s->writeBlockY(blockY1.coeffs);
            imageProperties blockY2 = performDCT(reinterpret_cast<char*>(&Y_buff[j * xSize + (i+8)]), xSize, ySize, 8, quality, false);
            s->writeBlockY(blockY2.coeffs);
            imageProperties blockY3 = performDCT(reinterpret_cast<char*>(&Y_buff[(j+8) * xSize + i]), xSize, ySize, 8, quality, false);
            s->writeBlockY(blockY3.coeffs);
            imageProperties blockY4 = performDCT(reinterpret_cast<char*>(&Y_buff[(j+8) * xSize + (i+8)]), xSize, ySize, 8, quality, false);
            s->writeBlockY(blockY4.coeffs);

            imageProperties blockU = performDCT(&U_buff[(j / 2) * (xSize / 2) + (i / 2)], xSize / 2, ySize / 2, 8, quality, true);
            s->writeBlockU(blockU.coeffs);

            imageProperties blockV = performDCT(&V_buff[(j / 2) * (xSize / 2) + (i / 2)], xSize / 2, ySize / 2, 8, quality, true);
            s->writeBlockV(blockV.coeffs);
        }
    }

    s ->finishStream();
    delete s;
}
