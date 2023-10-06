#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
static_assert (sizeof(size_t)==8, "we assume that its running on 64bit");
#define DEFAULT_BUFFER_SIZE 512
#define MAX_ITEMS 10


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



int saveItemDetails(const struct ItemDetails* arr, size_t nmemb, int fd) {
  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    perror("unable to open file");
    return 1;
  }

  fwrite(&nmemb,sizeof(size_t),1,fptr);     // header specified in the project-spec

  // Assuming that the items are already sanitized, loop each ItemDetails in the array and write each one. 
  for (size_t i = 0; i<nmemb;i++){
    fwrite(&arr[i].itemID,sizeof(uint64_t),1,fptr);
    fwrite(arr[i].name,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
    fwrite(arr[i].desc,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
  }

  fflush(fptr);
  fclose(fptr);
  return 0;
}


int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {
    FILE* fptr = fdopen(fd, "rb");
    if (fptr == NULL) {
        perror("unable to open file");
        return 1;
    }

    size_t numRead;
    size_t reading = fread(&numRead, sizeof(uint64_t), 1, fptr);
    if (reading != 1) {
        perror("reading heading failed");
        fclose(fptr);
        return 1;
    }
    *nmemb = numRead;

    // Create a new struct ItemDetails pointer
    struct ItemDetails* newPtr = (struct ItemDetails *)malloc(numRead * sizeof(struct ItemDetails));
    if (newPtr == NULL) {
        perror("malloc failed");
        fclose(fptr);
        return 1;
    }

    for (size_t i = 0; i < *nmemb; i++) {
        reading = fread(&newPtr[i].itemID, sizeof(uint64_t), 1, fptr);
        if (reading != 1) {
            perror("reading itemID failed");
            free(newPtr); // Free the newly allocated memory
            fclose(fptr);
            return 1;
        }

        reading = fread(newPtr[i].name, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
        if (reading != DEFAULT_BUFFER_SIZE) {
            perror("reading name failed");
            free(newPtr); // Free the newly allocated memory
            fclose(fptr);
            return 1;
        }
        newPtr[i].name[DEFAULT_BUFFER_SIZE - 1] = '\0';

        reading = fread(newPtr[i].desc, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
        if (reading != DEFAULT_BUFFER_SIZE) {
            perror("reading itemID failed");
            free(newPtr); // Free the newly allocated memory
            fclose(fptr);
            return 1;
        }
        newPtr[i].desc[DEFAULT_BUFFER_SIZE - 1] = '\0';
    }
    fclose(fptr);

    //Free the memory pointed to by the incoming pointer, if it's not NULL
    if (*ptr != NULL) {
        free(*ptr);
    }

    // Set the incoming pointer to the new pointer
    *ptr = newPtr;
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
    return 1; //success/valid
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
    return 1; //success/valid
  }
  return 0;
}


/**
 * checks whether an ItemDetails struct is valid – it is valid iff all of its fields are valid (as
described in the documentation for the struct and elsewhere in this project specification).
The name and desc fields must be valid name and multi-word fields, respectively; they also
must not be empty strings. This function returns 1 if the struct is valid, and 0 if not.
*/
// id name and multiwword
int isValidItemDetails(const struct ItemDetails *id) {

  struct ItemDetails itemCpy;
  memset(&itemCpy,0,sizeof (struct ItemDetails ));
  memcpy (&itemCpy, id, sizeof(struct ItemDetails));

  // 0 is false/invalid
  int checkID, checkName, checkMultiword;
  checkID = (itemCpy.itemID <= UINT64_MAX);
  checkName=isValidName(itemCpy.name);
  checkMultiword=isValidMultiword(itemCpy.desc);

  //printf("%i\t%i\t%i\n",checkID,checkName,checkMultiword);
  int result =(checkID && checkName && checkMultiword);

  return result; 
}

/**
 * checks whether a Character struct is valid – it is valid iff all of its fields are valid (as
described in the documentation for the struct and elsewhere in this project specification).
The following are all necessary preconditions for validity of the struct:
• the profession field must be a valid name field, and must not be the empty string;
• the name field must be a valid multi-word field, and must not be the empty string;
• the total number of items carried (that is: the sum of the quantity fields of the
ItemCarried structs that form part of the inventory) must not exceed MAX_ITEMS; and
• inventorySize must be less than or equal to MAX_ITEMS.
This function returns 1 if the struct is valid, and 0 if not.
*/
int isValidCharacter(const struct Character * c) {
  struct Character charCpy;
  memset (&charCpy, 0, sizeof(struct Character));
  memcpy (&charCpy, c, sizeof(struct Character));
  //need to check if memset success?

  // index in order: 0:ID, 1:SClass, 2:Proffession, 3:Name, 4:InvSize, 5:TotItmCount;
  int checkLst[6];
  if (memset(&checkLst, 0, sizeof(checkLst)) == NULL) {
    perror("memset failed on isValidChar");
    return 0;
  };

  checkLst[0] = (charCpy.characterID <= UINT64_MAX);
  checkLst[1]= (charCpy.socialClass >= MENDICANT && charCpy.socialClass <= ARISTOCRACY);// check if within the enum range
  checkLst[2]=isValidName(charCpy.profession);
  checkLst[3]=isValidMultiword(charCpy.name);
  checkLst[4] = (charCpy.inventorySize <=MAX_ITEMS); // num of items carried by char
  checkLst[5]= 1;
  size_t total = 0;

  // get the total. should not exceed MAX_ITEMS
  for (size_t i = 0; i<charCpy.inventorySize;i++){

    if(!(charCpy.inventory[i].itemID <= UINT64_MAX)){
      checkLst[5]=0;
      return 0;
    }
    total += charCpy.inventory[i].quantity;
  };
  checkLst[5]=(total<=MAX_ITEMS);
  
  //checks all fields in struct passed validation.
  for (int i = 0; i < 6; i++) {

    if (checkLst[i] != 1) {
        return 0; 
        break;
    }
  }
  return 1;
}


/**
 * must use validations  to validate records before saving
 * Header  - uint64 specifying the num of characters in saved file
 * Each char records have the following
 *  char ID - uint64
 *  socialclass - enum
 *  profession - namefield
 *  name - multiword field
 *  inventory size - uint64 number of unique items
 *  inventoryitem - inventory size * itemCarried(has itemid and quantity)
 * */ 
int saveCharacters(struct Character *arr, size_t nmemb, int fd) {

  if (nmemb > UINT64_MAX || nmemb==0){
    perror("incomming size is either emmpty or too much\n");  
    return 1;
  }
  uint64_t charCount = (uint64_t) nmemb;

  struct Character charCpy[charCount];
  memset(charCpy,0,sizeof(struct Character )* charCount);
  memcpy(&charCpy,arr,sizeof(struct Character)*charCount);
  //check if memset and cpy success

  for (uint64_t i = 0; i<charCount;i++){
    int charValid = isValidCharacter(&charCpy[i]);
    if (!charValid){
      printf("Invalid character number %lu\n", i+1);
      return 1;
    }
  }

  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    return 1;
  }
  //validated. just dump the whole struct
  fwrite(&charCount,sizeof(uint64_t),1,fptr);
  fwrite(&charCpy,sizeof(charCpy),1,fptr);
  
  fflush(fptr);
  fclose(fptr);

  return 0;
}

// must use validations to validate records before loading
// int loadCharacters(struct Character** ptr, size_t* nmemb, int fd) {
//   FILE *fptr = fdopen(fd,"rb");
//   if(fptr==NULL){
//     perror("unable to open file");
//     return(1);
//   }

//   size_t numRead;
//   size_t reading = fread(&numRead, sizeof(uint64_t), 1, fptr);
//   if (reading != 1){
//       perror("reading heading failed");
//       return 1;
//   }
//   *nmemb = numRead;


//   return 0;
// }



// ============MAIN=============================================MAIN============================================================MAIN===============
int main(){
  int fd;
  int res;

// ------------------------------------------   P1 - saveItemDetails()   -----------------------------------------
  struct ItemDetails itemArr[] = {{ .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." }};
  size_t itemArr_size = sizeof(itemArr)/sizeof(struct ItemDetails);

  FILE *ofp = fopen("submit_result.dat", "wb");
  assert(ofp != NULL);
  fd = fileno(ofp);
  assert(fd != -1);
  res = saveItemDetails(itemArr, itemArr_size, fd);
  assert(res == 0);
  fclose(ofp);


  // ------------------------------------------   P2 - loadItemDetails()   -----------------------------------------
  const char * infile_path = "items001.dat";
  fd = open(infile_path, O_RDONLY);
  size_t numItems = 0;
  struct ItemDetails * itemsArr = NULL;
  res = loadItemDetails(&itemsArr, &numItems, fd);
  close (fd);

  // for (size_t i = 0;i<numItems;i++){
  //     printf("%lu \t %s\t %s \n", itemsArr[i].itemID, itemsArr[i].name, itemsArr[i].desc);
  // }
  printf("loadItemDetails return ( 0 is success):\t %i\n", res);

  free(itemsArr);

  


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

    //fprintf(stderr, "checking equality of item %d, with expected itemID %zu\n",
    //        i, items001_expectedItems_test2[i].itemID
    //);
    // int result = memcmp(&(itemsArr[i]), &(items001_expectedItems_test2[i]), sizeof(struct ItemDetails));
    // printf("loaditem return ( 0 is success):\t %i\n", result);
    free(itemsArr);

}


  printf("loadItemDetails2 return ( 0 is success):\t %i\n", res);





// ------------------------------------------   P3 - isValidName()   -----------------------------------------
  int validName = isValidName("asdasd");
  printf("valid name return ( 0 is fail):\t %i\n", validName);

// ------------------------------------------   P4 - isValidMultiword()   -----------------------------------------

  int validMultiword = isValidMultiword("asdad");
  printf("valid Multiword return ( 0 is fail): %i\n", validMultiword);

// ------------------------------------------   P5 - isValidItemDetail()   -----------------------------------------
  int validItemDeets = isValidItemDetails(&itemArr[0]);
  printf("valid Item return ( 0 is fail): %i\n", validItemDeets);

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
   printf("valid char return ( 0 is fail): %i\n", validChar);


// ------------------------------------------   P7 - saveCharacters()   -----------------------------------------
  struct Character arr[] = { {
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
  } };

  size_t charArr_size = sizeof(arr)/sizeof(struct Character);

  FILE *ofps = fopen("savecharacter.dat", "wb");
  assert(ofp != NULL);
  int fdc = fileno(ofps);
  assert(fdc != -1);
  int actual_result = saveCharacters(arr, charArr_size, fdc);
  assert(actual_result == 0);
  printf("saveChar ( 0 is success): %i\n", actual_result);

  fclose(ofps);
  return 0;
}
