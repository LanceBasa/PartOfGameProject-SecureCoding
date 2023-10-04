#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
static_assert (sizeof(size_t)==8, "we assume that its running on 64bit");
#define DEFAULT_BUFFER_SIZE 512
#define MAX_ITEMS 10


struct ItemDetails {
  uint64_t itemID;
  char name[DEFAULT_BUFFER_SIZE];
  char desc[DEFAULT_BUFFER_SIZE];
};

/**
 * Used to record how many items of one particular
 * type are carried by a player character.
 */
struct ItemCarried {
  uint64_t itemID;
  size_t quantity;
};

/**
 * Used to record the social class of a character.
 */
enum CharacterSocialClass {
  MENDICANT,
  LABOURER,
  MERCHANT,
  GENTRY,
  ARISTOCRACY
};

/**
 * Records the base details of a player character.
 * (Other structs are used to record additional details
 * required for the game.)
 *
 * The 'profession' and 'name' fields must each contain a valid, NUL-terminated
 * string of length at most DEFAULT_BUFFER_SIZE-1.
 *
 * The sum of all 'quantity' fields in the character's *inventory*
 * must not exceed MAX_ITEMS. (And since that means a character
 * can carry at most MAX_ITEMS distinct types of item, the length of the inventory
 * array is capped at that length.)
 *
 * The number of array elements currently containing valid
 * ItemCarried records is stored in the 'inventorySize' field.
 *
 * In all other respects, a Character struct and should comply with the
 * requirements laid out in the project specification for valid Character
 * structs.
 */
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


    for (size_t i = 0; i<*nmemb;i++){
        reading = fread(&(*ptr)[i].itemID,sizeof(uint64_t),1,fptr);
        if (reading != 1){
            perror("reading itemID failed");
            free(*ptr);

            return 1;
        }   

        reading = fread((*ptr)[i].name,sizeof(char),DEFAULT_BUFFER_SIZE,fptr);
        if (reading != DEFAULT_BUFFER_SIZE){
            
            perror("reading itemID failed");
            free(*ptr);
            return 1;
            }
        (*ptr)[i].name[DEFAULT_BUFFER_SIZE - 1] = '\0';


        reading = fread((*ptr)[i].desc, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
        if (reading != DEFAULT_BUFFER_SIZE){
            perror("reading itemID failed");
            free(*ptr);
            return 1;
        }
        (*ptr)[i].desc[DEFAULT_BUFFER_SIZE - 1] = '\0';
    }

//   for (size_t i = 0; i < *nmemb; i++) {
//       printf("%ln \t %s \t %s \n", &((*ptr)[i].itemID), (*ptr)[i].name, (*ptr)[i].desc);
//   }

  fclose(fptr);

  return 0;
}




int main(){
     const char * infile_path = "items001.dat";
    int fd;
    fd = open(infile_path, O_RDONLY); // Use O_BINARY for binary files on some systems

    // printf("%d\n",fd);

    size_t numItems = 0;
    struct ItemDetails * itemsArr = NULL;
    int res = loadItemDetails(&itemsArr, &numItems, fd);

        close (fd);

    return res;
}