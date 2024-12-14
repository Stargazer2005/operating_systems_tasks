# task_02
In order to prevent information loss in the event of a disk failure, file backups (backups) are commonly used.

The simplest
backup is copying files from one directory to another. This method requires a lot of time and disk space.

Write a program that takes a more intelligent approach. The program should take two parameters from the command line: the names of the source directory and the destination directory.

It should recursively scan the source directory, make copies of all files that have not been copied before or that have been changed since the source directory was created copies or that have been modified since the last backup, placing them in the appropriate locations in the destination directory.

After copying each file, the gzip compression command should be invoked. This will reduce the required disk space, and the file will be renamed with the .gz extension added. Any errors that occur (no source directory, file is not readable, etc.) should be handled correctly with an appropriate message.

## Versions:
- `gzip.c` - copies files from the source directory to the destination directory, compressing them with gzip.
- `gzip_no_links.c` - does the same but ignores the symbolic links.