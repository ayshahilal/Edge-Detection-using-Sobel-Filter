
//Sobel filtresi ile edge detection yapma (pgm)


#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

//Normalizasyon icin olusturulan fonksiyon
void karaktereCevir(unsigned char **ptrK,int **final,int rows,int cols){
	int i,j;
	
	for(i=0;i<rows;i++){
		for(j=0;j<cols;j++){
			ptrK[i][j] = final[i][j];
		}
	}
}

void dosyayaYaz(FILE *fp, unsigned char **ptrK,int cols,int rows,int maxval){
	
	int i;
	
	fprintf(fp,"P5\n");
    fprintf(fp,"%d %d\n", cols, rows);
    fprintf(fp, "%d", maxval);

    for (i=0; i < rows; i++) {
        fwrite(&(ptrK[i][0]),sizeof(unsigned char),cols, fp);
    }	
    fclose(fp);
    
}

void normalization(int **mtr, int **final,int rows2, int cols2, int maxval){
	
	int sayi,x,y;
	int pixel_value;
	
	int min = 0;
    int max = maxval;
    
    for(x=1 ; x<rows2-2 ; x++){
    	for(y=1 ; y<cols2-2 ; y++){
    	//	printf("%d ", mtr[x][y]);
    		sayi = mtr[x][y];
    		if (sayi < min) min = sayi; 
      		if (sayi > max) max = sayi; 
		}	
	}
	
	for(x=1; x<rows2-2 ; x++){
    	for(y=1; y<cols2-2 ; y++){
    		pixel_value = mtr[x][y];
      		final[x][y] = round( ( (255*(pixel_value - min) ) / (max-min) ));
      		
		}
	}
}

void padding(int **final,int rows,int cols){
	int i,j;
	/*
	for(i=0;i<rows;i++){
  		for(j=0;j<cols;j++){
  		final[i][j] = 0;
	  }    		
	}
	*/
	

    for(i=0;i<rows;i++){
    	final[i][0] = 0;
    	final[i][rows-1] = 0;	
	}
	
    for(j=1;j<cols;j++){
    	final[0][j] = 0;
    	final[cols-1][j] =0;
	}
}

void SkipComments(FILE *fp){
	int ch;			/* dosyadan karakter okumak icin*/
	char line[100];
	while( (ch=fgetc(fp)) != EOF && isspace(ch) ){
		;
	}
	if(ch == '#'){
		fgets(line, sizeof(line), fp);
		SkipComments(fp);
	}
	else{
		fseek(fp,-1,SEEK_CUR);
	}
	
}

int main()
{
	FILE *fp;                  	/* dosya pointer */
    int i,j,x,y;               	/* index */					
    int binary;					/* dosya formatini ayirt etmek icin */
    int cols,rows;				/* resmin satir sutun bilgisini okumak icin */
    int maxval;                	/* resimdeki max pixel sayisini dosyadan okumak icin */
    unsigned char **ptr,**ptrK;
	int **ptr2,  **final, **Gx, **Gy, **image;             	/* image buffer icin pointer */
    unsigned long total_bytes; 	/*resmi okurken nekadar byte okudugunu tutmak icin */
    unsigned long nwritten = 0; /* yeni dosyaya yazarken nekadar pixel yazdigini tutmak icin  */
  	char filename[30];			/* dosya adini okumak icin char dizisi */
  	char line[100]; 			/* string okumak icin char dizisi */
	int pixel_value;			/* Sobel filtresi uygularken toplami tutmak icin */
	int sayi;					/* Sobel filtresi icin */
	int sumx,sumy;					/* Sobel filtresi icin */
	int sonuc;					/* Sobel filtresi uygularken sumx ve sumy nin karelerinin toplaminin karekokunu tutar */
    int min, max;				/* min-max normalizasyonu icin min ve max i tutar */
   
    /*ptr2 matrisinde dosyadaki verilerin int a donusmus halleri tutuluyor*/
    
	int kernel[3][3] = 	{{ -1,  0,  1 },				//x yonunde 3*3 kernel 
              			 { -2,  0,  2 },
              			 { -1,  0,  1 }};
              			 
    int kernel2[3][3] = {{ 1,  2,  1 },				//y yonunde 3*3 kernel
              			 { 0,  0,  0 },
              			 { -1,  -2,  -1 }};
  				
	/*input dosyasini oku*/		
	
	//dosya adini ister
	printf("What is the name of the file?\n");
    scanf("%s", filename);
	    
	char* ext = ".pgm";
	sprintf(filename, "%s%s", filename, ext);

    if((fp = fopen(filename, "rb")) == NULL)
    {
    printf("Unable to open %s for reading\n",filename);
    exit(1);
    }
    
 	fgets (line,200,fp);		//dosya formatini okur
 	
    if (line[0]=='P' && (line[1]=='2')) {
		binary = 0;
		printf (" \n File Format: P2\nSadece P5 icin calisir !\n"); 
		exit(1);
    }
    else if(line[0]=='P' && (line[1]=='5')) {
		binary = 1;
	   printf ("\nFORMAT: P5\n"); 
    }
    
    SkipComments(fp);
   			
   // printf("yorum satiri: # %s",line);
   
    fscanf(fp,"%d",&cols);			//satir ve sutun bilgisi oku
    fscanf(fp,"%d",&rows);
	printf("Columns: %d Rows: %d\n", cols,rows); 
    fscanf(fp,"%d",&maxval);		//maximum pixel bilgisini oku 
    printf("Maximum value: %d\n", maxval);

 	//resim üzerinde iþlem yapmak için ptr dizisini olustur
 		
 
    ptr = (unsigned char **)malloc(sizeof(unsigned char *) * (rows));
    if (ptr == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
 	for (i = 0; i < rows; ++i) {
        ptr[i] = (unsigned char *)malloc(sizeof(unsigned char) * (cols));
        if (ptr[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
   
	//dosyayi yeni olusturulan karakter matrisine (ptr) fread()le oku
	
	total_bytes=0;
	for (i = 0; i < rows; i++) {					
	        total_bytes += fread(&ptr[i][0],sizeof(unsigned char),cols,fp); 
		if (feof(fp)) break;
	}
	
	if (total_bytes < rows*cols) {
	printf ("ERROR: fewer pixels than rows*cols indicates\n\n");
    }
    fclose(fp);			//dosyayi kapat

	
    //karakter matrisini int matrise atamak icin yeni matris (ptr2) olustur
  	
  	ptr2 = (int **)malloc(sizeof(int *) * rows);
    if (ptr2 == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        ptr2[i] = (int *)malloc(sizeof(int) * cols);
        if (ptr2[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
	//dosyadaki verileri okudugum karakter matrisini(ptr) int matrise (ptr2) atýyorum

	for(i=0;i<rows;i++){
		for(j=0;j<cols;j++){
			ptr2[i][j] = ptr[i][j];
		}
	}
	
	
	
	//matrislere yer ac
	
  	Gx = (int **)malloc(sizeof(int *) * rows);
    if (Gx == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        Gx[i] = (int *)malloc(sizeof(int) * cols);
        if (Gx[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
    Gy = (int **)malloc(sizeof(int *) * rows);
    if (Gy == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        Gy[i] = (int *)malloc(sizeof(int) * cols);
        if (Gy[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
    image = (int **)malloc(sizeof(int *) * rows);
    if (image == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        image[i] = (int *)malloc(sizeof(int) * cols);
        if (image[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
      final = (int **)malloc(sizeof(int *) * rows);
    if (final == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        final[i] = (int *)malloc(sizeof(int) * cols);
        if (final[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
	//karakter matrisi
    ptrK = (unsigned char **)malloc(sizeof(unsigned char *) * rows);
    if (ptrK == NULL) {
        printf("memory allocation failure");
        exit(EXIT_FAILURE);
    }
    
 	for (i = 0; i < rows; ++i) {
        ptrK[i] = (unsigned char *)malloc(sizeof(unsigned char) * cols);
        if (ptrK[i] == NULL) {
            perror("memory allocation failure");
            exit(1);
        }
    }
    
    /* Sobel ile Edge Detection Ýslemleri*/
   
    for (x= 1; x< rows-2; x++) {
    	for (y = 1; y < cols-2; y++) {
      	sumx = 0;
      	sumy = 0;
      		for (i = 0; i< 3; i++) {
        		for (j = 0; j <3; j++) {   	
          		sumx += ( ptr2[x+i][y+j] * kernel[i][j]);	
          		sumy += ( ptr2[x+i][y+j] * kernel2[i][j]);	
        		}
        	}
        	Gx[x][y] = sumx;			//sonuclari Gx int matrise at
        
        	Gy[x][y] = sumy;			//sonuclari Gy int matrise at
        	
        	image[x][y] = (int)( sqrt( sumx*sumx + sumy*sumy) ) ;	
		//	if(image[x][y]>255) image[x][y]=255;
         //   if(image[x][y]<0) image[x][y]=0;
    	}
	}
  
     // PADDING // final matrisinin kenarlarýný sýfýrla
  	padding(final,rows,cols);
  	
    normalization(Gx,final,rows,cols,maxval);
   	//Yeni sonuclar final matrisinde 
 
   	//int matrisini karakter matrisine atýyorum (dosyaya geri yazmak icin)
   	
	karaktereCevir(ptrK,final,rows,cols);
	
   	//Yeni elde ettigin Gx cikisini yeni dosyaya yaz 	
   	if ((fp = fopen("Sobel_Gx.pgm", "wb")) == NULL){
        printf("ERROR: file open failed\n");
	    return(0);
    }
   
    dosyayaYaz(fp,ptrK,cols,rows,maxval);
    
   // matrisSifirla(final,rows,cols);
    
    //Gy normalizasyon
    normalization(Gy,final,rows,cols,maxval);		//Yeni sonuclar final matrisinde (int matrisi)
   	
   	//int matrisini karakter buffer ýna atýyorum
	karaktereCevir(ptrK,final,rows,cols);
	
   	//Yeni elde ettigin Gy cikisini yeni dosyaya yaz 	
   	
   	if ((fp = fopen("Sobel_Gy.pgm", "wb")) == NULL){
        printf("ERROR: file open failed\n");
	    return(0);
    }
    
    dosyayaYaz(fp,ptrK,cols,rows,maxval);		//Sobel_Gy dosyasý olustu
    
    //normalizasyon
    normalization(image,final,rows,cols,maxval);
   	//Yeni sonuclar final matrisinde (int matrisi)
   	
   	// final matrisini karakter buffer ýna atýyorum 	
   	karaktereCevir(ptrK,final,rows,cols);

   	//Yeni elde ettigin sonucu yeni dosyaya yaz 	
   	if ((fp = fopen("SOBEL.pgm", "wb")) == NULL){
        printf("ERROR: file open failed\n");
	    return(0);
    }
    
    dosyayaYaz(fp,ptrK,cols,rows,maxval);	//Sobel dosyasý olustu
    
        
    printf("\n\n\t***%s dosyasina Sobel Filtresi ile Edge Detection uygulandi. Olusan resim Sobel.pgm dosyasindadir***\n", filename);
    
    return 0;
    }
