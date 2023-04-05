# cdiff

_This readme is for the bash tool. There is a Qt GUI version in progress..._

A csv diff tool that allows the values of specified columns to differ

## Output

### Help

```
$ ./cdiff.sh -h
cdiff - Diff files while allowing a tolerance on specified columns.

Options:
    --column <col3>,<col21>    The column names that can differ
    --delimiter <delim>        Default ','
    --tolerance <float>        Default 0.1
    --verbose                  More output
    <file1>                    The base file for comparisons
    <file2>                    The file to compare with

Usage:
    cdiff -c colA,colB -t 0.4 <file1> <file2>
    cdiff -c colA -d '|' <file1> <file2>
```

### Fail

```
$ ./cdiff.sh -c colA,colB -v ./testA.csv ./testB.csv
Line 4 differs
Line 7 differs

Invalid rows    = 2/9
Invalid columns = colB=2

Files differ more than tolerable.
```

### Pass

```
$ ./cdiff.sh -c colA,colB -t .2 ./testA.csv ./testB.csv

Files are tolerable.

```