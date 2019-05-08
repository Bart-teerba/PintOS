#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

struct inode *
filesys_open_helper (struct inode *inode_dir, char *name)
   {
     struct dir *dir = dir_open(inode_dir);
     struct inode *inode = NULL;
     if (dir != NULL) {
       dir_lookup (dir, name, &inode);
     }
     dir_close (dir);
     return inode;
   }


bool
validate_path (char *path, struct inode **inode_ptr, char **file_name)
{
  struct thread *t = thread_current ();
  struct inode *cur_inode;
  struct inode *next_inode;
  if (strlen(path) > 0) {
    // if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) {
    //   return false;
    // }
    if (path[0] == '/') {
      cur_inode = inode_open(ROOT_DIR_SECTOR);
    } else {
      cur_inode = inode_reopen(t->cur_dir_inode);
    }
  } else {
    return false;
  }
  if (cur_inode->removed == true) {
    return false;
  }

  char *token, *save_ptr;
  for
   (token = strtok_r (path, "/", &save_ptr); token != NULL;
       token = strtok_r (NULL, "/", &save_ptr))
    {
      // printf("%s\n", token);
      if (*save_ptr == '\0') {
        break;
      }
      if (strcmp(token, ".") == 0) {
        continue;
      } else if (strcmp(token, "..") == 0) {
        next_inode = inode_open((cur_inode->data).parent);
        inode_close(cur_inode);
        cur_inode = next_inode;
      } else {
        next_inode = filesys_open_helper (cur_inode, token);
        if (next_inode != NULL && inode_isdir (next_inode)) {
          inode_close(cur_inode);
          cur_inode = next_inode;
        } else {
          inode_close(cur_inode);
          return false;
        }
      }
      if (cur_inode->removed == true) {
        return false;
      }
    }

    *inode_ptr = cur_inode;
    *file_name = token;
    if (token == NULL) {
      *file_name = ".";
    }
    return true;
}


/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format)
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format)
    do_format ();

  free_map_open ();

  /* Load current directory for the startup thread */
  thread_current()->cur_dir_inode = inode_open(ROOT_DIR_SECTOR);
  thread_current()->cur_dir_inode->data.isdir = true;
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void)
{
  free_map_close ();
}

/* Helper function for creating files/directories */
static bool
filesys_create_helper (const char *name, off_t initial_size, bool if_dir) {

    char path_temp[strlen(name) + 1];
    char *path = &path_temp[0];
    strlcpy (path, name, strlen(name) + 1);

    struct inode *inode;
    char *file_name;
    // printf("%s\n", path);
    bool validity = validate_path (path, &inode, &file_name);

    if (validity == false) {
      return false;
    }

    block_sector_t inode_sector = 0;
    struct dir *dir = dir_open(inode);
    bool suc1 = dir != NULL;
    bool suc2 = free_map_allocate (1, &inode_sector);
    bool suc3 = inode_create (inode_sector, initial_size);
    bool suc4 = dir_add (dir, file_name, inode_sector);
    bool success = suc1 && suc2 && suc3 && suc4;

    // printf("%d, %d, %d, %d, %d\n", suc1, suc2, suc3, suc4, success);
    if (!success && inode_sector != 0) {
      free_map_release (inode_sector, 1);
    } else if (success) {
      struct inode_disk buffer;
      block_read(fs_device, inode_sector, &buffer);
      buffer.num_entries = 0;
      buffer.isdir = if_dir;
      block_write(fs_device, inode_sector, &buffer);
    }
    dir_close (dir);
    return success;
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size)
{
  return filesys_create_helper(name, initial_size, false);
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */



struct file *
filesys_open (const char *name)
{
  char path_temp[strlen(name) + 1];
  char *path = &path_temp[0];
  strlcpy (path, name, strlen(name) + 1);

  struct inode *inode;
  char *file_name;
  bool validity = validate_path (path, &inode, &file_name);
  if (validity == false) {
    return NULL;
  }

  return file_open (filesys_open_helper (inode, file_name));
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name)
{
  char path_temp[strlen(name) + 1];
  char *path = &path_temp[0];
  strlcpy (path, name, strlen(name) + 1);

  struct inode *inode;
  char *file_name;
  bool validity = validate_path (path, &inode, &file_name);
  if (validity == false) {
    return false;
  }

  struct dir *dir = dir_open(inode);

  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir);

  return success;
}

/* Change the current directory of the process.
   If the path is not valid, return false. */
bool
filesys_chdir (const char *name)
{
  char path_temp[strlen(name) + 1];
  char *path = &path_temp[0];
  strlcpy (path, name, strlen(name) + 1);

  struct inode *inode;
  char *file_name;
  bool validity = validate_path (path, &inode, &file_name);
  if (validity == false) {
    return false;
  }

  struct inode *new_inode = filesys_open_helper (inode, file_name);
  inode_close(inode);
  if (new_inode == NULL) {
    return false;
  }

  struct thread *t = thread_current ();
  inode_close(t->cur_dir_inode);
  t->cur_dir_inode = new_inode;
  return true;
}

bool
filesys_mkdir (const char *name, off_t initial_size)
{
  return filesys_create_helper(name, initial_size, true);
}


/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}
