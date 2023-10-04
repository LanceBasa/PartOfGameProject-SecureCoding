#include <p_and_p.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <stddef.h>

static_assert (sizeof(size_t)==8, "we assume that its running on 64bit");

/**
 * serializes an array of ItemDetails structs. It should store the array using the itemDetails file format.
*/
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


int saveItemDetailsToPath(const struct ItemDetails* arr, size_t nmemb, const char* filename) {
  return 0;
}


/*
 * It takes as argument the address of a pointer-to-ItemDetails struct, and the address of a
  size_t, which on successful deserialization will be written to by the function, and a file
  descriptor for the file being deserialized.
  If deserialization is successful, the function will:
  • allocate enough memory to store the number of records contained in the file, and
  write the address of that memory to ptr. The memory is to be freed by the caller.
  • write all records contained in the file into the allocated memory.
  • write the number of records to nmemb.
  If an error occurs in the serialization process, the function should return a 1, and no net
  memory should be allocated (that is – any allocated memory should be freed). Otherwise,
  the function should return 0.'

  header is number of items (64 uint)

*/
int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {

 
  FILE *fptr = fdopen(fd,"rb");
  if(fptr==NULL){
    perror("unable to open file");
    return(1);
  }

  size_t reading = fread(nmemb, sizeof(uint64_t),1,fptr);  // pass in the size of the items
  if (reading != 1){
    perror("reading heading failed");
    return 1;
  }

  // access the outer pointer
  *ptr=malloc(sizeof(struct ItemDetails)*(*nmemb));
  if (*ptr==NULL){
    perror("Malloc failed");
    fclose(fptr);
    return 1;
  }


  for (size_t i = 0; i<*nmemb;i++){
    memset(&(*ptr)[i],0,sizeof(struct ItemDetails));
    for (size_t i = 0; i < sizeof(struct ItemDetails); i++) {
        if ((char *)ptr[i] != 0) {
            return 0; // Failed to zero memory
        }
    }

    reading = fread(nmemb, sizeof(uint64_t),1,fptr);  // pass in the size of the items

    reading = fread(&(*ptr)[i].itemID,sizeof(uint64_t),1,fptr);
    if (reading != 1){
      perror("reading itemID failed");
      return 1;
    }

    reading = fread((*ptr)[i].name,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
    if (reading != DEFAULT_BUFFER_SIZE){
      perror("reading itemID failed");
      return 1;
    }

    reading = fread((*ptr[i]).desc,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
    if (reading != DEFAULT_BUFFER_SIZE){
      perror("reading itemID failed");
      return 1;
    }
  }

  for (int i = 0; i < *nmemb; i++) {
      printf("%llu \t %s \t %s \n", &((*ptr)[i].itemID), (*ptr)[i].name, (*ptr)[i].desc);
  }

  fclose(fptr);

  return 0;
}

 


/**
 * checks whether a string constitutes a valid name field. It returns 1 if so, and 0 if not.
*/
int isValidName(const char *str) {
  return 0;
}


/**
 * checks whether a string constitutes a valid name field. It returns 1 if so, and 0 if not.
*/
int isValidMultiword(const char *str) {
  return 0;
}


/**
 * checks whether an ItemDetails struct is valid – it is valid iff all of its fields are valid (as
described in the documentation for the struct and elsewhere in this project specification).
The name and desc fields must be valid name and multi-word fields, respectively; they also
must not be empty strings. This function returns 1 if the struct is valid, and 0 if not.
*/
int isValidItemDetails(const struct ItemDetails *id) {
  return 0;
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
  struct Character copy;

  memcpy (&copy, c, sizeof(struct Character));
  return 0;
}

// must use validations  to validate records before saving
int saveCharacters(struct Character *arr, size_t nmemb, int fd) {
  return 0;
}

// must use validations to validate records before loading
int loadCharacters(struct Character** ptr, size_t* nmemb, int fd) {
  return 0;
}


/**
 * When the executable starts running, it does the following things (which you need not implement):
• Temporarily drops privileges, then loads and saves files owned by the user who invoked
the executable.
• Calls a function secureLoad, implemented by you; this acquires appropriate permissions,
loads the ItemDetails database, and then permanently drops permissions.
It should attempt to acquire appropriate permissions for opening the ItemDetails database (that
is: the effective userID should be set to the userID of pitchpoltadmin), should load the database
from the specified file, and then (after permanently dropping privileges), call the function playgame()
to which it should pass the loaded data and the number of items in the loaded data. If an error
occurs during the deserialization process, the function should return 1. It should check the
running process’s permissions to ensure that the executable it was launched from was indeed a
setUID executable owned by user pitchpoltadmin. If that is not the case, or if an error occurs
in acquiring or dropping permissions, the function should return 2. In all other cases, it should
return 0.
It should follow all best practices for a setUID program.

*/
int secureLoad(const char *filepath) {
  return 0;
}

void playGame(struct ItemDetails* ptr, size_t nmemb);

