#define _POSIX_C_SOURCE 200809L//strnlen

#define _POSIX_C_SOURCE 200809L // strnlen

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
#include <pwd.h>

static_assert (sizeof(size_t)==8,"using 64-bit");

/**
 * A function that validates the data, opens the incomming file descriptor for
 * writing and writes the data to the file. It makes a copy of ItemDetails array
 * and check the copy if it is valid by calling other isValidItemDetails. It
 * returns 0 on success and 1 on failure.
 *
 * @param arr The array of ItemDetails to save.
 * @param nmemb The number of elements in the array.
 * @param fd The file descriptor to write to.
 * @return 0 on success, 1 on failure.
 */
int saveItemDetails(const struct ItemDetails *arr, size_t nmemb, int fd) {
  if (nmemb > UINT64_MAX || nmemb == 0) {
    return 1;
  }
  uint64_t itemCount = (uint64_t)nmemb;

  struct ItemDetails itemCpy[itemCount];
  if (memset(&itemCpy, 0, sizeof(struct ItemDetails) * itemCount) == NULL) {
    return 1;
  }
  memcpy(&itemCpy, arr, sizeof(struct ItemDetails) * itemCount);
  if (memcmp(arr, &itemCpy, sizeof(struct ItemDetails) * nmemb)) {
    return 1;
  }

  // Before open file validate everyting and catch error early
  for (uint64_t i = 0; i < itemCount; i++) {
    int itemValid = isValidItemDetails(&itemCpy[i]);
    if (!itemValid) {
      return 1;
    }
  }

  FILE *fptr = fdopen(fd, "wb");
  if (fptr == NULL) {
    return 1;
  }

  size_t bytesWritten = 0;
  // Swapped sizeof and count to get bytes being written to file
  bytesWritten += fwrite(&itemCount, 1, sizeof(uint64_t), fptr);
  bytesWritten += fwrite(&itemCpy, 1, sizeof(itemCpy), fptr);
  // something went wrong during save, not all bytes written
  if (bytesWritten != (sizeof(itemCpy) + sizeof(itemCount))) {
    memset(&itemCpy, 0, sizeof(struct ItemDetails) * itemCount);
    return 1;
  }
  fflush(fptr);
  fclose(fptr);
  memset(&itemCpy, 0, sizeof(itemCpy));

  return 0;
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
int saveItemDetailsToPath(const struct ItemDetails *arr, size_t nmemb,
                          const char *filename) {

  FILE *ofp = fopen(filename, "wb");
  assert(ofp != NULL);
  int fd = fileno(ofp);
  assert(fd != -1);
  int res = saveItemDetails(arr, nmemb, fd);
  fclose(ofp);
  return res;
}

/**
 * This function reads ItemDetails from a file descriptor, validates the data
 * and stores it in the provided pointer. It checks each filed in the struct
 * if they pass valid functions. After passing, only copy the string up to
 * null byte terminator and sanitizes bytes after the null bye.
 * It returns 0 on success and 1 on failure.
 *
 * @param ptr Pointer to store the loaded data.
 * @param nmemb Number of elements loaded.
 * @param fd File descriptor to read from.
 * @return 0 on success, 1 on failure.
 */
int loadItemDetails(struct ItemDetails **ptr, size_t *nmemb, int fd) {
  FILE *fptr = fdopen(fd, "rb");
  if (fptr == NULL) {
    fclose(fptr);
    return 1;
  }

  if (fread(nmemb, sizeof(uint64_t), 1, fptr) != 1) {
    fclose(fptr);
    return 1;
  }

  // Create a new struct ItemDetails pointer and allocate space
  struct ItemDetails *newItmPtr = calloc(*nmemb, sizeof(struct ItemDetails));
  if (newItmPtr == NULL) {
    fclose(fptr);
    return 1;
  }
  // Load and validate each ItemDetails
  for (size_t i = 0; i < *nmemb; i++) {
    if (fread(&newItmPtr[i], sizeof(struct ItemDetails), 1, fptr) != 1) {
      free(newItmPtr);
      fclose(fptr);
      return 1;
    }
    if (!isValidItemDetails(&(newItmPtr[i]))) {
      free(newItmPtr);
      fclose(fptr);

      return 1;
    }

    // only copy the validname(up to null byte) to the pointer and padd it with
    // null bytes
    char tempBuffer[DEFAULT_BUFFER_SIZE];
    if (strncpy(tempBuffer, newItmPtr[i].name, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newItmPtr);
      fclose(fptr);
      return 1;
    }
    if (strncpy(newItmPtr[i].name, tempBuffer, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newItmPtr);
      fclose(fptr);
      return 1;
    }

    // similar to name, only copy up to null byte if isValid
    if (strncpy(tempBuffer, newItmPtr[i].desc, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newItmPtr);
      fclose(fptr);
      return 1;
    }
    if (strncpy(newItmPtr[i].desc, tempBuffer, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newItmPtr);
      fclose(fptr);
      return 1;
    }
    memset(tempBuffer, 0, DEFAULT_BUFFER_SIZE);
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
  if (str == NULL) {
    return 0;
  }

  size_t nameLength = strnlen(str, DEFAULT_BUFFER_SIZE);
  if (nameLength == 0) {
    return 0;
  }

  if (nameLength < DEFAULT_BUFFER_SIZE) {
    for (size_t i = 0; i < nameLength; i++) {
      if (isgraph(str[i]) == 0) {
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
  if (str == NULL) {
    return 0;
  }
  size_t nameLength = strnlen(str, DEFAULT_BUFFER_SIZE);
  if (nameLength == 0) {
    return 0;
  }
  if (nameLength < DEFAULT_BUFFER_SIZE) {
    if (str[0] == ' ' || str[nameLength - 1] == ' ') {
      return 0;
    };
    for (size_t i = 0; i < nameLength; i++) {
      if (!isgraph(str[i]) && str[i] != ' ') {
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
  memset(&itemCpy, 0, sizeof(struct ItemDetails));
  memcpy(&itemCpy, id, sizeof(struct ItemDetails));
  if (memcmp(id, &itemCpy, sizeof(struct ItemDetails))) {
    return 0;
  }

  // 0 is false/invalid
  int checkID, checkName, checkMultiword;
  checkID = (itemCpy.itemID <= UINT64_MAX);
  checkName = isValidName(itemCpy.name);
  checkMultiword = isValidMultiword(itemCpy.desc);

  int result = (checkID && checkName && checkMultiword);
  memset(&itemCpy, 0, sizeof(struct ItemDetails));
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
int isValidCharacter(const struct Character *c) {
  struct Character charCpy;
  memset(&charCpy, 0, sizeof(struct Character));
  memcpy(&charCpy, c, sizeof(struct Character));
  if (memcmp(c, &charCpy, sizeof(struct ItemDetails))) {
    return 0;
  }

  uint64_t totalItemCount = 0;

  // index in order: 0:ID, 1:SClass, 2:Proffession, 3:Name, 4:InvSize,
  // 5:TotItmCount
  int checkLst[6];
  checkLst[0] = (charCpy.characterID <= UINT64_MAX);
  checkLst[1] =
      (charCpy.socialClass >= MENDICANT &&
       charCpy.socialClass <= ARISTOCRACY); // check if within the enum range
  checkLst[2] = isValidName(charCpy.profession);
  checkLst[3] = isValidMultiword(charCpy.name);
  checkLst[4] =
      (charCpy.inventorySize <= MAX_ITEMS); // num of items carried by char

  // get the total. should not exceed MAX_ITEMS
  for (size_t i = 0; i < charCpy.inventorySize; i++) {
    if ((charCpy.inventory[i].quantity > MAX_ITEMS)) {
      checkLst[5] = 0;
      return 0;
    }
    totalItemCount += charCpy.inventory[i].quantity;
  }
  checkLst[5] = (totalItemCount <= MAX_ITEMS);

  // checks all fields in struct passed validation. Must be all 1 to pass
  for (int i = 0; i < 6; i++) {
    if (checkLst[i] != 1) {
      return 0;
      break;
    }
  }

  memset(&charCpy, 0, sizeof(struct Character));
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

  if (nmemb > UINT64_MAX || nmemb == 0) {
    return 1;
  }
  uint64_t charCount = (uint64_t)nmemb;

  struct Character charCpy[charCount];
  memset(charCpy, 0, sizeof(struct Character) * charCount);
  memcpy(&charCpy, arr, sizeof(struct Character) * charCount);
  if (memcmp(arr, &charCpy, sizeof(struct Character) * nmemb)) {
    return 1;
  }

  // Before open file validate everyting and catch error early
  for (uint64_t i = 0; i < charCount; i++) {
    int charValid = isValidCharacter(&charCpy[i]);
    if (!charValid) {
      return 1;
    }
  }

  FILE *fptr = fdopen(fd, "wb");
  if (fptr == NULL) {
    return 1;
  }

  size_t bytesWritten = 0;
  // Swapped sizeof and count to get bytes being written to file
  bytesWritten += fwrite(&charCount, 1, sizeof(uint64_t), fptr);
  bytesWritten += fwrite(&charCpy, 1, sizeof(charCpy), fptr);

  if (bytesWritten != (sizeof(charCpy) + sizeof(charCount))) {
    memset(&charCpy, 0, sizeof(charCpy));
    return 1;
  }
  fflush(fptr);
  fclose(fptr);
  memset(&charCpy, 0, sizeof(charCpy));
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
int loadCharacters(struct Character **ptr, size_t *nmemb, int fd) {
  FILE *fptr = fdopen(fd, "rb");
  if (fptr == NULL) {
    return (1);
  }

  if (fread(nmemb, sizeof(uint64_t), 1, fptr) != 1) {
    return 1;
  }

  struct Character *newCharPtr = calloc(*nmemb, sizeof(struct Character));
  if (newCharPtr == NULL) {
    free(newCharPtr);
    fclose(fptr);
    return 1;
  }

  for (size_t i = 0; i < *nmemb; i++) {
    if (fread(&newCharPtr[i], sizeof(struct Character), 1, fptr) != 1) {
      free(newCharPtr);
      fclose(fptr);

      return 1;
    }
    if (!isValidCharacter(&newCharPtr[i])) {
      free(newCharPtr);
      fclose(fptr);

      return 1;
    }

    // for every character copy only the profession and name use strncpy to
    // avoid copying malicous code after null byte
    char tempBuffer[DEFAULT_BUFFER_SIZE];
    if (strncpy(tempBuffer, newCharPtr[i].profession, DEFAULT_BUFFER_SIZE) ==
        NULL) {
      free(newCharPtr);
      fclose(fptr);
      return 1;
    }
    if (strncpy(newCharPtr[i].profession, tempBuffer, DEFAULT_BUFFER_SIZE) ==
        NULL) {
      free(newCharPtr);
      fclose(fptr);
      return 1;
    }

    // similar to profession, only copy up to null byte if isValid
    if (strncpy(tempBuffer, newCharPtr[i].name, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newCharPtr);
      fclose(fptr);
      return 1;
    }
    if (strncpy(newCharPtr[i].name, tempBuffer, DEFAULT_BUFFER_SIZE) == NULL) {
      free(newCharPtr);
      fclose(fptr);
      return 1;
    }
    memset(tempBuffer, 0, DEFAULT_BUFFER_SIZE);
  }
  fclose(fptr);

  if (*ptr != NULL) {
    free(*ptr);
  }
  *ptr = newCharPtr;
  return 0;
}

/**
 * This function attempt to acquire appropriate permissions for opening the
 * ItemDetails database and load the database from the specified file, and then
 * call the function playgame() to which it should pass the loaded data and the
 * number of items in the loaded data. It returns 0 on success, 1 on
 * deserialization error, and 2 on permission or other errors.
 *
 * @param filepath The path to the file to load data from.
 * @return 0 on success, 1 on deserialization error, 2 on permission or other
 * errors.
 */
int secureLoad(const char *filepath) {

  // Get details of the database owner
  const char *admin = "pitchpoltadmin"; 
  struct passwd *ownerDetails = getpwnam(admin);
  if (ownerDetails == NULL) {
    return 2;
  }

  // Gets the stat of filepath. Fail if it doesnt exist or not a file
  struct stat filestats;
  if (stat(filepath, &filestats) || !S_ISREG(filestats.st_mode)) {
    return 2;
  }

  // If database stats doesnt match admin owner stats (UID AND GID) return
  // error.
  size_t uidCheck = (ownerDetails->pw_uid == filestats.st_uid);
  size_t gidCheck = (ownerDetails->pw_gid == filestats.st_gid);
  if (!(uidCheck && gidCheck)) {
    return 2;
  }

  // store the original privileges and elevate to least privilege
  gid_t originalEUID = geteuid();
  gid_t originalEGID = getegid();
  if (!(filestats.st_mode & S_IROTH)) {
    if (!(filestats.st_mode & S_IRGRP)) {
      if (!(filestats.st_mode & S_IRUSR)) {
        return 2;
      } else {
        if ((originalEUID != filestats.st_uid) &&
            (seteuid(filestats.st_uid) != 0)) {
          return 2;
        }
      }
    } else {
      if ((originalEGID != filestats.st_gid) &&
          (setegid(filestats.st_gid) != 0)) {
        return 2;
      }
    }
  }

  size_t itemCount = 0;
  struct ItemDetails *loadedItems = NULL;

  // open file and immedialely drop privilege after opening. Opened file should
  // still be readable.
  int filedesc = open(filepath, O_RDONLY);
  if (filedesc == -1) {
    return 2;
  }

  if (seteuid(originalEUID)) {
    return 2;
  }
  if (setegid(originalEGID)) {
    return 2;
  }

  int isloaded = loadItemDetails(&loadedItems, &itemCount, filedesc);
  if (isloaded == 1) {
    return 1;
  }
  close(filedesc);

  playGame(loadedItems, itemCount);

  free(loadedItems);
  return 0;
}

void playGame(struct ItemDetails* ptr, size_t nmemb);
