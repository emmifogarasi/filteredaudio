#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <math.h>
#include <complex.h>


// defining the SDL Window width and height, as well as the total amount of bytes from the byte that are used for fft
#define winWidth 1000
#define winHeight 600
#define arrayMax 8192


// global variables for the mathematics of fft
double PI;
typedef _Dcomplex cplx;


//importing the raw data from the audio.raw file
char* readAudioData(const char* name)
{
	
	FILE *filePointer = fopen("audio.raw", "rb");       //opening a read file
	fseek(filePointer, 0, SEEK_END);                    //goes to the end of the file         
	long sizeofFile = ftell(filePointer);                 
	//printf("%d", sizeofFile);                        // figuring out how big the file is exactly (11331)
	unsigned char* audiodata = malloc(sizeofFile);     // we create an audiodata array
	fseek(filePointer, 0, SEEK_SET);                   
	fread(audiodata, 1, sizeofFile, filePointer);      //we put the data into audiodata array
	fclose(filePointer);
	

	return audiodata;
}

//Part 1 start of reference used from https://rosettacode.org/wiki/Fast_Fourier_transform#C 
//Fast Fourier Transform 
void _fft(cplx buf[], cplx out[], int n, int step)         //putting in an input array and an output array, the array size and the step
{
	if (step < n) 
	{
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);         //recursive algorithm

		for (int k = 0; k < n; k += 2 * step) {

			// cplx t = cexp(-I * PI * k / n) * out[k + step];  (reference used from Rosetta - had to adapt it to work in Visual Studio 2019)

			double piin = PI * -1 * k / n;                // 
			_Dcomplex ima = _Cbuild(0, 1);                //imaginary number (square root of -1)
			_Dcomplex impiin = _Cmulcr(ima, piin);        
			cplx t1 = cexp(impiin);
			cplx t = _Cmulcc(t1, out[k + step]);

			buf[k / 2] = _Cbuild(creal(out[k]) + creal(t), cimag(out[k]) + cimag(t));        
			buf[(k + n) / 2] = _Cbuild(creal(out[k]) - creal(t), cimag(out[k]) - cimag(t));     
		}
	}
}


void fft(cplx buf[], int n)
{

	cplx out[arrayMax];

	for (int i = 0; i < n; i++) 
	{
		out[i] = _Cbuild(creal(buf[i]), cimag(buf[i]));        //putting buf into out
	}

	_fft(buf, out, arrayMax, 1);

}
//Part 1 End of reference from https://rosettacode.org/wiki/Fast_Fourier_transform#C

//main
int main(int argc, char* argv[])
{
	int i;
	
	char* data = readAudioData("audio.raw");
	for (i = 0; i < arrayMax; i++)                                 //putting the data into an array
	{
		unsigned int amp = data[i];                                      
		data[i] = data[i] - 128;                                  // taking away 128 from it to make the bytes signed instead of unsigned
		//printf("BEGINNING DATA: %d\n", data[i]);                //checking not raw data - this one is modified so that -128 
	}

	//part 2 Start of reference https://rosettacode.org/wiki/Fast_Fourier_transform#C
	cplx buf[arrayMax];           //buf is a complex array for the fft
	PI = atan2(1, 1) * 4;         //calculating PI

	
	for (i = 0; i < arrayMax; i++)         
	{
		buf[i] = _Cbuild(data[i], 0);        //raw data goes into buf, imaginary part is 0
	}
	

	fft(buf, arrayMax);                     //doing fft on raw data, getting the results in buf

	
	double freqamp[arrayMax];                   //creating a double array for the frequency domain
	
	for (i = 0; i < arrayMax; i++)
	{
		freqamp[i] = sqrt(norm(buf[i]));        //calculating the magnitude and putting it in freqamp
		//printf("%f\n", freq[i]);              //checking what freqamp[i] produces

	}
	//part 2 End of reference from https://rosettacode.org/wiki/Fast_Fourier_transform#C

	
	for (i = 0; i < arrayMax; i++)          // FILTERING AMPLITUDE THAT'S ABOVE 8K
	{
		if (freqamp[i] > 8000)
		{
			freqamp[i] = 0;
			buf[i] = _Cbuild(0, 0);        //making sure the buf is Nulled as well
		}
	}

	//Start of Inverse Fast Fourier Transform ( reference from https://www.dsprelated.com/showarticle/800.php?fbclid=IwAR06l1dKYEsxoq9beiPDWdECFnHWC8HNvvi5ZgeRDy-y7aaE-qExMfC60oU )
	for (i = 0; i < arrayMax; i++)
	{
		buf[i] = _Cbuild(cimag(buf[i]), creal(buf[i]));       //INVERSE FFT PART ONE (switching the real and imaginary values)
		
	}


	fft(buf, arrayMax);                                   

	for (i = 0; i < arrayMax; i++)
	{
		buf[i] = _Cbuild(cimag(buf[i]) / arrayMax, creal(buf[i]) / arrayMax);       //INVERSE FFT PART TWO (switching the real and imainary values back and dividing them by N (arrayMax)
		//printf("CIMAG %f\n", cimag(buf[i]));
	}

	
	double data2[arrayMax];

	for (i = 0; i < arrayMax; i++)                   
	{
		data2[i] = creal(buf[i]);                                       // final data as double (because the imaginary numbers are 0)
		//printf("data %f\n", data2[i]);

	}
	//End of Inverse Fast Fourier Transform ( reference used from https://www.dsprelated.com/showarticle/800.php?fbclid=IwAR06l1dKYEsxoq9beiPDWdECFnHWC8HNvvi5ZgeRDy-y7aaE-qExMfC60oU )


	char datafin[arrayMax];                                          

	for (i = 0; i < arrayMax; i++)
	{
		datafin[i] = (char)trunc(data2[i]);                                 //putting the final data for the filtered audio into a signed char array - cutting data off after the dot
		//printf("FINAL FILTERED BYTES: %d\n", datafin[i]);                   
	}
	
	FILE* filePointerfinal = fopen("filteredAudio8000.raw", "wb");         // putting the final filtered audio data in a raw file
	fwrite(datafin, 1, arrayMax, filePointerfinal);
	fclose(filePointerfinal);             

	
	printf("Initialising SDL.\n");

	int go;


	SDL_Window* window;
	SDL_Renderer* renderer;


	//initializing SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		perror("Cannot initialize SDL."); //if there's an error it returns to -1
		return -1;
	}

	//creating the window 
	window = SDL_CreateWindow("part a", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	//we must call SDL_CreateRenderer in order for draw calls to affect this window
	renderer = SDL_CreateRenderer(window, -1, 0);

	//making sure the window stays open
	go = 1;
	while (go)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				go = 0;
				break;
			}
		}

		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  //select the colour for sdl window
		SDL_RenderClear(renderer);	


		// drawing the time domain (data for unfiltered, data2 for filtered
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  //select the colour for drawing
		for (int j = 0; j < arrayMax; j++ )
		{
			
			unsigned int iamp = (int)data2[j] + 300;

			SDL_RenderDrawPoint(renderer, j * 0.12 ,iamp);
		}


		// drawing the frequency domain
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   //select the colour for drawing
		for (int j = 0; j < arrayMax; j++)
		{

			SDL_RenderDrawLine(renderer, j * 0.12 , freqamp[j] * -0.05 + 600, j * 0.12 , 600 );  

		}

		//update the renderer
		SDL_RenderPresent(renderer);

	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;

}





