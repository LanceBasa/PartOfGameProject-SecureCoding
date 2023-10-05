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
int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {
    FILE *fptr = fdopen(fd,"rb");
    if(fptr==NULL){
      perror("unable to open file");
      return(1);
    }


    size_t numRead;
    size_t reading = fread(&numRead, sizeof(uint64_t), 1, fptr);
    if (reading != 1){
        perror("reading heading failed");
        return 1;
    }
    *nmemb = numRead;

  
    *ptr = calloc(*nmemb, sizeof(struct ItemDetails));
    if (*ptr == NULL) {
        perror("Calloc failed");
        fclose(fptr);
        return 1;
    }
    // else{
    //     printf("allocated %ld number of bytes\n", ((*nmemb) * sizeof(struct ItemDetails)));
    // }


    for (size_t i = 0; i<*nmemb;i++){
        reading = fread(&(*ptr)[i].itemID,sizeof(uint64_t),1,fptr);
        if (reading != 1){
            perror("reading itemID failed");
            free(*ptr);
            return 1;
        }  
        //printf("%ld \t\t", ((uint64_t)&(*ptr)[i].itemID));

        reading = fread((*ptr)[i].name,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
        if (reading != DEFAULT_BUFFER_SIZE){
            perror("reading name failed");
            free(*ptr);
            return 1;
            }
        (*ptr)[i].name[DEFAULT_BUFFER_SIZE - 1] = '\0';
       // printf("%s \t\t", (*ptr)[i].name);



        reading = fread((*ptr)[i].desc, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
        if (reading != DEFAULT_BUFFER_SIZE){
            perror("reading itemID failed");
            free(*ptr);
            return 1;
        }
        (*ptr)[i].desc[DEFAULT_BUFFER_SIZE - 1] = '\0';
        //printf("%s \n", (*ptr)[i].desc);

    }

//   for (size_t i = 0; i < *nmemb; i++) {
//       printf("%ln \t %s \t %s \n", &((*ptr)[i].itemID), (*ptr)[i].name, (*ptr)[i].desc);
//   }

  fclose(fptr);

  return 0;
}


int saveItemDetails(const struct ItemDetails* arr, size_t nmemb, int fd) {
  FILE *fptr = fdopen(fd,"wb");
  if(fptr==NULL){
    perror("unable to open file");
    return(1);
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
  checkID = (itemCpy.itemID >= 0 && itemCpy.itemID <= UINT64_MAX);
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

  checkLst[0] = (charCpy.characterID >= 0 && charCpy.characterID <= UINT64_MAX);
  checkLst[1]= (charCpy.socialClass >= MENDICANT && charCpy.socialClass <= ARISTOCRACY);// check if within the enum range
  checkLst[2]=isValidName(charCpy.profession);
  checkLst[3]=isValidMultiword(charCpy.name);
  checkLst[4] = (charCpy.inventorySize >= 0 && charCpy.inventorySize <=MAX_ITEMS); // num of items carried by char
  checkLst[5]= 1;
  size_t total = 0;

  // get the total. should not exceed MAX_ITEMS
  for (size_t i = 0; i<charCpy.inventorySize;i++){

    if( !(charCpy.inventory[i].itemID >= 0 && charCpy.inventory[i].itemID <= UINT64_MAX)){
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






// ============MAIN=============================================MAIN============================================================MAIN===============
int main(){
  int fd;
  int res;

// ------------------------------------------   P1 - saveItemDetails()   -----------------------------------------
  struct ItemDetails itemArr[] = {{ .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." }};
  size_t itemArr_size = sizeof(itemArr)/sizeof(struct ItemDetails);

  char* file_conts = NULL;
  size_t file_size = 0;
  FILE *ofp = fopen("submit_result.dat", "wb");
  assert(ofp != NULL);
  fd = fileno(ofp);
  assert(fd != -1);
  res = saveItemDetails(itemArr, itemArr_size, fd);
  assert(res == 0);
  fclose(ofp);

  printf("Save Item result%ld\t%s\n", file_size, file_conts);

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
  printf("loadItemDetails return ( 0 is fail):\t %i\n", res);

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



  return 1;
}