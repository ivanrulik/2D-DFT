/*
2D DFT
Ivan Alexander Rulik Cote
10/16/2021
*/

/*
run command example: ./test1 Knee.pgm tes-imgOut1.pgm  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc/malloc.h>  
#include <memory.h>

#define max(x, y) ((x>y) ? (x):(y))
#define min(x, y) ((x<y) ? (x):(y))


int xdim;
int ydim;
int maxraw;
int *image;
int *imageOut; // This format does not consider negative values and thus we need to cahnge it

/*
New variables defined for the project
*/

int P; // number of rows of the zero padded image
int Q; // number of columns of the zero padded image
double *Preal;
double *Pimg;
double *RealFou;
double *ImgFou;

void ReadPGM(FILE*);
void WritePGM(FILE*);
/* This new fuction will take a given file and apply 2D DFT to it */
void freqCentering(FILE*);
void scaling8Bit(FILE*);
int minim(int [], int);
int maxim(int [], int);
void zeroPadding(FILE*);
void DFT(FILE*);
void BilinearInterpolation(FILE*, float t);

int main(int argc, char **argv)
{
  int i, j;
  FILE *fp;

  if (argc != 3){
    printf("Usage: MyProgram <input_ppm> <output_ppm> \n");
    printf("       <input_ppm>: PGM file \n");
    printf("       <output_ppm>: PGM file \n");
    exit(0);              
  }

  /* begin reading PGM.... */
  printf("begin reading PGM.... \n");
  if ((fp=fopen(argv[1], "rb"))==NULL){
    printf("read error...\n");
    exit(0);
  }
  ReadPGM(fp);
 
  // your application here 
   /* Calculate the new dimension of the output image by multiplying the 
  scale factor and then rounding it to the closest integer */
  P = (2*xdim)-1; 
  Q = (2*ydim)-1;
  // double *Preal[P*Q];
  // double *Pimg[P*Q];
  // double *RealFou[P*Q];
  // double *ImgFou[P*Q];
  printf("P=%d, Q=%d \n",P,Q);
  freqCentering(fp);
  scaling8Bit(fp);
  zeroPadding(fp);
  printf ("Zero-Padding Finished \n");
  DFT(fp);
  /* Begin writing PGM.... */
  printf("Begin writing PGM.... \n");
  if ((fp=fopen(argv[2], "wb")) == NULL){
     printf("write pgm error....\n");
     exit(0);
   }

  WritePGM(fp);

  free(image);
  free(imageOut);
  free(Preal);
  free(Pimg);
  free(RealFou);
  free(ImgFou);

  return (1);
}



void ReadPGM(FILE* fp)
{
    int c;
    int i,j;
    int val;
    unsigned char *line;
    char buf[1024];


    while ((c=fgetc(fp)) == '#')
        fgets(buf, 1024, fp);
     ungetc(c, fp);
     if (fscanf(fp, "P%d\n", &c) != 1) {
       printf ("read error ....");
       exit(0);
     }
     if (c != 5 && c != 2) {
       printf ("read error ....");
       exit(0);
     }

     if (c==5) {
       while ((c=fgetc(fp)) == '#')
         fgets(buf, 1024, fp);
       ungetc(c, fp);
       if (fscanf(fp, "%d%d%d",&xdim, &ydim, &maxraw) != 3) {
         printf("failed to read width/height/max\n");
         exit(0);
       }
       printf("Width=%d, Height=%d \nMaximum=%d\n",xdim,ydim,maxraw);

       image = (int*)malloc(sizeof(int)*xdim*ydim);
       getc(fp);

       line = (unsigned char *)malloc(sizeof(unsigned char)*xdim);
       for (j=0; j<ydim; j++) {
          fread(line, 1, xdim, fp);
          for (i=0; i<xdim; i++) {
            image[j*xdim+i] = line[i];
         }
       }
       free(line);

     }

     else if (c==2) {
       while ((c=fgetc(fp)) == '#')
         fgets(buf, 1024, fp);
       ungetc(c, fp);
       if (fscanf(fp, "%d%d%d", &xdim, &ydim, &maxraw) != 3) {
         printf("failed to read width/height/max\n");
         exit(0);
       }
       printf("Width=%d, Height=%d \nMaximum=%d,\n",xdim,ydim,maxraw);

       image = (int*)malloc(sizeof(int)*xdim*ydim);
       getc(fp);

       for (j=0; j<ydim; j++)
         for (i=0; i<xdim; i++) {
            fscanf(fp, "%d", &val);
            image[j*xdim+i] = val;
         }

     }

     fclose(fp);
}

void WritePGM(FILE* fp)
{
  int i,j;
  

  fprintf(fp, "P5\n%d %d\n%d\n", P, Q, 255);
  for (j=0; j<Q; j++)
    for (i=0; i<P; i++) {
      // printf("OutPixel = %d\n",imageOut[j*P+i]);
      // fputc(imageOut[j*P+i], fp);
      fputc(RealFou[j*P+i], fp);
    }

  fclose(fp);
  
}

void freqCentering(FILE* fp)
{
  int i,j;
  for (j=0; j<ydim; j++)
  {
    for (i=0; i<xdim; i++) 
    {
      image[j*xdim+i] = image[j*xdim+i]*pow(-1.0,(j+i));
      // printf("index = %d ,  sign = %f ,  pixel Val =%d,\n",(j+i),pow(-1.0,(j+i)),image[j*xdim+i]);
    }
  }
}
void scaling8Bit(FILE* fp)
{
  int i,j;
  int image_min = minim(image,xdim*ydim);
  int image_max = maxim(image,xdim*ydim);
  int out_min = 0;
  int out_max = 255;
  printf("min = %d,  max = %d \n",image_min,image_max);  
  for (j=0; j<ydim; j++)
  {
    for (i=0; i<xdim; i++) 
    {
      // printf("before = %d ,  after =%d,\n",image[j*xdim+i],((image[j*xdim+i] - image_min)*(out_max - out_min)/(image_max - image_min) + out_min));
      image[j*xdim+i] = (image[j*xdim+i] - image_min)*(out_max - out_min)/(image_max - image_min) + out_min;
      
    }
  }
    
}

int minim(int inp[], int len)
{
  int out;
  int i;
	out=inp[0];
	for(i=1;i<len;i++)
        {
    if(inp[i]<out)
			out=inp[i];
		// if(inp[i]>out[1])
		// 	out[1]=inp[i];
	}
	return(out);
}
int maxim(int inp[], int len)
{
  int out;
  int i;
	out=inp[0];
	for(i=1;i<len;i++)
        {
    // if(inp[i]<out)
		// 	out=inp[i];
		if(inp[i]>out)
			out=inp[i];
	}
	return(out);
}

void zeroPadding(FILE* fp)
{
  int i,j;
  imageOut = (int*)malloc(sizeof(int)*P*Q);
  for (j=0; j<Q; j++)
    for (i=0; i<P; i++) {
      if(j>=0 && j<ydim)
      {
        if(i >=0 && i <xdim)
        {
          imageOut[j*P+i] = image[j*xdim+i];
        }
      }
      else
      {
        imageOut[j*P+i] = 0;
      }
    }
}

void DFT(FILE* fp)
{
  int x,y,u,v;
  //Rows
  Preal=(double*)malloc(sizeof(double)*P*Q);
  Pimg=(double*)malloc(sizeof(double)*P*Q);
  RealFou=(double*)malloc(sizeof(double)*P*Q);
  ImgFou=(double*)malloc(sizeof(double)*P*Q);
  for(y = 0;y<Q;y++)
  {
    // printf("Im alive");
    for(u = 0;u<P;u++)
    {
      // printf("Im alive");
      Preal[P*y+u]=0;
      Pimg[P*y+u]=0;
      for(x = 0;x<P;x++)
      {
        // printf("Im alive");
        Preal[P*y+u]+=imageOut[P*y+x]*cos((2*M_PI*u*x)/P);
        Pimg[P*y+u]+=-imageOut[P*y+x]*sin((2*M_PI*u*x)/P);
      }
    }
  }
  printf ("row DFT done \n");
  // Column
  for(x = 0;x<P;x++)
  {
    for(v = 0;v<Q;v++)
    {
      RealFou[P*v+x]=0;
      ImgFou[P*v+x]=0;
      for(y = 0;y<Q;y++)
      {
        RealFou[P*v+x]+=Preal[P*y+x]*cos((2*M_PI*v*y)/Q)+Pimg[P*y+x]*sin((2*M_PI*v*y)/Q);
        ImgFou[P*v+x]+=-Preal[P*y+x]*sin((2*M_PI*v*y)/Q)+Preal[P*y+x]*cos((2*M_PI*v*y)/Q);
      }
    }
  }
  printf ("column DFT done \n");
}



