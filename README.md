# Welcome to undupes' repository

`undupes` attempts to solve the problem of finding duplicate files and deleting them if needed.  It tries to do so using by leveraging the Unix philosophy.

# Quick Actions

### Installation

On the supported versions of Ubuntu and Debian, you could uses the `deb` files provided in the release section.  Please run:

```
$ sudo dpkg -i undupes_<version>_<architecture>_<distro>_<distro_codename>.deb
```

This would create the structure in `/opt/undupes-<version>/` . You can create a softlink in `/usr/local/bin` like:

```
$ sudo ln -s /opt/undupes-<version>/bin/undupes /usr/local/bin/undupes
```

Keeping all the files in `/opt`makes it easy to remove `undupes`.  In order to remove it you could just do: `rm -rf /opt/undupes-<version> /usr/local/bin/undupes` 

I have not tested on the `rpm` based distros. Since there are almost no library dependencies which are a prerequisite, you should able to build from source for the `rpm` based distros.

# Building from source

There are no prerequisite libraries needed to be installed separately for `undupes`. It has been tested with Ubuntu (24.04, 22.04, 20.04) and Debian (bookworm, buster). So the usual cmake installation steps should work for at least those distros.

```
mkdir build
cd build
cmake ..
make -j
sudo make install # This would put the files is /usr/local by default.
```



# Using undupes

```
$  ./install/bin/undupes
Remove duplicate files.
Usage:
  undupes [OPTION...]

  -d, --delete       Delete duplicate files.
  -m, --summary      Summary for the files found equal.
  -y, --dry-run arg  Do a dry run (i.e. do not delete files). Pass a file
                     to write to.
  -h, --help         Print usage

```

##### Finding duplicate files and outputting them in json format.

Note that in this case, hard-links would be treated as normal files.

```
find $HOME/my_dir1 $HOME/my_dir2 -type f -print0 | undupes
```

##### Removing duplicates

```
find $HOME/my_dir1 -type f -print0 | undupes -d
```

##### Making a dry run

```
find $HOME/my_dir1 -type f -print0 | undupes -d --dry-run dry_run_file.txt
```

##### Printing a summary

```
find $HOME/my_dir1 $HOME/my_dir2 -type f -print0 | undupes -m
```

# Philosophy

The purpose of this repo is to solve the problem of removing duplicate files using our understanding of the Unix philosophy.  We try to see, how we can go about reproducing [fdupes](https://github.com/adrianlopezroche/fdupes)' functionality in the Unix tradition.  We write a small program, and leverage the Unix philosophy to get features that fdupes supports.   We also would like to promote `find` command because [we find it very powerful]([find(1) - Linux manual page](https://www.man7.org/linux/man-pages/man1/find.1.html).

#### Design

The input to `undupes` is the file paths (absolute or relative to the current path) and the output will be the sets of files which have the same content. 

The accepted input to `undupes` can be a list of regular files, symlinks or hard-links. We don't make it handle symlinks especially,  because we have `find` for it.  With `find` command, one can specify `-L` for handling the symlinks, `-links` for handling hard-links, and `-type f` for handling the files. 

#### Input

A list of  files, where each file is separated by `\0` , and each file in a set if separated by `\0`.  We chose `\0` instead of whitespace because Linux can have even `\n` in file names and many tools on Linux like `sort`, `find`, `cut`, etc. already support `\0`.

###### File remove interaction

Like the following:

```
[+] [1] A
[+] [2] B

Specify comma or dash separated values for files to keep (ex: 1,2,3-4). [n]one, [a]ll.
>>>
```

The `+` 's indicate that by default all the files would be kept.  We try to stick to the io style of j/f-dupes, to reduce the cognitive load of learning a new io style.

### Supporting fdupes' functionality

##### Recursion

Fdupes does `-r`, we could do the following:

`find $HOME/my_dir1 $HOME/my_dir2 -type f -print0 | undupes`

```
# This will recursively go into the sub-directories
find $HOME/my_dir1 -type f -print0 | undupes

# This will not go into the sub-directories recursively
find $HOME/my_dir2 -maxdepth 1 -type f -print0 | undupes
```

##### Soft links

Fdupes does `-s` for following symlinks, we could let find take care of it for us:

`find $HOME/my_dir1 -type f,l -print0 | undupes`

##### Hard links

Fdupes does `-H` for handling hard-links, `find` by default treats hard-links as files, and `-links 1` can be used to discard hard-links.

```
# To take hard-links
find $HOME/my_dir1 -type f -print0 | undupes`

# To discard hard links.
find $HOME/my_dir1 -type f -links 1 -print0 | undupes`
```

##### File sizes

Fdupes does `-G`/`-L` for considering only files greater/less than a specific number of bytes, we could use find's `-size` option:

```
# For all files greater than 100 bytes.
find $HOME/my_dir1 -type f -size '+100c' -print0 | undupes

# For all files less than 100 bytes.
find $HOME/my_dir1 -type f -size '-100c' -print0 | undupes

# For non-empty files
find $HOME/my_dir1 -type f -size '+1c' -print0 | undupes
```

##### File type

No hidden directories or files:

```
find $HOME/my_dir1 -name '.*' -prune -o -print0 | undupes
```

Under git version control (for `master` branch):

```
git ls-tree -zr --name-only master | undupes
```

##### Choose first file from each set

Here again, we can leverage the Unix philosophy and use `jq`.

```
find $HOME/my_dir1 -type f -print0 | undupes | jq '.[].file_list[0]'
```

##### Ordering of files

The order of input is kept in the sets.  So if `A` and `B` are files with the same content and `A` occurs before `B` in the input to `undupes`, `A` will occur before `B` in the set as well.  
