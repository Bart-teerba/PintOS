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

     if (dir != NULL)
       dir_lookup (dir, name, &inode);
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
    if (path[0] == "/") {
      cur_inode = dir_get_inode(dir_open_root ());
    } else {
      cur_inode = dir_get_inode(t->cur_dir);
    }
  }

  char *token, *save_ptr;
  for
   (token = strtok_r (path, "/", &save_ptr); token != NULL;
       token = strtok_r (NULL, "/", &save_ptr))
    {
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
          return 0;
        }
      }
    }

    *inode_ptr = cur_inode;
    *file_name = token;
    return 1;
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
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void)
{
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size)
{

  char path_temp[strlen(name) + 1];
  char *path = &path_temp[0];
  strlcpy (path, name, strlen(name) + 1);

  struct inode *inode;
  char *file_name;
  bool validity = validate_path (path, &inode, &file_name);
  if (validity == 0) {
    return 0;
  }

  block_sector_t inode_sector = 0;
  struct dir *dir = dir_open(inode);

  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size)
                  && dir_add (dir, file_name, inode_sector));
  if (!success && inode_sector != 0)
    free_map_release (inode_sector, 1);
  dir_close (dir);

  return success;
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
  if (validity == 0) {
    return 0;
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
  if (validity == 0) {
    return 0;
  }

  struct dir *dir = dir_open(inode);

  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir);

  return success;
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
