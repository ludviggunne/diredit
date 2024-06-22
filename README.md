## diredit - Manage files with your favorite editor

**diredit** is a simple program that opens a list of files and allows you to edit their names
with your prefered editor. The editor command is read from **EDITOR**, or fall backs to nano.
Note that order of files are preserved, so if you exchange two lines in the temporary file,
the corresponding file names will be exchanged as well. If the number of lines doesn't match
the original number of files, diredit will do nothing. The directory to edit is read from the
first command line argument. If it's omitted, the current working directory is used.
