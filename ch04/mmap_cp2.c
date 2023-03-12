#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define BUF_SIZE 4096

int main(int argc, char* argv[]) {
    int inputFd, outputFd;    //從inputFd將檔案寫到outputFd
    ssize_t numIn, numOut;//讀進多少，寫出多少
    char buffer[BUF_SIZE];//把檔案內容讀到buffer，再寫出去
    char *inputPtr, *outputPtr;

    //if (argc != 3) {//確定使用者輸入二個參數
        //char* filename=basename(argv[0]);
        //printf("『%s』的功能是檔案複製，要有二個參數，來源檔案和目標檔案\n", filename);
        //exit(0);
    //}
 
    //打開來源檔案
    inputFd = open (argv [1], O_RDONLY);
    if (inputFd == -1) {
        perror ("cannot open the file for read"); 
        exit(1); 
    }
 
    //打開目的檔案
    outputFd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR| S_IWUSR);
    if(outputFd == -1){
        perror("canot open the file for write"); 
        exit(1); 
    }
    
    //與 mycp 不同的地方
    off_t data_off=0, hole_off=0, cur_off=0;
    long long fileSize, blockSize, pos=0;
    //拿到檔案大小的方法，用lseek移到檔案尾巴，看回傳值
    fileSize = lseek(inputFd, 0, SEEK_END);
    //讀到大小後記得用lseek回到原位（0）
    lseek(inputFd, 0, SEEK_SET);

    inputPtr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED , inputFd , 0);
    ftruncate(outputFd, fileSize);//設定檔案大小
    outputPtr = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED , outputFd , 0);

    //如果目前檔案位置不等於檔案大小，就代表沒讀完
	while (lseek(outputFd, 0, SEEK_CUR) != fileSize) { 
		cur_off = lseek(inputFd, cur_off, SEEK_DATA);
        data_off = cur_off;
		cur_off = lseek(inputFd, cur_off, SEEK_HOLE);
        hole_off = cur_off;
        //第一種情況，資料在前面，洞在後面，不用特別處理
        //第二種情況，洞在前面，資料在後面，處理一下
        if (data_off > hole_off) 
            continue;
        //至此，data_off一定在前面，hole_off一定在後面
		blockSize = hole_off - data_off;
        //這一行實現檔案複製
		memcpy(outputPtr + data_off, inputPtr + data_off, blockSize);//🐶 🐱 🐭 🐹 🐰 🦊
        lseek(outputFd, hole_off, SEEK_SET);
    }
    assert(munmap(inputPtr, fileSize) == 0);
    assert(munmap(outputPtr, fileSize) == 0);
    
    assert(close (inputFd) == 0);
    assert(close (outputFd) == 0);
 
    return (EXIT_SUCCESS);
}

