#define _POSIX_C_SOURCE 200809L//strnlen

#include <p_and_p.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

static_assert (sizeof(size_t)==8);

int saveItemDetails(const struct ItemDetails* arr, size_t nmemb, int fd) {
  uint64_t itemCount = nmemb;
  
  struct ItemDetails itemCpy[itemCount];
  // Before open file validate everyting and catch error early
  for (uint64_t i = 0; i<itemCount;i++){
    int itemValid = isValidItemDetails(&itemCpy[i]);
    if (!itemValid){
      return 1;
    }
  }

  FILE *fptr = fdopen(fd,"wb");

  fwrite(&itemCount,1,sizeof(uint64_t),fptr);
  fwrite(&itemCpy,1,sizeof(itemCpy),fptr);


  fclose(fptr);

  return 0;
}



int saveItemDetailsToPath(const struct ItemDetails* arr, size_t nmemb, const char* filename) {

  FILE *ofp = fopen(filename, "wb");
  assert(ofp != NULL);
  int fd = fileno(ofp);
  assert(fd != -1);
  int res = saveItemDetails(arr, nmemb, fd);
  fclose(ofp);
  return res;
}



int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {
  FILE* fptr = fdopen(fd, "rb");
  if (fptr == NULL) {
      return 1;
  }

  fread(nmemb, sizeof(uint64_t), 1, fptr)!=1;
  
  // Create a new struct ItemDetails pointer and allocate space
  struct ItemDetails* newItmPtr = calloc(*nmemb, sizeof(struct ItemDetails));
  if (newItmPtr == NULL) {
    fclose(fptr);
    return 1;
  }

  for (size_t i = 0; i < *nmemb; i++) {
    if(fread(&newItmPtr[i],sizeof(struct ItemDetails),1,fptr)!=1){
      return 1;
    }
  }

  fclose(fptr);
  *ptr = newItmPtr;
  return 0;
  }

 

int isValidName(const char *str) {
  if (str==NULL){
      return 0;
  }
  
  size_t nameLength = strnlen(str,DEFAULT_BUFFER_SIZE);
  if (nameLength==0){
      return 0;
  }

  if(nameLength < DEFAULT_BUFFER_SIZE) {
    for (size_t i=0; i<nameLength; i++){
      if (isgraph(str[i]) == 0){
        return 0;
      }
    } 
    return 1; 
  }
  return 0;
}



int isValidMultiword(const char *str) {
  if (str==NULL){
    return 0;
  }
  size_t nameLength = strnlen(str,DEFAULT_BUFFER_SIZE);
  if (nameLength==0){
    return 0;
  }
  if(nameLength < DEFAULT_BUFFER_SIZE) {
    if (str[0]==' ' || str[nameLength-1]== ' '){
      return 0;
    };
    for (size_t i=0; i<nameLength; i++){
      if (!isgraph(str[i]) && str[i] != ' '){
        return 0;
      }
    } 
    return 1; 
  }
  return 0;
}



int isValidItemDetails(const struct ItemDetails *id) {

  struct ItemDetails itemCpy;
  memcpy (&itemCpy, id, sizeof(struct ItemDetails));


  // 0 is false/invalid
  int checkID, checkName, checkMultiword;
  checkID = (itemCpy.itemID <= UINT64_MAX);
  checkName=isValidName(itemCpy.name);
  checkMultiword=isValidMultiword(itemCpy.desc);

  int result =(checkID && checkName && checkMultiword);
  return result; 
}



int isValidCharacter(const struct Character * c) {
  struct Character charCpy;
  memcpy (&charCpy, c, sizeof(struct Character));


  uint64_t totalItemCount = 0;
  // index in order: 0:ID, 1:SClass, 2:Proffession, 3:Name, 4:InvSize, 5:TotItmCount
  int checkLst[6];

  checkLst[0] = (charCpy.characterID <= UINT64_MAX);
  checkLst[1]= (charCpy.socialClass >= MENDICANT && charCpy.socialClass <= ARISTOCRACY);// check if within the enum range
  checkLst[2]=isValidName(charCpy.profession);
  checkLst[3]=isValidMultiword(charCpy.name);
  checkLst[4] = (charCpy.inventorySize <=MAX_ITEMS); // num of items carried by char
  checkLst[5]= 1;

  // get the total. should not exceed MAX_ITEMS
  for (size_t i = 0; i<charCpy.inventorySize;i++){
    if((charCpy.inventory[i].quantity > MAX_ITEMS)){
      checkLst[5]=0;
      return 0;
    }
    totalItemCount += charCpy.inventory[i].quantity;
  }
  checkLst[5]=(totalItemCount<=MAX_ITEMS);

  
  //checks all fields in struct passed validation.
  for (int i = 0; i < 6; i++) {
    if (checkLst[i] != 1) {
        return 0; 
        break;
    }
  }

  return 1;
}



int saveCharacters(struct Character *arr, size_t nmemb, int fd) {
  if (nmemb > UINT64_MAX || nmemb==0){
    return 1;
  }
  uint64_t charCount = (uint64_t) nmemb;
  struct Character charCpy[charCount];
  memcpy(&charCpy,arr,sizeof(struct Character)*charCount);


  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    return 1;
  }

  fwrite(&charCount,1,sizeof(uint64_t),fptr);
  fwrite(&charCpy,1,sizeof(charCpy),fptr);

  fflush(fptr);
  fclose(fptr);
  return 0;
}


int loadCharacters(struct Character** ptr, size_t* nmemb, int fd) {
  
  return 0;
}




int secureLoad(const char *filepath) {
  
  return 0;
}

void playGame(struct ItemDetails* ptr, size_t nmemb);

