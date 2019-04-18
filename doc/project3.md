Design Document for Project 3: File Systems
======================================

## Group Members

* Yifan Ning <yifanning@berkeley.edu>
* Zhi Chen <zhichen98@berkeley.edu>
* Jerry Li <jerry.li.cr@berkeley.edu>
* Bart Ba <bart98@berkeley.edu>

<br />
<br />
<br />
<br />

## Task 1: Buffer cache

<br />

### 1. Data Structures and functions

**ADDITION:**

**cache.h / cache.c**

```c
static struct cache_block[64] cache;
static uint32_t clock_hand;
/* Makes sure only one eviction and load at a time */
static struct lock le_lock; 

struct cache_block 
{
    block_sector_t sector;
    bool dirty = 0;
    bool valid = 1;
    /* For clock algorithm
     * 0 after a clock_hand visit
     * 2 when the rw_lock is required
     * 1 if block has been visited but not under use */
    bool used = 0;     
    /* Pointer to data  in cache */
    void *data = NULL; 
    /* Makes sure only one read or write on the
     * same cache block at a time */
    struct lock rw_lock;
}

void cache_init ();
void cache_read (block_sector_t sector, void *buffer);
void cache_write (block_sector_t sector, void *buffer);
void cache_flush ();

/* Create a thread typically runs this function
 * to periodically flush the cache
 * Optional */
// void* periodic_flusher(); 

cache_block *get_cache_block (block_sector_t sector);

/* Runs the clock algorithm and update `
 * used` value in cache clock */
void get_cache_block_to_evict (); 
```

**MODIFICATION:**

**inode.c**
```c
/* Substitute all `block_read` & `block_write` to
 * `cache_read` & `cache_write` */
off_t inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset){}
off_t inode_write_at (struct inode *inode, void *buffer_, off_t size, off_t offset){}
```
**filesys.c**
    
```c
void filesys_init (bool format)
{
    ...
    cache_init();
    ...
}

void filesys_done (void)
{
    ...
    /* Write back dirty blocks when filesys is done */
    cache_flush(); 
    ...
}
```



<br />

### 2. Algorithms

<br />

#### Section 1 -- Cache and `struct cache_block`:
1. Cache contains 64 `cache_block`s and uses `static struct cache_block[64] cache` to track
2. Init `cache_block` to store pointers of 64 dummy `cache_block` whose sector is dummy, `used` is 0, `dirty` is 0, and `valid` is 0
3. In each `cache_block`, there are six variables:
    - `block_sector_t sector`: stores the block sector of the corresponding cached block in disk
    - `bool dirty`: 1 if data has been modified in cache else 0
    - `bool valid`: 1 if data is valid else 0
    - `bool used`: For clock algorithm 
        - 0 after a clock_hand visit
        - 1 if block has been used but not under use
        - 2 when the rw_lock is aquired
    - `void *data`: Pointer to the data stored in the cache
    - `struct lock rw_lock`: Makes sure only one read or write on the same cache block at a time
4. Cache has a `static uint32_t clock_hand` which loop through 0 to 63 for clock algorithm and a `static struct lock le_lock` which is acquired when evicting and loading data

#### Section 2 -- Clock Algorithm:
1. Use Clock Algorithm as replacement policy
2. Loop `clock_hand` through 0 to 63 until finding a `cache_block` with `used` as 0
3. When `clock_hand` is $i$, check `used` and `valid` variable in $i^{th}$ `cache_block` in `cache`
    - If `valid` is 0, evict it
    - If `used` is 2, it means the current block is under use so skip it
    - If `used` is 1, it means the current block has been used after last visit so change it to 0
    - If `used` is 0, evict it
4. Write back the evicting block if it is dirty
5. Move `clock_hand` to the next step

#### Section 3 -- Cache_flush:
1. Writes back all dirty blocks and resets their `dirty`
2. Called when `filesys_done()` and machine shutdown
3. Optional: write `periodic_flusher()` an infinite loop with `timer_sleep()` and `cache_flush()` to call `cache_flush()`

#### Section 4 -- Read:
1. Try to find the target sector in cache
    - If found
        - check valid and invalid means not found
        - change `used` to 2
        - acquire `rw_lock`
        - get the data
    - If not
        - acquire `le_lock`
        - use clock algorithm to find a evicting block in cache
        - change `used` to 2
        - acquire `rw_lock`
        - `block_read()` data
        - update the corresponding `cache_block`
        - release `le_lock`
2. Release `rw_lock`
3. Change `used` to 1

#### Section 5 -- Write:
1. Try to find the target sector in cache
    - If found
        - change `used` to 2
        - change `valid` to 1
        - acquire `rw_lock`
    - If not
        - acquire `le_lock`
        - use clock algorithm to find a evicting block in cache
        - change `used` to 2
        - acquire `rw_lock`
        - update the corresponding `cache_block`
        - release `le_lock`
2. Change the data in cache
3. Change `dirty` to 1
4. Release `rw_lock`
5. Change `used` to 1

#### Section 6 -- Evict and load:
1. Make sure `le_lock` has already been acquired when evicting
    -  acquire `rw_lock`
    -  acquire `le_lock`
2. Write back the block to be evicted if the block is dirty before evicting
3. Load the new data block into cache by setting all variables in current cache block
    -  change `used` to 1
    -  change `valid` to 1
    -  change `dirty` to 0
    -  change `sector` to sector number of the disk block
    -  change `data` to point to data space for the cache
    -  release `le_lock`
    -  release `rw_lock`

<br />

### 3. Synchronization

1. When one process is actively reading or writing data in a buffer cache block, how are other processes prevented from evicting that block?

    - When one process is actively reading or writing data in cache, process owns rw_lock of current cache block. If one process want to evict certain block, it have to acquire rw_lock of that block before acquire the le_lock. Since the rw_lock has been acquired by the first process, other processes cannot evict any cache block

2. During the eviction of a block from the cache, how are other processes prevented from attempting to access the block?

    - To do an eviction operation, process need to first acquire the rw_lock and then acquire le_lock. If current process is evicting a block from the cache, it means the process has owned both rw_lock and le_lock, so that other processes cannot acquire the rw_lock in order to access the block

3. If a block is currently being loaded into the cache, how are other processes prevented from also loading it into a different cache entry? How are other processes prevented from accessing the block before it is fully loaded?

    - Since the block is loaded in the cache, when other cache want to access same data, they will find this block and know that they don't need to load this block again. Before a process want to load a block, it will first acquire rw_lock and then acquire le_lock. Other processes cannot access that block without getting the rw_lock



<br />

### 4. Rationale

- Use array instead of linked list to implement cache
    - Don't need to find struct by `list_elem`
    - Change data inplace
    - Same time to find a sector
    - Given max size is 64

<br />
<br />
<br />
<br />


## Task 2: Extensible Files


<br />

### 1. Data structures and functions

**inode.c**

```c
/* Modification */
struct inode_disk
  {
    ...
    off_t length;                    
    unsigned magic; 
    
    /* 12 direct pointers */
    block_sector_t direct[12]; 		
    
    /* A singly indirect pointer */
    block_sector_t indirect; 		
    
    /* A doublly indirect pointer */
    block_sector_t doubly_indirect; 	
    
    /* Not used. To fill size */
    uint32_t unused[...]; 					
    ...
  };

static block_sector_t byte_to_sector (const struct inode *inode, off_t pos);
bool inode_create (block_sector_t sector, off_t length);
struct inode * inode_close (struct inode *inode);
off_t inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
off_t inode_write_at (struct inode *inode, const void *buffer_, off_t size, off_t offset); 


/* Addition */

inode_grow (struct inode *inode, off_t size);

/* Add helper functions */
/* Given the index of a sector, this 
 * function returns the address of 
 * the corresponding block sector */
block_sector_t* ind_to_sector(struct disk_inode *inode_disk, off_t index)
```



**free-map.c**

```c
/* Addition */
static struct lock freemap_scan_flip_lock;

/* Modification */
/* Acquire lock before test-setting bits and release after done */
bool free_map_allocate (size_t cnt, block_sector_t *sectorp);
```



<br />


### 2. Algorithms
<br />

#### Section 0 -- Summary

1. Instead of using `block_read` or `block_write`, now we should be able to use the `cache_read` and `cache_write` implemented in task1 to do memory access

2. Since we need to support 8MB file, our design decision is to use an array of direct blocks, one indirect block, and one doubly indirectly block

3. Note since now we are using index structure rather than single content, there're several problems with old implementation we need to address: 
    - Translate from byte offset to block device sector number (`byte_to_sector`)
    - Managing inodes (`create`, `open`, etc)
    - How to extend a file
    

#### Section 1 -- `byte_to_sector(const struct inode *inode, off_t pos)`:

1. If byte offset is greater than 8MB, failure (`-1`) is returned immediately
2. If the byte offset is inside the range addressed by 12 direct blocks, we simply acquire `direct[pos / BLOCK_SECTOR_SIZE]`
3. If  the byte offset is inside the range addressed by singly indirect block, we use `cache_read` to grab the the block, and find the according `block_sector_t` by offsets on grabbed block
4. If  the byte offset is inside the range addressed by doubly indirect block, we use `cache_read` to grab the  block, and find the according ``block_sector_t``, which is a singly indirect block. Then do another `cache_read` to grab the singly indirect block, find the according `block_sector_t` by offsets

#### Section 2 -- `inode_create (block_sector_t sector, off_t length)`:

1. First figure out how many blocks needed, which is `num_sectors_needed = bytes_to_sectors(length) `
2. Since we now don't assume contiguous memory, we will need to call `free_map_allocate (1, &pos))` exactly `num_sectors_needed` times. Note, each time , `pos` is figured out by the helper function `ind_to_sector()`, which converts the index to the corresponding address storing the sector. For example, if we are at our second call of `free_map_allocate (1, &pos))`, then we should find the address of storing second sector, which is `direct[1]`. `inode_disk->indirect` and `inode_disk->doubly_indirect` are similarly addressed
3. The rest should generally be the same except using `cache_write` rather than `block_write`

#### Section 3 -- `inode_close (struct inode *inode)`:

1. Like create, during close, we shouldn't directly free all the contiguous sectors by `free_map_release (inode->data.start, bytes_to_sectors (inode->data.length))`, but call `free_map_release (&pos, 1)` exactly `num_sectors_needed = bytes_to_sectors(length) ` times
2. `pos` is all the addressable pointers storing valid sectors, inside `direct[12]`, `indirect`, or `doubly_indirect`

#### Section 4 -- Growing File:

1. File should grow each time when writing past the end of file 
2. Growing file should be pretty similar to creating an inode with certain size, except that we call `free_map_allocate (1, &pos))` the `bytes_to_sectors(extra_size)` times and start from the first unused slot that can store a sector rather than start from `direct[0]`
3. Also, fill any gap between the previous EOF and the start of the `write()` with zeros. We choose to actually allocate and write real data blocks for implicitly zeroed blocks

#### Section 5 -- Rollback: 

1. It is possible that when creating or growing files we found out there's no enough disk space
    - To deal with such case, we only actually allocate disk space if we are able to set all the bits we need when calling `free_map_allocate (1, &pos))` series of times 
    - If we are able to set all the bits, meaning there's enough space on disk and we can go ahead allocate
3. If any of the calls failed, meaning there's not enough space and we should flip back the bits we already changed. This can be done by `free_map_release(block_sector_t sector, 1)` on the sectors previous set due to `free_map_allocate (size_t cnt, block_sector_t *sectorp)`

#### Section 6 -- Implement `inumber(int fd)`:

1. Since we've stored a `file_to_fd` mapping during project 2, we can retrieve the corresponding `struct file*` through that mapping, inside which we can have its `struct inode *inode`
2. As for unique inode number, the first sector, i.e. `direct[0]` should suffice

<br />

### 3. Synchronization

<br />

- Remove the global `filesys_lock`

There are several synchronization problems we need to solve: 

1. Our list structure is not thread-safe, so operations related with `open_inodes` list management such as `open`, `close` can cause synchronization issues
    - We choose to add an `inode_open_close_lock`, so that operation that can insert or delete an element in the list will first have to acquire this lock, and then go on
    - As soon as we are done with the list operation, we should be able to release the lock
    - Since the above process does not involve any disk operation yet and thus is supposed to be fast, adding this lock should not affect the performance too much

2. Operations acting on diﬀerent disk sectors should be allowed to run simultaneously
    - This is achieved by  task1 implementation because now we use `cache_read` and `cache_write` for disk access

3. However, we do need to ensure thread-safe updates to `free_map`. Although most operations set the bits atomically, testing bits is not atomic with setting bits
    - Therefore, we choose to add a `freemap_scan_flip_lock` so that before any testing and set, we have to acquire this lock, and release the lock after we are done
    - Writing the `*free_map_file` is same as writing a file, which is similarly handled as in 


### 4. Rationale

1. In implementing `struct inode_disk`, we choose to use an array of 12 direct blocks, one indirect block and one doubly indirect block
    - We use an array of direct blocks for fast access to small files, and one directly block for relatively fast access to medium size file
    - For file size up to 8MB, doubly indirect block should far more than enough because itself can hold $2^7 * 2^7 * 2^9 = 2^{23}  \approx 8MB$
3. For filling gaps between the previous EOF and the start of the `write()` with zeros, we choose to actually allocate real blocks of data for ease of implementation

<br />
<br />
<br />
<br />


## Task 3: Subdirectories

<br />

### 1. Data structures and functions

**syscall.c**
```c
/* Modification */
/* Add handler for syscalls */
static void syscall_handler (struct intr_frame *f UNUSED){}

/* Many other helper functions */
/* Check `isdir` for helper functions in Proj 2 like 
 * `open`, `remove`, `read`, `write`, etc.
 * which deals with normal files */
```


**thread.h**

```c
/* Addition */
struct thread 
{
    ...
#ifdef FILESYS
    struct dir cur_dir;
    char cur_dir_path[128];
#endif
    ...
}
```
**inode.h / inode.c**

```c
/* Addition */
/* Return `inode->data.isdir` */
bool inode_isdir (const struct inode *inode);


/* Modification */
struct inode_disk
{
    ...  
    /* Indicates if it is a directory */
    bool isdir;              
    /* The number of subdirectories or files */
    uint32_t num_entries;    
    // uint32_t unused[] will be modified to satisfy the size requirement.
    ...
}
```
**filesys.c**
```c
/* Addition */

/* Format a path so that no . or .. is contained 
 * Return a absolute path */
char * format_path (const char *path);

/* Validate a path using `dir_lookup()` and return the remaining path 
 * close dir after using it */
char * validate_path (const char *path, struct dir **dir_ptr, char *filename);

/* Return file name splitted by "/" like `strtok_r()` */
char * parse_path (const char *path, char **save_ptr); 

/* Modification */
/* Call `validate_path()` to get the target dir 
 * close dir after using it */
bool filesys_create (const char *name, off_t initial_size) {}
bool filesys_remove (const char *name) {}
/* move its content to a helper function which returns inode 
 * Use the result to file_open(inode) */
struct file *filesys_open (const char *name){}
struct inode *filesys_open_helper (const char *name){}
```

<br />

### 2. Algorithms

<br />

#### Section 0 -- Side Notes

1. Get `file` from `fd` by `get_file()`
2. Get `inode` from `file` by `file_get_inode()`
3. Get `inode_disk` from `inode` by `inode->data`
4. Get `sector` from `inode` by `inode_get_inumber()`
5. Get `inode` from `sector` by `inode_open(sector)`
6. Get `dir` from `inode` by `dir_open(&inode)` // This creates a dir
7. Get `inode` from `dir` by `dir_get_inode(&dir)`
8. Get `inode` from `name` by `filesys_open_helper`

#### Section 1 -- `inode_disk` struct

1. `isdir` shows whether it is a dir or a file
2. `unused` array size is dependent on both task2 and task 3
3. `unused` array size is set to make the size of `inode_disk` struct equals `BLOCK_SECTOR_SIZE` bytes

#### Section 2 -- Relative Path

1. Create `cur_dir` and `cur_dir_path` in `thread` struct to store the current working directory
2. In `format_path`, append relative path to `cur_dir_path` and correctly deal with and then remove `..` and `.`
3. Return a absolute path which contains no `.` or `..`
4. Absolute path and relative path are distinguished by whether begin with `/`
5. In `validate_path`, check all the directories on the way until we are reaching the last one
    - Keep the last file name and the second to last working directory
    
#### Section 3 -- chdir()

1. Get `inode` and then `inode_disk` from `name` with `filesys_open_helper`
2. Create a new current working dir with `inode`
3. Close the old current working dir

#### Section 4 -- mkdir()

1. Call `create()` in `syscall.c`
2. Get `inode` and then `inode_disk` from `name` with `filesys_open_helper`
3. Set `isdir` to 1

#### Section 5 -- readdir()

1. Get `file` pointer and then `inode` and then create `dir` from `fd` in `syscall.c`
2. Calls `dir_readdir()` in `directory.c`

#### Section 6 -- isdir()

1. Get `file` pointer and then `inode` from `fd` in `syscall.c`
2. Calls `inode_isdir()` in `inode.c`

#### Section 7 -- inumber()

1. Get `file` pointer and then `inode` from `fd` in `syscall.c`
2. Returns the value of sector in the `inode` by `inode_get_inumber()`

<br />

### 3. Synchronization

1. How will your filesystem take a relative path like ../my_files/notes.txt and locate the corresponding directory? Also, how will you locate absolute paths like /cs162/solutions.md?
    - In Section 2 of Algorithms, it is explained
    - Relative path is turned into absolute path without `.` and `..` in `format_path()`
    - Parse the absolute path by "/" with `parse_path()`
    - Get the correct `dir` and `file_name` with `validate_path()`

3. Will a user process be allowed to delete a directory if it is the cwd of a running process? The test suite will accept both “yes” and “no”, but in either case, you must make sure that new files cannot be created in deleted directories.
    - Logically, a user process should not be allowed to delete a directory if a running process is using it
    - Cannot free inode if `inode->open_cnt > 0`
    - Always close `dir` when it is no longer used or is has been changed to another `dir`
    - `dir` is created with `inode` and `inode` is created with `sector`
    - In `dir_remove()`, `inode` has been removed and thus no new files an be created in deleted directories

5. How will your syscall handlers take a file descriptor, like 3, and locate the corresponding file or directory struct?
    - Explained in Section 0 of Algorithms
    - Handle it in `syscall.c` with mechanism from Proj 2
    - `file` and `fd` are stored in `struct fd_file_map`
    - Get `file` from `fd` by `get_file()`
    - Get `inode` from `file` by `file->inode`
    - Get `dir` from `inode` by `dir_open(&inode)` // This creates a dir
 
7. You are already familiar with handling memory exhaustion in C, by checking for a NULL return value from malloc. In this project, you will also need to handle disk space exhaustion. When your filesystem is unable to allocate new disk blocks, you must have a strategy to abort the current operation and rollback to a previous good state.
    - Check returned bool values of functions like `filesys_create` and `inode_create`
    - The given `goto done` structure helps
<br />

### 4. Rationale

1. Before accessing a file or a directory, we always need to first validate a name of file or directory , and then follow by setting the `dir` pointer point to the secondary directory (c for a/b/c/d), so we decided to merge those two functionality into one function `validate_path`
2. There are many situations that we only need to access inode instead of a file. We decide to write a helper function `filesys_open_helper` which return inode directly given name
3. We need to be careful to free directory after finishing using it because every time using `dir_open` function, it will always `calloc` a new position.
4. For `parse_path`, here's an example of how to translate a relative path into precise absolute path 
    - name: `./c/.././a/r` , cur_dir_path: `/b/e`, output: `/b/e/a/r`
    - first we add cur_dir_path to the front of name in order to get a complex absolute path, which will be `/b/e/./c/.././a/r`
    - Then delete all `/.`, since `/.` represents current directory to get `/b/e/c/../a/r`
    - Then we deal with `/..` by just removing `/..` and the token in front of `../`, which is `/c` here
    - Thus we get `/b/e/a/r` as output
<br />
<br />
<br />
<br />

## Additional Questions



1. For this project, there are 2 optional buffer cache features that you can implement: write-behind and read-ahead. A buffer cache with write-behind will periodically flush dirty blocks to the filesys- tem block device, so that if a power outage occurs, the system will not lose as much data. Without write-behind, a write-back cache only needs to write data to disk when (1) the data is dirty and gets evicted from the cache, or (2) the system shuts down. A cache with read-ahead will predict which block the system will need next and fetch it in the background. A read-ahead cache can greatly improve the performance of sequential file reads and other easily-predictable file access patterns. Please discuss a possible implementation strategy for write-behind and a strategy for read-ahead. You must answer this question regardless of whether you actually decide to implement these features

<br />

**Answer**

1. **For write-behind cache, we would like to use the timer designed in project 1 to record time and create a thread** that specifically run function `periodic_flusher` which periodically flush dirty blocks according to timer
2. **For read-ahead cache, when we read a new block, we can read and record more blocks/sectors followed by that block into the cache.** It will increase the hit rate when there are some continous accesses of data. The core idea is predicting that sequential blocks will be accessed more likely


