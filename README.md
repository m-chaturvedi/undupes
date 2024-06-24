

# Quick Actions

### Installation

There are no prerequisites libraries needed to be installed separately for `undupes`.  It has been tested with Ubuntu (24.04, 22.04, 20.04) and Debian (bookwom, buster).  So the usual cmake installation steps should work for those distros.

```
mkdir build
cd build
cmake ..
make -j
sudo make install
```

You could also use `bash -x install.sh` to do the installation.

```
$  undupes --help
Remove duplicate files.
Usage:
  undupes [OPTION...]

  -d, --delete       Delete duplicate files.
  -m, --summary      Summary for the files found.
  -y, --dry-run arg  Do a dry run (do not delete files). Pass a file to
                     write to.
  -h, --help         Print usage
```



# Find duplicates (Unix style)

The purpose of this repo is to solve the problem of removing duplicate files in our understanding of the Unix philosophy.  We try to see, how we can go about reproducing [fdupes](https://github.com/adrianlopezroche/fdupes)' functionality in the Unix tradition.

### Design

The input to `undupes` is the file paths (absolute or relative to the current path) and the output will be the sets of files depending on some function.

The input to `undupes` would be just a list of regular files or symlinks.  We don't make it handle symlinks especially.  We do so because we have `find` for it.  In the `find` command, one can specify `-L` for handling the links, and `-type f` for handling the files.  We do not want to solve an already solved problem.

#### Input

A list of  sets of files, where each set if separated by `\0` , and each file in a set if separated by `\0`.  We don't hadle directories or links.

###### File remove interaction

Like the following:

```
[1] [+] A
[2] [-] B
Specify keep values or [n]one, [a]ll, [p]reselected. Preselected values will be lost 
```

The files are deleted as soon as the user writes an acceptable option.   Preselected values will be lost when the values are supplied.  The the values will be printed.  When all files are kept, no need to print.  Double check before deletion.

```
   [-] A
   [-] B
   The files marked with [-] will be deleted, press y to accept, n to re-enter.
```

- Try to see if they can be printed in green/red color.

- Add an option for dry run, which output files to be deleted and kept.  Create a file with dry run which can be parsed and used to keep or delete files.

- Add an option to summarize like fdupes `-m`.

- `keep` and `delete` regex can be used for preselection `+` and `-`. By default the values will be preselected based on these.  If there are any conflicts then a warning will be displayed and everything will be kept. 

- Add a noprompt option to not prompt, if `keep` and `delete` regex matches, go ahead and delete.  If there are any conflicts then don't go ahead.

- keep and delete can be plain-prefixes or regexes.  If plain prefixes, make sure that one is not a subdirectory of the other.

##### Output

A list of sets after filtering the files.   

#### Interactively removing files

When we have the '\0\0' sets of files, we could write a wrapper over it, which interacts with the user to delete files in various ways. This wrapper could be written in python or something that calls this code.

### Recursion

Fdupes does `-r`, we could use find instead:

`find $HOME/my_dir1 $HOME/my_dir2 -type f -print0 | finddupes`

Note that finddupes take null separated list of files that is sorted by size.

Fdupes does `-R` for only recursing into specific directories (the above command recurses into all the sub-directories):

```
# This will recursively go into the sub-directories
find $HOME/my_dir1 -type f -print0 > /tmp/files.txt'

# This will not go into the sub-directories recursively
find $HOME/my_dir2 -maxdepth 1 -type f -print0 >> /tmp/files.txt'

# And then do:
cat /tmp/files.txt | finddupes
```

### Soft links

Fdupes does `-s` for following symlinks, we could let find take care of it for us:

`find $HOME/my_dir1 -type f,l -print0 | finddupes`

### Hard links

Fdupes does `-H` for handling hard-links, we haven't yet looked into it properly, yet.

### File sizes

Fdupes does `-G`/`-L` for considering only files greater/less than a specific number of bytes, we could use find's `-size` option:

```
find $HOME/my_dir1 -type f -size '+100c' -print0 # For all files greater than 100 bytes.

find $HOME/my_dir1 -type f -size '-100c' -print0 # For all files less than 100 bytes.

find $HOME/my_dir1 -type f -size '+1c' -print0 # For only non-empty-files
```

### File type

No hidden directories or files:

`find $HOME/my_dir1 -name '.*' -prune -o -print0`

## 
