#define _POSIX_C_SOURCE 200809L//strnlen

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>



static_assert (sizeof(size_t)==8);
#define DEFAULT_BUFFER_SIZE 512
#define MAX_ITEMS 10


// #define _POSIX_C_SOURCE 200809L//strnlen

// #include <p_and_p.h>
// #include <ctype.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>
// static_assert (sizeof(size_t)==8);










struct ItemDetails {
  uint64_t itemID;
  char name[DEFAULT_BUFFER_SIZE];
  char desc[DEFAULT_BUFFER_SIZE];
};
struct ItemCarried {
  uint64_t itemID;
  size_t quantity;
};
enum CharacterSocialClass {
  MENDICANT,
  LABOURER,
  MERCHANT,
  GENTRY,
  ARISTOCRACY
};
struct Character {
  uint64_t characterID;
  enum CharacterSocialClass socialClass;
  char profession[DEFAULT_BUFFER_SIZE];
  char name[DEFAULT_BUFFER_SIZE];
  size_t inventorySize;
  struct ItemCarried inventory[MAX_ITEMS];
};

//just declaring
int isValidItemDetails(const struct ItemDetails *id);
void playGame(struct ItemDetails* ptr, size_t nmemb);


/**
 * This function validates the data, opens the incomming file descriptor for writing
 * and writes the data to the file. It returns 0 on success and 1 on failure.
 *
 * @param arr The array of ItemDetails to save.
 * @param nmemb The number of elements in the array.
 * @param fd The file descriptor to write to.
 * @return 0 on success, 1 on failure.
 */
int saveItemDetails(const struct ItemDetails* arr, size_t nmemb, int fd) {
  if (nmemb > UINT64_MAX || nmemb==0){
    return 1;
  }
  uint64_t itemCount = (uint64_t) nmemb;
  
  struct ItemDetails itemCpy[itemCount];
  memset(&itemCpy,0,sizeof(struct ItemDetails )* itemCount);
  memcpy(&itemCpy,arr,sizeof(struct ItemDetails)*itemCount);
  if(memcmp(arr,&itemCpy,sizeof(struct ItemDetails)*nmemb)){
    return 1;
  }

  // Before open file validate everyting and catch error early
  for (uint64_t i = 0; i<itemCount;i++){
    int itemValid = isValidItemDetails(&itemCpy[i]);
    if (!itemValid){
      return 1;
    }
  }

  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    return 1;
  }


  size_t bytesWritten=0;
  // Swapped sizeof and count to get bytes being written to file
  bytesWritten+=fwrite(&itemCount,1,sizeof(uint64_t),fptr);
  bytesWritten+=fwrite(&itemCpy,1,sizeof(itemCpy),fptr);

  if (bytesWritten!= (sizeof(itemCpy) +sizeof(itemCount))){
      memset(&itemCpy,0,sizeof(struct ItemDetails )* itemCount);
    return 1;
  }
  fflush(fptr);
  fclose(fptr);
  memset(&itemCpy,0,sizeof(itemCpy));

  return 0;
}

/**
 * This function reads ItemDetails from a file descriptor, validates the data
 * and stores it in the provided pointer. It returns 0 on success and 1 on failure.
 *
 * @param ptr Pointer to store the loaded data.
 * @param nmemb Number of elements loaded.
 * @param fd File descriptor to read from.
 * @return 0 on success, 1 on failure.
 */
int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {
  FILE* fptr = fdopen(fd, "rb");
  if (fptr == NULL) {
      return 1;
  }

  if(fread(nmemb, sizeof(uint64_t), 1, fptr)!=1){
    return 1;
  }

  // Create a new struct ItemDetails pointer and allocate space
  struct ItemDetails* newItmPtr = calloc(*nmemb, sizeof(struct ItemDetails));
  if (newItmPtr == NULL) {
    fclose(fptr);
    return 1;
  }

  // Load and validate each ItemDetails
  for (size_t i = 0; i < *nmemb; i++) {
    if(fread(&newItmPtr[i],sizeof(struct ItemDetails),1,fptr)!=1){
      free(newItmPtr);
      return 1;
    }
    if(!isValidItemDetails(&(newItmPtr[i]))){
      free(newItmPtr);
      return 1;
    }
  }

  fclose(fptr);
  if (*ptr != NULL) {
      free(*ptr);
  }
  *ptr = newItmPtr;
  return 0;
  }


/**
 * This function checks if a string is a valid name.
 * The incomming string should return true with function isgraph()
 * Return 1 on success and 0 on failure.
 *
 * @param str The string to validate.
 * @return 1 if valid, 0 if not.
 */
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

/**
 * This function checks if a string is a valid multiword.
 * The incomming parameter should return true with function isgraph()
 * The incomming parameter is allowed to have spaces but not at the start or end
 * Return 1 on success and 0 on failure.
 *
 * @param str The string to validate.
 * @return 1 if valid, 0 if not.
 */
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


/**
 * This function checks if an itemDetail srtuct is valid. 
 * Calls isValidName and isValidMultiword to check struct fields validity.
 * Return 1 on success and 0 on failure.
 *
 * @param id pointer to the item detail struct
 * @return 1 if valid, 0 if not.
 */
int isValidItemDetails(const struct ItemDetails *id) {

  struct ItemDetails itemCpy;
  memset(&itemCpy,0,sizeof (struct ItemDetails ));
  memcpy (&itemCpy, id, sizeof(struct ItemDetails));
  if(memcmp(id,&itemCpy,sizeof(struct ItemDetails))){
    return 0;
  }

  // 0 is false/invalid
  int checkID, checkName, checkMultiword;
  checkID = (itemCpy.itemID <= UINT64_MAX);
  checkName=isValidName(itemCpy.name);
  checkMultiword=isValidMultiword(itemCpy.desc);

  int result =(checkID && checkName && checkMultiword);
  memset(&itemCpy,0,sizeof (struct ItemDetails ));
  return result; 
}

/**
 * This function checks whether a Character struct is valid 
 * It is valid iff all of its fields are valid 
 *    the profession field must be a valid name field
 *    the name field must be a valid multi-word field
 *    the total number of items carried must not exceed MAX_ITEMS
 *    inventorySize must be less than or equal to MAX_ITEMS.
 * Returns 1 if the struct is valid, and 0 if not.
 * @param c Pointer to the Character struct to validate.
 * @return 1 if valid, 0 if not.
*/
int isValidCharacter(const struct Character * c) {
  struct Character charCpy;
  memset (&charCpy, 0, sizeof(struct Character));
  memcpy (&charCpy, c, sizeof(struct Character));
  if(memcmp(c,&charCpy,sizeof(struct ItemDetails))){
    return 0;
  }

  uint64_t totalItemCount = 0;

  // index in order: 0:ID, 1:SClass, 2:Proffession, 3:Name, 4:InvSize, 5:TotItmCount
  int checkLst[6];
  checkLst[0] = (charCpy.characterID <= UINT64_MAX);
  checkLst[1]= (charCpy.socialClass >= MENDICANT && charCpy.socialClass <= ARISTOCRACY);// check if within the enum range
  checkLst[2]=isValidName(charCpy.profession);
  checkLst[3]=isValidMultiword(charCpy.name);
  checkLst[4] = (charCpy.inventorySize <=MAX_ITEMS); // num of items carried by char

  // get the total. should not exceed MAX_ITEMS
  for (size_t i = 0; i<charCpy.inventorySize;i++){
    if((charCpy.inventory[i].quantity > MAX_ITEMS)){
      checkLst[5]=0;
      return 0;
    }
    totalItemCount += charCpy.inventory[i].quantity;
  }
  checkLst[5]=(totalItemCount<=MAX_ITEMS);

  
  //checks all fields in struct passed validation. Must be all 1 to pass
  for (int i = 0; i < 6; i++) {
    if (checkLst[i] != 1) {
        return 0; 
        break;
    }
  }

  memset (&charCpy, 0, sizeof(struct Character));
  return 1;
}


/**
 * This function validates the data, opens a file descriptor for writing,
 * and writes the data to the file. 
 * It is valid iff all character entries passes isValidCharacter() function
 * It returns 0 on success and 1 on failure.
 *
 * @param arr The array of Character to save.
 * @param nmemb The number of elements in the array.
 * @param fd The file descriptor to write to.
 * @return 0 on success, 1 on failure.
 */
int saveCharacters(struct Character *arr, size_t nmemb, int fd) {

  if (nmemb > UINT64_MAX || nmemb==0){
    return 1;
  }
  uint64_t charCount = (uint64_t) nmemb;

  struct Character charCpy[charCount];
  memset(charCpy,0,sizeof(struct Character )* charCount);
  memcpy(&charCpy,arr,sizeof(struct Character)*charCount);
  if(memcmp(arr,&charCpy,sizeof(struct Character)*nmemb)){
    return 1;
  }

  // Before open file validate everyting and catch error early
  for (uint64_t i = 0; i<charCount;i++){
    int charValid = isValidCharacter(&charCpy[i]);
    if (!charValid){
      return 1;
    }
  }

  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    return 1;
  }

  size_t bytesWritten=0;
  // Swapped sizeof and count to get bytes being written to file
  bytesWritten+=fwrite(&charCount,1,sizeof(uint64_t),fptr);
  bytesWritten+=fwrite(&charCpy,1,sizeof(charCpy),fptr);

  if (bytesWritten!= (sizeof(charCpy) +sizeof(charCount))){
      memset(&charCpy,0,sizeof(charCpy));
    return 1;
  }
  fflush(fptr);
  fclose(fptr);
  memset(&charCpy,0,sizeof(charCpy));
  return 0;
}

/**
 * This function reads Character from a file descriptor, validates the data
 * and stores it in the incomming pointer. 
 * It created and allocates space depending on the number of Characters to load
 * Each characters read must pass isValidCharacter() validation
 * It returns 0 on success and 1 on failure.
 *
 * @param ptr Pointer to store the loaded data.
 * @param nmemb Number of elements loaded.
 * @param fd File descriptor to read from.
 * @return 0 on success, 1 on failure.
 */
int loadCharacters(struct Character** ptr, size_t* nmemb, int fd) {
  FILE *fptr = fdopen(fd,"rb");
  if(fptr==NULL){
    return(1);
  }

  if(fread(nmemb, sizeof(uint64_t), 1, fptr)!=1){
    return 1;
  }

struct Character* newCharPtr = calloc(*nmemb, sizeof(struct Character));
    if (newCharPtr == NULL ) {
        free(newCharPtr);
        fclose(fptr);
        return 1;
    }

for (size_t i = 0; i < *nmemb; i++) {
  if(fread(&newCharPtr[i],sizeof(struct Character),1,fptr)!=1){
    free(newCharPtr);
    return 1;
  }
  if(!isValidCharacter(&newCharPtr[i])){
    free(newCharPtr);
    return 1;
  }
}
fclose(fptr);

if (*ptr != NULL) {
    free(*ptr);
}
  *ptr = newCharPtr;
return 0;
}


/**
 * This function attempt to acquire appropriate permissions for opening the ItemDetails database
 * and load the database from the specified file, and then call the function playgame() to which it should pass the loaded data and the number of items in the loaded data. 
 * It returns 0 on success, 1 on deserialization error, and 2 on permission or other errors.
 * 
 * @param filepath The path to the file to load data from.
 * @return 0 on success, 1 on deserialization error, 2 on permission or other errors.
*/
int secureLoad(const char *filepath) {
  struct stat filestats;
  int fstat = stat(filepath,&filestats);
  printf("%d\n",filestats.st_mode);
  // fail to open get stats or maybe not a file
  if (fstat !=0 || !S_ISREG(filestats.st_mode)){
    return 2;
  }
  gid_t originalEUID=geteuid();
  gid_t originalEGID=getegid();
 if (!(filestats.st_mode & S_IROTH)) {
    if (!(filestats.st_mode & S_IRGRP)) {
      printf("Group users cannot read\n");
      if (!(filestats.st_mode & S_IRUSR)) {
        printf("User users cannot read all reads are blocked\n");
        return 2;
      }else{
        printf("Yes owner User can read\n");

        if((originalEUID!= filestats.st_uid) && (seteuid(filestats.st_uid)!=0)){
          printf("user set fails\n");
          return 2;
        }
        printf("user set success\n");
      }
    }else{
      printf("Yes Group users can read set gid to getegid\n");
      if((originalEGID!= filestats.st_gid) && (setegid(filestats.st_gid)!=0)){
          printf("user set fails\n");
          return 2;
        }
        printf("group set success\n");
    }
  }

  printf("Escalated ruid:%d\teuid:%d\tgid:%d\tgeid:%d\n", getuid(),geteuid(),getgid(),getegid());
  size_t itemCount = 0;
  struct ItemDetails * loadedItems = NULL;

  // open file and immedialely drop privilege after opening. Opened file should still be readable.
  int filedesc = open(filepath, O_RDONLY);
  if(filedesc==-1){
    return 2;     
  }

  seteuid(originalEUID);
  setegid(originalEGID);

  int isloaded = loadItemDetails(&loadedItems, &itemCount, filedesc);
  if (isloaded==1){
    return 1;
  }
  close(filedesc);

  playGame(loadedItems,itemCount);


  printf("Close ruid:%d\teuid:%d\tgid:%d\tgeid:%d\n", getuid(),geteuid(),getgid(),getegid());
  free(loadedItems);
  return 0;
}



void playGame(struct ItemDetails* ptr, size_t nmemb){
  printf("success %d \t%li\n", *ptr[1].name, nmemb);

}

/**
 * For Testing purposes only.
 * This function saves an array of ItemDetails to a binary file. It first opens
 * the file, then calls saveItemDetails to perform the actual data saving. After
 * the data is saved, the file is closed.
 *
 * @param arr Pointer to the array of ItemDetails to be saved.
 * @param nmemb Number of elements in the array.
 * @param filename Path to the binary file where data will be saved.
 *
 * @return 0 on success, non-zero on failure.
 */
int saveItemDetailsToPath(const struct ItemDetails* arr, size_t nmemb, const char* filename) {

  FILE *ofp = fopen(filename, "wb");
  assert(ofp != NULL);
  int fd = fileno(ofp);
  assert(fd != -1);
  int res = saveItemDetails(arr, nmemb, fd);
  fclose(ofp);
  return res;
}



// ============MAIN=============================================MAIN============================================================MAIN===============
int main(){
  int fd;
  int res;
  FILE *ofp;

// ------------------------------------------   P1 - saveItemDetails()   -----------------------------------------
  struct ItemDetails itemArr[] = {{ .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." }};
  size_t itemArr_size = sizeof(itemArr)/sizeof(struct ItemDetails);
  const char *fpath = "submit_result.dat";
  res = saveItemDetailsToPath(itemArr,itemArr_size,fpath);
  assert(res == 0); //success is 0
  printf("P1\tSave item success\n");

  // ------------------------------------------   P2 - loadItemDetails()   -----------------------------------------
  const char * infile_path = "items001.dat";
  fd = open(infile_path, O_RDONLY);
  size_t numItems = 0;
  struct ItemDetails * itemsArr = NULL;
  res = loadItemDetails(&itemsArr, &numItems, fd);
  close (fd);
  free(itemsArr);
  assert(res == 0); //success is 0
  printf("P2\tLoad item Details test1 success\n");

  //test 2

 struct ItemDetails items001_expectedItems_test2[] = {
    { .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." },
    { .itemID = 13744653742375254000UL, .name = "rope",           .desc = "hemp, 50 ft." },
    { .itemID = 3400701144194139000UL,  .name = "music-box",      .desc = "brass gears with tin-plated case, 6 in., cursed" },
    { .itemID = 734628920708863200UL,   .name = "billiard-ball",  .desc = "ivory, 2 in., set of 16 in wooden case, of mysterious origin" },
    { .itemID = 14734865628490349000UL, .name = "sword-cane",     .desc = "steel-bladed, concealed in Malacca-wood walking-cane, 36 in." },
    { .itemID = 14324391740292973000UL, .name = "dynamite-stick", .desc = "with paper wrapping, 1 in. diameter x 12 in." },
    { .itemID = 7562791295545618000UL,  .name = "Epsom-salts",    .desc = "6 oz, in glass bottle with cork stopper" },
    { .itemID = 13658877949582529000UL, .name = "camp-stool",     .desc = "canvas and wood, 12 in. seat height" },
    { .itemID = 2390949174291477500UL,  .name = "slide-rule",     .desc = "wood and brass, 12 in., cursed" },
  };

  size_t items001_expectedSize_test2 = sizeof(items001_expectedItems_test2)/sizeof(struct ItemDetails);
  for(size_t i = 0; i < items001_expectedSize_test2; i++) {

    ofp = fopen(infile_path, "rb");
    fd = fileno(ofp);
    numItems = 0;
    itemsArr = NULL;
    res = loadItemDetails(&itemsArr, &numItems, fd);
    // pre-requisite: we got the expected number of items
    assert(numItems == items001_expectedSize_test2);
    free(itemsArr);
}
  printf("P2\tLoad Item Details test2 success\n");


// ------------------------------------------   P3 - isValidName()   -----------------------------------------
  int validName = isValidName(items001_expectedItems_test2[3].name);
  assert(validName==1); //success is 1
  printf("P3\tisValidName checker success\n");

// ------------------------------------------   P4 - isValidMultiword()   -----------------------------------------

  int validMultiword = isValidMultiword(items001_expectedItems_test2[4].desc);
  assert(validMultiword==1);//success is 1
  printf("P4\tisvalidMultiword checker success\n");

// ------------------------------------------   P5 - isValidItemDetail()   -----------------------------------------
  int validItemDeets = isValidItemDetails(&itemArr[0]);
  assert(validItemDeets==1);//success is 1
  printf("P5\tisValidItemDetails checker success\n");

// ------------------------------------------   P6 - isValidCharacter()   -----------------------------------------
  const struct Character sample_character = {
    .characterID = 1,
    .socialClass = MERCHANT,
    .profession = "inn-keeper",
    .name = "Edgar Crawford",
    .inventorySize = 1,
    .inventory = {
      { .itemID = 200648657395984580,
        .quantity = 1
      }
    }
  };


  int validChar = isValidCharacter(&sample_character);
  assert(validChar==1);//success is 1
  printf("P6\tisValidCharacter checker success\n");

// ------------------------------------------   P7 - saveCharacters()   -----------------------------------------

struct Character arr[] = {
  {
    .characterID = 1,
    .socialClass = MERCHANT,
    .profession = "inn-keeper",
    .name = "Edgar Crawford",
    .inventorySize = 1,
    .inventory = {
      { .itemID = 200648657395984580,
        .quantity = 1
      }
    }
  },
  {
    .characterID = 2,
    .socialClass = MENDICANT,
    .profession = "beggar",
    .name = "Sarah Smith",
    .inventorySize = 2,
    .inventory = {
      { .itemID = 123456789,
        .quantity = 3
      },
      { .itemID = 987654321,
        .quantity = 2
      }
    }
  },
  {
    .characterID = 3,
    .socialClass = ARISTOCRACY,
    .profession = "noble",
    .name = "Lord William",
    .inventorySize = 3, // No inventory
    .inventory = {
      { .itemID = 111222333,
        .quantity = 5
      },
      { .itemID = 444555666,
        .quantity = 1
      },
      { .itemID = 777888999,
        .quantity = 4
      }
    }
  },
  {
    .characterID = 4,
    .socialClass = LABOURER,
    .profession = "blacksmith",
    .name = "John Johnson",
    .inventorySize = 3,
    .inventory = {
      { .itemID = 111222333,
        .quantity = 5
      },
      { .itemID = 444555666,
        .quantity = 1
      },
      { .itemID = 777888999,
        .quantity = 4
      }
    }
  },
  {
    .characterID = 5,
    .socialClass = GENTRY,
    .profession = "gentleman",
    .name = "Henry Smithson",
    .inventorySize = 2,
    .inventory = {
      { .itemID = 987654321,
        .quantity = 2
      },
      { .itemID = 222333444,
        .quantity = 1
      }
    }
  }
};


  size_t charArr_size = sizeof(arr)/sizeof(struct Character);

  FILE *ofps = fopen("savecharacter.dat", "wb");
  assert(ofp != NULL);
  int fdc = fileno(ofps);
  assert(fdc != -1);
  int actual_result = saveCharacters(arr, charArr_size, fdc);
  fclose(ofps);

  assert(actual_result == 0);
  printf("P7 \tsaveCharacters checker success \n");

// ------------------------------------------   P8 - loadCharacterDetails()   -----------------------------------------
  const char * savedchar = "characters02.dat";
  fd = open(savedchar, O_RDONLY);
  size_t numchars = 0;
  struct Character * charactersArray = NULL;

  res = loadCharacters(&charactersArray, &numchars, fd);
  close (fd);

  assert(actual_result == 0);
  printf("P8\tloadCharacters checker success\n");

  free(charactersArray);

// ------------------------------------------   P9 - secureLoad()   -----------------------------------------

  char *filepth = "items001.dat";
  res = secureLoad(filepth);
  assert(res==0);
  printf("P8\tsecureLoad checker success\n");
  
  return 0;
}
