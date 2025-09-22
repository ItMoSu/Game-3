#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <conio.h>
#define MAX 101
#define SIZE 36
#define ROWS 6
#define COLS 6

// buat warna karena males buat manual
#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define BLUE "\e[0;34m"
#define CYAN "\e[0;36m"
#define WHITE "\e[0;37m"
#define RESET "\033[0m"
#define ORANGE "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GREEN "\033[1;32m"
#define GREEN "\e[0;32m"
#define PURPLE "\e[0;35m"

typedef struct User{
    char username[MAX];
    char password[MAX];
    int highscore;
    struct User *next, *prev;
} User;

User* hashTable[SIZE];
bool userExist = false;
char currentUser[MAX];

// function Game
int grid[ROWS][COLS];
int userRow = 0, userCol = 0;
int lockedRow = -1, lockedCol = -1;
bool inLockedMode = false;
int movesLeft = 15;
int score = 0;
bool gamePaused = false;
bool isAnimating = false;
void displayGrid();
void swapTiles();
int randomInt(int min, int max);
bool isMatch(int row, int col);
bool hasPossibleMove();
void ensureStartingMatch();
bool hasInitialMatches();
void initializeGrid();
void fillEmptyTiles();
void dropTiles();
bool checkHorizontalMatch(int row, int col);
bool checkVerticalMatch(int row, int col);
bool isBombPattern(int row, int col);
void clearBombPattern();
void handleMatches();
void displayGrid();
bool isAdjacent(int row1, int col1, int row2, int col2);
void swapTiles(int row1, int col1, int row2, int col2);
bool isValidSwap(int row1, int col1, int row2, int col2);
void handleInput(int input);

// function Menu
void quickSort(User *arr[], int low, int high);
int partition(User *arr[], int low, int high);
int hash(char c);
void insert(User *newUser);
void start();
void playGame();
void hallOfFame();
void display_pause(int select);
void display_menu(int select);
void display_login(int select);
void pause_menu();
void main_menu();
void login_menu();
void load_data();
void login_page();
void register_page();


void quickSort(User *arr[], int low, int high){
    if(low < high){
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int partition(User *arr[], int low, int high){
    int pivot = arr[high]->highscore;
    int i = (low - 1);

    for(int j = low; j <= high - 1; j++){
        if(arr[j]->highscore > pivot){
            i++;
            User *temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    User *temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

int hash(char c){
    if(c >= 'A' && c <= 'Z') c += 32;

    return(c >= 'a' && c <= 'z') ? c-'a' : 26;
}

void insert(User *newUser){
    int idx = hash(newUser->username[0]);

    if(!hashTable[idx]){
        hashTable[idx] = newUser;
    }else{
        User* temp = hashTable[idx];
        User* prev = NULL;

        while(temp && strcmp(temp->username, newUser->username) < 0){
            prev = temp;
            temp = temp->next;
        }

        //buat masuk di head
        if(!prev){
            newUser->next = hashTable[idx];
            if(hashTable[idx]) hashTable[idx]->prev = newUser;
            hashTable[idx] = newUser;

        //buat masuk di tengah atau tail
        }else{
            newUser->next = temp;
            newUser->prev = prev;
            prev->next = newUser;
            if(temp) temp->prev = newUser;
        }
    }
}

void start(){
    initializeGrid();
    score = 0;
    movesLeft = 15;
    system("cls");
    playGame();
}

void playGame(){
    while(movesLeft > 0){
        displayGrid();
        int input = getch();
        handleInput(input);
    }

    // kalau sudah habis movenya
    int idx = hash(currentUser[0]);
    User *temp = hashTable[idx];
    while(temp != NULL){
        if(strcmp(temp->username, currentUser) == 0){
            break;
        }
    }

    printf(CYAN "GOOD JOB! You Achieved Score of %d Points!\n", score);
    if(strcmp(temp->username, currentUser) == 0){
        if(temp->highscore < score){
            printf("You broke your previous high score of %d points!\n", temp->highscore);
            temp->highscore = score;

            FILE *fp = fopen("users.txt", "r+");
            if(fp != NULL){
                char line[MAX*3], hashedPass[MAX], fileUsername[MAX];
                long pos;
                while (fgets(line, sizeof(line), fp)){
                    pos = ftell(fp) - strlen(line) - 1;

                    sscanf(line, "%[^#]#%[^#]#%*d", fileUsername, hashedPass);
                    if(strcmp(fileUsername, currentUser) == 0){
                        fseek(fp, pos, SEEK_SET);
                        fprintf(fp, "%s#%s#%d\n", currentUser, hashedPass, temp->highscore);
                        fflush(fp);
                        break;
                    }
                }
                fclose(fp);
            }
        }else{
            printf("You are %d points off from your high score, %d points!\n", temp->highscore - score, temp->highscore);
        }
    }

    printf("Please press Enter to continue..." RESET);
    getch();
    system("cls");
    login_menu();
}

void hallOfFame(){
    User *showUsers[MAX];
    int totalUsers = 0;

    for(int i = 0; i < SIZE; i++){
        User *temp = hashTable[i];
        while(temp != NULL){
            showUsers[totalUsers] = temp;
            totalUsers++;
            temp = temp->next;
        }
    }

    if(totalUsers == 0){
        puts(BLUE"    XYtrus Leaderboard");
        puts("    ==================" WHITE);
        printf("  Username \t" CYAN "|" WHITE " High Score \n");
        puts(CYAN "==============================" WHITE);
        puts("    No User Data Currently");
        puts("\n\n\tPage 1 of 1\n\n");
        printf("    Press Enter to Exit");
        getch();
        system("cls");
        return;
    }else{
        // tampilan data berdasarkan quickSort
        quickSort(showUsers, 0, totalUsers-1);

        int page = 0, usersPerPage = 10;
        char input;

        do{ system("cls");
            puts(BLUE"    XYtrus Leaderboard");
            puts("    ==================" WHITE);
            printf("  Username \t" CYAN "|" WHITE " High Score \n");
            puts(CYAN "==============================" WHITE);
            int start = page * usersPerPage;
            int end = (start + usersPerPage > totalUsers) ? totalUsers : start + usersPerPage;

            for(int i = start; i < end; i++){
                if(showUsers[i]->highscore == 0){
                    printf("  %-13s " CYAN "|" WHITE " N/A\n", showUsers[i]->username);
                }else{
                    printf("  %-13s " CYAN "|" WHITE " %d\n", showUsers[i]->username, showUsers[i]->highscore);
                }
            }
            printf("\n\n\tPage %d of %d\n", page + 1, (totalUsers + usersPerPage - 1) / usersPerPage);

            if(page + 1 == 1){
                puts("\t\t > D\n");
            }else if(page + 1 == (totalUsers + usersPerPage - 1) / usersPerPage){
                puts("\tA < \n");
            }else{
                puts("\tA <  |  > D\n");
            }

            printf("     Press Enter to Exit");
            input = getch();
            if(input == 'a' || input == 'A'){
                if(page > 0){
                    page--;
                }
            }else if(input == 'd' || input == 'D'){
                if(end < totalUsers){
                    page++;
                }
            }
        }while(input != 13);

        system("cls");
        return;
    }
}

void display_pause(int select){
    system("cls");
    printf("Game Paused\n");

    const char* options[] = {"Resume Game      ", "Quit Game (Progress Will not be Saved)"};
    for(int i=0; i<2; i++){
        if(i == select){
            printf(GREEN "%s \t<<<\n" WHITE, options[i]);
        }else{
            printf("%s\n", options[i]);
        }
    }
}

void display_login(int select){
    system("cls");
    printf("Welcome to XYtrus, "CYAN "%s" WHITE "!\n", currentUser);

    const char* options[] = {"Play Game", "Hall of Fame", "Log Out", "Exit Game"};
    for(int i=0; i<4; i++){
        if(i == select){
            printf(GREEN "%s \t<<<\n" WHITE, options[i]);
        }else{
            printf("%s\n", options[i]);
        }
    }
}

void display_menu(int select){
    system("cls");
    puts("Welcome to XYtrus!");

    const char* options[] = {"Login  ", "Register", "Hall of Fame", "Exit Game"};
    for(int i=0; i<4; i++){
        if(i == select){
            printf(GREEN"%s \t<<<\n" WHITE, options[i]);
        }else{
            printf("%s\n", options[i]);
        }
    }
}

void pause_menu(){
    int key, select = 0;
    gamePaused = true;

    while(gamePaused){
        display_pause(select);
        key = getch();

        if(key == 'w' || key == 'W'){
            select = (select - 1 + 2) % 2;
        }else if(key == 's' || key == 'S'){
            select = (select + 1) % 2;
        }else if(key == 13){
            system("cls");
            switch(select){
                case 0:
                    gamePaused = false;
                    break;
                case 1:
                    login_menu();
                    break;
            }
        }
    }
}

void login_menu(){
    int key, select = 0;

    while(1){
        display_login(select);
        key = getch();

        if(key == 'w' || key == 'W'){
            select = (select - 1 + 4) % 4;
        }else if(key == 's' || key == 'S'){
            select = (select + 1) % 4;
        }else if(key == 13){
            system("cls");
            switch(select){
                case 0:
                    start();
                    break;
                case 1:
                    hallOfFame();
                    break;
                case 2:
                    strcpy(currentUser, "");
                    main_menu();
                    break;
                case 3:
                    puts("\n\tThanks for playing the game!\n");
                    sleep(1);
                    exit(0);
            }
        }
    }
}

void main_menu(){
    int key, select = 0;

    while(1){
        display_menu(select);
        key = getch();

        if(key == 'w' || key == 'W'){
            select = (select - 1 + 4) % 4;
        }else if(key == 's' || key == 'S'){
            select = (select + 1) % 4;
        }else if(key == 13){
            system("cls");
            switch(select){
                case 0:
                    login_page();
                    break;
                case 1:
                    register_page();
                    break;
                case 2:
                    hallOfFame();
                    break;
                case 3:
                    puts("\n\tThanks for playing the game!\n");
                    sleep(1);
                    exit(0);
            }
        }
    }
}

void load_data(){
    FILE *fp = fopen("users.txt","r");
    if(!fp){
        //puts("File users.txt not detected!");
        return;
    }

    char line[MAX*3], unhashedPass[MAX];
    while(fgets(line, sizeof(line), fp)){
        User *addData = (User*)malloc(sizeof(User));
        addData->prev = addData->next = NULL;

        sscanf(line, "%[^#]#%[^#]#%d", addData->username, unhashedPass, &addData->highscore);
        userExist = true;

        // buat unhash password dari users.txt
        int i;
        for(i = 0; unhashedPass[i] != '\0'; i++){
            char ch = unhashedPass[i];
            if(ch >= 'a' && ch <= 'z'){
                ch = ch - 10;
                if(ch < 'a') ch = ch + 26;
            }else if(ch >= 'A' && ch <= 'Z'){
                ch = ch - 10;
                if(ch < 'A') ch = ch + 26;
            }
            addData->password[i] = ch;
        }
        addData->password[i] = '\0';

        insert(addData);
    }

    fclose(fp);
}

void login_page(){
    char tempName[MAX], tempPass[MAX];
    bool valid = false;
    int ch = 0;

    while(!valid){
        system("cls");
        puts("Please Fill In Your Credentials");
        printf("Input Your Username : ");
        printf("\nPassword : ");
        printf("\n\nPress ESC to go back\n");

        // koordinat input username
        strcpy(tempName, "");
        printf("\033[F\033[F\033[F\033[F\033[%dC", 22);
        int i = 0;
        while(1){
            ch = getch();
            if(ch == 27){  // balik ke main menu
                strcpy(tempName, "");
                strcpy(tempPass, "");
                return;
            }else if(ch == 13){  // enter
                tempName[i] = '\0';
                break;
            } else if (ch == 8 && i > 0) {  // hapus
                printf("\b \b");
                i--;
            } else if (ch != 8) {  // normal
                tempName[i] = ch;
                i++;
                printf("%c", ch);
            }
        }

        // koordinat input password
        strcpy(tempPass, "");
        printf("\033[B\033[B\033[F\033[%dC", 11);
        i = 0;
        while(1){
            ch = getch();
            if(ch == 27){  // balik ke main menu
                strcpy(tempName, "");
                strcpy(tempPass, "");
                return;
            }else if(ch == 13){  // enter
                tempPass[i] = '\0';
                break;
            }else if(ch == 8 && i > 0){ // hapus
                printf("\b \b");
                i--;
            }else if(ch != 8){ // password
                tempPass[i] = ch;
                i++;
                printf("*");
            }
        }
        puts("");


        if(strlen(tempName) == 0 || strlen(tempPass) == 0){
            printf(RED "Both fields must be filled.\n" WHITE);
            printf("Press ESC to go back or Enter to try again...\n");
            ch = getch();
            if(ch == 27){
                strcpy(tempName, "");
                strcpy(tempPass, "");
                break; // balik ke mainmenu
            }
            continue;
        }

        // cari data di hashtable dari inisial nama
        int idx = hash(tempName[0]);
        User *temp = hashTable[idx];

        bool found = false;
        while(temp){
            if(strcmp(temp->username, tempName) == 0){ // tempName cocok
                found = true;
                if(strcmp(temp->password, tempPass) == 0){ // tempPass cocok
                    strcpy(currentUser, tempName);
                    printf(GREEN "Login successful! Welcome %s.\n" WHITE, currentUser);
                    strcpy(tempName, "");
                    strcpy(tempPass, "");
                    valid = true;
                    sleep(2);
                    login_menu();
                }else{
                    printf(RED "Incorrect password! Please try again.\n" WHITE);
                }
                break;
            }
            temp = temp->next;
        }

        if(!found){
            printf(RED "Username not found! Please try again.\n" WHITE);
        }

        if(!valid){
            printf("Press ESC to go back or Enter to try again...\n");
            ch = getch();
            if(ch == 27){
                strcpy(tempName, "");
                strcpy(tempPass, "");
                break; // balik ke mainmenu
            }
        }
    }
}

void register_page(){
    char tempName[MAX], tempPass[MAX], hashedPass[MAX];
    bool valid = false;
    int ch = 0;

    while(!valid){
        system("cls");
        puts("Register New Account, username only starts with alphabets");
        printf("Input Your Username : ");
        printf("\nPassword : ");
        printf("\n\nPress ESC to go back\n");

        // koordinat input username
        strcpy(tempName, "");
        printf("\033[F\033[F\033[F\033[F\033[%dC", 22);
        int i = 0;
        bool validUsername = true;
        while(1){
            ch = getch();
            if(ch == 27){
                strcpy(tempName, "");
                strcpy(tempPass, "");
                return;
            }else if(ch == 13){  // enter
                tempPass[i] = '\0';
                break;
            }else if(ch == 8 && i > 0){ // hapus
                printf("\b \b");
                i--;
            }else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')){
                tempName[i] = ch;
                printf("%c", ch);
                i++;
            }else{
                validUsername = false;
            }
        }

        // koordinat input password
        strcpy(tempPass, "");
        printf("\033[B\033[B\033[F\033[%dC", 11);
        i = 0;
        while(1){
            ch = getch();
            if(ch == 27){  // balik ke main menu
                strcpy(tempName, "");
                strcpy(tempPass, "");
                return;
            }else if(ch == 13){  // enter
                tempPass[i] = '\0';
                break;
            }else if(ch == 8 && i > 0){ // hapus
                printf("\b \b");
                i--;
            }else if(ch != 8){ // password
                tempPass[i] = ch;
                i++;
                printf("*");
            }
        }
        puts("");

        // validasi input
        if(!validUsername || strlen(tempName) == 0 || (tempName[0] >= '0' && tempName[0] <= '9')){
            printf(RED "Username can only contain letters and numbers, and must start with a letter.\n" WHITE);
            printf("Press ESC to go back or Enter to try again...\n");
            ch = getch();
            if(ch == 27){
                strcpy(tempName, "");
                strcpy(tempPass, "");
                break; // balik ke mainmenu
            }
            continue;
        }

        // mengecek apakah sudah ada di file atau belum
        FILE *fp = fopen("users.txt", "r");
        if(fp){
            char line[MAX * 3];
            char tempUser[MAX];
            bool usernameTaken = false;

            while(fgets(line, sizeof(line), fp)){
                sscanf(line, "%[^#]", tempUser);
                if(strcmp(tempUser, tempName) == 0){
                    usernameTaken = true;
                    break;
                }
            }
            fclose(fp);

            if(usernameTaken){
                printf(RED "Username already exists. Please choose a different username.\n" WHITE);
                printf("Press ESC to go back or Enter to try again...\n");
                ch = getch();
                if(ch == 27){
                    strcpy(tempName, "");
                    strcpy(tempPass, "");
                    break; // balik ke mainmenu
                }
                continue;
            }
        }

        if(strlen(tempPass) == 0){
            printf(RED "Password cannot be empty.\n" WHITE);
            printf("Press ESC to go back or Enter to try again...\n");
            ch = getch();
            if(ch == 27){
                strcpy(tempName, "");
                strcpy(tempPass, "");
                break; // balik ke mainmenu
            }
            continue;
        }

        // hashing password karena sudah berhasil
        for(i = 0; tempPass[i] != '\0'; i++){
            char ch = tempPass[i];
            if(ch <= 'z' && ch >= 'v' ){
                ch -= 16;
                hashedPass[i] = ch;
                continue;
            }
            if(ch >= 'a' && ch <= 'z'){
                ch += 10;
                if(ch > 'z') ch -= 26;
                hashedPass[i] = ch;
                continue;
            }else if(ch >= 'A' && ch <= 'Z'){
                ch += 10;
                if(ch > 'Z') ch -= 26;
                hashedPass[i] = ch;
                continue;
            }else if(ch >= '0' && ch <= '9'){
                hashedPass[i] = ch;
                continue;
            }
        }
        hashedPass[i] = '\0';

        // simpan username dan hashed password ke file users.txt
        fp = fopen("users.txt", "a");
        if(fp){
            fprintf(fp, "%s#%s#0\n", tempName, hashedPass);
            fclose(fp);
        }

        // masukkan data kedalam hash table
        int idx = hash(tempName[0]);
        User *newUser = (User*)malloc(sizeof(User));
        strcpy(newUser->username, tempName);
        strcpy(newUser->password, tempPass);
        newUser->highscore = 0;
        newUser->next = newUser->prev = NULL;
        insert(newUser);

        printf(GREEN "Registration successful! Welcome %s.\n" WHITE, tempName);
        userExist = true;
        strcpy(tempName, "");
        strcpy(tempPass, "");
        valid = true;
        sleep(2);
    }
}

int randomInt(int min, int max){
    return min + rand() % (max - min + 1);
}

bool isMatch(int row, int col){
    int tile = grid[row][col];
    if(tile == 0) return false;

    if(col >= 2 && grid[row][col - 1] == tile && grid[row][col - 2] == tile)
        return true;
    if(col <= COLS - 3 && grid[row][col + 1] == tile && grid[row][col + 2] == tile)
        return true;
    if(col >= 1 && col <= COLS - 2 && grid[row][col - 1] == tile && grid[row][col + 1] == tile)
        return true;

    if(row >= 2 && grid[row - 1][col] == tile && grid[row - 2][col] == tile)
        return true;
    if(row <= ROWS - 3 && grid[row + 1][col] == tile && grid[row + 2][col] == tile)
        return true;
    if(row >= 1 && row <= ROWS - 2 && grid[row - 1][col] == tile && grid[row + 1][col] == tile)
        return true;

    return false;
}

bool hasPossibleMove(){
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            if(j < COLS - 1){
                swapTiles(i, j, i, j + 1);
                if(isMatch(i, j) || isMatch(i, j + 1)){
                    swapTiles(i, j, i, j + 1);
                    return true;
                }
                swapTiles(i, j, i, j + 1);
            }
            if(i < ROWS - 1){
                swapTiles(i, j, i + 1, j);
                if(isMatch(i, j) || isMatch(i + 1, j)){
                    swapTiles(i, j, i + 1, j);
                    return true;
                }
                swapTiles(i, j, i + 1, j);
            }
        }
    }
    return false;
}

void ensureStartingMatch(){
    int matchRow = randomInt(0, ROWS - 1);
    int matchCol = randomInt(0, COLS - 3);

    int tile = randomInt(1, 4);
    grid[matchRow][matchCol] = tile;
    grid[matchRow][matchCol + 1] = tile;
    grid[matchRow][matchCol + 2] = tile;
}

bool hasInitialMatches(){
    for(int row = 0; row < ROWS; row++){
        for(int col = 0; col < COLS; col++){
            if(isMatch(row, col)){
                return true;
            }
        }
    }
    return false;
}

void initializeGrid(){
    srand(time(NULL));

    do{
        for(int i = 0; i < ROWS; i++){
            for(int j = 0; j < COLS; j++){
                grid[i][j] = randomInt(1, 4);
            }
        }

    } while(!hasPossibleMove() || hasInitialMatches());
}

void fillEmptyTiles(){
    for(int col = 0; col < COLS; col++){
        for(int row = 0; row < ROWS; row++){
            if(grid[row][col] == 0){
                grid[row][col] = randomInt(1, 4);
            }
        }
    }
}

void dropTiles(){
    isAnimating = true;
    for(int col = 0; col < COLS; col++){
        for(int row = ROWS - 1; row >= 0; row--){
            if(grid[row][col] == 0){
                for (int k = row; k > 0; k--){
                    grid[k][col] = grid[k - 1][col];
                }
                grid[0][col] = randomInt(1, 4);
                displayGrid();
                usleep(200000);
            }
        }
    }
    isAnimating = false;
}

bool checkHorizontalMatch(int row, int col){
    int tile = grid[row][col];
    if(tile == 0) return false;

    if(col <= COLS - 3 && grid[row][col + 1] == tile && grid[row][col + 2] == tile){
        if(col <= COLS - 4 && grid[row][col + 3] == tile){
            if (col <= COLS - 5 && grid[row][col + 4] == tile) return true;
            return true;
        }
        return true;
    }
    return false;
}

bool checkVerticalMatch(int row, int col){
    int tile = grid[row][col];
    if(tile == 0) return false;

    if(row <= ROWS - 3 && grid[row + 1][col] == tile && grid[row + 2][col] == tile){
        if(row <= ROWS - 4 && grid[row + 3][col] == tile){
            if (row <= ROWS - 5 && grid[row + 4][col] == tile) return true;
            return true;
        }
        return true;
    }
    return false;
}

bool isBombPattern(int row, int col){
    int tile = grid[row][col];
    if (tile == 0) return false;

    bool isT1 = (col >= 1 && col <= COLS - 2 && row <= ROWS - 2 &&
                 grid[row][col - 1] == tile && grid[row][col + 1] == tile && grid[row + 1][col] == tile);
    bool isT2 = (row >= 1 && row <= ROWS - 2 && col >= 1 &&
                 grid[row - 1][col] == tile && grid[row + 1][col] == tile && grid[row][col - 1] == tile);
    bool isT3 = (col >= 1 && col <= COLS - 2 && row >= 1 &&
                 grid[row][col - 1] == tile && grid[row][col + 1] == tile && grid[row - 1][col] == tile);
    bool isT4 = (row >= 1 && row <= ROWS - 2 && col <= COLS - 2 &&
                 grid[row - 1][col] == tile && grid[row + 1][col] == tile && grid[row][col + 1] == tile);

    bool isL1 = (row <= ROWS - 2 && col <= COLS - 2 &&
                 grid[row + 1][col] == tile && grid[row][col + 1] == tile && grid[row + 1][col + 1] == tile);
    bool isL2 = (row <= ROWS - 2 && col >= 1 &&
                 grid[row + 1][col] == tile && grid[row][col - 1] == tile && grid[row + 1][col - 1] == tile);
    bool isL3 = (row >= 1 && col <= COLS - 2 &&
                 grid[row - 1][col] == tile && grid[row][col + 1] == tile && grid[row - 1][col + 1] == tile);
    bool isL4 = (row >= 1 && col >= 1 &&
                 grid[row - 1][col] == tile && grid[row][col - 1] == tile && grid[row - 1][col - 1] == tile);

    return (isT1 || isT2 || isT3 || isT4 || isL1 || isL2 || isL3 || isL4);
}

void clearBombPattern(int row, int col){
    for(int i = row - 1; i <= row + 1; i++){
        for(int j = col - 1; j <= col + 1; j++){
            if(i >= 0 && i < ROWS && j >= 0 && j < COLS && grid[i][j] != 0){
                grid[i][j] = 0;
                score += 100;
            }
        }
    }
}

void handleMatches(){
    bool foundMatch;

    do{
        foundMatch = false;
        for (int row = 0; row < ROWS; row++){
            for (int col = 0; col < COLS; col++){
                int currentMatchLength = 0;
                bool horizontalMatch = false;
                bool verticalMatch = false;
                bool isBomb = false;

                if(isBombPattern(row, col)){
                    isBomb = true;
                    clearBombPattern(row, col);
                    score += 500;
                    movesLeft += 3;
                    foundMatch = true;
                }

                if(!isBomb){
                    if(checkHorizontalMatch(row, col)){
                        horizontalMatch = true;
                        currentMatchLength = 3;
                        if(col + 3 < COLS && grid[row][col] == grid[row][col + 3]) currentMatchLength = 4;
                        if(col + 4 < COLS && grid[row][col] == grid[row][col + 4]) currentMatchLength = 5;
                    }

                    if(checkVerticalMatch(row, col)){
                        verticalMatch = true;
                        currentMatchLength = 3;
                        if(row + 3 < ROWS && grid[row][col] == grid[row + 3][col]) currentMatchLength = 4;
                        if(row + 4 < ROWS && grid[row][col] == grid[row + 4][col]) currentMatchLength = 5;
                    }

                    if (horizontalMatch || verticalMatch){
                        foundMatch = true;
                        if(horizontalMatch){
                            for(int i = 0; i < currentMatchLength; i++) grid[row][col + i] = 0;
                        }else{
                            for(int i = 0; i < currentMatchLength; i++) grid[row + i][col] = 0;
                        }

                        if(currentMatchLength == 5) score += 500, movesLeft += 3;
                        else if(currentMatchLength == 4) score += 400, movesLeft += 1;
                        else score += 300;

                        displayGrid();
                        usleep(150000);
                    }
                }
            }
        }
        dropTiles();
        fillEmptyTiles();
        displayGrid();
    } while(foundMatch);
}

void displayGrid(){
    system("cls");

    if(!inLockedMode){
        puts("Press " CYAN"'w','a','s','d'"RESET " to move around, "CYAN "'Enter'"RESET " to choose your tile, " CYAN "'Esc'"RESET " to pause game");
    }else{
        puts("Press " CYAN"'e'"RESET " to unchoose your tile, "CYAN "'Enter'"RESET " to swap with that tile, " CYAN "'Esc'"RESET " to pause game");
    }
    puts("");
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            char *color;
            switch (grid[i][j]) {
                case 1: color = BLUE; break;
                case 2: color = ORANGE; break;
                case 3: color = PURPLE; break;
                case 4: color = RED; break;
                default: color = RESET; break;
            }

            if(inLockedMode && i == lockedRow && j == lockedCol){
                if(j == 0) printf(" ");
                if(i == userRow && j == userCol && j == 0) printf("\b \b");
                if(i == userRow && j == userCol) printf(YELLOW " [%d] " RESET, grid[i][j]);
                else printf(YELLOW "[%d] " RESET, grid[i][j]);

                if(i == userRow && j+1 == userCol) printf("\b \b");
            }else if (i == userRow && j == userCol){
                printf(LIGHT_GREEN ":[%d]:" RESET, grid[i][j]);
            }else{
                if(j == 0) printf(" ");
                if(grid[i][j] == 0) printf("[ ] ");
                else  printf("%s[%d] " RESET, color, grid[i][j]);

                if(i == userRow && j+1 == userCol) printf("\b \b");
            }
        }
        puts("");
    }
    int idx = hash(currentUser[0]);
    User *temp = hashTable[idx];
    while(temp != NULL){
        if(strcmp(temp->username, currentUser) == 0){
            break;
        }
        temp = temp->next;
    }
    puts("");
    if(temp->highscore == 0) printf("%s's Hi-score: " YELLOW "N/A\n" RESET, currentUser);
    else printf("%s's Hi-score: " GREEN "%d\n" RESET, currentUser, temp->highscore);

    printf("Current Score: " GREEN "%d\n" RESET, score);
    printf("Moves Left: " GREEN "%d\n\n" RESET, movesLeft);
}

bool isAdjacent(int row1, int col1, int row2, int col2){
    return (abs(row1 - row2) + abs(col1 - col2)) == 1;
}

void swapTiles(int row1, int col1, int row2, int col2){
    int temp = grid[row1][col1];
    grid[row1][col1] = grid[row2][col2];
    grid[row2][col2] = temp;

    userRow = row2;
    userCol = col2;
}

bool isValidSwap(int row1, int col1, int row2, int col2){
    swapTiles(row1, col1, row2, col2);

    bool valid = isMatch(row1, col1) || isMatch(row2, col2);

    if(!valid){
        swapTiles(row1, col1, row2, col2);
    }

    return valid;
}

void handleInput(int input){
    if(isAnimating) return;

    if(input == 27){ // Esc
        pause_menu();
        return;
    }

    if(!inLockedMode){
        switch(input){
            case 87: case 119:
                if (userRow > 0) userRow--;
                break;
            case 83: case 115: // S/s
                if (userRow < ROWS - 1) userRow++;
                break;
            case 65: case 97: // A/a
                if (userCol > 0) userCol--;
                break;
            case 68: case 100: // D/d
                if (userCol < COLS - 1) userCol++;
                break;
            case 13: // Enter
                lockedRow = userRow;
                lockedCol = userCol;
                inLockedMode = true;
                break;
        }
    }else{
        switch(input){
            case 87: case 119: // W/w
                if(lockedRow == 0) break;
                userRow = lockedRow - 1;
                userCol = lockedCol;
                break;
            case 83: case 115: // S/s
                if(lockedRow == ROWS - 1) break;
                userRow = lockedRow + 1;
                userCol = lockedCol;
                break;
            case 65: case 97: // A/a
                if(lockedCol == 0) break;
                userRow = lockedRow;
                userCol = lockedCol - 1;
                break;
            case 68: case 100: // D/d
                if(lockedCol == COLS - 1) break;
                userRow = lockedRow;
                userCol = lockedCol + 1;
                break;
            case 13: // Enter
                if(isAdjacent(userRow, userCol, lockedRow, lockedCol)){
                    swapTiles(lockedRow, lockedCol, userRow, userCol);
                    if(isMatch(userRow, userCol) || isMatch(lockedRow, lockedCol)){
                        handleMatches();
                        movesLeft--;
                    }else{
                        swapTiles(lockedRow, lockedCol, userRow, userCol);
                    }
                    inLockedMode = false;
                }
                break;
            case 69: case 101: // E/e
                inLockedMode = false;
                break;
        }

        if(userRow < 0) userRow = 0;
        if(userRow >= ROWS) userRow = ROWS - 1;
        if(userCol < 0) userCol = 0;
        if(userCol >= COLS) userCol = COLS - 1;
    }
}

int main(){
    srand(time(NULL));
    load_data();
    main_menu();
    return 0;
}
