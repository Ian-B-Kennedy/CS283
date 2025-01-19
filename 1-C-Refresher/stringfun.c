#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);

//add additional prototypes here
int reverse_string(char *, int, int);
int word_print(char *, int, int, int);
int replace_word(char *, int, int, char *, char *);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    if (!buff || !user_str) {
        printf("Error: Null Pointer\n");
        exit(2); //Null Pointer Error
    }
    int count = 0;

    while (*user_str != '\0'){
        if (count >= len){
            printf("Error: Provided input string is to long\n");
            exit(3);
        }

        if (*user_str == ' ' || *user_str == '\t'){
            *(buff + count) = ' ';
            count++;
            while(*(user_str+1) == ' ' || *(user_str+1) == '\t'){
                user_str++;
            }
            user_str++;
        } else { 
            *(buff + count) = *(user_str);
            count++;
            user_str++;
        }
    }

    memset(buff+count, '.', len-count);

    return count;
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT  
    if(len < str_len){
        printf("Error: Invalid lengths inputed\n");
        exit(3); //Invalid Input Lengths
    }

    if(!buff){
        printf("Error: Null Pointer\n");
        exit(2); //Null Pointer Error
    }

    int count = 0;
    int inWord = 0;

    for(int i = 0; i < str_len; i++){
        if(*(buff+i) != ' '){
            if(!inWord){
                count++;
                inWord = 1;
            }
        } else {
            inWord = 0;
        }
    }
    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int reverse_string(char *buff, int len, int str_len) {
    if(len < str_len){
        printf("Error: Invalid lengths inputed\n");
        exit(3); //Invalid Input Lengths
    }

    if(!buff){
        printf("Error: Null Pointer\n");
        exit(2); //Null Pointer Error
    }

    int start = 0;
    int end = str_len - 1;

    while(start < end){
        char temp = *(buff+start);
        *(buff+start) = *(buff+end);
        *(buff+end) = temp;
        end--;
        start++;
    }

    return 0;
}

int word_print(char *buff, int len, int str_len, int words) {
    if(len < str_len){
        printf("Error: Invalid lengths inputed\n");
        exit(3); //Invalid Input Lengths
    }

    if(!buff){
        printf("Error: Null Pointer\n");
        exit(2); //Null Pointer Error
    }

    if(words < 0){
        printf("Error: Invalid number of words\n");
        exit(3);
    }

    int count = 0;
    int inWord;
    int spot;

    // Determines if the first character is a space/not part of a word
    if(*buff == ' '){
        spot = 1;
    } else {
        spot = 0;
    }

    printf("Word Print\n----------\n");    
    for(int i = 0; i < words; i++){
        printf("%d. ", i + 1);
        inWord = 1;
        while(inWord){
            if(*(buff+spot) != ' ' && spot < str_len){
                printf("%c", *(buff+spot));
                count++;
                spot++;
            } else {
                inWord = 0;
                printf(" (%d)\n", count);
                count = 0;   
                spot++;
            }
        }
    }
        
    return 0;
}

int replace_word(char *buff, int len, int str_len, char *word1, char *word2){
    if(len < str_len){
        printf("Error: Invalid lengths inputed\n");
        exit(3); //Invalid Input Lengths
    }

    if(!buff || !word1 || !word2){
        printf("Error: Null Pointer\n");
        exit(2); //Null Pointer Error
    }

    int len1 = 0;
    int len2 = 0;
    int spotFound = 0;
    int wordFound = 0;
    int compareChar = 0;

    while(*(word1+len1) != '\0'){
        len1++;
    }
    while(*(word2+len2) != '\0'){
        len2++;
    }

    if(str_len - len1 + len2 > len){
        printf("Error: Replacement word causes buffer overload\n");
        exit(3);
    }

    for(int i = 0; i < str_len; i++){
        if(*(buff+i) == *(word1+compareChar)){
            compareChar++;
        } else {
            compareChar = 0;
        }
        
        if(compareChar == len1){
            wordFound = 1;
            spotFound = i+1;
            break;
        }
    }
    if(!wordFound){
        printf("Error: Word to replace not in string\n");
        exit(3);
    }

    int tempSize = str_len - spotFound;
    char temp[tempSize];

    // Copies values after word1 into temp buffer
    memcpy(temp, (buff+spotFound), tempSize);
    // Copies word 2 into the spot where word 1 is
    memcpy((buff+spotFound-len1), word2, len2);
    // Copies back all values originall after word1 to be after word 2
    memcpy((buff+spotFound-len1+len2), temp, tempSize);
    // Puts back . after the end of the string incase word 2 is shorter than word 1
    if(len1>len2)
        memset((buff+spotFound-len1+len2+tempSize), '.', len - spotFound-len1+len2+tempSize);
    return spotFound-len1+len2+tempSize;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if argv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
    This is safe as it will detect if you have less than 2 arguments through argc b/c if argc < 2 before trying to read argv[1].
    This is because the code will not evaluate the second codition of the or conditional if the first one already fails. 
    It then will just print out the usage message and exit with an error.
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    } else if (opt == 'x'){ // Checks to ensure you have 3 string arguments for -x
        if(argc < 5){
            printf("Error: -x requires 3 string arguments\n");
            usage(argv[0]);
            exit(1);
        }
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
    The if statement is here because if argc < 3, that means you haven't supplied the input_string parameter so the code can't run.
    This means that argv[2] is empty which would cause an error in the following code section.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = malloc(BUFFER_SZ * sizeof(char)); 
    if (buff == NULL){
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options

        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc != 0){
                printf("Error reversing string, rc = %d\n", rc);
                exit(2);
            }
            printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++){
                printf("%c", *(buff+i));
            }
            printf("\n");
            break;
        case 'w':
            rc = word_print(buff, BUFFER_SZ, user_str_len, count_words(buff, BUFFER_SZ, user_str_len));
            if (rc != 0){
                printf("Error printing words, rc = %d\n", rc);
                exit(2);
            }
            break;
        case 'x':
            rc = replace_word(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
            if (rc < 0){
                printf("Error replacing words, rc = %d\n", rc);
                exit(2);
            }
            printf("Modified String: ");
            for (int i = 0; i < rc; i++){
                printf("%c", *(buff+i));
            }
            printf("\n");
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE

/*
This is a good idea because it ensures that you are able to prevent buffer overflow by using the parameter as a maximum index for the buffer.
It also prevents you from having an issue if a buffer of a different size is ever used when updating this code where hardcoding 50 could result in an index out of bounds error.
Finally, its also very easy to see the length of the buffer when going back through the code after having not worked on it for a while.
*/