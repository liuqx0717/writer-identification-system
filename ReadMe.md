# writer identification system
Some command-line tools I wrote for a writer identification system similar to the one in *Writer Identification Using GMM Supervectors and Exemplar-SVMs*.

These tools have a depencency on OpenCV library and Boost library. To compile, you need to set the include path and library path accordingly.

![screenshot](/screenshot1.png)

<br><br>
## RootSIFT/RootSIFT
Retrieve the features of images using RootSIFT algorithm.

```
usage:
  RootSIFT [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...

options:
  -j n      the number of threads to use, n is an integer.
            (default: 1)
  -f        overwrite existing files.
            zero-length files will be treated as non-existing
            files.
  --od      orientation-dependent mode. the output SIFT descriptors
            will be sensitive to the orientation of the original
            image.
  --dr      draw keypoints of the original image.
  --so      SIFT only, without hellinger normalization.

input:      input image file name or directory name.

filter:     a regular expression (ECMAScript syntax) for files in
            "input". for example, to process ".jpg" files only,
            use ".+\.jpg".
            (default: process all the subfiles)

output:     output must be a directory name. (format: csv, where
            features are stored as matrix ROWS)
            if the output directory doesn't exist, please use a
            trailing "\", for example, ".\results\", otherwise it
            will be treated as a file.
            (default: current directory)
```

<br><br>
## RootSIFT/PCA-Wh
Perform PCA-Whitening on data.

```
usage:
  PCA-Wh [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...

options:
  -j n      the number of threads to use, n is an integer.
            (default: 1)
  -k n      the number of eigenvectors to keep,  n is an integer.
            set n to 0 if you want to keep all the eigenvectors.
            (default: 0)
  -f        overwrite existing files.
            zero-length files will be treated as non-existing
            files.
  --po      PCA only, without whitening.
  --nozm    no zero-mean mode, process the data directly without
            ensuring that the data has zero-mean.

input:      input file name or directory name. (format: csv, where
            features are stored as matrix ROWS)

filter:     a regular expression (ECMAScript syntax) for files in
            "input". for example, to process ".csv" files only,
            use ".+\.csv".
            (default: process all the subfiles)

output:     output must be a directory name.
            if the output directory doesn't exist, please use a
            trailing "\", for example, ".\results\", otherwise it
            will be treated as a file.
            (default: current directory)
```

<br><br>
## GMMenc/GMMenc
Get GMM parameters (means, covs, weights), and transform GMM parameters to supervectors.

```
usage:
  GMMenc command [options] -i input1 [-fi filter1] [-o output1] -i input2 [-fi filter2] [-o output2] ...

command:
  em   get GMM parameters (means, covs, weights).
       the results will be stored in ".miu" ".sigma" ".pi" and
       ".gamma" files (format: csv).

       options:
           -j n      the number of threads to use, n is an integer.
                     (default: 1)
           -k n      the number of components of GMM, n is an integer.
                     (default: 50)
           -f        overwrite existing files.
                     zero-length files will be treated as non-existing
                     files.

       input:        input file name or directory name (format: csv).

       filter:       a regular expression (ECMAScript syntax) for files in
                     "input". for example, to process ".csv" files only,
                     use ".+\.csv".
                     (default: process all the subfiles)

       output:       output must be a directory name.
                     if the output directory doesn't exist, please use a
                     trailing "\", for example, ".\results\", otherwise it
                     will be treated as a file.
                     (default: current directory)

  sv   transform GMM parameters to supervectors.
       it will automatically search for corresponding ".miu" ".sigma" ".pi"
       ".gamma" and original data files.

       options:
           -j n      the number of threads to use, n is an integer.
                     (default: 1)
           -r r      the relevance factor, r is a float.
                     (default: 30)
           -s path   additional directory to search for the
                     original data files.
                     the original data files are deducted by the
                     input ".miu" ".sigma" ".pi" and ".gamma" files.
                     you can specify multiple -s options.
                     (default: the same directory specified by "-i")
           -f        overwrite existing files.
                     zero-length files will be treated as non-existing
                     files.
           --emonly  use the results of em only, without adaptation and
                     normalization.

       input:        input must be a directory name. it will automatically
                     search for corresponding ".miu" ".sigma" ".pi" and
                     ".gamma" files.

       filter:       a regular expression (ECMAScript syntax) for files in
                     "input". subfiles (in "input") that don't match the
                     pattern will be filtered out.
                     (default: process all the subfiles)

       output:       output must be a directory name.
                     if the output directory doesn't exist, please use a
                     trailing "\", for example, ".\results\", otherwise it
                     will be treated as a file.
                     (default: current directory)
```

<br><br>
## GMMenc/combine
The result of GMMenc consists of a lot of separate files, whose names are the original image names followed by ".csv".  
This tool retrieves writer IDs from the file names and stores them line by line in "labels.csv", with the corresponding supervectors stored line by line in "data.csv".

```
usage:
    combine input [output]

input:    input directory.

output:   output directory.
          (default: current directory)
```