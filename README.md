# DPP (Dylan’s Preprocessor)

A tiny preprocessor that enables you to embed shell code everywhere!

```html
!!get_h1() { t=$(grep '^#' "$1"); t=${t#'# '}; }
<!DOCTYPE html>
<html lang=en>
<head>
  <title>Dylan${1:+ - $1}</title>
  <style>$(cat css/style.css)</style>
</head>
<body>
  <h1>$1</h1>
  <ul>
!!
for f in post/*.md; do
  get_h1 "$f"
  echo "<li><a href="/${f%.md}">$t</a></li>"
done
!!
  </ul>
  <footer>
    <p>Page updated: $(date '+%a %D %I:%M')</p>
  </footer>
</body>
</html>
```

See the `example/` subdirectory for more examples or peruse the source code for
my websites which are built around `dpp`: <https://github.com/dylanaraps/wild.gr>
<https://github.com/dylanaraps/dylan.gr>.

## Table of Contents

<!-- vim-markdown-toc GFM -->

* [Installation](#installation)
* [Usage](#usage)
    * [Compilation](#compilation)
* [Environment Variables](#environment-variables)
* [Syntax - Preface](#syntax---preface)
* [Syntax](#syntax)
    * [Evaluate a line of code](#evaluate-a-line-of-code)
    * [Evaluate a block of code](#evaluate-a-block-of-code)
    * [Evaluate code inline](#evaluate-code-inline)
    * [Evaluate variables](#evaluate-variables)
    * [Evaluate arithmetic](#evaluate-arithmetic)
    * [Evaluate command-line arguments](#evaluate-command-line-arguments)
* [Frequently Asked Questions](#frequently-asked-questions)
    * [How Does DPP Work?](#how-does-dpp-work)
    * [Why?](#why)

<!-- vim-markdown-toc -->

## Installation

Using make (install requires appropriate privileges).

```sh
$ make
$ make PREFIX=/usr install
```

## Usage

DPP works like any “UNIXy” program, it reads input from `stdin` and outputs to
`stdout`. Any errors that occur are sent to `stderr`.

```sh
$ dpp < input > output
```

Passing command-line arguments to DPP makes them accessible via `$1`, `$2`, etc
within the input.

```sh
$ dpp a b c d < input > output
```

DPP has no command-line flags like `-h` (help) or `-v` (version). The version
number is accessible via `DPP_VERSION` within DPP.

```sh
$ echo '$DPP_VERSION' | dpp
```

### Compilation

DPP also comes with a second utility called `dpp-compile`. This command is
identical to `dpp` only it doesn’t execute the transformed input. It instead
prints the transformed input to `stdout` where it can be piped to a shell or
saved to a file for later execution.

Basically, it allows you to “compile” text files into programs for later
execution.

```sh
# Transform and execute dppfetch.
$ dpp < example/dppfetch

# Transform and save the "compiled" dppfetch to a file for later execution.
$ dpp-compile < example/dppfetch > dppfetch
$ chmod +x dppfetch
$ ./dppfetch
```


## Environment Variables

DPP can be controlled via environment variables.

* `DPP_BLOCK` (default `!!`)

  Change the marker DPP uses for single line and multi line evaluations.

* `DPP_INCLUDE` (default `''`)

  Set to a shell script that DPP will source at the beginning.

* `DPP_CAT` (default `cat`)

  Set to a shell command that DPP will run on ordinary text. The value can
  contain spaces (`DPP_CAT="md2html --github"`, for example).

* `DPP_SHELL` (default `/bin/sh`)

  The shell DPP will execute. The value must be an absolute path.

DPP also exposes some information via environment variables.

* `DPP_VERSION`

  The version of DPP.

* `DPP_LEVEL`

  The DPP nest level.


## Syntax - Preface

Before syntax is explained the reader must understand a few things.

* All shell code that DPP evaluates exists within the same shell environment.

  For example, if a variable or function is defined in one code block it
  remains defined and accessible for the remainder of the input. The input
  is basically an inside-out(?) shell script where whole programs can live
  interspersed inside of arbitrary data.

  ```sh
  !!
  cnt() { n=$((n + 1)); echo "$n"; }
  !!

  !!cnt

  The value of the counter is currently: $n

  The value of the counter is currently: $(cnt) (this runs in a subshell and so
  the value of '$n' does not change.)

  !!n=

  The value of the counter is currently: ${n:-null}
  ```

* All shell code is evaluated with `set -e` and `set -u` enabled.

  DPP will error and exit when evaluated code returns a non-zero exit code or
  when variables are unset. Any error messages will be sent to stderr. Also,
  `set -o pipefail` is enabled (if supported). These options can be modified at
  compile time in `config.h` or at runtime by simply using `set` in your code.


## Syntax

This should all be pretty self-explanatory to those familiar with the shell.


### Evaluate a line of code

```sh
Starting a line with !! tells DPP to evaluate the entire line.
!!echo "Hello, World!"
!! echo "The space after !! is optional"

Lines starting with a backslash will not be evaluated.
\!! echo "The space after !! is optional"

```

### Evaluate a block of code

```sh
!!
for l in a b c d e f g; do
  echo "$l"
done
!!

Blocks starting with a backslash will not be evaluated.
\!!
for l in a b c d e f g; do
  echo "$l"
done
\!!
```

### Evaluate code inline

```sh
The current date is: $(date +%d-%m-%Y)

Inline code blocks starting with a backslash will not be evaluated.
\$(echo "I will be treated as plain text")
```

###  Evaluate variables

```sh
The value of a is ${a:-unset}.

!!a=/home/dylan/projects/dpp/dpp

The value of a is $a.
The basename of a is ${a##*/}
The dirname of a is ${a%/*}

The user's shell is $SHELL.
The current version of DPP is $DPP_VERSION.
This is line $DPP_LINE of the input.

Variables starting with a backslash \$a will not be evaluated.
```

### Evaluate arithmetic

```sh
The value of a is ${a:-unset}.

!!a=3

The value of a is $a.

$((a * 3))

Arithmetic blocks starting with a backslash \$((a * 2)) will not be
evaluated.
```

### Evaluate command-line arguments

```sh
Any arguments given to DPP are accessible via $1, $2, etc.

<!DOCTYPE html>
<html lang=en>
<head>
  <title>Dylan ${1:+- $1}</title>
  <style>$(cat style.css)</style>
</head>
<body>
  <h1>$2</h1>
</body>
</html>

Variables starting with a backslash \$1 will not be evaluated.
```

## Frequently Asked Questions

### How Does DPP Work?

To see how DPP works inspect the output of the `dpp-compile` command.

### Why?

I like shell. Also, I have another version of DPP which uses C as the language
and optionally compiles itself using `tcc`. It's slighly more complex and I
might release it later, we'll see.

