gid_t originalUID=getuid();
   gid_t originalGID=getgid();

 
 
  printf("ruid:%d\teuid:%d\tgid:%d\tgeid:%d\n", getuid(),geteuid(),getgid(),getegid());

 //check what level of permissions are required, check current permissions
  if (!(filestats.st_mode & S_IROTH)) {
      printf("Other users cannot read\n");

    if (!(filestats.st_mode & S_IRGRP)) {
      printf("Group users cannot read\n");
      if (!(filestats.st_mode & S_IRUSR)) {
        printf("User users cannot read all reads are blocked\n");
        
      }else{
        printf("Yes owner User can read\n");

        if(originalUID!= filestats.st_uid){
          setuid(filestats.st_uid);
        }
        printf("user set set\n");
      }
    }else{
      printf("Yes Group users can read set gid to getegid\n");

      //if only groups can read then read set the gid to geuid
      // if the groups are not the same then set it
      printf("id%d\n",filestats.st_gid);
      printf("id%d\n",getgid());

      if(getegid()!=filestats.st_gid){
          printf("changing uid\n");
          if(seteuid(filestats.st_uid)!=0){
            printf("seteuid didnt work\n");

          }         
        if(setegid(filestats.st_gid)!=0){
          printf("setegiddidnt work\n");
        }

      }
      printf("group set\n");
    }
  }
  // if anyone can read just read

  printf("ruid:%d\teuid:%d\tgid:%d\tgeid:%d\n", getuid(),geteuid(),getgid(),getegid());

setuid(originalUID);
setgid(originalGID);
printf("ruid:%d\teuid:%d\tgid:%d\tgeid:%d\n", getuid(),geteuid(),getgid(),getegid());