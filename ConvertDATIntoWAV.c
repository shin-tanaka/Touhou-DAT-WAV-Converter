#include<stdio.h>
#include<time.h>
#define HEADER_SIZE 44
#define wait() printf("Press ENTER key to exit...\n");getchar();

/**
* 渡されたFILE構造体のポインタをチェックし、NULLであった場合に
* エラー分を表示する。
*
* return -1 : ポインタがNULLであった場合
* return  0 : 正常に代入されていた場合
*/
int checkNull(FILE *file){
	if(file == NULL){
		printf("[ERROR]Couldn't find the BGM file ""thbgm.dat""..\n");
		wait();
		return -1;
	}
	return 0;
}

/**
* 入力された値をリトルエンディアンの順に指定された配列に入れる
*
* (unsigned int)    data : リトルエンディアンに変換するunsigned int型の値.
* (int)             byte : 入力するバイト数.
* (unsigned char) *array : 結果を入れるためのunsigned char型の配列.
*                          byteの数以上の要素数を持つ必要がある.
*/
void getLittleEndian(unsigned int data, int byte, unsigned char *array){
	unsigned int mask = 0xFF;
	unsigned int tmp;
	for(int i = 0; i < byte; i++){
		tmp = data & mask;
		array[i] = (tmp & mask) >> i * 8;
		mask <<= 8;
	}
}

int main(void){

	//処理開始前時刻の取得
	unsigned int prevTime = time(NULL);

	//WAVEファイルのヘッダ 後に書き換える部分に関しては "0xFF"で埋めてある
	unsigned char header[HEADER_SIZE] = {
		0x52, 0x49, 0x46, 0x46, 0xFF, 0xFF, 0xFF, 0xFF,
		0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
		0x44, 0xAC, 0x00, 0x00, 0x10, 0xB1, 0x02, 0x00,
		0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
		0xFF, 0xFF, 0xFF, 0xFF
	};

	/**
	* FILE *fpRead  : thbgm.dataを読み込むファイルストリーム
	* FILE *fpWrite : 出力するファイルのファイルストリーム
	*/
	FILE *fpRead, *fpWrite;

	fpRead = fopen("thbgm.dat", "rb");
	fpWrite = fopen("BGM.wav", "wb");

	//NULLチェック
	if(checkNull(fpRead) == -1) {
		printf("[ERROR] Couldn't find the BGM file ""thbgm.dat""\n");
		wait();
		return -1;
	}
	if(checkNull(fpWrite) == -1){
		printf("[ERROR] Couldn't create "".wav"" file.\n");
		wait();
		return -1;
	}

	//ファイルサイズの取得
	fseek(fpRead, 0, SEEK_END);
	unsigned int FILE_SIZE = ftell(fpRead) - 16;
	fseek(fpRead, 0, SEEK_SET);

	//ファイルサイズの表示
	printf("FILE SIZE : %d byte (%d MB).\n", FILE_SIZE, FILE_SIZE/1024/1024);

	//ヘッダーの書き込み
	for(int i = 0; i < HEADER_SIZE; i++){
		fputc(header[i], fpWrite);
	}

	//ZWAVから始まる16バイトをスキップする
	fseek(fpRead, 16, SEEK_SET);

	//GBMデータのコピペ
	unsigned char tmp;
	for(int i = 0; i < FILE_SIZE - 16; i++){
		fputc(fgetc(fpRead), fpWrite);
	}

	//getLittleEndianに渡すための4バイト配列
	unsigned char buf[4];

	//ファイルバイト数の書き込み
	fseek(fpWrite, 4, SEEK_SET);
	getLittleEndian(FILE_SIZE - 8, 4, buf);
	for(int i = 0; i < 4; i++){
		fputc(buf[i], fpWrite);
	}

	//データ部バイト数の書き込み
	fseek(fpWrite, 40, SEEK_SET);
	getLittleEndian(FILE_SIZE-44, 4, buf);
	for(int i = 0; i < 4; i++){
		fputc(buf[i], fpWrite);
	}

	//処理終了後の時刻を取得し、処理時間を求めて表示
	unsigned int currentTime = time(NULL);
	currentTime -= prevTime;
	printf("DONE...\n");
	printf("PROCES TIME : %ds (%dm %ds) \n",
			currentTime, currentTime / 60, currentTime % 60);

	//ファイルを閉じる
	if(fpRead != NULL) fclose(fpRead);
	if(fpWrite != NULL) fclose(fpWrite);

	wait();

	return 0;
}