#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define colSize 15
#define blockSize 4096
#define datFile "Block.dat"
#define debug 1

union tint {
	char cint[sizeof(int)];
	int vint;
};

struct column{
	char name[colSize];
	char type;
	int len;
};

struct header{
	struct column *columns;
	int columnsQtd;
	int headerSize;
	int slotSize;
};

void buildBitMap(FILE *file, int headerSize, int rowSize){
	char *bitmap, *data;
	int slots = 0, i;

	int emptyBlock = (blockSize - headerSize - sizeof(int));
	while(emptyBlock >= rowSize + 1){
		slots++;
		emptyBlock -= (rowSize + 1);
	}

	bitmap = (char*)calloc(slots, sizeof(char));
	data = (char*)calloc(rowSize, sizeof(char));

	for(i = 0; i < slots; i++){
		bitmap[i] = '0';
	}
	
	fwrite(data, rowSize, slots, file);
	fwrite(bitmap, slots, 1, file);
	fwrite(&slots, sizeof(int), 1, file);

	if(debug){
		printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printf("DEBUG: \n");
		printf("-- Sobra de bloco: %d Bytes\n", emptyBlock);
		printf("-- headerSize: %d Bytes\n", headerSize);
		printf("-- Slots: %d de %d Bytes. Total de %d Bytes\n", slots, rowSize, rowSize * slots);
		printf("-- Bitmap: %d posicoes. Total de %d Bytes\n", slots, slots + 4);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
		system("read -p \"Pressione ENTER para continuar\" saindo");
	}
}

int readBitMap(int op){
	FILE *file;
	int qtdSlots;
	char slotStatus;

	file = fopen(datFile, "r");
	if(file == NULL){
		printf("Dat file no found\n");
		exit(0);
	}

	fseek(file, -4, SEEK_END);
	fread(&qtdSlots, sizeof(int), 1, file);	
	fseek(file, -4, SEEK_END);

	int i;
	for(i = 0; i < qtdSlots; i++){
		fseek(file, sizeof(char) * -1, SEEK_CUR);
		fread(&slotStatus, sizeof(char), 1, file);
		fseek(file, sizeof(char) * -1, SEEK_CUR);
		
		if(slotStatus == '0' && !op){
			fclose(file);
			return i;
		}else if(slotStatus == '1' && op){
			fclose(file);
			return i;
		}
	}
	fclose(file);
	return -1;
}

struct header readHeader(){
	FILE *file;
	int qtd = 0, size = 0, slotSize = 0;
	char separador;
	struct header hd;
	hd.columns = (struct column*)calloc(0, sizeof(struct column));

	file = fopen(datFile,"r");
	if(file == NULL){
		printf("Dat file not found\n");
		exit(0);
    }

    while(separador != '#'){
    	fread(&separador, sizeof(char), 1, file);
    	if(separador != '#'){
    		hd.columns = (struct column*)realloc(hd.columns, (qtd + 1) * sizeof(struct column));

    		fseek(file, -1, SEEK_CUR);
    		fread(hd.columns[qtd].name,colSize,1,file);
			fread(&hd.columns[qtd].type,1,1,file);
			fread(&hd.columns[qtd].len,sizeof(int),1,file);
			size += (colSize + sizeof(char) + sizeof(int));
			slotSize += hd.columns[qtd].len;
			qtd++;
    	}
    }
    hd.columnsQtd = qtd;
    hd.headerSize = size + 1;
    hd.slotSize = slotSize;
	fclose(file);

	return hd;
}

void insert(){
	FILE* file;
	char *buffer;
	char opt, c;
	int i, getInt, slot;
	struct header hd;
	hd = readHeader();

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[2] Inserir dados:\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	buffer = (char*)calloc(0, sizeof(char));

	do{
		slot = readBitMap(0);
		printf("# Inserir dados no slot [%d]:\n\n", slot);
		file = fopen(datFile, "r+");
		fseek(file, hd.headerSize + (slot * hd.slotSize), SEEK_SET);
		for(i = 0; i < hd.columnsQtd; i++){
			printf("- %s: ",hd.columns[i].name);
			buffer = (char*)calloc(hd.columns[i].len, sizeof(char));
			switch (hd.columns[i].type)
			{
				case 'S': fgets(buffer,hd.columns[i].len + 1,stdin);
				          if (buffer[strlen(buffer)-1]!='\n')  c = getchar();
				          else buffer[strlen(buffer)-1]=0;
				          buffer[hd.columns[i].len - 1] = '#';
				          fwrite(buffer,hd.columns[i].len,1,file);
				        break; 
				case 'C': buffer[0]=fgetc(stdin); 
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite(buffer,hd.columns[i].len,1,file);
				        break;
				case 'I': scanf("%d", &getInt);
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite(&getInt,hd.columns[i].len,1,file);
				        break;
		    }
		    free(buffer);
		}
		//Write in bitmap
		fseek(file, (sizeof(int) + slot + 1) * - 1, SEEK_END);
		fwrite("1", 1, 1, file);
		fclose(file);
		printf("\nContinuar (S/N): "); opt=getchar();
		printf("\n====================================================\n\n");
		while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	}while(opt == 'S' || opt == 's');
}

void selectAll(){
	FILE *file;
	struct header hd;
	int i = 0, j, k, space, slots, sizetop = 0;
	char *buffer, bitmap;
	union tint eint;

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[3] Buscar todos os dados:\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	hd = readHeader();
	file = fopen(datFile, "r");
	if(file == NULL){
		printf("Dat file not found\n");
		exit(0);
	}
	while(i < hd.columnsQtd && hd.columns[i].name[0] != '#'){
		printf(" ");
		printf("%s ",hd.columns[i].name);
		space = hd.columns[i].len - strlen(hd.columns[i].name);
		for (j = 1; j <= space; j++){
        	printf(" ");
        }        
      	printf("|");
      	sizetop += hd.columns[i].len + 4;
        i++;
	}
	printf("\n");
	sizetop += 1;
	for(k = 0; k < sizetop; k++){
		printf("=");
	}
	printf("\n");

	// Search in bitmap
	fseek(file, -1 * sizeof(int), SEEK_END);
	fread(&slots, sizeof(int), 1, file);
	for(k = 1; k <= slots; k++){
		fseek(file, (sizeof(int) + k) * -1, SEEK_END);
		fread(&bitmap, sizeof(char), 1, file);
		if(bitmap == '1'){
			// Read Data in Offset
			fseek(file, hd.headerSize + ((k - 1) * hd.slotSize), SEEK_SET);
			i = 0;
			while(i < hd.columnsQtd && hd.columns[i].name[0] != '#'){
				buffer = (char*)calloc(hd.columns[i].len, sizeof(char));
				if (!fread(buffer,hd.columns[i].len,1,file)) break;
				switch (hd.columns[i].type){
					case 'S': 
						printf(" ");
						for(j = 0; j < hd.columns[i].len && buffer[j] != 0;j++){
					    	printf("%c",buffer[j]);
					    	space = hd.columns[i].len - j - 1;
					    }
		                for(j = 0; j <= space; j++){
		                    printf(" ");
		                }
						break;
					case 'C': 
						printf(" %c",buffer[0]);
						space = strlen(hd.columns[i].name) - 1;
						for(j = 0; j <= space; j++){
		                    printf(" ");
		                }
						break;
					case 'I': 
						for (j=0; j < hd.columns[i].len; j++){
								eint.cint[j] = buffer[j];
						};
						printf(" %d", eint.vint); 
						space = strlen(hd.columns[i].name) - 2;
						for(j = 0; j <= space; j++){
		                    printf(" ");
		                }
						break;
				}
				printf("|");
				free(buffer);
				i++;
			}
		printf("\n");
		}
	}
	fclose(file);
	printf("\n\n");
	system("read -p \"Pressione ENTER para continuar\" saindo");
}

void removeSlot(){
	FILE *file;
	int slot;
	char c;

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[4] Deletar dados:\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	printf("Qual slot deletar os dados?\n");
	printf("~~> ");

	scanf("%d", &slot);
	while((c = getchar()) != '\n' && c != EOF); /// garbage collector

	file = fopen(datFile,"r+");
	fseek(file, (sizeof(int) + slot + 1) * -1, SEEK_END);
	fwrite("0", sizeof(char), 1, file);
	fclose(file);

	printf("\nDados do slot [%d] deletados com sucesso.\n\n", slot);
	system("read -p \"Pressione ENTER para continuar\" saindo");
}

void buildHeader(){
	char colName[colSize];
	char op, colType, c;
	int colLen, headerSize, rowSize = 0, qtd = 0;
	FILE *file;

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[1] Criar nova tabela:\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	file = fopen(datFile, "w+");
	if(file == NULL){
		printf("Arquivo %s nao existe\n", datFile);
		exit(0);
	}

	do{
		// Field Names
		printf("Column Name: ");
		fgets(colName, blockSize, stdin);
		if(colName[strlen(colName)-1] == '\n'){
			colName[strlen(colName)-1] = '\0';
		}

		//Field Type
		printf("Column Type [S = String, C = Char, I = Int]: ");
		scanf("%c", &colType);
		while((c = getchar()) != '\n' && c != EOF); /// garbage collector

		//Field lenght
		if(colType == 'I'){
			colLen = sizeof(int);
		}else if(colType == 'C'){
			colLen = sizeof(char);
		}else{
			printf("Column Length: ");
			scanf("%d", &colLen);
			while((c = getchar()) != '\n' && c != EOF); /// garbage collector
		}
		printf("\nAdicionar mais colunas? [S/N]\n");
		printf("~~> ");
		scanf("%c", &op);
		while((c = getchar()) != '\n' && c != EOF); /// garbage collector
		printf("\n====================================================\n\n");

		fwrite(colName, colSize, 1, file);
		fwrite(&colType, 1, 1, file);
		fwrite(&colLen, sizeof(int), 1, file);

		rowSize += colLen;
		qtd++;
	}while(op == 'S' || op == 's');
	fwrite("#", 1, 1, file);
	headerSize = (colSize + sizeof(char) + sizeof(int) + sizeof(char)) * qtd;
	buildBitMap(file, headerSize, rowSize);
	fclose(file);
}

int menu(){
	char op, c;
	printf("T1 - Criação de paginas com tamanho fixo de 4kB\n\n\n");
	printf("                MENU                  \n");
	printf("======================================\n");
	printf(" [1] Criar nova tabela\n");
	printf(" [2] Inserir dados\n");
	printf(" [3] Buscar todos os dados\n");
	printf(" [4] Deletar dados\n");
	printf(" [X] Terminar programa\n");
	printf("======================================\n");
	printf("~~> ");
	scanf("%c", &op);
	while((c = getchar()) != '\n' && c != EOF); /// garbage collector

	switch(op){
		case '1':
			system("clear");
			buildHeader();
			break;
		case '2':
			system("clear");
			insert();
			break;
		case '3':
			system("clear");
			selectAll();
			break;
		case '4':
			system("clear");
			removeSlot();
			break;
		case 'X':
			printf("\nAdios Muchacho.\n\n");
			return 1;
		case 'x':
			printf("\nAdios Muchacho.\n\n");
			return 1;
		default:
			printf("\n[Error] ## Operacao nao reconhecida. Tente novamente!\n\n");
			system("read -p \"Pressione ENTER para continuar\" saindo");
	}
	system("clear");
	return 0;
}



int main(){
	while(!menu());
	return 0;
}