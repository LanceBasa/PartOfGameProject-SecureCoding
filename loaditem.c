int loadItemDetails(struct ItemDetails** ptr, size_t* nmemb, int fd) {
  FILE* fptr = fdopen(fd, "rb");
  if (fptr == NULL) {
    perror("unable to open file");
    return 1;
  };
  
  size_t numRead;
  size_t reading = fread(&numRead, sizeof(uint64_t), 1, fptr);
  if (reading != 1) {
      perror("reading heading failed!");
      fclose(fptr);
      return 1;
  };
  *nmemb = numRead;

  struct ItemDetails* temp = calloc(*nmemb, sizeof(struct ItemDetails));
  if (temp == NULL) {
      printf("Calloc failed");
      fclose(fptr);
      return 1;
  }

  for (size_t i = 0; i < *nmemb; i++) {
      reading = fread(&temp[i].itemID, sizeof(uint64_t), 1, fptr);
      if (reading != 1) {
          perror("reading itemID failed");
          free(temp);
          fclose(fptr);
          return 1;
      }

      reading = fread(temp[i].name, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
      if (reading != DEFAULT_BUFFER_SIZE) {
          perror("reading name failed");
          free(temp);
          fclose(fptr);
          return 1;
      }
      temp[i].name[DEFAULT_BUFFER_SIZE - 1] = '\0';

      reading = fread(temp[i].desc, sizeof(char), DEFAULT_BUFFER_SIZE, fptr);
      if (reading != DEFAULT_BUFFER_SIZE) {
          perror("reading itemID failed");
          free(temp);
          fclose(fptr);
          return 1;
      }
      temp[i].desc[DEFAULT_BUFFER_SIZE - 1] = '\0';
  }

  // If everything succeeded, update *ptr to point to the allocated data
  *ptr=NULL;
  *ptr = temp;
if (*ptr != NULL) {
  free(*ptr);
  *ptr = NULL;
}
  fclose(fptr);

  return 0;
}