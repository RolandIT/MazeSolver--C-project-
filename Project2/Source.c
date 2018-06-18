/*
projekt2.c -- Projekt2 - Tezeus a Minotaurus
Roland Szarka, 19.4.2017 13:57:39
Po spustenÌ programu sa ako prvÈ vo funkcii main zavol· funkcia inicializ·cia , ktor· otovrÌ s˙bor so zadanÌm 
( v prÌpade ûe naËÌtavame zo s˙boru) a alokuje pamat pre tri glob·lne 2D polia vo velkosti 31*31.

Funkcia nacitaj_mapu naËÌta bludisko do glob·lneho 2D pola Bludisko, v prÌpade ûe sa podarilo 
naËÌtaù bludisko vr·ti hodnotu 1 inak vr·ti hodnotu 0. 
Prv˙ a ötvrt˙ ˙lohu riesi funkcia kresli_mapu ktor· pomocou pomocn˝ch funkcii prejde rekurzivne cele bludisko od pozicie Tezeusa 
pokial sme nepresli max. pocet dveri alebo neprejde cele bludisko. Vsetky chodby kam sa dostane oznacuje v pomoc. bludisku FarebBlud 
farbou(int cislo) ktora reprezentuje uroven chodby od Tezeusa.Nasledne po vyfarbeni vo vnorenom cykle postupne zapisujeme 
do BMP farby podla bludiska #-cierna,X-cervena,T-modra, .-biela, 1-zelena.

Ulohu 2 riesi funkcia kresli_miestnosti ktor· pomocou pomocnej funkcie zafarbenie rekurzivne prejde cele bludisko zo vsetkych policok 
a v poli FarebBlud ozaci nejakou farbou (hodnota od 1 po 100) vsetky chodby , po zafarbeni chodby sa zvysi index farby pre dalsie chodby.
Pre 100 farieb(RGB) sa vygeneruju do pola Farby nahodne hodnoty od 0 po 255 a podobne ako pri funkcii kresli_mapu podla hodnot v poli 
Bludisko do BMP zapise farby #-cierna, .- biela,T-modra,-X-cervena,A-Z -siva a podla pola FarebneBlud pre rozne farby (1-100) zapise farbu podla indexu.

Ulohu 3 riesi funkcia vypis_susednosti ktora pomocou pomocnej funkcie MapovanieDveri zisti x,y suradnice pre vsetky dvere v bludisku 
a funkcia zisti_susednosti prejde rekurzivne pole od pozicie na ktoru sa zavola (od A po Z).Ak narazi na dvere, do prislusneho zoznamu vlozi prvok. 
*/
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX 31 //max sirka a vyska bludiska
#define CLI true //meni nacitavanie bud z stdin alebo zo suboru 
//globalne premenne
int sirkaBlud = 0, vyskaBlud = 0; //sirka a vyska nacitaneho bludiska
char **Bludisko = NULL; //pointer na bludisko
int **FarebBlud = NULL;//pointer na bludisko s vyfarbenymi cestickami
int **ZeleneBlud = NULL;//pointer na bludisko pre vyfarbene cesty podla K  				
FILE *zadanie; //Subor s bludiskom 
struct Zoznam *z[26];//pole struktur (pre kazdy vyznacny bod)

//struktury a funkcie pre pracu s .BMP 
#pragma pack(push, 1)
struct BitmapFileHeader {
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffBits;
};
#pragma pack(pop)

struct BitmapInfoHeader {
	unsigned int biSize;
	int biWidth;
	int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
};
void write_head(FILE *f, int width, int height)
{
	struct BitmapInfoHeader bih;
	bih.biSize = sizeof(struct BitmapInfoHeader);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biSizeImage = bih.biWidth * bih.biHeight*3;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	struct BitmapFileHeader bfh;
	bfh.bfType = 0x4D42;
	bfh.bfSize = sizeof(struct BitmapFileHeader) + sizeof(struct BitmapInfoHeader) + bih.biSizeImage;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(struct BitmapFileHeader) + bih.biSize;

	fwrite(&bfh, sizeof(struct BitmapFileHeader), 1, f);
	fwrite(&bih, sizeof(struct BitmapInfoHeader), 1, f);
}
void write_pixel(FILE *f, unsigned char r, unsigned char g, unsigned char b)
{
	fwrite(&r, 1, 1, f);
	fwrite(&g, 1, 1, f);
	fwrite(&b, 1, 1, f);
}

//spajany zoznam pre dvere , potrebne struktuy a funkcie  
struct Prvok{
	char znak; //znak ktory reprezentuje dvere v bludisku 
	struct Prvok *dalsi; //adresa na dalsi prvok 
};
struct Zoznam {
	int xos; //x-ova suradnica dveri 
	int yos; //y-ova suradnica dveri 
	int znak;//znak ktory reprezentuje dvere 
	struct Prvok *prvy; //pointer na prvy prvok v zozname 
};
//funkcia na vytvorenie zoznamu , alokuje miesto v pamati pre zoznam a vrati jeho adresu 
struct Zoznam *zoznam_vytvor(char znak,int xos,int yos)
{
	struct Zoznam *z = (struct Zoznam*)malloc(sizeof(struct Zoznam));
	z->prvy = NULL;
	z->znak = znak;
	z->xos = xos;
	z->yos = yos;
	return z;
}
//funkcia na vytvorenie prvku, alokuje miesto v pamati pre strukturu prvok a nasledne nastavi jeho hodnoty podla argumentov
//funkcie,vytvori sa vzdy na prve miesto v zozname 
struct Prvok *prvok_vytvor(char znak,struct Prvok *dalsi)
{
	struct Prvok *p = (struct Prvok *)malloc(sizeof(struct Prvok));
	p->dalsi = dalsi; //pointer na dalsi prvok je predosli prvy prvok v zozname 
	p->znak = znak;
	return p; 
}
//pomocou funkcie prvok vytvor vlozi do zoznamu na prve miesto dalsi prvok 
void zoznam_vloz(struct Zoznam *z, char znak) {
	
			z->prvy = prvok_vytvor(znak,z->prvy);
}
//funckia ktora vypise obsah celeho spajaneho zoznamu po posledny prvok 
void zoznam_vypis(struct Zoznam *z)
{
	struct Prvok *p = z->prvy;
	printf("%c:", z->znak);
	while (p != NULL)
	{
		printf(" %c", p->znak);
		p = p->dalsi;
	}
	printf("\n");
}

//funckia na nacitanie mapy zo suboru , da sa zmenit  aby nacitavala zo vstupu pomocou CLI 
//postupne nacita bludisko do pola Bludisko, nastavÌ sirku a vysku bludiska a vrati hodnotu 1 ked 
//sa podarilo nacitat bludisko m 0 v pripade ze sa nepodarilo nacitat 
int nacitaj_mapu()
{
#ifndef CLI
	//INPUT Z CLI 
	char buf; //pomocne premenne
	int i = 0,j = 0;
	while (scanf("%c", &buf) != EOF) //nacitavame po end of file
	{
		if (buf == '\n' && j == 0)//ak sme nacitali endline a sme na prvej pozicii v riadku ukoncime nacitavanie 
			break;
		if (buf == '\n') //ak sme narazili na koniec riadku vynulujeme j(sirka) a zvysime i(vyska)
		{
			Bludisko[i][j] = 0;
			i++;
			j = 0;
			continue;
		}
		else
		{
			Bludisko[i][j] = buf;
			j++;
		}
	}
	sirkaBlud = strlen(Bludisko[0]); //nastavenie vysky a sirky bludiska
	vyskaBlud = i;
	for (i = 0; i < vyskaBlud; i++) //vypis nacitaneho bludiska
	{
		for (j = 0; j < sirkaBlud; j++)
			printf("%c", Bludisko[i][j]);
		printf("\n");
	}
	if (i>0) //ak sa nam podarilo nacitat nieco , vratime 1
		return 1;
	return 0;
#else
	
	int i = 0, j = 0; //pomocne premenne
	char buf;
	while (fscanf(zadanie, "%c", &buf)!=EOF)// zo suboru nacitavame znaky po end of file 
	{
		if (buf == '\n' && j == 0) //ak sme nacitali endline a sme na prvej pozicii v riadku ukoncime nacitavanie 
			break;
		if (buf == '\n') //ak sme narazili na koniec riadku vynulujeme j(sirka) a zvysime i(vyska)
		{
			Bludisko[i][j] = 0;
			i++;
			j = 0;
			continue;
		}
		else
		{
			Bludisko[i][j] = buf;
			j++;
		}
	}
	sirkaBlud = strlen(Bludisko[0]); //nastavenie vysky a sirky bludiska
	vyskaBlud = i;
	printf(" vyska %d---sirka%d\n", vyskaBlud, sirkaBlud);
	for (i = 0; i < vyskaBlud; i++) //vypis nacitaneho bludiska
	{
		for (j = 0; j < sirkaBlud; j++)
			printf("%c", Bludisko[i][j]);
		printf("\n");
	}

	if (i > 0) //ak sa nam podarilo nacitat nieco , vratime 1
		return 1;
	return 0;
#endif
}

//mapovanie suradnic dveri v bludisku, Do pola struktur zoznam vytvorime zoznam pre A-Z so suradnicami -1.
//nasledne prejdeme vsetky policka v bludisku a ak policko dane policko su dvere zapiseme do prislusneho zoznamu suradnice 
void mapovanieDveri()
{
	int i = 0, j = 0;

	for (i = 0; i < 26; i++) //vytvorime spajany zoznam pre vsetky prvky pola 
		z[i] = zoznam_vytvor('A' + i, -1, -1);

	for (i = 0; i < vyskaBlud; i++) //zapis suradnic dveri do zoznamu pre kazde dvere 
		for (j = 0; j < sirkaBlud; j++)
		{
			if (Bludisko[i][j] >= 'A' && Bludisko[i][j] <= 'Z')
			{
				z[Bludisko[i][j]-'A']->xos = i;
				z[Bludisko[i][j]-'A']->yos = j;
			}
		}
}

//vyfarbenie chodieb podla K, rekurzivne prechadza bludisko od policka na ktore sa zavola ( Tezeus) 
//a oznacuje chodby farbou (cislo od 1 ktora reprezentuje uroven chodby) kym nenarazi na dvere. V pripade ze narazi k sa znizi,
//zvysi sa hodnota farby kedze sa meni uroven chodby a znak dveri,ktora reprezenutje dvere odkial sme sa dostali do aktualnej chodby 
//(aby sme sa nevydali cestou naspat cez rovnake dvere). Ak k=0 uz dalej nic neoznaci 
void vyfarbenie(int xos,int yos,char dvere,int k,int farba)
{
	if (k < 0) //v pripade ze nic nevyfarbujeme 
		return;
	if (xos < 0 || xos >= vyskaBlud || yos < 0 || yos >= sirkaBlud) //kontrola ci je pozicia este nezafarbena chodba
		return;
	//ak dane policko nie je chodba a zaroven miesto odkial pokracujeme dalej do inych miestnosti alebo sme uz danu chodbu oznacli ako prejdenu 
	if ((Bludisko[xos][yos] != '.' && Bludisko[xos][yos] != dvere) || FarebBlud[xos][yos] <=farba) 
	{
		if (Bludisko[xos][yos] >= 'A' && Bludisko[xos][yos] <= 'Z') //ak sme narazili na dvere,upravime k,farbu a dvere
		{
			farba++;
			k--;
			dvere = Bludisko[xos][yos];
		}
		else
			return;
	}
	if (Bludisko[xos][yos] != dvere)
		FarebBlud[xos][yos] = farba; //v pom. bludisku oznacime dane policko farbou 
	vyfarbenie(xos + 1, yos, dvere, k,farba); //krok dole 
	vyfarbenie(xos - 1, yos, dvere, k,farba); //krok hore
	vyfarbenie(xos, yos + 1, dvere, k,farba); //krok do prava
	vyfarbenie(xos, yos - 1, dvere, k,farba); //krok do lava
}

//funkcia na vyfarbenie chodieb ktore prejdeme podla k=pocet dveri cez ktore mozeme prejst. 
//Pomocou funkcii mapovanieDveri a vyfarbenie zafarbÌ od pozicie tezeusa chodby podla k. 
//do .BMP zapise farby podla znaku pre danu poziciu v bludisku ZeleneBlud , 1-100 reprezentuje zelenu farbu 
void  kresli_mapu(char *nazov_suboru,int k)
{
	int i, j; //pomocne premenne
	//pomocne bludisko FarebBlud vyplnime cislom ktore je vacsie ako nase farby (urovne) ale zaroven nebude pouzita ako farba 
	for (i = 0; i < vyskaBlud; i++)
		for (j = 0; j < sirkaBlud; j++)
				FarebBlud[i][j] = 101; 	
	mapovanieDveri();// zistenie pozicie dveri (tezeusa)
	vyfarbenie(z['T' - 'A']->xos, z['T' - 'A']->yos, 'T' - 'A', k,1); //rekurzivne vyfarbenie od pozicie tezeusa
	//pre vacsi obrazok v BMP zvacsime sirku a vysku cislom ktore je delitelne 4 
	int sirka = sirkaBlud * 16;
	int vyska = vyskaBlud * 16;
	FILE *f = fopen(nazov_suboru, "wb"); //otvorime bmp subor na pisanie 
	write_head(f, sirka, vyska);
	//prejde cyklus vyska * sirka krat 
	for (i = 0; i <vyska; i++)
		for (j = 0; j <sirka; j++)
		{
			int x = (vyska - i) / 16, y = j / 16; //upravime poziciu v bludisku podla ktorej vyberame farbu 
			if (Bludisko[x][y] == '#')
				write_pixel(f, 0, 0, 0);
			else if (Bludisko[x][y] == 'T')
				write_pixel(f, 255, 0, 0);
			else if (Bludisko[x][y] == 'X')
			{
				write_pixel(f, 0, 0, 255);
				continue;
			}
			else if (Bludisko[x][y] >= 'A' && Bludisko[x][y] <= 'Z')
				write_pixel(f, 128, 128, 128);
			else if(FarebBlud[x][y] >= 1 && FarebBlud[x][y] <= 100) //ak sme sa dostali sem skontrolujeme v pom. bludisku ci je dane policko zafarbene
				write_pixel(f, 0, 255, 0);
			else if (Bludisko[x][y] == '.')
				write_pixel(f, 255, 255, 255);
			else
				write_pixel(f,255,255,255);
		}
	fclose(f);
}

//pomocna funkcia pre vyfarbenie chodieb v BMP, od bodu na ktorej je zavolana rekurzivne prejde
//vsetky dostupne policka a oznaci ich v pomocnom bludisku FarebBlud cislom farba ktoru funkcia
//dostane ako argument , vrati 1 ked aspon 1 policko oznacilo 0 ked neoznacilo nic 
int zafarbenie(int xos,int yos,int farba)
{
	if (xos < 0 || xos >= vyskaBlud || yos < 0 || yos >= sirkaBlud || Bludisko[xos][yos] != '.' || FarebBlud[xos][yos]) //kontrola ci je pozicia este nezafarbena chodba
		return 0;
	FarebBlud[xos][yos] = farba;
	zafarbenie(xos + 1, yos, farba);//krok dole
	zafarbenie(xos - 1, yos, farba);//krok hore
	zafarbenie(xos, yos + 1, farba);//krok do prava
	zafarbenie(xos, yos - 1, farba);//krok do lava

	return 1;
}

//vyfarbenie miestnosti v BMP, zavola sa fukncia zafarbenie na vsetky policka v bludisku ktora oznaci 
//vsetky chodby inym cislom od 1 do 100.Do 2D pola vygeneruje 100 roznych kombinacii RGB hodnot.
//Do .bmp zapise farby podla znakov v Bludisku , v pripade ze sa na danej pozicii nenachadza stena alebo znak A-Z
//skontroluje hodnoty v pomocnom poli FarebBlud a podla cisla(farba) zapise do .bmp farbu pre danu poziciu 
void kresli_miestnosti(char *nazov_suboru)
{
	int i, j;
	for(i=0;i<vyskaBlud;i++) //naplni pomocne pole hodnotou 0 ak sa na danom mieste v bludisku nachadza chodba a pre vsetky ostatne hodnotou 101
		for (j = 0; j < sirkaBlud; j++)
		{
			if (Bludisko[i][j] != '.')
				FarebBlud[i][j] = 101; //farby si len do 100 , 101 oznacuje nedostupne miesto 
			else
				FarebBlud[i][j] = 0;
		}
	int Farba = 1;
	for (i = 0; i < vyskaBlud; i++) //zavola sa funkcia zafarbi na vsetky pozicie v bludisku 
		for (j = 0; j < sirkaBlud; j++)
			if (zafarbenie(i, j, Farba))
				Farba++; //v pripade ze sme farbou pouzili , zvysime hodnotu o 1
	
	int farby[101][3];//pole pre farby
	int sirka =sirkaBlud* 16; //pre vacsi obrazok v BMP sirku a vysku vynasobime cislom ktory je delitelny 4
	int vyska=vyskaBlud * 16;
	FILE *Obrazok = fopen(nazov_suboru, "wb"); //otvorime subor na zapisovanie 
	write_head(Obrazok, sirka, vyska);
	
	for (i = 0; i < 101; i++) //nahodne vygenerovanie farieb do pola farby
	{
		farby[i][0] = rand() % 256;
		farby[i][1] = rand() % 256;
		farby[i][2] = rand() % 256;
	}
	for (i = 0; i <vyska; i++)//prejde sirka*vyska krat co sme nastavili 
		for (j = 0; j <sirka; j++)
		{
			int x = (vyska - i) / 16, y = j / 16; //musime prisposobit miesto odkial cerpame udaj z bludiska kedze v .BMP to bude 16x vacsie
			if (Bludisko[x][y] == '#')
			{
				write_pixel(Obrazok, 0, 0, 0);
				continue;
			}
			else if (Bludisko[x][y] == 'T')
			{
				write_pixel(Obrazok, 255, 0, 0);
				continue;
			}
			else if (Bludisko[x][y] == 'X')
			{
				write_pixel(Obrazok, 0, 0, 255);
				continue;
			}
			else if (Bludisko[x][y] >= 'A' && Bludisko[x][y] <= 'Z')
				write_pixel(Obrazok, 128, 128, 128);
			else if (FarebBlud[x][y] >= 1 && FarebBlud[x][y]<=100) //ak predosle podmienky z pola Bludisko nevyhovuju skontrolujeme pole FarebBlud 
				write_pixel(Obrazok,farby[FarebBlud[x][y]][0], farby[FarebBlud[x][y]][1],farby[FarebBlud[x][y]][2]);
			else
				write_pixel(Obrazok, 255, 255, 255);
		}
	fclose(Obrazok);
}

//inicializacia,alokuje pamat pre polia ktore reprezentuju bludiska v poli, otvori subor so zadanim  pripade ze citame zo suboru 
void inicializacia()
{
#ifdef CLI
	zadanie=fopen("zadanie.txt", "r+"); //otvorime subor so zadanim 
#endif
	int i = 0, j = 0;
	Bludisko = malloc(MAX * sizeof(char*));//alokujeme miesto v pamati naraz pre bludisko aj pre nase pomocne polia FarebBlud a ZeleneBludisko
	FarebBlud = malloc(MAX * sizeof(int*));
	ZeleneBlud = malloc(MAX * sizeof(char*));
	for (i = 0; i < MAX; i++)
	{
		FarebBlud[i] = malloc(MAX * sizeof(int));
		Bludisko[i] = malloc(MAX * sizeof(char));
		ZeleneBlud[i] = malloc(MAX * sizeof(char));
	}
}

//pomocna funckia ktora zisti susednosti dveri a zapise do spajaneho zaznamu 
//rekurzivne prechadza pole od pozicie na ktoru je zavolna , ak narazi na dvere vlozi 
//do zaznamu znak dveri pomocou funckie zoznam_vloz a oznaci dvere za prejdene (zmeni pismenko na male)
void zisti_susednost(int xos,int yos,char dvere,struct Zoznam *z)
{
	if (xos < 0 || xos >= sirkaBlud || yos < 0 || yos >= vyskaBlud) //kontrola ci je pozicia este nezafarbena chodba
		return;

	if ((ZeleneBlud[xos][yos] != '.' && ZeleneBlud[xos][yos] != dvere) || ZeleneBlud[xos][yos] == '1')
	{
		if (ZeleneBlud[xos][yos] >= 'A' && ZeleneBlud[xos][yos] <= 'Z') //zistovanie ci sme nasli dvere 
		{
			zoznam_vloz(z, ZeleneBlud[xos][yos]); //vkladanie do zoznamu
			ZeleneBlud[xos][yos] += 32; //zmenime na male pismeno
		}
		return;
	}
	if (ZeleneBlud[xos][yos] != dvere)
		ZeleneBlud[xos][yos] = '1'; //oznacime policko za prejdene 
	zisti_susednost(xos + 1, yos,dvere,z);//krok dole
	zisti_susednost(xos - 1, yos,dvere,z);//krok hore
	zisti_susednost(xos, yos + 1,dvere,z);//krok do prava
	zisti_susednost(xos, yos - 1,dvere,z);//krok do lava
	return;
}

//funckia ktora vypise susednosti dveri pomocou spajaneho zoznamu 
//pomocou funkcie mapovanie dveri do zoznamu¥zapiseme x a y suradnice dveri 
//( vola sa to aj v tejto finkcii v pripade ze by bola pouzita skor ako kresli_mapu)
//pre kazde pismeno zistime susednosti tak , ûe najprv do pomocneho pola 
//ZeleneBludisko prekopirujeme cele bludisko, nasledne zavolame funkciu zisti_susednost pre vsetky 
//znaky potom pomocou funkcie zoznam_vypis vypiseme spojitosti dveri pre dvere ktore sa nachadzaju v bludisku 
void vypis_susednost()
{
	int i = 0, j = 0;
	mapovanieDveri();
	for (i = 0; i < 26; i++) //cyklus prejde pre vsetky prvky pola 
	{
		int x = 0, y = 0;
		for (x = 0; x < vyskaBlud; x++) //vycistime pomocne pole pred zistovanim 
			for (y = 0; y < sirkaBlud; y++)
			{
				ZeleneBlud[x][y] = Bludisko[x][y];
			}

		zisti_susednost(z[i]->xos, z[i]->yos, 'A' + i, z[i]); //zistime susednosti 
	}
	
	for(i=0;i<26;i++) //vypis susednosti 
		if (z[i]->xos != -1) //vynechame dvere ktore nem·me v bludisku 
			zoznam_vypis(z[i]);
}

int main(void)
{
		inicializacia(); // NEMAZAT!!! - MAXIMALNE 1 VAS RIADOK

		if (nacitaj_mapu() == 1)
		{
			kresli_miestnosti("mapa1_farby_miestnosti.bmp");
			kresli_mapu("mapa1_k0.bmp", 0);
			kresli_mapu("mapa1_k1.bmp", 1);
			kresli_mapu("mapa1_k2.bmp", 2);
			kresli_mapu("mapa1_k3.bmp", 3);
			kresli_mapu("mapa1_k4.bmp", 4);
			kresli_mapu("mapa1_k5.bmp", 5);
			kresli_mapu("mapa1_k6.bmp", 6);
			kresli_mapu("mapa1_k7.bmp", 7);
			kresli_mapu("mapa1_k8.bmp", 8);
			kresli_mapu("mapa1_k9.bmp", 9);
			vypis_susednost();
		}

		if (nacitaj_mapu() == 1)
		{
			vypis_susednost();
			kresli_miestnosti("mapa2_farby_miestnosti.bmp");
			kresli_mapu("mapa2_k0.bmp", 0);
			kresli_mapu("mapa2_k1.bmp", 1);
			kresli_mapu("mapa2_k2.bmp", 2);
			kresli_mapu("mapa2_k3.bmp", 3);
			kresli_mapu("mapa2_k4.bmp", 4);
			kresli_mapu("mapa2_k5.bmp", 5);
			kresli_mapu("mapa2_k6.bmp", 6);
			kresli_mapu("mapa2_k7.bmp", 7);
			kresli_mapu("mapa2_k8.bmp", 8);
			kresli_mapu("mapa2_k9.bmp", 9);
		}

		if (nacitaj_mapu() == 1)
		{
			kresli_mapu("mapa3_k9.bmp", 9);
			kresli_mapu("mapa3_k8.bmp", 8);
			kresli_mapu("mapa3_k7.bmp", 7);
			kresli_mapu("mapa3_k6.bmp", 6);
			kresli_miestnosti("mapa3_farby_miestnosti.bmp");
			kresli_mapu("mapa3_k5.bmp", 5);
			kresli_mapu("mapa3_k4.bmp", 4);
			kresli_mapu("mapa3_k3.bmp", 3);
			kresli_mapu("mapa3_k2.bmp", 2);
			vypis_susednost();
			kresli_mapu("mapa3_k1.bmp", 1);
			kresli_mapu("mapa3_k0.bmp", 0);
		}

		if (nacitaj_mapu() == 1)
		{
			kresli_mapu("mapa4_k0.bmp", 0);
			vypis_susednost();
			kresli_miestnosti("mapa4_farby_miestnosti.bmp");
			kresli_mapu("mapa4_k1.bmp", 1);
			kresli_mapu("mapa4_k2.bmp", 2);
			kresli_mapu("mapa4_k3.bmp", 3);
			kresli_mapu("mapa4_k4.bmp", 4);
			kresli_mapu("mapa4_k5.bmp", 5);
			kresli_mapu("mapa4_k6.bmp", 6);
			kresli_mapu("mapa4_k7.bmp", 7);
			kresli_mapu("mapa4_k8.bmp", 8);
			kresli_mapu("mapa4_k9.bmp", 9);
		}
		system("pause");
		return 0;
}
