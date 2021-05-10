#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

struct set_flags
{
   bool l;
   bool d;
   bool f;
   bool a;
   bool h;
};

void tree(struct set_flags flags, char *path_to_start_from, const int root);
void print_long_version(char *file_or_dir);
void help();

// build struct based on flags
// make calls to tree()
int main(int argc, char **argv)
{
   int opt, count = 0;

   struct set_flags flags = {false, false, false, false, false};
   while((opt = getopt(argc, argv, ":ldfah")) != -1)
   {
      switch(opt)
      {
         case 'l':
            flags.l = true;
            break;
         case 'd':
            flags.d = true;
            break;
         case 'a':
            flags.a = true;
            break;
         case 'f':
            flags.f = true;
            break;
         case 'h':
            flags.h = true;
            break;
         case ':':
            printf("Option needs a value\n");
            return 1;
         case '?':
            printf("Unknown option: %c\n", optopt);
            return 2;
      }
   }
   // if they need help get out quick
   if (flags.h == true)
   {
      help();
      return 3;
   }
   for (int idx = 1; idx < argc; idx++)
   {
      if (argv[idx][0] != '-')
         count++;
   }
   if (count == 0)
   {
      printf("NO DIRECTORY WAS SUPPLIED. STARTING FROM CWD.\n");
      tree(flags, ".", 0);
   }
   // call tree for each directory supplied
   else
   {
      while (count > 0)
      {
         tree(flags, argv[optind], 0);
         count--; optind++;
      } 
   }

   return 0;
}

// recursively traverse directory structure
// print files based on flags supplied
void tree(struct set_flags flags, char *base_path, const int root)
{
   int i, status;
   char path[1000];
   struct dirent *dp;
   DIR *dir = opendir(base_path);

   // error occurred or there are no more entrys in current directory stream
   if (!dir)
      return;
   // Each time we re-enter this loop we're in a new directory going through a new directory stream
   // call appropriate flag functions
   // handle regular print
   // make recursive call
   while ((dp = readdir(dir)))
   {
      bool dir_stat = false;
      bool suppress = true;
      bool long_version = false;
      bool follow = false;

       // tree formatting - we should do this regardless of flags
      for (i = 0; i < root; i++)
      {
         if (i % 2 == 0 || i == 0)
            printf("%s", "│");
         else
            printf(" ");
      }
      if (flags.d == true)
      {
         struct stat buffer;
         char *fullpath = realpath(dp->d_name, NULL);
         int status = lstat(fullpath, &buffer);

         if (S_ISDIR(buffer.st_mode))
            dir_stat = true;
         else
            continue;
      }
      // -a check
      if (flags.a == true)
         suppress = false;
      // -f check
      if (flags.f == true)
      {
         struct stat buffer;
         status = lstat(dp->d_name, &buffer);
         follow = true;

         if (dp->d_type == DT_LNK)
         {
            char *fullpath = realpath(dp->d_name, NULL);
            status = lstat(fullpath, &buffer);

            if (S_ISDIR(buffer.st_mode) == 1)
            {
               strcpy(dp->d_name, fullpath);
            } 
         }
      }
      // -l check
      if (flags.l == true)
         long_version = true;
      // logic to determine what we print based on boolean flags
      // defaults
      // suppress = true
      // long_version = false
      // dir_stat = false
      // follow = false
      if (suppress == false && dir_stat == true && long_version == true)
         print_long_version(dp->d_name);
      else if (suppress == false && dir_stat == true)
         printf("%s%s%s\n", "├", "─", dp->d_name);
      else if (suppress == false && long_version == true)
         print_long_version(dp->d_name);
      else if (dir_stat == true && long_version == true)
      {
         if (((char)dp->d_name[0]) == '.')
            continue;
         else
            print_long_version(dp->d_name);
      }
      else if (dir_stat == true)
      {
         if (((char)dp->d_name[0]) == '.')
            continue;
         else
            printf("%s%s%s\n", "├", "─", dp->d_name);
      }
      else if (suppress == false)
         printf("%s%s%s\n", "├", "─", dp->d_name);
      else if (long_version == true)
      {
         if (((char)dp->d_name[0]) == '.')
            continue;
         else
            print_long_version(dp->d_name);
      }
      else if (follow == true)
      {
         if (((char)dp->d_name[0]) == '.')
            continue;
         else
            printf("%s%s%s\n", "├", "─", dp->d_name);
      }
      // no flags case
      if (flags.a == false && flags.l == false && flags.f == false && flags.d == false)
      {
         // if it's hidden just go to next file/dir
         if (((char)dp->d_name[0]) == '.')
            continue;
         else
            printf("%s%s%s\n", "├", "─", dp->d_name);
      }
      // avoid recursing on . and .. regardless of whether -a is set 
      if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
      {
         // check if you got an absolute path from realpath
         if (dp->d_name[0] == '/')
            strcpy(path, dp->d_name);
         // its a relative path so prepend
         else
         {
            strcpy(path, base_path);
            // if base_path has a '/' on end, don't put another one there
            if ((strchr(path, '/') == NULL))
                strcat(path, "/");
            strcat(path, dp->d_name);
         }
         tree(flags, path, root + 2);
      }
   }
   closedir(dir);
}

// tell them how to use it 
void help()
{
   printf("Usage: dirls [(-[adflh]+) (dir)]*\n\t\t-a:  include dot files\n\t\t-f:  follow symbolic links\n\t\t-d:  only this directory\n\t\t-l:  long form\n\t\t-h:  prints this message\n");
}

// print long form data about the file
void print_long_version(char *object_name)
{
   struct stat buffer;
   int status = lstat(object_name, &buffer);

   if (S_ISDIR(buffer.st_mode))
      printf("%c", 'd');
   else if (S_ISFIFO(buffer.st_mode))
      printf("%c", 'f');
   else if (S_ISREG(buffer.st_mode))
      printf("%c", '-');
   else if (S_ISLNK(buffer.st_mode))
      printf("%c", 'l');
   else if (S_ISBLK(buffer.st_mode))
      printf("%c", 'b');
   else if (S_ISCHR(buffer.st_mode))
      printf("%c", 'c');
   else
      printf("%c", 's');

   printf( (buffer.st_mode & S_IRUSR) ? "r" : "-");
   printf( (buffer.st_mode & S_IWUSR) ? "w" : "-");
   printf( (buffer.st_mode & S_IXUSR) ? "x" : "-");
   printf( (buffer.st_mode & S_IRGRP) ? "r" : "-");
   printf( (buffer.st_mode & S_IWGRP) ? "w" : "-");
   printf( (buffer.st_mode & S_IXGRP) ? "x" : "-");
   printf( (buffer.st_mode & S_IROTH) ? "r" : "-");
   printf( (buffer.st_mode & S_IWOTH) ? "w" : "-");
   printf( (buffer.st_mode & S_IXOTH) ? "x " : "- ");

   printf("%ld ", buffer.st_size);

   struct passwd *st = getpwuid(buffer.st_uid);
   if (st)
      printf("%s ", st->pw_name);
   struct group *gp = getgrgid(buffer.st_gid);
   if (gp)
      printf("%s ", gp->gr_name);

   printf("%s ", object_name);
   
   printf("%s\n", ctime(&buffer.st_mtime));   
}